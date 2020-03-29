/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "roq/core/metrics/counter.h"
#include "roq/core/metrics/latency.h"
#include "roq/core/metrics/profile.h"

#include "roq/core/event/base.h"
#include "roq/core/event/dns_base.h"

#include "roq/core/web/socket.h"

#include "roq/bitmex/config.h"
#include "roq/bitmex/random.h"

#include "roq/bitmex/json/parser.h"

namespace roq {
namespace bitmex {

class Gateway;

class WebSocket final
    : public core::web::Socket::Handler,
      public json::Parser::Handler {
 public:
  WebSocket(
      Gateway& gateway,
      const Config& config,
      Random& random,
      core::event::Base& base,
      core::event::DNSBase& dns_base,
      core::ssl::Context& ssl_context);

  WebSocket(WebSocket&&) = delete;
  WebSocket(const WebSocket&) = delete;

  bool ready() const;

  void close();

  void operator()(const StartEvent&);
  void operator()(const StopEvent&);
  void operator()(const TimerEvent&);

  void subscribe(const std::string_view& topic);

  void subscribe(
      const std::string_view& topic,
      const std::vector<std::string>& filter);

  void operator()(Metrics& metrics);

 protected:
  std::string create_upgrade_headers();

  void operator()(const core::web::Socket::Connected&) override;
  void operator()(const core::web::Socket::Disconnected&) override;
  void operator()(const core::web::Socket::Ready&) override;
  void operator()(const core::web::Socket::Close&) override;
  void operator()(const core::web::Socket::Latency&) override;
  void operator()(const core::web::Socket::Text&) override;

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
  // authentication
  Random& _random;
  // connection
  core::web::Socket _connection;
  // buffers
  core::utils::Buffer _decode_buffer;
  // session
  std::chrono::nanoseconds _next_cancel_all_after = {};
  bool _received_handshake = false;
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
};

}  // namespace bitmex
}  // namespace roq
