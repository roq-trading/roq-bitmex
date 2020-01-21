/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/websocket.h"

#include <fmt/format.h>
// #include <fmt/chrono.h>

#include "roq/builtins.h"

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

#include "roq/bitmex/json/parser.h"

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
      _access_password(config.get_passphrase()),
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
        .error = create_profile("error"),
        .heartbeat = create_profile("heartbeat"),
        .subscriptions = create_profile("subscriptions"),
        .status = create_profile("status"),
        .received = create_profile("received"),
        .open = create_profile("open"),
        .match = create_profile("match"),
        .done = create_profile("done"),
        .change = create_profile("change"),
        .activate = create_profile("activate"),
        .ticker = create_profile("ticker"),
        .snapshot = create_profile("snapshot"),
        .l2update = create_profile("l2update"),
        .last_match = create_profile("last_match"),
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
}

void WebSocket::operator()(Metrics& metrics) {
  metrics
    // counter
    .write(_counter.disconnect)
    // profile
    .write(_profile.parse)
    .write(_profile.error)
    .write(_profile.heartbeat)
    .write(_profile.subscriptions)
    .write(_profile.status)
    .write(_profile.received)
    .write(_profile.open)
    .write(_profile.match)
    .write(_profile.done)
    .write(_profile.change)
    .write(_profile.activate)
    .write(_profile.ticker)
    .write(_profile.snapshot)
    .write(_profile.l2update)
    .write(_profile.last_match)
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
  for (auto& iter : _product)
    iter.second = Product {};
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
  _profile.parse(
      [&]() {
        try {
          parse_helper(message);
        } catch (std::exception& e) {
          LOG(FATAL)("ERROR what=\"{}\"", e.what());
        }
      });
}

void WebSocket::parse_helper(const std::string_view& message) {
  auto type = json::Parser::find_type(message);
  switch (json::Parser::parse_type(type)) {
    case json::Parser::Type::UNKNOWN: {
      throw std::runtime_error(
          fmt::format("Unknown message type=\"{}\"", type));
      break;
    }
    case json::Parser::Type::ERROR: {
      _profile.error(
          [&]() {
            (*this)(json::Error::parse(message));
          });
      break;
    }
    case json::Parser::Type::HEARTBEAT: {
      _profile.heartbeat(
          [&]() {
            (*this)(json::Heartbeat::parse(message));
          });
      break;
    }
    case json::Parser::Type::SUBSCRIPTIONS: {
      _profile.subscriptions(
          [&]() {
            core::json::Buffer buffer(_decode_buffer);
            (*this)(json::Subscriptions::parse(message, buffer));
          });
      break;
    }
    case json::Parser::Type::STATUS: {
      _profile.status(
          [&]() {
            core::json::Buffer buffer(_decode_buffer);
            (*this)(json::Status::parse(message, buffer));
          });
      break;
    }
    case json::Parser::Type::RECEIVED: {
      _profile.received(
          [&]() {
            (*this)(json::Received::parse(message));
          });
      break;
    }
    case json::Parser::Type::OPEN: {
      _profile.open(
          [&]() {
            (*this)(json::Open::parse(message));
          });
      break;
    }
    case json::Parser::Type::MATCH: {
      _profile.match(
          [&]() {
            (*this)(json::Match::parse(message));
          });
      break;
    }
    case json::Parser::Type::DONE: {
      _profile.done(
          [&]() {
            (*this)(json::Done::parse(message));
          });
      break;
    }
    case json::Parser::Type::CHANGE: {
      _profile.change(
          [&]() {
            (*this)(json::Change::parse(message));
          });
      break;
    }
    case json::Parser::Type::ACTIVATE: {
      _profile.activate(
          [&]() {
            (*this)(json::Activate::parse(message));
          });
      break;
    }
    case json::Parser::Type::TICKER: {
      _profile.ticker(
          [&]() {
            (*this)(json::Ticker::parse(message));
          });
      break;
    }
    case json::Parser::Type::SNAPSHOT: {
      _profile.snapshot(
          [&]() {
            core::json::Buffer buffer(_decode_buffer);
            (*this)(json::Snapshot::parse(message, buffer));
          });
      break;
    }
    case json::Parser::Type::L2UPDATE: {
      _profile.l2update(
          [&]() {
            core::json::Buffer buffer(_decode_buffer);
            (*this)(json::L2Update::parse(message, buffer));
          });
      break;
    }
    case json::Parser::Type::LAST_MATCH: {
      _profile.last_match(
          [&]() {
            (*this)(json::LastMatch::parse(message));
          });
      break;
    }
  }
}

