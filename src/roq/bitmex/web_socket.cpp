/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/web_socket.h"

#include <fmt/format.h>
// #include <fmt/chrono.h>

#include "roq/builtins.h"
#include "roq/patterns.h"

#include "roq/core/clock.h"

#include "roq/core/charconv.h"

#include "roq/bitmex/gateway.h"
#include "roq/bitmex/options.h"

namespace roq {
namespace bitmex {

namespace {
constexpr std::string_view CONNECTION = "ws";

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

WebSocket::WebSocket(
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
          core::URI(FLAGS_ws_uri),
          std::chrono::seconds { FLAGS_ping_freq_secs },
          FLAGS_decode_buffer_size,  // XXX need read buffer size
          FLAGS_encode_buffer_size,
          [this]() {
            return create_upgrade_headers();
          }),
      _decode_buffer(FLAGS_decode_buffer_size),
      _counter {
        .disconnect = create_counter("disconnect"),
      },
      _profile {
        .parse = create_profile("parse"),
        .cancel_all_after = create_profile("cancel_all_after"),
        .error = create_profile("error"),
        .execution = create_profile("execution"),
        .funding = create_profile("funding"),
        .handshake = create_profile("handshake"),
        .instrument = create_profile("instrument"),
        .liquidation = create_profile("liquidation"),
        .margin = create_profile("margin"),
        .order = create_profile("order"),
        .order_book_l2 = create_profile("order_book_l2"),
        .position = create_profile("position"),
        .quote = create_profile("quote"),
        .settlement = create_profile("settlement"),
        .subscribe = create_profile("subscribe"),
        .trade = create_profile("trade"),
      },
      _latency {
        .ping = create_latency("ping"),
        .heartbeat = create_latency("heartbeat"),
      } {
  (void) config;  // avoid warning
}

bool WebSocket::ready() const {
  return _connection.ready();
}

void WebSocket::close() {
  _connection.close();
}

void WebSocket::operator()(const StartEvent&) {
  _connection.start();
}

void WebSocket::operator()(const StopEvent&) {
  _connection.stop();
}

void WebSocket::operator()(const TimerEvent& event) {
  _connection.refresh(event.now);
  if (_connection.ready()) {
    if (FLAGS_cancel_on_disconnect &&
        FLAGS_cancel_all_after_secs &&
        _next_cancel_all_after <= event.now) {
      _next_cancel_all_after = event.now +
        std::chrono::seconds { FLAGS_cancel_all_after_secs / 4 };
      send_cancel_all_after(
          std::chrono::seconds {FLAGS_cancel_all_after_secs });
    }
  }
}

void WebSocket::subscribe(const std::string_view& topic) {
  auto message = fmt::format(
      FMT_STRING(
        "{{"
        "\"op\":\"subscribe\","
        "\"args\":"
        "\"{}\""
        "}}"),
      topic);
  _connection.send_text(message);
}

void WebSocket::subscribe(
    const std::string_view& topic,
    const std::vector<std::string>& filter) {
  if (filter.empty()) {
    subscribe(topic);
  } else {
    auto message = fmt::format(
        FMT_STRING(
          "{{"
          "\"op\":\"subscribe\","
          "\"args\":"
          "\"{}:{}\""
          "}}"),
        topic,
        fmt::join(filter, ","));
    _connection.send_text(message);
  }
}

void WebSocket::operator()(Metrics& metrics) {
  metrics
    // counter
    .write(_counter.disconnect)
    // profile
    .write(_profile.parse)
    .write(_profile.cancel_all_after)
    .write(_profile.error)
    .write(_profile.execution)
    .write(_profile.funding)
    .write(_profile.handshake)
    .write(_profile.instrument)
    .write(_profile.liquidation)
    .write(_profile.margin)
    .write(_profile.order)
    .write(_profile.order_book_l2)
    .write(_profile.position)
    .write(_profile.quote)
    .write(_profile.settlement)
    .write(_profile.subscribe)
    .write(_profile.trade)
    // latency
    .write(_latency.ping)
    .write(_latency.heartbeat);
}

void WebSocket::operator()(const core::web::Socket::Connected&) {
  _gateway(*this);
}

void WebSocket::operator()(const core::web::Socket::Disconnected&) {
  _next_cancel_all_after = {};
  _received_handshake = false;
  _gateway(*this);
}

void WebSocket::operator()(const core::web::Socket::Ready&) {
  _gateway(*this);
}

void WebSocket::operator()(const core::web::Socket::Close&) {
}

void WebSocket::operator()(const core::web::Socket::Latency& latency) {
  _latency.ping.update(
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          latency.sample).count());
}

void WebSocket::operator()(const core::web::Socket::Text& text) {
  parse(text.payload);
}

std::string WebSocket::create_upgrade_headers() {
  auto expires = std::chrono::duration_cast<std::chrono::seconds>(
      core::get_realtime_clock() + std::chrono::seconds {5});
  return _random.create_headers(
      expires,
      core::http::Method::GET,
      "/realtime",
      std::string_view());
}

void WebSocket::parse(const std::string_view& message) {
  VLOG(4)(
      FMT_STRING("message={}"),
      message);
  _profile.parse(
      [&]() {
        try {
          parse_helper(message);
        } catch (std::exception& e) {
          LOG(FATAL)(
              FMT_STRING("ERROR what=\"{}\""),
              e.what());
        }
      });
}

void WebSocket::parse_helper(const std::string_view& message) {
  core::json::Buffer buffer(_decode_buffer);
  json::Parser::dispatch(
      *this,
      message,
      buffer);
}

void WebSocket::send_cancel_all_after(std::chrono::seconds seconds) {
  auto message = fmt::format(
      FMT_STRING(
        "{{"
        "\"op\":\"cancelAllAfter\","
        "\"args\":{}"
        "}}"),
      seconds.count() * 1000);  // milliseconds
  _connection.send_text(message);
}

void WebSocket::operator()(
    const json::CancelAllAfter& cancel_all_after) {
  _profile.cancel_all_after(
      [&]() {
        VLOG(1)(
            FMT_STRING("cancel_all_after={}"),
            cancel_all_after);
      });
}

void WebSocket::operator()(const json::Error& error) {
  _profile.error(
      [&]() {
        LOG(WARNING)(
            FMT_STRING("error={}"),
            error);
      });
  _connection.close();
}

void WebSocket::operator()(const json::Handshake& handshake) {
  _profile.handshake(
      [&]() {
        VLOG(1)(
            FMT_STRING("handshake={}"),
            handshake);
        _received_handshake = true;
        _gateway(*this);
        if (FLAGS_cancel_on_disconnect == false ||
            FLAGS_cancel_all_after_secs == 0)
          send_cancel_all_after(std::chrono::seconds {});
      });
}

void WebSocket::operator()(const json::Subscribe& subscribe) {
  _profile.subscribe(
      [&]() {
        VLOG(1)(
            FMT_STRING("subscribe={}"),
            subscribe);
        if (subscribe.success) {
          assert(subscribe.failure == false);
          LOG(INFO)(
              FMT_STRING("Successfully subscribed to topic=\"{}\""),
              subscribe.subscribe);
        } else if (subscribe.failure) {
          assert(subscribe.success == false);
          LOG(WARNING)(
              FMT_STRING("Failed to subscribe topic=\"{}\""),
              subscribe.subscribe);
        } else {
          LOG(FATAL)("Expected success or failure");
        }
        // TODO(thraneh): clear timeout
      });
}

void WebSocket::operator()(
    const json::Action action,
    const json::Execution& execution) {
  _profile.execution(
      [&]() {
        VLOG(1)(
            FMT_STRING("action={}, execution={}"),
            action, execution);
      });
}

void WebSocket::operator()(
    const json::Action action,
    const json::Funding& funding) {
  _profile.funding(
      [&]() {
        VLOG(1)(
            FMT_STRING("action={}, funding={}"),
            action, funding);
      });
}

void WebSocket::operator()(
    const json::Action action,
    const json::Instrument& instrument) {
  _profile.instrument(
      [&]() {
        VLOG(1)(
            FMT_STRING("action={}, instrument={}"),
            action, instrument);
        _gateway(action, instrument);
      });
}

void WebSocket::operator()(
    const json::Action action,
    const json::Liquidation& liquidation) {
  _profile.liquidation(
      [&]() {
        VLOG(1)(
            FMT_STRING("action={}, liquidation={}"),
            action, liquidation);
      });
}

void WebSocket::operator()(
    const json::Action action,
    const json::Margin& margin) {
  _profile.margin(
      [&]() {
        VLOG(1)(
            FMT_STRING("action={}, margin={}"),
            action, margin);
      });
}

void WebSocket::operator()(
    const json::Action action,
    const json::Order& order) {
  _profile.order(
      [&]() {
        VLOG(1)(
            FMT_STRING("action={}, order={}"),
            action, order);
        _gateway(action, order);
      });
}

void WebSocket::operator()(
    const json::Action action,
    const json::OrderBookL2& order_book_l2) {
  _profile.order_book_l2(
      [&]() {
        VLOG(1)(
            FMT_STRING("action={}, order_book_l2={}"),
            action, order_book_l2);
        _gateway(action, order_book_l2);
      });
}

void WebSocket::operator()(
    const json::Action action,
    const json::Position& position) {
  _profile.position(
      [&]() {
        VLOG(1)(
            FMT_STRING("action={}, position={}"),
            action, position);
        _gateway(action, position);
      });
}

void WebSocket::operator()(
    const json::Action action,
    const json::Quote& quote) {
  _profile.quote(
      [&]() {
        VLOG(1)(
            FMT_STRING("action={}, quote={}"),
            action, quote);
        _gateway(action, quote);
      });
}

void WebSocket::operator()(
    const json::Action action,
    const json::Settlement& settlement) {
  _profile.settlement(
      [&]() {
        VLOG(1)(
            FMT_STRING("action={}, settlement={}"),
            action, settlement);
        _gateway(action, settlement);
      });
}

void WebSocket::operator()(
    const json::Action action,
    const json::Trade& trade) {
  _profile.trade(
      [&]() {
        VLOG(1)(
            FMT_STRING("action={}, trade={}"),
            action, trade);
        _gateway(action, trade);
      });
}

}  // namespace bitmex
}  // namespace roq
