/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/bitmex/web_socket.h"

#include <fmt/format.h>

#include "roq/core/patterns.h"

#include "roq/core/clock.h"

#include "roq/core/charconv.h"

#include "roq/bitmex/flags.h"

namespace roq {
namespace bitmex {

namespace {
constexpr std::string_view CONNECTION = "ws";

static auto create_counter(const std::string_view &function) {
  return core::metrics::Counter(Flags::name(), CONNECTION, function);
}

static auto create_profile(const std::string_view &function) {
  return core::metrics::Profile(Flags::name(), CONNECTION, function);
}

static auto create_latency(const std::string_view &function) {
  return core::metrics::Latency(Flags::name(), CONNECTION, function);
}
}  // namespace

WebSocket::WebSocket(
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
          core::URI(Flags::ws_uri()),
          std::string_view(),  // query
          std::chrono::seconds{Flags::ws_ping_freq_secs()},
          Flags::decode_buffer_size(),  // XXX need read buffer size
          Flags::encode_buffer_size(),
          [this]() { return create_upgrade_headers(); }),
      decode_buffer_(Flags::decode_buffer_size()),
      counter_{
          .disconnect = create_counter("disconnect"),
      },
      profile_{
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
      latency_{
          .ping = create_latency("ping"),
          .heartbeat = create_latency("heartbeat"),
      } {
}

bool WebSocket::ready() const {
  return connection_.ready();
}

void WebSocket::close() {
  connection_.close();
}

void WebSocket::operator()(const Event<Start> &) {
  connection_.start();
}

void WebSocket::operator()(const Event<Stop> &) {
  connection_.stop();
}

void WebSocket::operator()(const Event<Timer> &event) {
  if (connection_.refresh(event.value.now) == false)
    return;
  if (Flags::ws_cancel_on_disconnect() && Flags::ws_cancel_all_after_secs() && ready_ &&
      next_cancel_all_after_ <= event.value.now) {
    next_cancel_all_after_ =
        event.value.now + std::chrono::seconds{Flags::ws_cancel_all_after_secs() / 4};
    send_cancel_all_after(std::chrono::seconds{Flags::ws_cancel_all_after_secs()});
  }
}

void WebSocket::subscribe(const std::string_view &topic) {
  auto message = fmt::format(
      R"({{)"
      R"("op":"subscribe",)"
      R"("args":"{}")"
      R"(}})",
      topic);
  connection_.send_text(message);
}

void WebSocket::subscribe(const std::string_view &topic, const std::vector<std::string> &filter) {
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
    connection_.send_text(message);
  }
}

void WebSocket::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::COUNTER)
      // profile
      .write(profile_.parse, metrics::PROFILE)
      .write(profile_.cancel_all_after, metrics::PROFILE)
      .write(profile_.error, metrics::PROFILE)
      .write(profile_.execution, metrics::PROFILE)
      .write(profile_.funding, metrics::PROFILE)
      .write(profile_.handshake, metrics::PROFILE)
      .write(profile_.instrument, metrics::PROFILE)
      .write(profile_.liquidation, metrics::PROFILE)
      .write(profile_.margin, metrics::PROFILE)
      .write(profile_.order, metrics::PROFILE)
      .write(profile_.order_book_l2, metrics::PROFILE)
      .write(profile_.position, metrics::PROFILE)
      .write(profile_.quote, metrics::PROFILE)
      .write(profile_.settlement, metrics::PROFILE)
      .write(profile_.subscribe, metrics::PROFILE)
      .write(profile_.trade, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY)
      .write(latency_.heartbeat, metrics::LATENCY);
}

void WebSocket::operator()(const core::web::Socket::Connected &) {
  // note! don't notify gateway: wait for ready
}

void WebSocket::operator()(const core::web::Socket::Disconnected &) {
  ready_ = false;
  next_cancel_all_after_ = {};
  handler_(*this);
}

void WebSocket::operator()(const core::web::Socket::Ready &) {
  // note! don't notify gateway: wait for handshake
}

void WebSocket::operator()(const core::web::Socket::Close &) {
}

void WebSocket::operator()(const core::web::Socket::Latency &latency) {
  server::TraceInfo trace_info;
  ExternalLatency external_latency{
      .name = CONNECTION,
      .latency = latency.sample,
  };
  handler_(external_latency, trace_info);
  latency_.ping.update(latency.sample);
}

void WebSocket::operator()(const core::web::Socket::Text &text) {
  parse(text.payload);
}

