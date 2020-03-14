/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/rest.h"

#include <fmt/format.h>
#include <fmt/chrono.h>

#include <chrono>

#include "roq/bitmex/gateway.h"
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
}  // namespace

Rest::Rest(
    Gateway& gateway,
    const Config& config,
    Random& random,
    core::event::Base& base,
    core::event::DNSBase& dns_base,
    core::ssl::Context& ssl_context)
    : _gateway(gateway),
      _random(random),
      _connection(
          *this,
          base,
          dns_base,
          ssl_context,
          core::URI(FLAGS_rest_uri),
          std::chrono::seconds { FLAGS_ping_freq_secs },
          FLAGS_decode_buffer_size,
          FLAGS_encode_buffer_size),
      _decode_buffer(FLAGS_decode_buffer_size),
      _counter {
        .disconnect = create_counter("disconnect"),
      },
      _profile {
        .success = create_profile("success"),
        .failure = create_profile("failure"),
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
    .write(_profile.success)
    .write(_profile.failure)
    .write(_profile.products)
    .write(_profile.accounts)
    // latency
    .write(_latency.ping);
}

void Rest::create_order(
    const CreateOrder& create_order,
    const std::string_view& cl_ord_id) {
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
        R"("timeInForce":"{}")"
        R"(}})"),
      cl_ord_id,
      create_order.symbol,
      json::map(create_order.side).as_raw_text(),
      create_order.price,
      create_order.quantity,
      json::map(create_order.order_type).as_raw_text(),
      json::map(create_order.time_in_force).as_raw_text());
  LOG(INFO)(
      FMT_STRING("DEBUG: body=\"{}\""),
      message);
  _connection.request(
      core::http::Method::POST,
      "/order",
      std::string_view(),  // headers
      message,
      [this](const auto status, const auto& body) {
        (void) status;  // avoid warning
        _profile.create_order(
            [&]() {
              LOG(INFO)(
                  FMT_STRING("DEBUG: body=\"{}\""),
                  body);
              auto order_item = core::json::Parser::create<json::OrderItem>(
                  body);
              LOG(INFO)(FMT_STRING("DEBUG: order_item={}"), order_item);
              // _gateway(json::Action::INSERT, order);
            });
      },
      [](const auto& e) {
        LOG(WARNING)(
            FMT_STRING("Exception what=\"{}\""),
            e.what());
        LOG(WARNING)("Unable to create order");
        LOG(FATAL)("Unexpected -- now what?");  // FIXME(thraneh): ...
      });
}

void Rest::cancel_order(
    const CancelOrder& cancel_order,
    const std::string_view& request_id,
    const server::OMS_Order& order) {
  (void) cancel_order;  // avoid warning
  (void) request_id;  // avoid warning
  auto message = fmt::format(
      FMT_STRING(
        R"({{)"
        R"("OrderID":"{}")"
        R"(}})"),
      order.external_order_id);
  LOG(INFO)(
      FMT_STRING("DEBUG: body=\"{}\""),
      message);
  _connection.request(
      core::http::Method::DELETE,
      "/order",
      std::string_view(),  // headers
      message,
      [this](const auto status, const auto& body) {
        (void) status;  // avoid warning
        _profile.cancel_order(
            [&]() {
              LOG(INFO)(
                  FMT_STRING("DEBUG: body=\"{}\""),
                  body);
              core::json::Buffer buffer(_decode_buffer);
              auto order = core::json::Parser::create<json::Order>(
                  body,
                  buffer);
              LOG(INFO)(FMT_STRING("DEBUG: order={}"), order);
              _gateway(json::Action::DELETE, order);
            });
      },
      [](const auto& e) {
        LOG(WARNING)(
            FMT_STRING("Exception what=\"{}\""),
            e.what());
        LOG(WARNING)("Unable to cancel order");
        LOG(FATAL)("Unexpected -- now what?");  // FIXME(thraneh): ...
      });
}

void Rest::get_products() {
  _connection.request(
      core::http::Method::GET,
      "/products",
      std::string_view(),  // headers
      std::string_view(),  // body
      [this](const auto status, const auto& body) {
        (void) status;  // avoid warning
        (void) body;  // avoid warning
        _profile.products(
            [&]() {
              /*
              core::json::Buffer buffer(_decode_buffer);
              auto products = json::Products::parse(
                  body,
                  buffer);
              VLOG(1)("products={}", products);
              _gateway(products);
              */
            });
      },
      [](const auto& e) {
        LOG(WARNING)(
            FMT_STRING("Exception what=\"{}\""),
            e.what());
        LOG(WARNING)("Unable to get products");
        LOG(FATAL)("Unexpected -- now what?");  // FIXME(thraneh): ...
      });
}

void Rest::get_accounts() {
  _connection.request(
      core::http::Method::GET,
      "/accounts",
      std::string_view(),  // headers
      std::string_view(),  // body
      [this](const auto status, const auto& body) {
        (void) status;  // avoid warning
        (void) body;  // avoid warning
        _profile.accounts(
            [&]() {
              /*
              core::json::Buffer buffer(_decode_buffer);
              auto accounts = json::Accounts::parse(
                  body,
                  buffer);
              VLOG(1)("accounts={}", accounts);
              _gateway(accounts);
              */
            });
      },
      [this](const auto& e) {
        LOG(WARNING)(
            FMT_STRING("Exception what=\"{}\""),
            e.what());
        LOG(WARNING)("Unable to get accounts");
        LOG(FATAL)("Unexpected -- now what?");  // FIXME(thraneh): ...
      });
}

void Rest::operator()(const core::web::Client::Connected&) {
  _gateway(*this);
}

void Rest::operator()(const core::web::Client::Disconnected&) {
  _gateway(*this);
  ++_counter.disconnect;
}

void Rest::operator()(const core::web::Client::Latency& latency) {
  _latency.ping.update(
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          latency.sample).count());
}

}  // namespace bitmex
}  // namespace roq
