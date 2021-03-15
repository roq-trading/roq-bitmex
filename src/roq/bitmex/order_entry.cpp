/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/bitmex/order_entry.h"

#include <fmt/chrono.h>

#include <chrono>
#include <utility>

#include "roq/core/update.h"

#include "roq/core/metrics/factory.h"

#include "roq/bitmex/flags.h"
#include "roq/bitmex/order_update.h"

#include "roq/bitmex/json/utils.h"

using namespace roq::literals;

namespace roq {
namespace bitmex {

namespace {
static const auto CONNECTION = "om"_sv;

static const auto ACCEPT_JSON = "application/json"_sv;
static const auto CONTENT_TYPE_JSON = "application/json"_sv;

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(const std::string_view &group, const std::string_view &function)
      : core::metrics::Factory(Flags::name(), group, function) {}
};

static auto compute_expires() {
  auto result = core::get_realtime_clock() + Flags::rest_expires_timeout();
  return std::chrono::ceil<std::chrono::seconds>(result);
}
}  // namespace

OrderEntry::OrderEntry(
    Handler &handler,
    core::io::Context &context,
    uint16_t stream_id,
    Security &security,
    Shared &shared)
    : handler_(handler), stream_id_(stream_id),
      name_(roq::format("{}:{}:{}"_fmt, stream_id_, CONNECTION, security.get_account())),
      connection_(
          *this,
          context,
          core::URI(Flags::rest_uri()),
          ROQ_PACKAGE_NAME,
          true,  // keep alive
          Flags::rest_request_queue_depth(),
          Flags::rest_request_timeout(),
          Flags::rest_rate_limit_interval(),
          Flags::rest_rate_limit_max_requests(),
          Flags::rest_ping_freq(),
          Flags::decode_buffer_size(),
          Flags::encode_buffer_size(),
          Flags::rest_ping_path()),
      decode_buffer_(Flags::decode_buffer_size()),
      counter_{
          .disconnect = create_metrics(name_, "disconnect"_sv),
      },
      profile_{
          .products = create_metrics(name_, "products"_sv),
          .accounts = create_metrics(name_, "accounts"_sv),
          .create_order = create_metrics(name_, "create_order"_sv),
          .modify_order = create_metrics(name_, "modify_order"_sv),
          .cancel_order = create_metrics(name_, "cancel_order"_sv),
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
      // latency
      .write(latency_.ping, metrics::LATENCY);
}

void OrderEntry::operator()(
    const Event<CreateOrder> &event,
    const std::string_view &request_id,
    [[maybe_unused]] uint32_t gateway_order_id) {
  create_order(event.value, request_id, [this](auto &promise) {
    try {
      (*this)(promise.get());
      /*
      case core::http::Status::BAD_REQUEST:   // 400
      case core::http::Status::UNAUTHORIZED:  // 401
      case core::http::Status::FORBIDDEN:     // 403
      case core::http::Status::NOT_FOUND:     // 404
      */
    } catch (NetworkError &e) {
      // XXX send ack failure
      LOG(FATAL)(R"(Unexpected what="{}")"_fmt, e.what());
    }
  });
}

void OrderEntry::operator()(
    const Event<ModifyOrder> &event,
    const std::string_view &request_id,
    const server::OMS_Order &order) {
  modify_order(event.value, request_id, order, [this](auto &promise) {
    try {
      (*this)(promise.get());
      /*
      case core::http::Status::BAD_REQUEST:   // 400
      case core::http::Status::UNAUTHORIZED:  // 401
      case core::http::Status::FORBIDDEN:     // 403
      case core::http::Status::NOT_FOUND:     // 404
      */
    } catch (NetworkError &e) {
      // XXX send ack failure
      LOG(FATAL)(R"(Unexpected what="{}")"_fmt, e.what());
    }
  });
}

void OrderEntry::operator()(
    const Event<CancelOrder> &event,
    const std::string_view &request_id,
    const server::OMS_Order &order) {
  cancel_order(event.value, request_id, order, [this](auto &promise) {
    try {
      (*this)(promise.get());
      /*
      case core::http::Status::BAD_REQUEST:   // 400
      case core::http::Status::UNAUTHORIZED:  // 401
      case core::http::Status::FORBIDDEN:     // 403
      case core::http::Status::NOT_FOUND:     // 404
      */
    } catch (NetworkError &e) {
      // XXX send ack failure
      LOG(FATAL)(R"(Unexpected what="{}")"_fmt, e.what());
    }
  });
}

void OrderEntry::operator()(const core::web::Client::Connected &) {
  (*this)(GatewayStatus::READY);
}

void OrderEntry::operator()(const core::web::Client::Disconnected &) {
  ++counter_.disconnect;
  (*this)(GatewayStatus::DISCONNECTED);
}

void OrderEntry::operator()(const core::web::Client::Latency &latency) {
  server::TraceInfo trace_info;
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .name = name_,
      .latency = latency.sample,
  };
  server::create_trace_and_dispatch(trace_info, external_latency, handler_);
  latency_.ping.update(latency.sample);
}

void OrderEntry::operator()(GatewayStatus status) {
  if (core::update(status_, status)) {
    server::TraceInfo trace_info;
    OrderManagerStatus order_manager_status{
        .stream_id = stream_id_,
        .account = security_.get_account(),
        .status = status_,
    };
    LOG(INFO)("order_manager_status={}"_fmt, order_manager_status);
    server::create_trace_and_dispatch(trace_info, order_manager_status, handler_);
  }
}

