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

#define PREFIX "[WS] "

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
    core::event::Base& base,
    core::event::DNSBase& dns_base,
    core::ssl::Context& ssl_context)
    : _gateway(gateway),
      _access_key(config.get_api_key()),
      _access_secret(config.get_secret()),
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

void WebSocket::subscribe(const std::vector<std::string>& symbols) {
  /*
  // *must* be seconds (see bitmex-pro api documentation)
  auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
      core::get_realtime_clock());
  auto signature = Random::create_signature(
      timestamp,
      core::http::Method::GET,
      "/users/self/verify",
      _access_secret);
  auto text = fmt::format(
      "{{"
      "\"type\":\"subscribe\","
      "\"product_ids\":[\"{}\"],"
      "\"channels\":["
      "\"full\","
      "\"heartbeat\","
      "\"level2\","
      "\"matches\","
      "\"status\","
      "\"ticker\","
      "\"user\""
      "],"
      "\"key\":\"{}\","
      "\"signature\":\"{}\","
      "\"timestamp\":{},"
      "\"passphrase\":\"{}\""
      "}}",
      fmt::join(symbols, "\", \""),
      _access_key,
      signature,
      timestamp.count(),
      _access_password);
  core::ws::Writer writer(_encode_buffer);
  core::ws::Encoder::text(
      writer,
      text);
  send(writer.finish());
  */
  VLOG(1)("DEBUG: subscribe");
  auto text = fmt::format(
      "{{"
      "\"op\":\"subscribe\","
      "\"args\":["
      "\"instrument\","
      "\"orderBookL2\""
      //"\"orderBookL2:XBTUSD\""
      "]"
      "}}");
  core::ws::Writer writer(_encode_buffer);
  core::ws::Encoder::text(
      writer,
      text);
  send(writer.finish());
}

void WebSocket::operator()(Metrics& metrics) {
  metrics
    // counter
    .write(_counter.disconnect)
    // profile
    .write(_profile.parse)
    // latency
    .write(_latency.ping)
    .write(_latency.heartbeat);
}

void WebSocket::send(const core::utils::Message& message) {
  _connection.send(message);
}

