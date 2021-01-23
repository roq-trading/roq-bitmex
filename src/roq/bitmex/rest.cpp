/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/rest.h"

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <chrono>
#include <utility>

#include "roq/bitmex/flags.h"

#include "roq/bitmex/json/utils.h"

namespace roq {
namespace bitmex {

namespace {
constexpr std::string_view CONNECTION = "rest";

static const std::string_view ACCEPT_JSON{"application/json"};
static const std::string_view CONTENT_TYPE_JSON{"application/json"};

static auto create_counter(const std::string_view &function) {
  return core::metrics::Counter(Flags::name(), CONNECTION, function);
}

static auto create_profile(const std::string_view &function) {
  return core::metrics::Profile(Flags::name(), CONNECTION, function);
}

static auto create_latency(const std::string_view &function) {
  return core::metrics::Latency(Flags::name(), CONNECTION, function);
}

static auto compute_expires() {
  auto result = core::get_realtime_clock() +
                std::chrono::seconds{Flags::rest_expires_timeout_secs()};
  return std::chrono::ceil<std::chrono::seconds>(result);
}
}  // namespace

Rest::Rest(
    Handler &handler,
    [[maybe_unused]] const Config &config,
    Random &random,
    core::event::Base &base,
    core::event::DNSBase &dns_base,
    core::ssl::Context &ssl_context)
    : handler_(handler), random_(random),
      connection_(
          *this,
          base,
          dns_base,
          ssl_context,
          core::URI(Flags::rest_uri()),
          ROQ_PACKAGE_NAME,
          true,  // keep alive
          Flags::rest_request_queue_depth(),
          std::chrono::seconds{Flags::rest_request_timeout_secs()},
          std::chrono::seconds{Flags::rest_rate_limit_interval_secs()},
          Flags::rest_rate_limit_max_requests(),
          std::chrono::seconds{Flags::rest_ping_freq_secs()},
          Flags::decode_buffer_size(),
          Flags::encode_buffer_size(),
          Flags::rest_ping_path()),
      decode_buffer_(Flags::decode_buffer_size()),
      counter_{
          .disconnect = create_counter("disconnect"),
      },
      profile_{
          .products = create_profile("products"),
          .accounts = create_profile("accounts"),
          .create_order = create_profile("create_order"),
          .modify_order = create_profile("modify_order"),
          .cancel_order = create_profile("cancel_order"),
      },
      latency_{
          .ping = create_latency("ping"),
      } {
}

bool Rest::ready() const {
  return connection_.ready();
}

void Rest::operator()(const Event<Start> &) {
  connection_.start();
}

void Rest::operator()(const Event<Stop> &) {
  connection_.stop();
}

void Rest::operator()(const Event<Timer> &event) {
  connection_.refresh(event.value.now);
}

void Rest::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::COUNTER)
      // profile
      .write(profile_.products, metrics::PROFILE)
      .write(profile_.accounts, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY);
}

void Rest::create_order(
    const CreateOrder &create_order,
    const std::string_view &cl_ord_id,
    std::function<void(const core::Promise<json::OrderItem> &)> &&callback) {
  constexpr auto method = core::http::Method::POST;
  constexpr std::string_view path = "/api/v1/order";
  auto expires = compute_expires();
  // XXX use encode buffer
  auto message = fmt::format(
      R"({{)"
      R"("clOrdID":"{}",)"
      R"("symbol":"{}",)"
      R"("side":"{}",)"
      R"("price":{},)"
      R"("orderQty":{},)"
      R"("ordType":"{}",)"
      R"("timeInForce":"{}",)"
      R"("execInst":"{}")"
      R"(}})",
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
  DLOG(INFO)(R"(body="{}")", message);
  auto headers = random_.create_headers(expires, method, path, message);
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
            auto order_item =
                core::json::Parser::create<json::OrderItem>(response.body());
            VLOG(1)(R"(order_item={})", order_item);
            core::Promise<json::OrderItem> promise(order_item);
            callback(promise);
          } catch (NetworkError &e) {
            LOG(WARNING)
            (R"(Exception type={}, what="{}")", typeid(e).name(), e.what());
            core::Promise<json::OrderItem> promise(std::current_exception());
            callback(promise);
          }
        });
      });
}