void WebSocket::operator()(const json::Error& error) {
  LOG(WARNING)(PREFIX "error={}", error);
  _gateway(error);
}

void WebSocket::operator()(const json::Heartbeat& heartbeat) {
  auto now = core::get_realtime_clock();  // before logging
  VLOG(1)(PREFIX "heartbeat={}", heartbeat);
  auto latency =
    std::chrono::duration_cast<std::chrono::nanoseconds>(
        now - heartbeat.time);
  _latency.heartbeat.update(latency.count());
  check(
      heartbeat.product_id,
      heartbeat.sequence);
  _gateway(heartbeat);
}

void WebSocket::operator()(const json::Subscriptions& subscriptions) {
  VLOG(1)(PREFIX "subscriptions={}", subscriptions);
  _gateway(subscriptions);
}

void WebSocket::operator()(const json::Status& status) {
  VLOG(1)(PREFIX "status={}", status);
  _gateway(status);
}

void WebSocket::operator()(const json::Received& received) {
  VLOG(1)(PREFIX "received={}", received);
  check(
      received.product_id,
      received.sequence);
  _gateway(received);
}

void WebSocket::operator()(const json::Open& open) {
  VLOG(1)(PREFIX "open={}", open);
  check(
      open.product_id,
      open.sequence);
  _gateway(open);
}

void WebSocket::operator()(const json::Match& match) {
  VLOG(1)(PREFIX "match={}", match);
  auto& product = check(
      match.product_id,
      match.sequence);
  auto trade_summary = product.match_last_trade_id != match.trade_id;
  product.match_last_trade_id = match.trade_id;
  auto trade_update =
    product.fill_last_trade_id != match.trade_id &&
    (match.maker_user_id.empty() == false ||
    match.taker_user_id.empty() == false);
  if (trade_update)
    product.fill_last_trade_id = match.trade_id;
  _gateway(
      match,
      trade_summary,
      trade_update);
}

void WebSocket::operator()(const json::Done& done) {
  VLOG(1)(PREFIX "done={}", done);
  check(
      done.product_id,
      done.sequence);
  _gateway(done);
}

void WebSocket::operator()(const json::Change& change) {
  VLOG(1)(PREFIX "change={}", change);
  check(
      change.product_id,
      change.sequence);
  _gateway(change);
}

void WebSocket::operator()(const json::Activate& activate) {
  VLOG(1)(PREFIX "activate={}", activate);
  _gateway(activate);
}

void WebSocket::operator()(const json::Ticker& ticker) {
  VLOG(1)(PREFIX "ticker={}", ticker);
  check(
      ticker.product_id,
      ticker.sequence);
  _gateway(ticker);
}

void WebSocket::operator()(const json::Snapshot& snapshot) {
  VLOG(1)(PREFIX "snapshot={}", snapshot);
  _gateway(snapshot);
}

void WebSocket::operator()(const json::L2Update& l2update) {
  VLOG(1)(PREFIX "l2update={}", l2update);
  _gateway(l2update);
}

void WebSocket::operator()(const json::LastMatch& last_match) {
  VLOG(1)(PREFIX "last_match={}", last_match);
  check(
      last_match.product_id,
      last_match.sequence);
  _gateway(last_match);
}

WebSocket::Product& WebSocket::check(
    const std::string_view& product_id,
    uint64_t sequence) {
  auto iter = _product.find(product_id);
  if (unlikely(iter == _product.end())) {
    iter = _product.emplace(
        std::string(product_id),
        Product {}).first;
  }
  auto& item = (*iter).second;
  auto& previous = item.last_sequence;
  auto expected = previous + 1;
  if ((previous && sequence == expected) || (previous == 0)) {
    // all good
  } else if (sequence < previous) {
    LOG(WARNING)(PREFIX
        "*** SEQUENCE REPLAY *** "
        "product_id=\"{}\" current={} previous={} distance={}",
        product_id,
        sequence,
        previous,
        previous - sequence);
  } else if (sequence > expected) {
    LOG(WARNING)(PREFIX
        "*** SEQUENCE GAP *** "
        "product_id=\"{}\" current={} previous={} distance={}",
        product_id,
        sequence,
        previous,
        sequence - previous);
  } else {
    assert(sequence == previous);
  }
  previous = sequence;
  return item;
}

}  // namespace bitmex
}  // namespace roq
