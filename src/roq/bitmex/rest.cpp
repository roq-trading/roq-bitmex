/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/rest.h"

#include <fmt/format.h>
#include <fmt/chrono.h>

#include <chrono>
#include <utility>

#include "roq/bitmex/options.h"

#include "roq/bitmex/json/utils.h"

namespace roq {
namespace bitmex {

namespace {
constexpr std::string_view CONNECTION = "rest";

static auto create_counter(
    const std::string_view& function) {
  return core::metrics::Counter(
      FLAGS_name,
      CONNECTION,
      function);
}

static auto create_profile(
    const std::string_view& function) {
  return core::metrics::Profile(
      FLAGS_name,
      CONNECTION,
      function);
}

static auto create_latency(
    const std::string_view& function) {
  return core::metrics::Latency(
      FLAGS_name,
      CONNECTION,
      function);
}

static auto compute_expires() {
  auto result = core::get_realtime_clock()
    + std::chrono::seconds{ FLAGS_rest_expires_timeout_secs };
  return std::chrono::ceil<std::chrono::seconds>(result);
}
}  // namespace

Rest::Rest(
    Handler& handler,
    const Config& config,
    Random& random,
    core::event::Base& base,
    core::event::DNSBase& dns_base,
    core::ssl::Context& ssl_context)
    : _handler(handler),
      _random(random),
      _connection(
          *this,
          base,
          dns_base,
          ssl_context,
          core::URI(FLAGS_rest_uri),
          PACKAGE_NAME,
          true,  // keep alive
          std::chrono::seconds { FLAGS_rest_rate_limit_interval_secs },
          FLAGS_rest_rate_limit_max_requests,
          std::chrono::seconds { FLAGS_rest_ping_freq_secs },
          FLAGS_decode_buffer_size,
          FLAGS_encode_buffer_size,
          FLAGS_rest_ping_path),
      _decode_buffer(FLAGS_decode_buffer_size),
      _counter {
        .disconnect = create_counter("disconnect"),
      },
      _profile {
        .products = create_profile("products"),
        .accounts = create_profile("accounts"),
        .create_order = create_profile("create_order"),
        .modify_order = create_profile("modify_order"),
        .cancel_order = create_profile("cancel_order"),
      },
      _latency {
        .ping = create_latency("ping"),
      } {
  (void) config;  // avoid warning
}

bool Rest::ready() const {
  return _connection.ready();
}

void Rest::operator()(const StartEvent&) {
  _connection.start();
}

void Rest::operator()(const StopEvent&) {
  _connection.stop();
}

void Rest::operator()(const TimerEvent& event) {
  _connection.refresh(event.now);
}

void Rest::operator()(Metrics& metrics) {
  metrics
    // counter
    .write(_counter.disconnect)
    // profile
    .write(_profile.products)
    .write(_profile.accounts)
    // latency
    .write(_latency.ping);
}

void Rest::create_order(
    const CreateOrder& create_order,
    const std::string_view& cl_ord_id,
    std::function<void(const core::Promise<json::OrderItem>&)>&& callback) {
  constexpr auto method = core::http::Method::POST;
  constexpr std::string_view path = "/api/v1/order";
  auto expires = compute_expires();
  // XXX use encode buffer
  auto message = fmt::format(
      FMT_STRING(
        R"({{)"
        R"("clOrdID":"{}",)"
        R"("symbol":"{}",)"
        R"("side":"{}",)"
        R"("price":{},)"
        R"("orderQty":{},)"
        R"("ordType":"{}",)"
        R"("timeInForce":"{}",)"
        R"("execInst":"{}")"
        R"(}})"),
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
  DLOG(INFO)(
      FMT_STRING(R"(body="{}")"),
      message);
  auto headers = _random.create_headers(
      expires,
      method,
      path,
      message);
  _connection.request(
      method,
      path,
      std::string_view(),  // query
      headers,
      message,
      [this, callback](auto& response) {
    _profile.create_order(
        [&]() {
      try {
        response.expect(core::http::Status::OK);
        auto order_item =
          core::json::Parser::create<json::OrderItem>(response.body());
        VLOG(1)(
            FMT_STRING(R"(order_item={})"),
            order_item);
        core::Promise<json::OrderItem> promise(order_item);
        callback(promise);
      } catch (NetworkError& e) {
        LOG(WARNING)(
            FMT_STRING(R"(Exception type={}, what="{}")"),
            typeid(e).name(),
            e.what());
        core::Promise<json::OrderItem> promise(std::current_exception());
        callback(promise);
      }
    });
  });
}

void Rest::modify_order(
    const ModifyOrder& modify_order,
    const std::string_view& request_id,
    const server::OMS_Order& order,
    std::function<void(const core::Promise<json::OrderItem>&)>&& callback) {
  (void) request_id;  // avoid warning
  constexpr auto method = core::http::Method::PUT;
  constexpr std::string_view path = "/api/v1/order";
  auto expires = compute_expires();
  // XXX use encode buffer
  auto message = fmt::format(
      FMT_STRING(
        R"({{)"
        R"("orderID":"{}",)"
        R"("orderQty":{},)"
        R"("price":{})"
        R"(}})"),
      order.external_order_id,
      modify_order.quantity,
      modify_order.price);
  DLOG(INFO)(
      FMT_STRING(R"(body="{}")"),
      message);
  auto headers = _random.create_headers(
      expires,
      method,
      path,
      message);
  _connection.request(
      method,
      path,
      std::string_view(),  // query
      headers,
      message,
      [this, callback](auto& response) {
    _profile.modify_order(
        [&]() {
      try {
        response.expect(core::http::Status::OK);
        auto order_item =
          core::json::Parser::create<json::OrderItem>(response.body());
        VLOG(1)(
            FMT_STRING(R"(order_item={})"),
            order_item);
        core::Promise<json::OrderItem> promise(order_item);
        callback(promise);
      } catch (NetworkError& e) {
        LOG(WARNING)(
            FMT_STRING(R"(Exception type={}, what="{}")"),
            typeid(e).name(),
            e.what());
        core::Promise<json::OrderItem> promise(std::current_exception());
        callback(promise);
      }
    });
  });
}

void Rest::cancel_order(
    const CancelOrder& cancel_order,
    const std::string_view& request_id,
    const server::OMS_Order& order,
    std::function<void(const core::Promise<json::Order>&)>&& callback) {
  (void) cancel_order;  // avoid warning
  (void) request_id;  // avoid warning
  constexpr auto method = core::http::Method::DELETE;
  constexpr std::string_view path = "/api/v1/order";
  auto expires = compute_expires();
  // XXX use encode buffer
  auto message = fmt::format(
      FMT_STRING(
        R"({{)"
        R"("orderID":"{}")"
        R"(}})"),
      order.external_order_id);
  DLOG(INFO)(
      FMT_STRING(R"(body="{}")"),
      message);
  auto headers = _random.create_headers(
      expires,
      method,
      path,
      message);
  _connection.request(
      method,
      path,
      std::string_view(),  // query
      headers,
      message,
      [this, callback](auto& response) {
    _profile.cancel_order(
        [&]() {
      try {
        response.expect(core::http::Status::OK);
        core::json::Buffer buffer(_decode_buffer);
        auto order = core::json::Parser::create<json::Order>(
            response.body(),
            buffer);
        VLOG(1)(
            FMT_STRING(R"(order={})"),
            order);
        core::Promise<json::Order> promise(order);
        callback(promise);
      } catch (NetworkError& e) {
        LOG(WARNING)(
            FMT_STRING(R"(Exception type={}, what="{}")"),
            typeid(e).name(),
            e.what());
        core::Promise<json::Order> promise(std::current_exception());
        callback(promise);
      }
    });
  });
}

/* 20200512 -- doesn't look like anything real
template <>
void Rest::get(
    std::function<void(const core::Promise<json::Accounts>&)>&& callback) {
  constexpr auto method = core::http::Method::GET;
  constexpr std::string_view path = "/api/v1/accounts";
  _connection.request(
      method,
      path,
      std::string_view(),  // query
      std::string_view(),  // headers
      std::string_view(),  // body
      [this, callback](auto& response) {
    _profile.accounts(
        [&]() {
      try {
        response.expect(core::http::Status::OK);
        core::json::Buffer buffer(_decode_buffer);
        auto accounts = json::Accounts::parse(
            response.body(),
            buffer);
        VLOG(1)("accounts={}", accounts);
        core::Promise<json::Accounts> promise(accounts);
        callback(promise);
      } catch (NetworkError& e) {
        LOG(WARNING)(
            FMT_STRING(R"(Exception type={}, what="{}")"),
            typeid(e).name(),
            e.what());
        core::Promise<json::Accounts> promise(std::current_exception());
        callback(promise);
      }
    });
  });
}
*/

void Rest::operator()(const core::web::Client::Connected&) {
  _handler(*this);
}

void Rest::operator()(const core::web::Client::Disconnected&) {
  _handler(*this);
  ++_counter.disconnect;
}

void Rest::operator()(const core::web::Client::Latency& latency) {
  _latency.ping.update(
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          latency.sample).count());
}

}  // namespace bitmex
}  // namespace roq
