/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/web_socket.h"

#include <fmt/format.h>

#include "roq/core/patterns.h"

#include "roq/core/clock.h"

#include "roq/core/charconv.h"

#include "roq/bitmex/options.h"

namespace roq {
namespace bitmex {

namespace {
constexpr std::string_view CONNECTION = "ws";

static auto create_counter(const std::string_view &function) {
  return core::metrics::Counter(FLAGS_name, CONNECTION, function);
}

static auto create_profile(const std::string_view &function) {
  return core::metrics::Profile(FLAGS_name, CONNECTION, function);
}

static auto create_latency(const std::string_view &function) {
  return core::metrics::Latency(FLAGS_name, CONNECTION, function);
}
}  // namespace

WebSocket::WebSocket(
    Handler &handler,
    [[maybe_unused]] const Config &config,
    Random &random,
    core::event::Base &base,
    core::event::DNSBase &dns_base,
    core::ssl::Context &ssl_context)
    : _handler(handler), _random(random),
      _connection(
          *this,
          base,
          dns_base,
          ssl_context,
          core::URI(FLAGS_ws_uri),
          std::string_view(),  // query
          std::chrono::seconds{FLAGS_ws_ping_freq_secs},
          FLAGS_decode_buffer_size,  // XXX need read buffer size
          FLAGS_encode_buffer_size,
          [this]() { return create_upgrade_headers(); }),
      _decode_buffer(FLAGS_decode_buffer_size),
      _counter{
          .disconnect = create_counter("disconnect"),
      },
      _profile{
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
      _latency{
          .ping = create_latency("ping"),
          .heartbeat = create_latency("heartbeat"),
      } {
}

bool WebSocket::ready() const {
  return _connection.ready();
}

void WebSocket::close() {
  _connection.close();
}

void WebSocket::operator()(const Event<Start> &) {
  _connection.start();
}

void WebSocket::operator()(const Event<Stop> &) {
  _connection.stop();
}

void WebSocket::operator()(const Event<Timer> &event) {
  if (_connection.refresh(event.value.now) == false) return;
  if (FLAGS_ws_cancel_on_disconnect && FLAGS_ws_cancel_all_after_secs &&
      _ready && _next_cancel_all_after <= event.value.now) {
    _next_cancel_all_after =
        event.value.now +
        std::chrono::seconds{FLAGS_ws_cancel_all_after_secs / 4};
    send_cancel_all_after(std::chrono::seconds{FLAGS_ws_cancel_all_after_secs});
  }
}

void WebSocket::subscribe(const std::string_view &topic) {
  auto message = fmt::format(
      R"({{)"
      R"("op":"subscribe",)"
      R"("args":"{}")"
      R"(}})",
      topic);
  _connection.send_text(message);
}

void WebSocket::subscribe(
    const std::string_view &topic, const std::vector<std::string> &filter) {
  if (filter.empty()) {
    subscribe(topic);
  } else {
    auto message = fmt::format(
        R"({{)"
        R"("op":"subscribe",)"
        R"("args":"{}:{}")"
        R"(}})",
        topic,
        fmt::join(filter, ","));
    _connection.send_text(message);
  }
}

void WebSocket::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(_counter.disconnect, metrics::COUNTER)
      // profile
      .write(_profile.parse, metrics::PROFILE)
      .write(_profile.cancel_all_after, metrics::PROFILE)
      .write(_profile.error, metrics::PROFILE)
      .write(_profile.execution, metrics::PROFILE)
      .write(_profile.funding, metrics::PROFILE)
      .write(_profile.handshake, metrics::PROFILE)
      .write(_profile.instrument, metrics::PROFILE)
      .write(_profile.liquidation, metrics::PROFILE)
      .write(_profile.margin, metrics::PROFILE)
      .write(_profile.order, metrics::PROFILE)
      .write(_profile.order_book_l2, metrics::PROFILE)
      .write(_profile.position, metrics::PROFILE)
      .write(_profile.quote, metrics::PROFILE)
      .write(_profile.settlement, metrics::PROFILE)
      .write(_profile.subscribe, metrics::PROFILE)
      .write(_profile.trade, metrics::PROFILE)
      // latency
      .write(_latency.ping, metrics::LATENCY)
      .write(_latency.heartbeat, metrics::LATENCY);
}

void WebSocket::operator()(const core::web::Socket::Connected &) {
  // note! don't notify gateway: wait for ready
}

void WebSocket::operator()(const core::web::Socket::Disconnected &) {
  _ready = false;
  _next_cancel_all_after = {};
  _handler(*this);
}

void WebSocket::operator()(const core::web::Socket::Ready &) {
  // note! don't notify gateway: wait for handshake
}

void WebSocket::operator()(const core::web::Socket::Close &) {
}

void WebSocket::operator()(const core::web::Socket::Latency &latency) {
  _latency.ping.update(
      std::chrono::duration_cast<std::chrono::nanoseconds>(latency.sample)
          .count());
}

void WebSocket::operator()(const core::web::Socket::Text &text) {
  parse(text.payload);
}

std::string WebSocket::create_upgrade_headers() {
  auto expires = std::chrono::duration_cast<std::chrono::seconds>(
      core::get_realtime_clock() + std::chrono::seconds{5});
  return _random.create_headers(
      expires, core::http::Method::GET, "/realtime", std::string_view());
}

void WebSocket::parse(const std::string_view &message) {
  VLOG(4)(R"(message={})", message);
  _profile.parse([&]() {
    try {
      parse_helper(message);
    } catch (std::exception &e) {
      LOG(WARNING)(R"(message="{}")", message);
      LOG(FATAL)(R"(ERROR what="{}")", e.what());
    }
  });
}

void WebSocket::parse_helper(const std::string_view &message) {
  server::TraceInfo trace_info;
  core::json::Buffer buffer(_decode_buffer);
  json::Parser::dispatch(*this, message, buffer, trace_info);
}

void WebSocket::send_cancel_all_after(std::chrono::seconds seconds) {
  auto message = fmt::format(
      R"({{)"
      R"("op":"cancelAllAfter",)"
      R"("args":{})"
      R"(}})",
      seconds.count() * 1000);  // milliseconds
  DLOG(INFO)("message={}", message);
  _connection.send_text(message);
}

void WebSocket::operator()(const json::CancelAllAfter &cancel_all_after) {
  _profile.cancel_all_after([&]() {
    VLOG(1)(R"(cancel_all_after={})", cancel_all_after);
  });
}

void WebSocket::operator()(const json::Error &error) {
  _profile.error([&]() { LOG(WARNING)(R"(error={})", error); });
  _connection.close();
}

void WebSocket::operator()(const json::Handshake &handshake) {
  _profile.handshake([&]() {
    VLOG(1)(R"(handshake={})", handshake);
    LOG(INFO)("Ready");
    assert(_ready == false);
    _ready = true;
    _handler(*this);
    if (FLAGS_ws_cancel_on_disconnect == false ||
        FLAGS_ws_cancel_all_after_secs == 0)
      send_cancel_all_after(std::chrono::seconds{});
  });
}

void WebSocket::operator()(const json::Subscribe &subscribe) {
  _profile.subscribe([&]() {
    VLOG(1)(R"(subscribe={})", subscribe);
    if (subscribe.success) {
      assert(subscribe.failure == false);
      LOG(INFO)
      (R"(Successfully subscribed to topic="{}")", subscribe.subscribe);
    } else if (subscribe.failure) {
      assert(subscribe.success == false);
      LOG(WARNING)(R"(Failed to subscribe topic="{}")", subscribe.subscribe);
    } else {
      LOG(FATAL)("Expected success or failure");
    }
    // TODO(thraneh): clear timeout
  });
}

void WebSocket::operator()(
    const json::Action action,
    const json::Execution &execution,
    const server::TraceInfo &trace_info) {
  _profile.execution([&]() {
    VLOG(1)(R"(action={}, execution={})", action, execution);
    _handler(action, execution, trace_info);
  });
}

void WebSocket::operator()(
    const json::Action action,
    const json::Funding &funding,
    const server::TraceInfo &) {
  _profile.funding([&]() {
    VLOG(2)(R"(action={}, funding={})", action, funding);
    // XXX not used
  });
}

void WebSocket::operator()(
    const json::Action action,
    const json::Instrument &instrument,
    const server::TraceInfo &trace_info) {
  _profile.instrument([&]() {
    VLOG(2)(R"(action={}, instrument={})", action, instrument);
    _handler(action, instrument, trace_info);
  });
}

void WebSocket::operator()(
    const json::Action action,
    const json::Liquidation &liquidation,
    const server::TraceInfo &) {
  _profile.liquidation([&]() {
    VLOG(2)(R"(action={}, liquidation={})", action, liquidation);
    /// XXX not used
  });
}

void WebSocket::operator()(
    const json::Action action,
    const json::Margin &margin,
    const server::TraceInfo &) {
  _profile.margin([&]() {
    VLOG(2)(R"(action={}, margin={})", action, margin);
    /// XXX not used
  });
}

void WebSocket::operator()(
    const json::Action action,
    const json::Order &order,
    const server::TraceInfo &trace_info) {
  _profile.order([&]() {
    VLOG(1)(R"(action={}, order={})", action, order);
    _handler(action, order, trace_info);
  });
}

void WebSocket::operator()(
    const json::Action action,
    const json::OrderBookL2 &order_book_l2,
    const server::TraceInfo &trace_info) {
  _profile.order_book_l2([&]() {
    VLOG(3)(R"(action={}, order_book_l2={})", action, order_book_l2);
    _handler(action, order_book_l2, trace_info);
  });
}

void WebSocket::operator()(
    const json::Action action,
    const json::Position &position,
    const server::TraceInfo &trace_info) {
  _profile.position([&]() {
    VLOG(2)(R"(action={}, position={})", action, position);
    _handler(action, position, trace_info);
  });
}

void WebSocket::operator()(
    const json::Action action,
    const json::Quote &quote,
    const server::TraceInfo &trace_info) {
  _profile.quote([&]() {
    VLOG(3)(R"(action={}, quote={})", action, quote);
    _handler(action, quote, trace_info);
  });
}

void WebSocket::operator()(
    const json::Action action,
    const json::Settlement &settlement,
    const server::TraceInfo &trace_info) {
  _profile.settlement([&]() {
    VLOG(3)(R"(action={}, settlement={})", action, settlement);
    _handler(action, settlement, trace_info);
  });
}

void WebSocket::operator()(
    const json::Action action,
    const json::Trade &trade,
    const server::TraceInfo &trace_info) {
  _profile.trade([&]() {
    VLOG(2)(R"(action={}, trade={})", action, trade);
    _handler(action, trade, trace_info);
  });
}

}  // namespace bitmex
}  // namespace roq
