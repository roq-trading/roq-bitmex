/* Copyright (c) 2017-2021, Hans Erik Thrane */

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

using namespace roq::literals;

namespace roq {
namespace bitmex {

namespace {
static const auto NAME = "om"_sv;

static const auto SUPPORTS = utils::Mask{
    SupportType::CREATE_ORDER,
    SupportType::MODIFY_ORDER,
    SupportType::CANCEL_ORDER,
    SupportType::ORDER_ACK,
};

static const auto ALLOW_PIPELINING = true;

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(const std::string_view &group, const std::string_view &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

static auto compute_expires() {
  auto result = core::get_realtime_clock() + Flags::rest_expires_timeout();
  return std::chrono::ceil<std::chrono::seconds>(result);
}

static auto get_quality_of_service() {
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
      name_(fmt::format("{}:{}:{}"_sv, stream_id_, NAME, security.get_account())),
      connection_(
          *this,
          context,
          Flags::decode_buffer_size(),
          Flags::encode_buffer_size(),
          core::URI(Flags::rest_uri()),
          ROQ_PACKAGE_NAME,
          core::http::Connection::KEEP_ALIVE,
          ALLOW_PIPELINING,
          Flags::rest_request_timeout(),
          Flags::rest_rate_limit_interval(),
          Flags::rest_rate_limit_max_requests(),
          Flags::rest_ping_freq(),
          Flags::rest_ping_path()),
      decode_buffer_(Flags::decode_buffer_size()),
      counter_{
          .disconnect = create_metrics(name_, "disconnect"_sv),
      },
      profile_{
          .products = create_metrics(name_, "products"_sv),
          .accounts = create_metrics(name_, "accounts"_sv),
          .create_order = create_metrics(name_, "create_order"_sv),
          .create_order_ack = create_metrics(name_, "create_order_ack"_sv),
          .modify_order = create_metrics(name_, "modify_order"_sv),
          .modify_order_ack = create_metrics(name_, "modify_order_ack"_sv),
          .cancel_order = create_metrics(name_, "cancel_order"_sv),
          .cancel_order_ack = create_metrics(name_, "cancel_order_ack"_sv),
          .cancel_all_orders = create_metrics(name_, "cancel_all_orders"_sv),
          .cancel_all_orders_ack = create_metrics(name_, "cancel_all_orders_ack"_sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"_sv),
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
      .write(profile_.products, metrics::PROFILE)
      .write(profile_.accounts, metrics::PROFILE)
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
    const Event<CreateOrder> &event, const std::string_view &request_id) {
  profile_.create_order([&]() {
    if (!ready())
      throw server::OMS_ErrorException(
          Origin::GATEWAY, RequestStatus::REJECTED, Error::GATEWAY_NOT_READY);
    auto &[message_info, create_order] = event;
    auto method = core::http::Method::POST;
    auto path = "/api/v1/order"_sv;
    auto expires = compute_expires();
    auto side = json::map(create_order.side).as_raw_text();
    auto ord_type = json::map(create_order.order_type).as_raw_text();
    auto time_in_force = json::map(create_order.time_in_force).as_raw_text();
    auto exec_inst = create_order.execution_instruction == ExecutionInstruction{}
                         ? std::string_view{}
                         : json::map(create_order.execution_instruction).as_raw_text();
    // XXX use encode buffer
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
        R"(}})"_sv,
        request_id,
        create_order.symbol,
        side,
        create_order.price,
        create_order.quantity,
        ord_type,
        time_in_force,
        exec_inst);
    log::debug(R"(body="{}")"_sv, body);
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
        .rate_limit_weight = 1,
    };
    connection_(
        request,
        [this, user_id = message_info.source, order_id = create_order.order_id](auto &response) {
          profile_.create_order_ack([&]() { create_order_ack(response, user_id, order_id); });
        });
  });
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    const Event<ModifyOrder> &event,
    const server::OMS_Order &,
    const std::string_view &request_id,
    const std::string_view &previous_request_id) {
  profile_.modify_order([&]() {
    if (!ready())
      throw server::OMS_ErrorException(
          Origin::GATEWAY, RequestStatus::REJECTED, Error::GATEWAY_NOT_READY);
    auto &[message_info, modify_order] = event;
    auto method = core::http::Method::PUT;
    auto path = "/api/v1/order"_sv;
    auto expires = compute_expires();
    // XXX use encode buffer
    auto body = fmt::format(
        R"({{)"
        R"("clOrdID":"{}",)"
        R"("origClOrdID":"{}",)"
        R"("orderQty":{},)"
        R"("price":{})"
        R"(}})"_sv,
        request_id,
        previous_request_id,
        modify_order.quantity,
        modify_order.price);
    log::debug(R"(body="{}")"_sv, body);
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
        .rate_limit_weight = 1,
    };
    connection_(
        request,
        [this,
         user_id = message_info.source,
         order_id = modify_order.order_id,
         version = modify_order.version](auto &response) {
          profile_.modify_order_ack(
              [&]() { modify_order_ack(response, user_id, order_id, version); });
        });
  });
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    const Event<CancelOrder> &event,
    const server::OMS_Order &order,
    [[maybe_unused]] const std::string_view &request_id,
    [[maybe_unused]] const std::string_view &previous_request_id) {
  profile_.cancel_order([&]() {
    if (!ready())
      throw server::OMS_ErrorException(
          Origin::GATEWAY, RequestStatus::REJECTED, Error::GATEWAY_NOT_READY);
    auto &[message_info, cancel_order] = event;
    auto method = core::http::Method::DELETE;
    auto path = "/api/v1/order"_sv;
    auto expires = compute_expires();
    // XXX use encode buffer
    auto body = fmt::format(
        R"({{)"
        R"("orderID":"{}")"
        R"(}})"_sv,
        order.external_order_id);
    log::debug(R"(body="{}")"_sv, body);
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
        .rate_limit_weight = 1,
    };
    connection_(
        request,
        [this,
         user_id = message_info.source,
         order_id = cancel_order.order_id,
         version = cancel_order.version](auto &response) {
          profile_.cancel_order_ack(
              [&]() { cancel_order_ack(response, user_id, order_id, version); });
        });
  });
  return stream_id_;
}

uint16_t OrderEntry::operator()(const Event<CancelAllOrders> &event) {
  profile_.cancel_order([&]() {
    if (ready()) {
      auto method = core::http::Method::DELETE;
      auto path = "/api/v1/order/all"_sv;
      auto expires = compute_expires();
      auto body = "{}"_sv;
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
          .rate_limit_weight = 1,
      };
      connection_(request, [this](auto &response) {
        profile_.cancel_all_orders_ack([&]() { cancel_all_orders_ack(response); });
      });
    } else {
      auto &[message_info, cancel_all_orders] = event;
      log::warn(
          R"(*** NOT CONNECTED! UNABLE TO CANCEL ALL ORDERS FOR ACCOUNT="{}")"_sv,
          cancel_all_orders.account);
    }
  });
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
  server::TraceInfo trace_info;
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .latency = latency.sample,
  };
  server::create_trace_and_dispatch(trace_info, external_latency, handler_);
  latency_.ping.update(latency.sample);
}

