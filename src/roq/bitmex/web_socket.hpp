/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>

#include "roq/core/download.hpp"

#include "roq/core/metrics/counter.hpp"
#include "roq/core/metrics/latency.hpp"
#include "roq/core/metrics/profile.hpp"

#include "roq/core/io/context.hpp"

#include "roq/core/web/client_socket.hpp"

#include "roq/server.hpp"

#include "roq/bitmex/security.hpp"
#include "roq/bitmex/shared.hpp"
#include "roq/bitmex/web_socket_state.hpp"

#include "roq/bitmex/json/stream_parser.hpp"

namespace roq {
namespace bitmex {

class WebSocket final : public core::web::ClientSocket::Handler, public json::StreamParser::Handler {
 public:
  struct Handler {
    virtual void operator()(Trace<StreamStatus const> const &) = 0;
    virtual void operator()(Trace<ExternalLatency const> const &) = 0;
    virtual void operator()(Trace<TradeUpdate const> const &, bool is_last, uint8_t user_id) = 0;
    virtual void operator()(Trace<PositionUpdate const> const &, bool is_last) = 0;
  };

  WebSocket(Handler &, core::io::Context &, uint16_t stream_id, Security &, Shared &);

  WebSocket(WebSocket &&) = delete;
  WebSocket(WebSocket const &) = delete;

  bool ready() const { return ready_; }

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &);

  uint16_t operator()(Event<CreateOrder> const &, oms::Order const &, std::string_view const &request_id);
  uint16_t operator()(
      Event<ModifyOrder> const &,
      oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);
  uint16_t operator()(
      Event<CancelOrder> const &,
      oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);

  uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id);

 protected:
  void operator()(core::web::ClientSocket::Connected const &) override;
  void operator()(core::web::ClientSocket::Disconnected const &) override;
  void operator()(core::web::ClientSocket::Ready const &) override;
  void operator()(core::web::ClientSocket::Close const &) override;
  void operator()(core::web::ClientSocket::Latency const &) override;
  void operator()(core::web::ClientSocket::Text const &) override;
  void operator()(core::web::ClientSocket::Binary const &) override;

 private:
  void operator()(ConnectionStatus);

  void create_order(Event<CreateOrder> const &, oms::Order const &, std::string_view const &request_id);

  void modify_order(
      Event<ModifyOrder> const &,
      oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);

  void cancel_order(
      Event<CancelOrder> const &,
      oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);

  void cancel_all_orders(Event<CancelAllOrders> const &, std::string_view const &request_id);

  void send_cancel_all_after(std::chrono::nanoseconds timeout);

  uint32_t download(WebSocketState);

  void parse(std::string_view const &message);
  void parse_helper(std::string_view const &message);

  void operator()(Trace<json::CancelAllAfter const> const &) override;
  void operator()(Trace<json::Error const> const &) override;
  void operator()(Trace<json::Handshake const> const &) override;
  void operator()(Trace<json::Subscribe const> const &) override;
  void operator()(Trace<json::Unsubscribe const> const &) override;

  void operator()(Trace<json::Execution const> const &, json::Action) override;
  void operator()(Trace<json::Margin const> const &, json::Action) override;
  void operator()(Trace<json::Order const> const &, json::Action) override;
  void operator()(Trace<json::Position const> const &, json::Action) override;
  // ... unexpected
  void operator()(Trace<json::Funding const> const &, json::Action) override;
  void operator()(Trace<json::Instrument const> const &, json::Action) override;
  void operator()(Trace<json::Liquidation const> const &, json::Action) override;
  void operator()(Trace<json::OrderBookL2 const> const &, json::Action) override;
  void operator()(Trace<json::Quote const> const &, json::Action) override;
  void operator()(Trace<json::Settlement const> const &, json::Action) override;
  void operator()(Trace<json::Trade const> const &, json::Action) override;

  // utilities

  std::string create_upgrade_headers();

 private:
  Handler &handler_;
  // config
  const uint16_t stream_id_;
  const std::string name_;
  // connection
  core::web::ClientSocket connection_;
  // buffers
  core::Buffer decode_buffer_;
  // metrics
  struct {
    core::metrics::Counter disconnect;
  } counter_;
  struct {
    core::metrics::Profile parse,  //
        create_order, modify_order, cancel_order, cancel_all_orders, cancel_all_after, error, execution, handshake,
        margin, order, position;
  } profile_;
  struct {
    core::metrics::Latency ping, heartbeat;
  } latency_;
  // security
  Security &security_;
  // cache
  Shared &shared_;
  // state
  bool ready_ = false;
  std::chrono::nanoseconds next_cancel_all_after_ = {};
  ConnectionStatus status_ = {};
  core::Download<WebSocketState> download_;
  struct {
    bool order = false;
    // XXX maybe everything else too?
  } partial_received_;
};

}  // namespace bitmex
}  // namespace roq
