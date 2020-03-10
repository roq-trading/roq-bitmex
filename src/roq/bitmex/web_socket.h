/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

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

#include "roq/bitmex/config.h"
#include "roq/bitmex/random.h"

#include "roq/bitmex/json/parser.h"

namespace roq {
namespace bitmex {

class Gateway;

class WebSocket final
    : public core::net::Manager::Handler,
      public core::http::Response::Handler,
      public json::Parser::Handler {
  enum class State {
    DISCONNECTED,
    UPGRADE_SENT,
    AWAIT_HANDSHAKE,
    READY,
  };

 public:
  WebSocket(
      Gateway& gateway,
      const Config& config,
      Random& random,
      core::event::Base& base,
      core::event::DNSBase& dns_base,
      core::ssl::Context& ssl_context);

  WebSocket(const WebSocket&) = delete;
  WebSocket(WebSocket&&) = delete;

  bool ready() const;

  void operator()(const StartEvent&);
  void operator()(const StopEvent&);
  void operator()(const TimerEvent&);

  void subscribe(const std::string_view& topic);

  void subscribe(
      const std::string_view& topic,
      const std::vector<std::string>& filter);

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

  void send_cancel_all_after();

  // json:
  void operator()(const json::CancelAllAfter&) override;
  void operator()(const json::Error&) override;
  void operator()(const json::Handshake&) override;
  void operator()(const json::Subscribe&) override;
  // table
  void operator()(const json::Action, const json::Execution&) override;
  void operator()(const json::Action, const json::Funding&) override;
  void operator()(const json::Action, const json::Instrument&) override;
  void operator()(const json::Action, const json::Liquidation&) override;
  void operator()(const json::Action, const json::Margin&) override;
  void operator()(const json::Action, const json::Order&) override;
  void operator()(const json::Action, const json::OrderBookL2&) override;
  void operator()(const json::Action, const json::Position&) override;
  void operator()(const json::Action, const json::Quote&) override;
  void operator()(const json::Action, const json::Settlement&) override;
  void operator()(const json::Action, const json::Trade&) override;

 private:
  Gateway& _gateway;
  // config
  // authentication
  Random& _random;
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
  std::chrono::nanoseconds _next_cancel_all_after = {};
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
      cancel_all_after,
      error,
      execution,
      funding,
      handshake,
      instrument,
      liquidation,
      margin,
      order,
      order_book_l2,
      position,
      quote,
      settlement,
      subscribe,
      trade;
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
};

}  // namespace bitmex
}  // namespace roq