void OrderEntry::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    server::TraceInfo trace_info;
    StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = security_.get_account(),
        .supports = SUPPORTS.get(),
        .status = status_,
        .type = StreamType::REST,
        .priority = Priority::PRIMARY,
    };
    log::info("stream_status={}"_sv, stream_status);
    server::create_trace_and_dispatch(trace_info, stream_status, handler_);
  }
}

void OrderEntry::create_order_ack(
    const core::web::Response &response, const uint8_t user_id, const uint32_t order_id) {
  log::debug("user_id={}, order_id={}"_sv, user_id, order_id);
  server::TraceInfo trace_info;
  uint8_t version = 1;
  try {
    auto category = core::http::to_category(response.raw_status());
    switch (category) {
      case core::http::Category::SUCCESS: {  // 2xx
        auto body = response.body();
        auto order_item = core::json::Parser::create<json::OrderItem>(body);
        OrderUpdate{shared_, stream_id_, security_.get_account()}(
            order_item, trace_info, RequestType::CREATE_ORDER, user_id, order_id, version);
        break;
      }
      case core::http::Category::CLIENT_ERROR: {
        std::string_view text;
        auto body = response.body();
        if (json::ErrorParser::dispatch(body, [&](auto &error) {
              log::warn("error={}"_sv, error);
              text = error.message;
            })) {
        } else {
          log::warn(R"(Unable to parse response="{}")"_sv, body);
          text = "Unknown"_sv;
        }
        server::OMS_Ack ack{
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
                user_id, order_id, stream_id_, trace_info, ack, []([[maybe_unused]] auto &order) {
                })) {
        } else {
          log::warn("Did not find order: user_id={}, order_id={}"_sv, user_id, order_id);
        }
        break;
      }
      default:
        response.expect(core::http::Status::OK);  // throws
    }
  } catch (NetworkError &e) {
    log::warn(R"(Exception type={}, what="{}")"_sv, typeid(e).name(), e.what());
    server::OMS_Ack ack{
        .type = RequestType::CREATE_ORDER,
        .origin = Origin::GATEWAY,
        .status = RequestStatus::REJECTED,
        .error = Error::NETWORK_ERROR,
        .text = e.what(),
        .version = version,
        .request_id = {},
        .quantity = NaN,
        .price = NaN,
    };
    if (shared_.update_order(
            user_id, order_id, stream_id_, trace_info, ack, []([[maybe_unused]] auto &order) {})) {
    } else {
      log::warn("Did not find order: user_id={}, order_id={}"_sv, user_id, order_id);
    }
  }
}

