/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/websocket.h"

#include <fmt/format.h>
// #include <fmt/chrono.h>

#include "roq/builtins.h"
#include "roq/patterns.h"

#include "roq/core/clock.h"

#include "roq/core/charconv.h"

#include "roq/core/http/response.h"

#include "roq/core/ws/decoder.h"
#include "roq/core/ws/encoder.h"
#include "roq/core/ws/random.h"
#include "roq/core/ws/upgrade.h"

#include "roq/bitmex/gateway.h"

#include "roq/bitmex/options.h"
#include "roq/bitmex/random.h"

namespace roq {
namespace bitmex {

constexpr std::string_view CONNECTION("ws");

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

WebSocket::WebSocket(
    Gateway& gateway,
    const Config& config,
    Random& random,
    core::event::Base& base,
    core::event::DNSBase& dns_base,
    core::ssl::Context& ssl_context)
    : _gateway(gateway),
      _random(random),
      _connection_factory(
          base,
          dns_base,
          ssl_context,
          FLAGS_ws_uri),
      _connection(
          *this,
          _connection_factory),
      _encode_buffer(FLAGS_encode_buffer_size),
      _decode_buffer(FLAGS_decode_buffer_size),
      _counter {
        .disconnect = create_counter("disconnect"),
      },
      _profile {
        .parse = create_profile("parse"),
        .instrument = create_profile("instrument"),
        .order_book_l2 = create_profile("order_book_l2"),
      },
      _latency {
        .ping = create_latency("ping"),
        .heartbeat = create_latency("heartbeat"),
      } {
}

bool WebSocket::ready() const {
  return _state == State::READY;
}

void WebSocket::operator()(const StartEvent&) {
  _connection.start();
}

void WebSocket::operator()(const StopEvent&) {
  _connection.stop();
}

void WebSocket::operator()(const TimerEvent& event) {
  auto now = event.now;
  switch (_state) {
    case State::READY: {
      if (_next_heartbeat <= now) {
        _next_heartbeat = now +
          std::chrono::seconds { FLAGS_ping_freq_secs };
        send_ping();
      }
      break;
    }
    default:
      _connection.refresh(now);
  }
}

void WebSocket::subscribe(const std::string_view& topic) {
  auto text = fmt::format(
      FMT_STRING(
        "{{"
        "\"op\":\"subscribe\","
        "\"args\":"
        "\"{}\""
        "}}"),
      topic);
  core::ws::Writer writer(_encode_buffer);
  core::ws::Encoder::text(
      writer,
      text);
  send(writer.finish());
}

void WebSocket::subscribe(
    const std::string_view& topic,
    const std::vector<std::string>& filter) {
  if (filter.empty()) {
    subscribe(topic);
  } else {
    auto text = fmt::format(
        FMT_STRING(
          "{{"
          "\"op\":\"subscribe\","
          "\"args\":"
          "\"{}:{}\""
          "}}"),
        topic,
        fmt::join(filter, ","));
    core::ws::Writer writer(_encode_buffer);
    core::ws::Encoder::text(
        writer,
        text);
    send(writer.finish());
  }
}

void WebSocket::operator()(Metrics& metrics) {
  metrics
    // counter
    .write(_counter.disconnect)
    // profile
    .write(_profile.parse)
    .write(_profile.instrument)
    .write(_profile.order_book_l2)
    // latency
    .write(_latency.ping)
    .write(_latency.heartbeat);
}

void WebSocket::send(const core::utils::Message& message) {
  _connection.send(message);
}

void WebSocket::send_upgrade_request() {
  LOG(INFO)("Sending upgrade request");
  auto key = core::ws::Random::create_sec_websocket_key();
  assert(_response_key.empty());
  _response_key = core::ws::Random::create_response(key);
  auto expires = std::chrono::duration_cast<std::chrono::seconds>(
      core::get_realtime_clock() + std::chrono::seconds {5});
  auto headers = _random.create_headers(
      expires,
      core::http::Method::GET,
      "/realtime");
  core::ws::Writer writer(_encode_buffer);
  core::ws::Upgrade::create(
      writer,
      core::URI(FLAGS_ws_uri),
      key,
      headers);
  send(writer.finish());
}

void WebSocket::send_close() {
  core::ws::Writer writer(_encode_buffer);
  core::ws::Encoder::close(
      writer,
      1000);
  send(writer.finish());
  // FIXME(thraneh): it is mandated to shutdown our end of the connection
}

void WebSocket::send_ping() {
  std::chrono::nanoseconds now = core::get_system_clock();
  _buffer.clear();
  core::charconv::to_string(
      std::back_inserter(_buffer),
      now.count());
  core::ws::Writer writer(_encode_buffer);
  core::ws::Encoder::ping(
      writer,
      _buffer.data(),
      _buffer.size());
  send(writer.finish());
}

void WebSocket::operator()(State state) {
  auto previous = ready();
  _state = state;
  if (ready() != previous) {
    if (previous)
      ++_counter.disconnect;
    _gateway(*this);
  }
}

void WebSocket::operator()(const core::net::Manager::Connected&) {
  assert(_state == State::DISCONNECTED);
  assert(static_cast<bool>(_response) == false);
  _response = std::make_unique<core::http::Response>(*this);
  send_upgrade_request();
  (*this)(State::UPGRADE_SENT);
}

void WebSocket::operator()(const core::net::Manager::Disconnected&) {
  _response_key.clear();
  _response.reset();
  (*this)(State::DISCONNECTED);
  ++_counter.disconnect;
}

void WebSocket::operator()(const core::net::Manager::Read& read) {
  auto length = read.buffer.length();
  if (length == 0)
    return;
  auto buffer = read.buffer.pullup(length);
  decltype(length) total = 0;
  for (;;) {
    size_t bytes = 0;
    switch (_state) {
      case State::DISCONNECTED:
        LOG(FATAL)("Unexpected");
        break;
      case State::UPGRADE_SENT:
        bytes = _response->dispatch(
            reinterpret_cast<const char *>(buffer),
            length);
        break;
      case State::AWAIT_HANDSHAKE:
      case State::READY:
        bytes = core::ws::Decoder::dispatch(
            overloaded {
              [](const core::ws::continuation_t&) {
                LOG(FATAL)("Unexpected");
              },
              [this](const core::ws::text_t& text) {
                (*this)(text);
              },
              [](const core::ws::binary_t&) {
                LOG(FATAL)("Unexpected");
              },
              [this](const core::ws::close_t& close) {
                (*this)(close);
              },
              [this](const core::ws::ping_t& ping) {
                (*this)(ping);
              },
              [this](const core::ws::pong_t& pong) {
                (*this)(pong);
              },
            },
            buffer,
            length);
        break;
      default:
        LOG(FATAL)("Unexpected");
    }
    assert(bytes <= length);
    if (bytes == 0)
      break;
    total += bytes;
    buffer += bytes;
    length -= bytes;
  }
  if (total)
    read.buffer.drain(total);
}

// http

void WebSocket::operator()(
    const core::http::Response::MessageBegin&) {
  assert(_status == core::http::Status::UNKNOWN);
  assert(_header == core::http::Header::UNKNOWN);
}

void WebSocket::operator()(
    const core::http::Response::URL&) {
  assert(false);  // only client
}

void WebSocket::operator()(
    const core::http::Response::Status& status) {
  assert(_header == core::http::Header::UNKNOWN);
  LOG(INFO)(
      FMT_STRING("HTTP response status={} text=\"{}\""),
      status.code,
      status.text);
  _status = core::http::parse_status(status.code);
  if (_status == core::http::Status::SWITCHING_PROTOCOLS) {
    VLOG(4)(
        FMT_STRING("status={} ({})"),
        status.code,
        _status);
  } else {
    // XXX what about redirect?
    throw std::runtime_error(
        fmt::format(
          FMT_STRING(
            "Expected status code 101 (Switching Protocols),"
            "got status code {} ({})"),
          status.code,
          _status));
  }
}

void WebSocket::operator()(
    const core::http::Response::HeaderField& header_field) {
  _header = core::http::parse_header(header_field.text);
}

void WebSocket::operator()(
    const core::http::Response::HeaderValue& header_value) {
  switch (_header) {
    case core::http::Header::CONNECTION: {
      VLOG(4)(
          FMT_STRING("{}=\"{}\""),
          _header,
          header_value.text);
      if (header_value.text.compare("upgrade") == 0) {
        _connection_upgrade = true;
      } else {
        LOG(WARNING)(
            FMT_STRING("Expected \"upgrade\", got \"{}\""),
            header_value.text);
      }
      break;
    }
    case core::http::Header::UPGRADE: {
      VLOG(4)(
          FMT_STRING("{}=\"{}\""),
          _header,
          header_value.text);
      if (header_value.text.compare("websocket") == 0) {
        _upgrade_websocket = true;
      } else {
        LOG(WARNING)(
            FMT_STRING("Expected \"websocket\", got \"{}\""),
            header_value.text);
      }
      break;
    }
    case core::http::Header::SEC_WEBSOCKET_ACCEPT: {
      VLOG(4)(
          FMT_STRING("{}=\"{}\""),
          _header, header_value.text);
      if (header_value.text.compare(_response_key) == 0) {
        _sec_websocket_accept = true;
      } else {
        LOG(WARNING)(
            FMT_STRING("Expected \"websocket\", got \"{}\""),
            header_value.text);
      }
      break;
    }
    default: {
    }
  }
  _header = core::http::Header::UNKNOWN;
}

void WebSocket::operator()(
    const core::http::Response::HeadersComplete&) {
  assert(_header == core::http::Header::UNKNOWN);
}

void WebSocket::operator()(
    const core::http::Response::ChunkHeader&) {
  assert(_header == core::http::Header::UNKNOWN);
  LOG(WARNING)("Unexpected [chunk header]");
}

void WebSocket::operator()(
    const core::http::Response::Body&) {
  assert(_header == core::http::Header::UNKNOWN);
  LOG(WARNING)("Unexpected [body]");
}

void WebSocket::operator()(
    const core::http::Response::ChunkComplete&) {
  assert(_header == core::http::Header::UNKNOWN);
  LOG(WARNING)("Unexpected [chunk complete]");
}

void WebSocket::operator()(
    const core::http::Response::MessageComplete&) {
  assert(_header == core::http::Header::UNKNOWN);
  _status = core::http::Status::UNKNOWN;
  if (_connection_upgrade && _upgrade_websocket && _sec_websocket_accept) {
    LOG(INFO)("Upgraded");
    (*this)(State::AWAIT_HANDSHAKE);
  } else {
    throw std::runtime_error("Connection has not been correctly upgraded to websocket");
  }
}

// ws

void WebSocket::operator()(const core::ws::text_t& text) {
  LOG_IF(WARNING, text.last == false)("message is fragmented");
  parse(text.payload);
}

void WebSocket::operator()(const core::ws::close_t& close) {
  LOG(WARNING)(
      FMT_STRING("close reason={}"),
      close.reason);
}

void WebSocket::operator()(const core::ws::ping_t& ping) {
  VLOG(1)(
      FMT_STRING("ping(length={})"),
      ping.length);
  core::ws::Writer writer(_encode_buffer);
  core::ws::Encoder::pong(
      writer,
      ping.payload,
      ping.length);
  send(writer.finish());
}

void WebSocket::operator()(const core::ws::pong_t& pong) {
  auto now = core::get_system_clock();
  VLOG(3)(
      FMT_STRING("pong(length={})"),
      pong.length);
  if (pong.length) {
    std::string_view text(
        reinterpret_cast<const char *>(pong.payload),
        pong.length);
    auto send_time = core::from_chars<uint64_t>(text);
    auto latency =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          now - decltype(now){send_time}) / 2;  // 1-way
    _latency.ping.update(latency.count());
  }
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

void WebSocket::operator()(const json::Error& error) {
  LOG(FATAL)(
      FMT_STRING("error={}"),
      error);
}

void WebSocket::operator()(const json::Execution& execution) {
  VLOG(1)(
      FMT_STRING("execution={}"),
      execution);
}

void WebSocket::operator()(const json::Funding& funding) {
  VLOG(1)(
      FMT_STRING("funding={}"),
      funding);
}

void WebSocket::operator()(const json::Handshake& handshake) {
  VLOG(1)(
      FMT_STRING("handshake={}"),
      handshake);
  (*this)(State::READY);
  _gateway(*this);
}

void WebSocket::operator()(const json::Instrument& instrument) {
  VLOG(1)(
      FMT_STRING("instrument={}"),
      instrument);
  _gateway(instrument);
}

void WebSocket::operator()(const json::Liquidation& liquidation) {
  VLOG(1)(
      FMT_STRING("liquidation={}"),
      liquidation);
}

void WebSocket::operator()(const json::Margin& margin) {
  VLOG(1)(
      FMT_STRING("margin={}"),
      margin);
}

void WebSocket::operator()(const json::Order& order) {
  VLOG(1)(
      FMT_STRING("order={}"),
      order);
  _gateway(order);
}

void WebSocket::operator()(const json::OrderBookL2& order_book_l2) {
  VLOG(1)(
      FMT_STRING("order_book_l2={}"),
      order_book_l2);
  _gateway(order_book_l2);
}

void WebSocket::operator()(const json::Position& position) {
  VLOG(1)(
      FMT_STRING("position={}"),
      position);
  _gateway(position);
}

void WebSocket::operator()(const json::Quote& quote) {
  VLOG(1)(
      FMT_STRING("quote={}"),
      quote);
  _gateway(quote);
}

void WebSocket::operator()(const json::Settlement& settlement) {
  VLOG(1)(
      FMT_STRING("settlement={}"),
      settlement);
  _gateway(settlement);
}

void WebSocket::operator()(const json::Trade& trade) {
  VLOG(1)(
      FMT_STRING("trade={}"),
      trade);
  _gateway(trade);
}

void WebSocket::operator()(const json::Subscribe& subscribe) {
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
}

}  // namespace bitmex
}  // namespace roq
