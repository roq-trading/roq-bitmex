/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "roq/core/hash_map.h"

#include "roq/core/stack/buffer.h"

#include "roq/core/metrics/counter.h"
#include "roq/core/metrics/latency.h"
#include "roq/core/metrics/profile.h"

#include "roq/core/event/base.h"
#include "roq/core/event/dns_base.h"

#include "roq/core/net/tcp_ssl_connection_factory.h"
#include "roq/core/net/manager.h"

#include "roq/core/http/response.h"

#include "roq/core/ws/decoder.h"

#include "roq/bitmex/json/activate.h"
#include "roq/bitmex/json/change.h"
#include "roq/bitmex/json/done.h"
#include "roq/bitmex/json/error.h"
#include "roq/bitmex/json/heartbeat.h"
#include "roq/bitmex/json/last_match.h"
#include "roq/bitmex/json/l2update.h"
#include "roq/bitmex/json/match.h"
#include "roq/bitmex/json/open.h"
#include "roq/bitmex/json/received.h"
#include "roq/bitmex/json/snapshot.h"
#include "roq/bitmex/json/status.h"
#include "roq/bitmex/json/subscriptions.h"
#include "roq/bitmex/json/ticker.h"

#include "roq/bitmex/config.h"

namespace roq {
namespace bitmex {

class Gateway;

class WebSocket final
    : public core::net::Manager::Handler,
      public core::http::Response::Handler {
  enum class State {
    DISCONNECTED,
    UPGRADE_SENT,
    READY,
  };
  struct Product final {
    uint64_t last_sequence = 0;
    uint32_t match_last_trade_id = 0;
    uint32_t fill_last_trade_id = 0;
  };

 public:
  WebSocket(
      Gateway& gateway,
      const Config& config,
      core::event::Base& base,
      core::event::DNSBase& dns_base,
      core::ssl::Context& ssl_context);

  WebSocket(const WebSocket&) = delete;
  WebSocket(WebSocket&&) = delete;

  void operator=(const WebSocket&) = delete;
  void operator=(WebSocket&&) = delete;

  bool ready() const;

  void operator()(const StartEvent&);
  void operator()(const StopEvent&);
  void operator()(const TimerEvent&);

  void subscribe(const std::vector<std::string>& symbols);

  void operator()(Metrics& metrics);

 protected:
  void send(const core::utils::Message& message);

  void send_upgrade_request();
  void send_close();
  void send_ping();

  void operator()(State state);

  void operator()(const core::net::Manager::Connected&) override;
  void operator()(const core::net::Manager::Disconnected&) override;
  void operator()(const core::net::Manager::Read&) override;

  // http:
  void operator()(const core::http::Response::MessageBegin&) override;
  void operator()(const core::http::Response::URL&) override;
  void operator()(const core::http::Response::Status&) override;
  void operator()(const core::http::Response::HeaderField&) override;
  void operator()(const core::http::Response::HeaderValue&) override;
  void operator()(const core::http::Response::HeadersComplete&) override;
  void operator()(const core::http::Response::ChunkHeader&) override;
  void operator()(const core::http::Response::Body&) override;
  void operator()(const core::http::Response::ChunkComplete&) override;
  void operator()(const core::http::Response::MessageComplete&) override;

  // ws:
  void operator()(const core::ws::text_t&);
  void operator()(const core::ws::close_t&);
  void operator()(const core::ws::ping_t&);
  void operator()(const core::ws::pong_t&);

  void parse(const std::string_view& message);
  void parse_helper(const std::string_view& message);

  void operator()(const json::Error& error);
  void operator()(const json::Heartbeat& heartbeat);
  void operator()(const json::Subscriptions& subscriptions);
  void operator()(const json::Status& status);
  void operator()(const json::Received& received);
  void operator()(const json::Open& open);
  void operator()(const json::Match& match);
  void operator()(const json::Done& done);
  void operator()(const json::Change& change);
  void operator()(const json::Activate& activate);
  void operator()(const json::Ticker& ticker);
  void operator()(const json::Snapshot& snapshot);
  void operator()(const json::L2Update& l2update);
  void operator()(const json::LastMatch& last_match);

  Product& check(
      const std::string_view& product_id,
      uint64_t sequence);

 private:
  Gateway& _gateway;
  // config
  const std::string _access_key;
  const std::string _access_password;
  const std::string _access_secret;
  // connection
  core::net::TcpSslConnectionFactory _connection_factory;
  core::net::Manager _connection;
  // buffers
  core::utils::Buffer _encode_buffer;
  core::utils::Buffer _decode_buffer;
  // session
  State _state = State::DISCONNECTED;
  std::string _response_key;
  std::unique_ptr<core::http::Response> _response;
  std::chrono::nanoseconds _next_heartbeat = {};
  // other
  core::stack::Buffer<char, 32> _buffer;
  // metrics
  struct {
    core::metrics::Counter
      disconnect;
  } _counter;
  struct {
    core::metrics::Profile
      parse,
      error,
      heartbeat,
      subscriptions,
      status,
      received,
      open,
      match,
      done,
      change,
      activate,
      ticker,
      snapshot,
      l2update,
      last_match;
  } _profile;
  struct {
    core::metrics::Latency
      ping,
      heartbeat;
  } _latency;
  // upgrade
  core::http::Status _status = core::http::Status::UNKNOWN;
  core::http::Header _header = core::http::Header::UNKNOWN;
  bool _connection_upgrade = false;
  bool _upgrade_websocket = false;
  bool _sec_websocket_accept = false;
  // sequence (by product)
  core::hash_map<std::string, Product> _product;
};

}  // namespace bitmex
}  // namespace roq