void OrderEntry::modify_order_ack(
    const core::web::Response &response,
    const uint8_t user_id,
    const uint32_t order_id,
    const uint32_t version) {
  log::debug("user_id={}, order_id={}, version={}"_sv, user_id, order_id, version);
  server::TraceInfo trace_info;
  try {
    auto category = core::http::to_category(response.raw_status());
    switch (category) {
      case core::http::Category::SUCCESS: {  // 2xx
        auto order_item = core::json::Parser::create<json::OrderItem>(response.body());
        OrderUpdate{shared_, stream_id_, security_.get_account()}(
            order_item, trace_info, RequestType::MODIFY_ORDER, user_id, order_id, version);
        break;
      }
      case core::http::Category::CLIENT_ERROR: {  // 4xx
        std::string_view text;
        auto body = response.body();
        if (json::ErrorParser::dispatch(body, [&](auto &error) {
              log::warn("error={}"_sv, error);
              text = error.message;
            })) {
        } else {
          log::warn(R"(Unable to parse response="{}")"_sv, body);
          text = "Unknown"_sv;
        }
        server::OMS_Ack ack{
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
                user_id, order_id, stream_id_, trace_info, ack, []([[maybe_unused]] auto &order) {
                })) {
        } else {
          log::warn(
              "Did not find order: user_id={}, order_id={}, version={}"_sv,
              user_id,
              order_id,
              version);
        }
        break;
      }
      default:
        response.expect(core::http::Status::OK);  // throws
    }
  } catch (NetworkError &e) {
    log::warn(R"(Exception type={}, what="{}")"_sv, typeid(e).name(), e.what());
    server::OMS_Ack ack{
        .type = RequestType::MODIFY_ORDER,
        .origin = Origin::GATEWAY,
        .status = RequestStatus::REJECTED,
        .error = Error::NETWORK_ERROR,
        .text = e.what(),
        .version = version,
        .request_id = {},
        .quantity = NaN,
        .price = NaN,
    };
    if (shared_.update_order(
            user_id, order_id, stream_id_, trace_info, ack, []([[maybe_unused]] auto &order) {})) {
    } else {
      log::warn(
          "Did not find order: user_id={}, order_id={}, version={}"_sv, user_id, order_id, version);
    }
  }
}