void OrderEntry::create_order(
    const CreateOrder &create_order,
    const std::string_view &cl_ord_id,
    std::function<void(const core::Promise<json::OrderItem> &)> &&callback) {
  constexpr auto method = core::http::Method::POST;
  constexpr std::string_view path = "/api/v1/order"_sv;
  auto expires = compute_expires();
  // XXX use encode buffer
  auto message = roq::format(
      R"({{)"
      R"("clOrdID":"{}",)"
      R"("symbol":"{}",)"
      R"("side":"{}",)"
      R"("price":{},)"
      R"("orderQty":{},)"
      R"("ordType":"{}",)"
      R"("timeInForce":"{}",)"
      R"("execInst":"{}")"
      R"(}})"_fmt,
      cl_ord_id,
      create_order.symbol,
      json::map(create_order.side).as_raw_text(),
      create_order.price,
      create_order.quantity,
      json::map(create_order.order_type).as_raw_text(),
      json::map(create_order.time_in_force).as_raw_text(),
      create_order.execution_instruction == ExecutionInstruction::UNDEFINED
          ? std::string_view()
          : json::map(create_order.execution_instruction).as_raw_text());
  DLOG(INFO)(R"(DEBUG: body="{}")"_fmt, message);
  auto headers = security_.create_headers(expires, method, path, message);
  connection_.request(
      method,
      path,
      std::string_view(),  // query
      ACCEPT_JSON,
      CONTENT_TYPE_JSON,
      headers,
      message,
      [this, callback{std::move(callback)}](auto &response) {
        profile_.create_order([&]() {
          try {
            response.expect(core::http::Status::OK);
            auto order_item = core::json::Parser::create<json::OrderItem>(response.body());
            VLOG(1)(R"(order_item={})"_fmt, order_item);
            core::Promise<json::OrderItem> promise(order_item);
            callback(promise);
          } catch (NetworkError &e) {
            LOG(WARNING)
            (R"(Exception type={}, what="{}")"_fmt, typeid(e).name(), e.what());
            core::Promise<json::OrderItem> promise(std::current_exception());
            callback(promise);
          }
        });
      });
}

void OrderEntry::modify_order(
    const ModifyOrder &modify_order,
    [[maybe_unused]] const std::string_view &request_id,
    const server::OMS_Order &order,
    std::function<void(const core::Promise<json::OrderItem> &)> &&callback) {
  constexpr auto method = core::http::Method::PUT;
  constexpr std::string_view path = "/api/v1/order"_sv;
  auto expires = compute_expires();
  // XXX use encode buffer
  auto message = roq::format(
      R"({{)"
      R"("orderID":"{}",)"
      R"("orderQty":{},)"
      R"("price":{})"
      R"(}})"_fmt,
      order.external_order_id,
      modify_order.quantity,
      modify_order.price);
  DLOG(INFO)(R"(DEBUG: body="{}")"_fmt, message);
  auto headers = security_.create_headers(expires, method, path, message);
  connection_.request(
      method,
      path,
      std::string_view(),  // query
      ACCEPT_JSON,
      CONTENT_TYPE_JSON,
      headers,
      message,
      [this, callback{std::move(callback)}](auto &response) {
        profile_.modify_order([&]() {
          try {
            response.expect(core::http::Status::OK);
            auto order_item = core::json::Parser::create<json::OrderItem>(response.body());
            VLOG(1)(R"(order_item={})"_fmt, order_item);
            core::Promise<json::OrderItem> promise(order_item);
            callback(promise);
          } catch (NetworkError &e) {
            LOG(WARNING)
            (R"(Exception type={}, what="{}")"_fmt, typeid(e).name(), e.what());
            core::Promise<json::OrderItem> promise(std::current_exception());
            callback(promise);
          }
        });
      });
}

void OrderEntry::cancel_order(
    [[maybe_unused]] const CancelOrder &cancel_order,
    [[maybe_unused]] const std::string_view &request_id,
    const server::OMS_Order &order,
    std::function<void(const core::Promise<json::Order> &)> &&callback) {
  constexpr auto method = core::http::Method::DELETE;
  constexpr std::string_view path = "/api/v1/order"_sv;
  auto expires = compute_expires();
  // XXX use encode buffer
  auto message = roq::format(
      R"({{)"
      R"("orderID":"{}")"
      R"(}})"_fmt,
      order.external_order_id);
  DLOG(INFO)(R"(DEBUG: body="{}")"_fmt, message);
  auto headers = security_.create_headers(expires, method, path, message);
  connection_.request(
      method,
      path,
      std::string_view(),  // query
      ACCEPT_JSON,
      CONTENT_TYPE_JSON,
      headers,
      message,
      [this, callback{std::move(callback)}](auto &response) {
        profile_.cancel_order([&]() {
          try {
            response.expect(core::http::Status::OK);
            core::json::Buffer buffer(decode_buffer_);
            auto order = core::json::Parser::create<json::Order>(response.body(), buffer);
            VLOG(1)(R"(order={})"_fmt, order);
            core::Promise<json::Order> promise(order);
            callback(promise);
          } catch (NetworkError &e) {
            LOG(WARNING)
            (R"(Exception type={}, what="{}")"_fmt, typeid(e).name(), e.what());
            core::Promise<json::Order> promise(std::current_exception());
            callback(promise);
          }
        });
      });
}

void OrderEntry::operator()(const json::OrderItem &order_item) {
  server::TraceInfo trace_info;
  OrderUpdate{shared_}(order_item, trace_info);
}

void OrderEntry::operator()(const json::Order &order) {
  server::TraceInfo trace_info;
  OrderUpdate{shared_}(order, trace_info);
}

}  // namespace bitmex
}  // namespace roq
