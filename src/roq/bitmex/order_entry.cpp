/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/bitmex/order_entry.h"

#include <fmt/chrono.h>

#include <chrono>
#include <utility>

#include "roq/utils/mask.h"
#include "roq/utils/update.h"

#include "roq/core/metrics/factory.h"

#include "roq/bitmex/flags.h"
#include "roq/bitmex/order_update.h"

#include "roq/bitmex/json/error_parser.h"
#include "roq/bitmex/json/utils.h"

using namespace std::literals;

namespace roq {
namespace bitmex {

namespace {
const auto NAME = "om"sv;

const auto SUPPORTS = utils::Mask{
    SupportType::CREATE_ORDER,
    SupportType::MODIFY_ORDER,
    SupportType::CANCEL_ORDER,
    SupportType::ORDER_ACK,
};

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(const std::string_view &group, const std::string_view &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

auto create_connection(auto &handler, auto &context) {
  core::web::Client::Config config{
      .decode_buffer_size = Flags::decode_buffer_size(),
      .encode_buffer_size = Flags::encode_buffer_size(),
      .validate_certificate = server::Flags::tls_validate_certificate(),
      .uri = Flags::rest_uri(),
      .proxy = Flags::rest_proxy(),
      .user_agent = ROQ_PACKAGE_NAME,
      .connection = core::http::Connection::KEEP_ALIVE,
      .allow_pipelining = true,
      .request_timeout = Flags::rest_request_timeout(),
      .ping_frequency = Flags::rest_ping_freq(),
      .ping_path = Flags::rest_ping_path(),
  };
  return core::web::Client{handler, context, config};
}

auto compute_expires() {
  auto result = core::get_realtime_clock() + Flags::rest_expires_timeout();
  return std::chrono::ceil<std::chrono::seconds>(result);
}

auto get_quality_of_service() {
  return Flags::rest_allow_order_request_pipeline() ? core::web::QualityOfService::IMMEDIATE
                                                    : core::web::QualityOfService::CRITICAL;
}
}  // namespace

OrderEntry::OrderEntry(
    Handler &handler,
    core::io::Context &context,
    uint16_t stream_id,
    Security &security,
    Shared &shared)
    : handler_(handler), stream_id_(stream_id),
      name_(fmt::format("{}:{}:{}"sv, stream_id_, NAME, security.get_account())),
      connection_(create_connection(*this, context)), decode_buffer_(Flags::decode_buffer_size()),
      counter_{
          .disconnect = create_metrics(name_, "disconnect"sv),
      },
      profile_{
          .create_order = create_metrics(name_, "create_order"sv),
          .create_order_ack = create_metrics(name_, "create_order_ack"sv),
          .modify_order = create_metrics(name_, "modify_order"sv),
          .modify_order_ack = create_metrics(name_, "modify_order_ack"sv),
          .cancel_order = create_metrics(name_, "cancel_order"sv),
          .cancel_order_ack = create_metrics(name_, "cancel_order_ack"sv),
          .cancel_all_orders = create_metrics(name_, "cancel_all_orders"sv),
          .cancel_all_orders_ack = create_metrics(name_, "cancel_all_orders_ack"sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"sv),
      },
      security_(security), shared_(shared) {
}

void OrderEntry::operator()(const Event<Start> &) {
  connection_.start();
}

void OrderEntry::operator()(const Event<Stop> &) {
  connection_.stop();
}

void OrderEntry::operator()(const Event<Timer> &event) {
  connection_.refresh(event.value.now);
}

void OrderEntry::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::COUNTER)
      // profile
      .write(profile_.create_order, metrics::PROFILE)
      .write(profile_.create_order_ack, metrics::PROFILE)
      .write(profile_.modify_order, metrics::PROFILE)
      .write(profile_.modify_order_ack, metrics::PROFILE)
      .write(profile_.cancel_order, metrics::PROFILE)
      .write(profile_.cancel_order_ack, metrics::PROFILE)
      .write(profile_.cancel_all_orders, metrics::PROFILE)
      .write(profile_.cancel_all_orders_ack, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY);
}

uint16_t OrderEntry::operator()(
    const Event<CreateOrder> &event, const oms::Order &order, const std::string_view &request_id) {
  create_order(event, order, request_id);
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    const Event<ModifyOrder> &event,
    const oms::Order &order,
    const std::string_view &request_id,
    const std::string_view &previous_request_id) {
  modify_order(event, order, request_id, previous_request_id);
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    const Event<CancelOrder> &event,
    const oms::Order &order,
    const std::string_view &request_id,
    const std::string_view &previous_request_id) {
  cancel_order(event, order, request_id, previous_request_id);
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    const Event<CancelAllOrders> &event, const std::string_view &request_id) {
  cancel_all_orders(event, request_id);
  return stream_id_;
}

void OrderEntry::operator()(const core::web::Client::Connected &) {
  (*this)(ConnectionStatus::READY);
}

void OrderEntry::operator()(const core::web::Client::Disconnected &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
}

void OrderEntry::operator()(const core::web::Client::Latency &latency) {
  auto trace_info = server::create_trace_info();
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .account = security_.get_account(),
      .latency = latency.sample,
  };
  server::create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void OrderEntry::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    auto trace_info = server::create_trace_info();
    StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = security_.get_account(),
        .supports = SUPPORTS.get(),
        .status = status_,
        .type = StreamType::REST,
        .priority = Priority::PRIMARY,
    };
    log::info("stream_status={}"sv, stream_status);
    server::create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

// create-order

void OrderEntry::create_order(
    const Event<CreateOrder> &event, const oms::Order &, const std::string_view &request_id) {
  profile_.create_order([&]() {
    if (!ready())
      throw oms::NotReady("not ready"sv);
    auto &[message_info, create_order] = event;
    auto method = core::http::Method::POST;
    auto path = "/api/v1/order"sv;
    auto expires = compute_expires();
    auto side = json::map(create_order.side).as_raw_text();
    auto ord_type = json::map(create_order.order_type).as_raw_text();
    auto time_in_force = json::map(create_order.time_in_force).as_raw_text();
    auto exec_inst = create_order.execution_instruction == ExecutionInstruction{}
                         ? std::string_view{}
                         : json::map(create_order.execution_instruction).as_raw_text();
    auto body = fmt::format(
        R"({{)"
        R"("clOrdID":"{}",)"
        R"("symbol":"{}",)"
        R"("side":"{}",)"
        R"("price":{},)"
        R"("orderQty":{},)"
        R"("ordType":"{}",)"
        R"("timeInForce":"{}",)"
        R"("execInst":"{}")"
        R"(}})"sv,
        request_id,
        create_order.symbol,
        side,
        create_order.price,
        create_order.quantity,
        ord_type,
        time_in_force,
        exec_inst);
    log::debug(R"(body="{}")"sv, body);
    auto headers = security_.create_headers(expires, method, path, body);
    core::web::Request request{
        .method = method,
        .path = path,
        .query = {},
        .accept = core::http::Accept::JSON,
        .content_type = core::http::ContentType::JSON,
        .headers = headers,
        .body = body,
        .quality_of_service = get_quality_of_service(),
    };
    connection_(
        request_id,
        request,
        [this, user_id = message_info.source, order_id = create_order.order_id](
            [[maybe_unused]] auto &request_id, auto &response) {
          auto trace_info = server::create_trace_info();
          server::Trace event(trace_info, response);
          uint32_t version = 1;
          create_order_ack(event, user_id, order_id, version);
        });
  });
}

void OrderEntry::create_order_ack(
    const server::Trace<core::web::Response> &event,
    uint8_t user_id,
    uint32_t order_id,
    uint32_t version) {
  profile_.create_order_ack([&]() {
    auto &[trace_info, response] = event;
    log::debug("user_id={}, order_id={}, version={}"sv, user_id, order_id, version);
    try {
      auto [status, category, body] = response.result();
      log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
      switch (category) {
        case core::http::Category::SUCCESS: {  // 2xx
          auto order_item = core::json::Parser::create<json::OrderItem>(body);
          OrderUpdate{shared_, stream_id_, security_.get_account()}(
              order_item, trace_info, RequestType::CREATE_ORDER, user_id, order_id, version);
          break;
        }
        case core::http::Category::CLIENT_ERROR: {
          std::string_view text;
          if (json::ErrorParser::dispatch(body, [&](auto &error) {
                log::warn("error={}"sv, error);
                text = error.message;
              })) {
          } else {
            log::warn(R"(Unable to parse response="{}")"sv, body);
            text = "Unknown"sv;
          }
          oms::Response response{
              .type = RequestType::CREATE_ORDER,
              .origin = Origin::EXCHANGE,
              .status = RequestStatus::REJECTED,
              .error = json::guess_error(text),
              .text = text,
              .version = version,
              .request_id = {},
              .quantity = NaN,
              .price = NaN,
          };
          if (shared_.update_order(
                  user_id,
                  order_id,
                  stream_id_,
                  trace_info,
                  response,
                  []([[maybe_unused]] auto &order) {})) {
          } else {
            log::warn("Did not find order: user_id={}, order_id={}"sv, user_id, order_id);
          }
          break;
        }
        default:
          response.expect(core::http::Status::OK);  // throws
      }
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      oms::Response response{
          .type = RequestType::CREATE_ORDER,
          .origin = Origin::GATEWAY,
          .status = e.request_status(),
          .error = e.error(),
          .text = e.what(),
          .version = version,
          .request_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      if (shared_.update_order(
              user_id,
              order_id,
              stream_id_,
              trace_info,
              response,
              []([[maybe_unused]] auto &order) {})) {
      } else {
        log::warn("Did not find order: user_id={}, order_id={}"sv, user_id, order_id);
      }
    }
  });
}

// modify-order

void OrderEntry::modify_order(
    const Event<ModifyOrder> &event,
    const oms::Order &,
    const std::string_view &request_id,
    const std::string_view &previous_request_id) {
  profile_.modify_order([&]() {
    if (!ready())
      throw oms::NotReady("not ready"sv);
    auto &[message_info, modify_order] = event;
    auto method = core::http::Method::PUT;
    auto path = "/api/v1/order"sv;
    auto expires = compute_expires();
    auto body = fmt::format(
        R"({{)"
        R"("clOrdID":"{}",)"
        R"("origClOrdID":"{}",)"
        R"("orderQty":{},)"
        R"("price":{})"
        R"(}})"sv,
        request_id,
        previous_request_id,
        modify_order.quantity,
        modify_order.price);
    log::debug(R"(body="{}")"sv, body);
    auto headers = security_.create_headers(expires, method, path, body);
    core::web::Request request{
        .method = method,
        .path = path,
        .query = {},
        .accept = core::http::Accept::JSON,
        .content_type = core::http::ContentType::JSON,
        .headers = headers,
        .body = body,
        .quality_of_service = get_quality_of_service(),
    };
    connection_(
        request_id,
        request,
        [this,
         user_id = message_info.source,
         order_id = modify_order.order_id,
         version = modify_order.version]([[maybe_unused]] auto &request_id, auto &response) {
          auto trace_info = server::create_trace_info();
          server::Trace event(trace_info, response);
          modify_order_ack(event, user_id, order_id, version);
        });
  });
}

void OrderEntry::modify_order_ack(
    const server::Trace<core::web::Response> &event,
    uint8_t user_id,
    uint32_t order_id,
    uint32_t version) {
  profile_.modify_order_ack([&]() {
    auto &[trace_info, response] = event;
    log::debug("user_id={}, order_id={}, version={}"sv, user_id, order_id, version);
    try {
      auto [status, category, body] = response.result();
      log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
      switch (category) {
        case core::http::Category::SUCCESS: {  // 2xx
          auto order_item = core::json::Parser::create<json::OrderItem>(body);
          OrderUpdate{shared_, stream_id_, security_.get_account()}(
              order_item, trace_info, RequestType::MODIFY_ORDER, user_id, order_id, version);
          break;
        }
        case core::http::Category::CLIENT_ERROR: {  // 4xx
          std::string_view text;
          auto body = response.body();
          if (json::ErrorParser::dispatch(body, [&](auto &error) {
                log::warn("error={}"sv, error);
                text = error.message;
              })) {
          } else {
            log::warn(R"(Unable to parse response="{}")"sv, body);
            text = "Unknown"sv;
          }
          oms::Response response{
              .type = RequestType::MODIFY_ORDER,
              .origin = Origin::EXCHANGE,
              .status = RequestStatus::REJECTED,
              .error = json::guess_error(text),
              .text = text,
              .version = version,
              .request_id = {},
              .quantity = NaN,
              .price = NaN,
          };
          if (shared_.update_order(
                  user_id,
                  order_id,
                  stream_id_,
                  trace_info,
                  response,
                  []([[maybe_unused]] auto &order) {})) {
          } else {
            log::warn(
                "Did not find order: user_id={}, order_id={}, version={}"sv,
                user_id,
                order_id,
                version);
          }
          break;
        }
        default:
          response.expect(core::http::Status::OK);  // throws
      }
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      oms::Response response{
          .type = RequestType::MODIFY_ORDER,
          .origin = Origin::GATEWAY,
          .status = e.request_status(),
          .error = e.error(),
          .text = e.what(),
          .version = version,
          .request_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      if (shared_.update_order(
              user_id,
              order_id,
              stream_id_,
              trace_info,
              response,
              []([[maybe_unused]] auto &order) {})) {
      } else {
        log::warn(
            "Did not find order: user_id={}, order_id={}, version={}"sv,
            user_id,
            order_id,
            version);
      }
    }
  });
}

// cancel-order

void OrderEntry::cancel_order(
    const Event<CancelOrder> &event,
    const oms::Order &order,
    [[maybe_unused]] const std::string_view &request_id,
    [[maybe_unused]] const std::string_view &previous_request_id) {
  profile_.cancel_order([&]() {
    if (!ready())
      throw oms::NotReady("not ready"sv);
    auto &[message_info, cancel_order] = event;
    auto method = core::http::Method::DELETE;
    auto path = "/api/v1/order"sv;
    auto expires = compute_expires();
    auto body = fmt::format(
        R"({{)"
        R"("orderID":"{}")"
        R"(}})"sv,
        order.external_order_id);
    log::debug(R"(body="{}")"sv, body);
    auto headers = security_.create_headers(expires, method, path, body);
    core::web::Request request{
        .method = method,
        .path = path,
        .query = {},
        .accept = core::http::Accept::JSON,
        .content_type = core::http::ContentType::JSON,
        .headers = headers,
        .body = body,
        .quality_of_service = get_quality_of_service(),
    };
    connection_(
        request_id,
        request,
        [this,
         user_id = message_info.source,
         order_id = cancel_order.order_id,
         version = cancel_order.version]([[maybe_unused]] auto &request_id, auto &response) {
          auto trace_info = server::create_trace_info();
          server::Trace event(trace_info, response);
          cancel_order_ack(event, user_id, order_id, version);
        });
  });
}

void OrderEntry::cancel_order_ack(
    const server::Trace<core::web::Response> &event,
    uint8_t user_id,
    uint32_t order_id,
    uint32_t version) {
  profile_.cancel_order_ack([&]() {
    auto &[trace_info, response] = event;
    log::debug("user_id={}, order_id={}, version={}"sv, user_id, order_id, version);
    try {
      auto [status, category, body] = response.result();
      log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
      switch (category) {
        case core::http::Category::SUCCESS: {  // 2xx
          core::json::Buffer buffer(decode_buffer_);
          auto order = core::json::Parser::create<json::Order>(body, buffer);
          OrderUpdate{shared_, stream_id_, security_.get_account()}(
              order, trace_info, RequestType::CANCEL_ORDER, user_id, order_id, version);
          break;
        }
        case core::http::Category::CLIENT_ERROR: {  // 4xx
          std::string_view text;
          auto body = response.body();
          if (json::ErrorParser::dispatch(body, [&](auto &error) {
                log::warn("error={}"sv, error);
                text = error.message;
              })) {
          } else {
            log::warn(R"(Unable to parse response="{}")"sv, body);
            text = "Unknown"sv;
          }
          oms::Response response{
              .type = RequestType::CANCEL_ORDER,
              .origin = Origin::EXCHANGE,
              .status = RequestStatus::REJECTED,
              .error = json::guess_error(text),
              .text = text,
              .version = version,
              .request_id = {},
              .quantity = NaN,
              .price = NaN,
          };
          if (shared_.update_order(
                  user_id,
                  order_id,
                  stream_id_,
                  trace_info,
                  response,
                  []([[maybe_unused]] auto &order) {})) {
          } else {
            log::warn(
                "Did not find order: user_id={}, order_id={}, version={}"sv,
                user_id,
                order_id,
                version);
          }
          break;
        }
        default:
          response.expect(core::http::Status::OK);  // throws
      }
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      oms::Response response{
          .type = RequestType::CANCEL_ORDER,
          .origin = Origin::GATEWAY,
          .status = e.request_status(),
          .error = e.error(),
          .text = e.what(),
          .version = version,
          .request_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      if (shared_.update_order(
              user_id,
              order_id,
              stream_id_,
              trace_info,
              response,
              []([[maybe_unused]] auto &order) {})) {
      } else {
        log::warn(
            "Did not find order: user_id={}, order_id={}, version={}"sv,
            user_id,
            order_id,
            version);
      }
    }
  });
}

// cancel-all-orders

void OrderEntry::cancel_all_orders(
    const Event<CancelAllOrders> &event, const std::string_view &request_id) {
  profile_.cancel_all_orders([&]() {
    if (ready()) {
      auto method = core::http::Method::DELETE;
      auto path = "/api/v1/order/all"sv;
      auto expires = compute_expires();
      auto body = "{}"sv;
      auto headers = security_.create_headers(expires, method, path, body);
      core::web::Request request{
          .method = method,
          .path = path,
          .query = {},
          .accept = core::http::Accept::JSON,
          .content_type = core::http::ContentType::JSON,
          .headers = headers,
          .body = body,
          .quality_of_service = get_quality_of_service(),
      };
      connection_(request_id, request, [this]([[maybe_unused]] auto &request_id, auto &response) {
        auto trace_info = server::create_trace_info();
        server::Trace event(trace_info, response);
        cancel_all_orders_ack(event);
      });
    } else {
      auto &[message_info, cancel_all_orders] = event;
      log::warn(
          R"(*** NOT CONNECTED! UNABLE TO CANCEL ALL ORDERS FOR ACCOUNT="{}")"sv,
          cancel_all_orders.account);
    }
  });
}

void OrderEntry::cancel_all_orders_ack(const server::Trace<core::web::Response> &event) {
  profile_.cancel_all_orders_ack([&]() {
    auto &[trace_info, response] = event;
    try {
      auto [status, category, body] = response.result();
      log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
      switch (category) {
        case core::http::Category::SUCCESS: {  // 2xx
          core::json::Buffer buffer(decode_buffer_);
          auto order = core::json::Parser::create<json::Order>(body, buffer);
          OrderUpdate{shared_, stream_id_, security_.get_account()}(order, trace_info, false);
          break;
        }
        case core::http::Category::CLIENT_ERROR: {  // 4xx
          if (json::ErrorParser::dispatch(
                  body, [&](auto &error) { log::warn("error={}"sv, error); })) {
          } else {
            log::warn(R"(Unable to parse response="{}")"sv, body);
          }
          // note! this event does not require a response
          break;
        }
        default:
          response.expect(core::http::Status::OK);  // throws
      }
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      // note! this event does not require a response
    }
  });
}

// utilities

void OrderEntry::operator()(const json::OrderItem &order_item) {
  auto trace_info = server::create_trace_info();
  OrderUpdate{shared_, stream_id_, security_.get_account()}(order_item, trace_info, false);
}

void OrderEntry::operator()(const json::Order &order) {
  auto trace_info = server::create_trace_info();
  OrderUpdate{shared_, stream_id_, security_.get_account()}(order, trace_info, false);
}

}  // namespace bitmex
}  // namespace roq