std::string WebSocket::create_upgrade_headers() {
  auto expires = std::chrono::duration_cast<std::chrono::seconds>(
      core::get_realtime_clock() + std::chrono::seconds{5});
  return random_.create_headers(expires, core::http::Method::GET, "/realtime", std::string_view());
}

void WebSocket::parse(const std::string_view &message) {
  VLOG(4)(R"(message={})", message);
  profile_.parse([&]() {
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
  core::json::Buffer buffer(decode_buffer_);
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
  connection_.send_text(message);
}

void WebSocket::operator()(const json::CancelAllAfter &cancel_all_after) {
  profile_.cancel_all_after([&]() { VLOG(1)(R"(cancel_all_after={})", cancel_all_after); });
}

void WebSocket::operator()(const json::Error &error) {
  profile_.error([&]() { LOG(WARNING)(R"(error={})", error); });
  connection_.close();
}

void WebSocket::operator()(const json::Handshake &handshake) {
  profile_.handshake([&]() {
    VLOG(1)(R"(handshake={})", handshake);
    LOG(INFO)("Ready");
    assert(ready_ == false);
    ready_ = true;
    handler_(*this);
    if (Flags::ws_cancel_on_disconnect() == false || Flags::ws_cancel_all_after_secs() == 0)
      send_cancel_all_after(std::chrono::seconds{});
  });
}

void WebSocket::operator()(const json::Subscribe &subscribe) {
  profile_.subscribe([&]() {
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
  profile_.execution([&]() {
    VLOG(1)(R"(action={}, execution={})", action, execution);
    handler_(action, execution, trace_info);
  });
}

void WebSocket::operator()(
    const json::Action action, const json::Funding &funding, const server::TraceInfo &) {
  profile_.funding([&]() {
    VLOG(2)(R"(action={}, funding={})", action, funding);
    // XXX not used
  });
}

void WebSocket::operator()(
    const json::Action action,
    const json::Instrument &instrument,
    const server::TraceInfo &trace_info) {
  profile_.instrument([&]() {
    VLOG(2)(R"(action={}, instrument={})", action, instrument);
    handler_(action, instrument, trace_info);
  });
}

void WebSocket::operator()(
    const json::Action action, const json::Liquidation &liquidation, const server::TraceInfo &) {
  profile_.liquidation([&]() {
    VLOG(2)(R"(action={}, liquidation={})", action, liquidation);
    /// XXX not used
  });
}

void WebSocket::operator()(
    const json::Action action, const json::Margin &margin, const server::TraceInfo &) {
  profile_.margin([&]() {
    VLOG(2)(R"(action={}, margin={})", action, margin);
    /// XXX not used
  });
}

void WebSocket::operator()(
    const json::Action action, const json::Order &order, const server::TraceInfo &trace_info) {
  profile_.order([&]() {
    VLOG(1)(R"(action={}, order={})", action, order);
    handler_(action, order, trace_info);
  });
}

void WebSocket::operator()(
    const json::Action action,
    const json::OrderBookL2 &order_book_l2,
    const server::TraceInfo &trace_info) {
  profile_.order_book_l2([&]() {
    VLOG(3)(R"(action={}, order_book_l2={})", action, order_book_l2);
    handler_(action, order_book_l2, trace_info);
  });
}

void WebSocket::operator()(
    const json::Action action,
    const json::Position &position,
    const server::TraceInfo &trace_info) {
  profile_.position([&]() {
    VLOG(2)(R"(action={}, position={})", action, position);
    handler_(action, position, trace_info);
  });
}

void WebSocket::operator()(
    const json::Action action, const json::Quote &quote, const server::TraceInfo &trace_info) {
  profile_.quote([&]() {
    VLOG(3)(R"(action={}, quote={})", action, quote);
    handler_(action, quote, trace_info);
  });
}

void WebSocket::operator()(
    const json::Action action,
    const json::Settlement &settlement,
    const server::TraceInfo &trace_info) {
  profile_.settlement([&]() {
    VLOG(3)(R"(action={}, settlement={})", action, settlement);
    handler_(action, settlement, trace_info);
  });
}

void WebSocket::operator()(
    const json::Action action, const json::Trade &trade, const server::TraceInfo &trace_info) {
  profile_.trade([&]() {
    VLOG(2)(R"(action={}, trade={})", action, trade);
    handler_(action, trade, trace_info);
  });
}

}  // namespace bitmex
}  // namespace roq
