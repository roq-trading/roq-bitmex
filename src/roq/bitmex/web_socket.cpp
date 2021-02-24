/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/bitmex/web_socket.h"

#include "roq/core/patterns.h"

#include "roq/core/clock.h"

#include "roq/core/charconv.h"

#include "roq/bitmex/flags.h"

using namespace roq::literals;

namespace roq {
namespace bitmex {

namespace {
constexpr std::string_view CONNECTION = "ws"_sv;

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
    core::io::Context &context)
    : handler_(handler), random_(random),
      connection_(
          *this,
          context,
          core::URI(Flags::ws_uri()),
          std::string_view(),  // query
          std::chrono::seconds{Flags::ws_ping_freq_secs()},
          Flags::decode_buffer_size(),  // XXX need read buffer size
          Flags::encode_buffer_size(),
          [this]() { return create_upgrade_headers(); }),
      decode_buffer_(Flags::decode_buffer_size()),
      counter_{
          .disconnect = create_counter("disconnect"_sv),
      },
      profile_{
          .parse = create_profile("parse"_sv),
          .cancel_all_after = create_profile("cancel_all_after"_sv),
          .error = create_profile("error"_sv),
          .execution = create_profile("execution"_sv),
          .funding = create_profile("funding"_sv),
          .handshake = create_profile("handshake"_sv),
          .instrument = create_profile("instrument"_sv),
          .liquidation = create_profile("liquidation"_sv),
          .margin = create_profile("margin"_sv),
          .order = create_profile("order"_sv),
          .order_book_l2 = create_profile("order_book_l2"_sv),
          .position = create_profile("position"_sv),
          .quote = create_profile("quote"_sv),
          .settlement = create_profile("settlement"_sv),
          .subscribe = create_profile("subscribe"_sv),
          .trade = create_profile("trade"_sv),
      },
      latency_{
          .ping = create_latency("ping"_sv),
          .heartbeat = create_latency("heartbeat"_sv),
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
  auto message = roq::format(
      R"({{)"
      R"("op":"subscribe",)"
      R"("args":"{}")"
      R"(}})"_fmt,
      topic);
  connection_.send_text(message);
}

void WebSocket::subscribe(const std::string_view &topic, const std::vector<std::string> &filter) {
  if (filter.empty()) {
    subscribe(topic);
  } else {
    auto message = roq::format(
        R"({{)"
        R"("op":"subscribe",)"
        R"("args":"{}:{}")"
        R"(}})"_fmt,
        topic,
        roq::join(filter, ","_sv));
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
  return random_.create_headers(
      expires, core::http::Method::GET, "/realtime"_sv, std::string_view());
}

void WebSocket::parse(const std::string_view &message) {
  VLOG(4)(R"(message={})"_fmt, message);
  profile_.parse([&]() {
    try {
      parse_helper(message);
    } catch (std::exception &e) {
      LOG(WARNING)(R"(message="{}")"_fmt, message);
      LOG(FATAL)(R"(ERROR what="{}")"_fmt, e.what());
    }
  });
}

void WebSocket::parse_helper(const std::string_view &message) {
  server::TraceInfo trace_info;
  core::json::Buffer buffer(decode_buffer_);
  json::Parser::dispatch(*this, message, buffer, trace_info);
}

void WebSocket::send_cancel_all_after(std::chrono::seconds seconds) {
  auto message = roq::format(
      R"({{)"
      R"("op":"cancelAllAfter",)"
      R"("args":{})"
      R"(}})"_fmt,
      seconds.count() * 1000);  // milliseconds
  connection_.send_text(message);
}

void WebSocket::operator()(const json::CancelAllAfter &cancel_all_after) {
  profile_.cancel_all_after([&]() { VLOG(1)(R"(cancel_all_after={})"_fmt, cancel_all_after); });
}

void WebSocket::operator()(const json::Error &error) {
  profile_.error([&]() { LOG(WARNING)(R"(error={})"_fmt, error); });
  connection_.close();
}

void WebSocket::operator()(const json::Handshake &handshake) {
  profile_.handshake([&]() {
    VLOG(1)(R"(handshake={})"_fmt, handshake);
    LOG(INFO)("Ready"_sv);
    assert(ready_ == false);
    ready_ = true;
    handler_(*this);
    if (Flags::ws_cancel_on_disconnect() == false || Flags::ws_cancel_all_after_secs() == 0)
      send_cancel_all_after(std::chrono::seconds{});
  });
}

void WebSocket::operator()(const json::Subscribe &subscribe) {
  profile_.subscribe([&]() {
    VLOG(1)(R"(subscribe={})"_fmt, subscribe);
    if (subscribe.success) {
      assert(subscribe.failure == false);
      LOG(INFO)
      (R"(Successfully subscribed to topic="{}")"_fmt, subscribe.subscribe);
    } else if (subscribe.failure) {
      assert(subscribe.success == false);
      LOG(WARNING)(R"(Failed to subscribe topic="{}")"_fmt, subscribe.subscribe);
    } else {
      LOG(FATAL)("Expected success or failure"_sv);
    }
    // TODO(thraneh): clear timeout
  });
}

void WebSocket::operator()(
    const json::Action action,
    const json::Execution &execution,
    const server::TraceInfo &trace_info) {
  profile_.execution([&]() {
    VLOG(1)(R"(action={}, execution={})"_fmt, action, execution);
    handler_(action, execution, trace_info);
  });
}

void WebSocket::operator()(
    const json::Action action, const json::Funding &funding, const server::TraceInfo &) {
  profile_.funding([&]() {
    VLOG(2)(R"(action={}, funding={})"_fmt, action, funding);
    // XXX not used
  });
}

void WebSocket::operator()(
    const json::Action action,
    const json::Instrument &instrument,
    const server::TraceInfo &trace_info) {
  profile_.instrument([&]() {
    VLOG(2)(R"(action={}, instrument={})"_fmt, action, instrument);
    handler_(action, instrument, trace_info);
  });
}

void WebSocket::operator()(
    const json::Action action, const json::Liquidation &liquidation, const server::TraceInfo &) {
  profile_.liquidation([&]() {
    VLOG(2)(R"(action={}, liquidation={})"_fmt, action, liquidation);
    /// XXX not used
  });
}

void WebSocket::operator()(
    const json::Action action, const json::Margin &margin, const server::TraceInfo &) {
  profile_.margin([&]() {
    VLOG(2)(R"(action={}, margin={})"_fmt, action, margin);
    /// XXX not used
  });
}

void WebSocket::operator()(
    const json::Action action, const json::Order &order, const server::TraceInfo &trace_info) {
  profile_.order([&]() {
    VLOG(1)(R"(action={}, order={})"_fmt, action, order);
    handler_(action, order, trace_info);
  });
}

void WebSocket::operator()(
    const json::Action action,
    const json::OrderBookL2 &order_book_l2,
    const server::TraceInfo &trace_info) {
  profile_.order_book_l2([&]() {
    VLOG(3)(R"(action={}, order_book_l2={})"_fmt, action, order_book_l2);
    handler_(action, order_book_l2, trace_info);
  });
}

void WebSocket::operator()(
    const json::Action action,
    const json::Position &position,
    const server::TraceInfo &trace_info) {
  profile_.position([&]() {
    VLOG(2)(R"(action={}, position={})"_fmt, action, position);
    handler_(action, position, trace_info);
  });
}

void WebSocket::operator()(
    const json::Action action, const json::Quote &quote, const server::TraceInfo &trace_info) {
  profile_.quote([&]() {
    VLOG(3)(R"(action={}, quote={})"_fmt, action, quote);
    handler_(action, quote, trace_info);
  });
}

void WebSocket::operator()(
    const json::Action action,
    const json::Settlement &settlement,
    const server::TraceInfo &trace_info) {
  profile_.settlement([&]() {
    VLOG(3)(R"(action={}, settlement={})"_fmt, action, settlement);
    handler_(action, settlement, trace_info);
  });
}

void WebSocket::operator()(
    const json::Action action, const json::Trade &trade, const server::TraceInfo &trace_info) {
  profile_.trade([&]() {
    VLOG(2)(R"(action={}, trade={})"_fmt, action, trade);
    handler_(action, trade, trace_info);
  });
}

}  // namespace bitmex
}  // namespace roq