void WebSocket::send_upgrade_request() {
  LOG(INFO)(PREFIX
      "Sending upgrade request");
  auto key = core::ws::Random::create_sec_websocket_key();
  assert(_response_key.empty());
  _response_key = core::ws::Random::create_response(key);
  core::ws::Writer writer(_encode_buffer);
  core::ws::Upgrade::create(
      writer,
      core::URI(FLAGS_ws_uri),
      key);
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
  if (ready() != previous)
    _gateway(*this);
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
        LOG(FATAL)(PREFIX
            "Unexpected");
        break;
      case State::UPGRADE_SENT:
        bytes = _response->dispatch(
            reinterpret_cast<const char *>(buffer),
            length);
        break;
      case State::READY:
        bytes = core::ws::Decoder::dispatch(
            overloaded {
              [](const core::ws::continuation_t&) {
                LOG(FATAL)(PREFIX
                    "Unexpected");
              },
              [this](const core::ws::text_t& text) {
                (*this)(text);
              },
              [](const core::ws::binary_t&) {
                LOG(FATAL)(PREFIX
                    "Unexpected");
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
        LOG(FATAL)(PREFIX
            "Unexpected");
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
  _status = core::http::parse_status(status.code);
  if (_status == core::http::Status::SWITCHING_PROTOCOLS) {
    VLOG(4)(PREFIX
        "status={} ({})", status.code, _status);
  } else {
    // XXX what about redirect?
    throw std::runtime_error(
        fmt::format(
          "Expected status code 101 (Switching Protocols),"
          "got status code {} ({})",
          status.code, _status));
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
      VLOG(4)(PREFIX
          "{}=\"{}\"", _header, header_value.text);
      if (header_value.text.compare("upgrade") == 0) {
        _connection_upgrade = true;
      } else {
        LOG(WARNING)(PREFIX
            "Expected \"upgrade\", got \"{}\"", header_value.text);
      }
      break;
    }
    case core::http::Header::UPGRADE: {
      VLOG(4)(PREFIX
          "{}=\"{}\"", _header, header_value.text);
      if (header_value.text.compare("websocket") == 0) {
        _upgrade_websocket = true;
      } else {
        LOG(WARNING)(PREFIX
            "Expected \"websocket\", got \"{}\"", header_value.text);
      }
      break;
    }
    case core::http::Header::SEC_WEBSOCKET_ACCEPT: {
      VLOG(4)(PREFIX
          "{}=\"{}\"", _header, header_value.text);
      if (header_value.text.compare(_response_key) == 0) {
        _sec_websocket_accept = true;
      } else {
        LOG(WARNING)(PREFIX
            "Expected \"websocket\", got \"{}\"", header_value.text);
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
  LOG(WARNING)(PREFIX
      "Unexpected [chunk header]");
}

void WebSocket::operator()(
    const core::http::Response::Body&) {
  assert(_header == core::http::Header::UNKNOWN);
  LOG(WARNING)(PREFIX
      "Unexpected [body]");
}

void WebSocket::operator()(
    const core::http::Response::ChunkComplete&) {
  assert(_header == core::http::Header::UNKNOWN);
  LOG(WARNING)(PREFIX
      "Unexpected [chunk complete]");
}

void WebSocket::operator()(
    const core::http::Response::MessageComplete&) {
  assert(_header == core::http::Header::UNKNOWN);
  _status = core::http::Status::UNKNOWN;
  if (_connection_upgrade && _upgrade_websocket && _sec_websocket_accept) {
    LOG(INFO)(PREFIX "Upgraded");
    (*this)(State::READY);
  } else {
    throw std::runtime_error("Connection has not been correctly upgraded to websocket");
  }
}

// ws

void WebSocket::operator()(const core::ws::text_t& text) {
  LOG_IF(WARNING, text.last == false)(PREFIX
      "message is fragmented");
  parse(text.payload);
}

void WebSocket::operator()(const core::ws::close_t& close) {
  LOG(WARNING)(PREFIX
      "close reason={}", close.reason);
}

void WebSocket::operator()(const core::ws::ping_t& ping) {
  VLOG(1)(PREFIX
      "ping(length={})", ping.length);
  core::ws::Writer writer(_encode_buffer);
  core::ws::Encoder::pong(
      writer,
      ping.payload,
      ping.length);
  send(writer.finish());
}

void WebSocket::operator()(const core::ws::pong_t& pong) {
  auto now = core::get_system_clock();
  VLOG(3)(PREFIX
      "pong(length={})", pong.length);
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
  VLOG(1)("DEBUG: message={}", message);
  _profile.parse(
      [&]() {
        try {
          parse_helper(message);
        } catch (std::exception& e) {
          LOG(FATAL)("ERROR what=\"{}\"", e.what());
        }
      });
}

// welcome:
//   message={"info":string,"version":string(timestamp),"timestamp":timstamp,"docs":string,"limit":{"remaining":int}}
// one for each subscribe argument:
//   message={"success":bool,"subscribe":string,"request":{"op":string,"args":[...]}}
// snapshot:
//   message={"table":string,"action":"partial","keys":[...],"types":{...},"foreignKeys":{...},"attributes":{...},"filter":{...},"data":[{...},...]}
// update/insert/delete:
//   message={"table":string,"action":"update","data":[{...},...]}
// PATTERNS:
// "info": (initial handshake)
// "success": OR "subscribe": (subscribe response)
// "failure": OR "subscribe": (subscribe response)
// "table": (subscription snapshot or update)
// ... not verified ...
// "now": OR "cancelTime": (cancelAllAfter response)
// "status": OR "error": (rate limiter)
// "error": (malformed request or not accessible)
// shared:
//   action
//   + attributes
//   * data
//   docs
//   failure
//   + filter
//   + foreign_keys
//   + keys
//   * request
//   subscribe
//   success
//   table
//   timestamp
//   version

void WebSocket::parse_helper(const std::string_view& message) {
  LOG(INFO)("DEBUG: {}", message);
  core::json::Buffer buffer(_decode_buffer);
  json::Parser::dispatch(
      *this,
      message,
      buffer);
}

void WebSocket::operator()(const json::Instruments& instruments) {
  LOG(INFO)("DEBUG: instruments={}", instruments);
}

void WebSocket::operator()(const json::MarketByPrice& market_by_price) {
  LOG(INFO)("DEBUG: market_by_price={}", market_by_price);
}

}  // namespace bitmex
}  // namespace roq