void Rest::modify_order(
    const ModifyOrder &modify_order,
    [[maybe_unused]] const std::string_view &request_id,
    const server::OMS_Order &order,
    std::function<void(const core::Promise<json::OrderItem> &)> &&callback) {
  constexpr auto method = core::http::Method::PUT;
  constexpr std::string_view path = "/api/v1/order";
  auto expires = compute_expires();
  // XXX use encode buffer
  auto message = fmt::format(
      R"({{)"
      R"("orderID":"{}",)"
      R"("orderQty":{},)"
      R"("price":{})"
      R"(}})",
      order.external_order_id,
      modify_order.quantity,
      modify_order.price);
  DLOG(INFO)(R"(body="{}")", message);
  auto headers = random_.create_headers(expires, method, path, message);
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
            auto order_item =
                core::json::Parser::create<json::OrderItem>(response.body());
            VLOG(1)(R"(order_item={})", order_item);
            core::Promise<json::OrderItem> promise(order_item);
            callback(promise);
          } catch (NetworkError &e) {
            LOG(WARNING)
            (R"(Exception type={}, what="{}")", typeid(e).name(), e.what());
            core::Promise<json::OrderItem> promise(std::current_exception());
            callback(promise);
          }
        });
      });
}

void Rest::cancel_order(
    [[maybe_unused]] const CancelOrder &cancel_order,
    [[maybe_unused]] const std::string_view &request_id,
    const server::OMS_Order &order,
    std::function<void(const core::Promise<json::Order> &)> &&callback) {
  constexpr auto method = core::http::Method::DELETE;
  constexpr std::string_view path = "/api/v1/order";
  auto expires = compute_expires();
  // XXX use encode buffer
  auto message = fmt::format(
      R"({{)"
      R"("orderID":"{}")"
      R"(}})",
      order.external_order_id);
  DLOG(INFO)(R"(body="{}")", message);
  auto headers = random_.create_headers(expires, method, path, message);
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
            auto order = core::json::Parser::create<json::Order>(
                response.body(), buffer);
            VLOG(1)(R"(order={})", order);
            core::Promise<json::Order> promise(order);
            callback(promise);
          } catch (NetworkError &e) {
            LOG(WARNING)
            (R"(Exception type={}, what="{}")", typeid(e).name(), e.what());
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
  connection_.request(
      method,
      path,
      std::string_view(),  // query
      std::string_view(),  // headers
      std::string_view(),  // body
      [this, callback{std::move(callback)}](auto& response) {
    profile_.accounts(
        [&]() {
      try {
        response.expect(core::http::Status::OK);
        core::json::Buffer buffer(decode_buffer_);
        auto accounts = json::Accounts::parse(
            response.body(),
            buffer);
        VLOG(1)("accounts={}", accounts);
        core::Promise<json::Accounts> promise(accounts);
        callback(promise);
      } catch (NetworkError& e) {
        LOG(WARNING)(
            R"(Exception type={}, what="{}")",
            typeid(e).name(),
            e.what());
        core::Promise<json::Accounts> promise(std::current_exception());
        callback(promise);
      }
    });
  });
}
*/

void Rest::operator()(const core::web::Client::Connected &) {
  handler_(*this);
}

void Rest::operator()(const core::web::Client::Disconnected &) {
  handler_(*this);
  ++counter_.disconnect;
}

void Rest::operator()(const core::web::Client::Latency &latency) {
  latency_.ping.update(
      std::chrono::duration_cast<std::chrono::nanoseconds>(latency.sample)
          .count());
}

}  // namespace bitmex
}  // namespace roq