void OrderEntry::cancel_order_ack(
    const core::web::Response &response,
    const uint8_t user_id,
    const uint32_t order_id,
    const uint32_t version) {
  log::debug("user_id={}, order_id={}, version={}"_sv, user_id, order_id, version);
  server::TraceInfo trace_info;
  try {
    auto category = core::http::to_category(response.raw_status());
    switch (category) {
      case core::http::Category::SUCCESS: {  // 2xx
        core::json::Buffer buffer(decode_buffer_);
        auto order = core::json::Parser::create<json::Order>(response.body(), buffer);
        OrderUpdate{shared_, stream_id_, security_.get_account()}(
            order, trace_info, RequestType::CANCEL_ORDER, user_id, order_id, version);
        break;
      }
      case core::http::Category::CLIENT_ERROR: {  // 4xx
        std::string_view text;
        auto body = response.body();
        if (json::ErrorParser::dispatch(body, [&](auto &error) {
              log::warn("error={}"_sv, error);
              text = error.message;
            })) {
        } else {
          log::warn(R"(Unable to parse response="{}")"_sv, body);
          text = "Unknown"_sv;
        }
        server::OMS_Ack ack{
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
                user_id, order_id, stream_id_, trace_info, ack, []([[maybe_unused]] auto &order) {
                })) {
        } else {
          log::warn(
              "Did not find order: user_id={}, order_id={}, version={}"_sv,
              user_id,
              order_id,
              version);
        }
        break;
      }
      default:
        response.expect(core::http::Status::OK);  // throws
    }
  } catch (NetworkError &e) {
    log::warn(R"(Exception type={}, what="{}")"_sv, typeid(e).name(), e.what());
    server::OMS_Ack ack{
        .type = RequestType::CANCEL_ORDER,
        .origin = Origin::GATEWAY,
        .status = RequestStatus::REJECTED,
        .error = Error::NETWORK_ERROR,
        .text = e.what(),
        .version = version,
        .request_id = {},
        .quantity = NaN,
        .price = NaN,
    };
    if (shared_.update_order(
            user_id, order_id, stream_id_, trace_info, ack, []([[maybe_unused]] auto &order) {})) {
    } else {
      log::warn(
          "Did not find order: user_id={}, order_id={}, version={}"_sv, user_id, order_id, version);
    }
  }
}

void OrderEntry::cancel_all_orders_ack(const core::web::Response &response) {
  server::TraceInfo trace_info;
  try {
    auto category = core::http::to_category(response.raw_status());
    switch (category) {
      case core::http::Category::SUCCESS: {  // 2xx
        core::json::Buffer buffer(decode_buffer_);
        auto order = core::json::Parser::create<json::Order>(response.body(), buffer);
        OrderUpdate{shared_, stream_id_, security_.get_account()}(order, trace_info, false);
        break;
      }
      case core::http::Category::CLIENT_ERROR: {  // 4xx
        auto body = response.body();
        if (json::ErrorParser::dispatch(
                body, [&](auto &error) { log::warn("error={}"_sv, error); })) {
        } else {
          log::warn(R"(Unable to parse response="{}")"_sv, body);
        }
        // note! this event does not require an ack
        break;
      }
      default:
        response.expect(core::http::Status::OK);  // throws
    }
  } catch (NetworkError &e) {
    log::warn(R"(Exception type={}, what="{}")"_sv, typeid(e).name(), e.what());
    // note! this event does not require an ack
  }
}

void OrderEntry::operator()(const json::OrderItem &order_item) {
  server::TraceInfo trace_info;
  OrderUpdate{shared_, stream_id_, security_.get_account()}(order_item, trace_info, false);
}

void OrderEntry::operator()(const json::Order &order) {
  server::TraceInfo trace_info;
  OrderUpdate{shared_, stream_id_, security_.get_account()}(order, trace_info, false);
}

}  // namespace bitmex
}  // namespace roq
