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

class WebSocket final : public core::web::ClientSocket::Handler,
                        public json::StreamParser::Handler {
 public:
  struct Handler {
    virtual void operator()(const Trace<StreamStatus> &) = 0;
    virtual void operator()(const Trace<ExternalLatency> &) = 0;
    virtual void operator()(const Trace<TradeUpdate> &, bool is_last, uint8_t user_id) = 0;
    virtual void operator()(const Trace<PositionUpdate> &, bool is_last) = 0;
  };

  WebSocket(Handler &, core::io::Context &, uint16_t stream_id, Security &, Shared &);

  WebSocket(WebSocket &&) = delete;
  WebSocket(const WebSocket &) = delete;

  bool ready() const { return ready_; }

  void operator()(const Event<Start> &);
  void operator()(const Event<Stop> &);
  void operator()(const Event<Timer> &);

  void operator()(metrics::Writer &);

  uint16_t operator()(
      const Event<CreateOrder> &, const oms::Order &, const std::string_view &request_id);
  uint16_t operator()(
      const Event<ModifyOrder> &,
      const oms::Order &,
      const std::string_view &request_id,
      const std::string_view &previous_request_id);
  uint16_t operator()(
      const Event<CancelOrder> &,
      const oms::Order &,
      const std::string_view &request_id,
      const std::string_view &previous_request_id);

  uint16_t operator()(const Event<CancelAllOrders> &, const std::string_view &request_id);

 protected:
  void operator()(const core::web::ClientSocket::Connected &) override;
  void operator()(const core::web::ClientSocket::Disconnected &) override;
  void operator()(const core::web::ClientSocket::Ready &) override;
  void operator()(const core::web::ClientSocket::Close &) override;
  void operator()(const core::web::ClientSocket::Latency &) override;
  void operator()(const core::web::ClientSocket::Text &) override;
  void operator()(const core::web::ClientSocket::Binary &) override;

 private:
  void operator()(ConnectionStatus);

  void create_order(
      const Event<CreateOrder> &, const oms::Order &, const std::string_view &request_id);

  void modify_order(
      const Event<ModifyOrder> &,
      const oms::Order &,
      const std::string_view &request_id,
      const std::string_view &previous_request_id);

  void cancel_order(
      const Event<CancelOrder> &,
      const oms::Order &,
      const std::string_view &request_id,
      const std::string_view &previous_request_id);

  void cancel_all_orders(const Event<CancelAllOrders> &, const std::string_view &request_id);

  void send_cancel_all_after(std::chrono::nanoseconds timeout);

  uint32_t download(WebSocketState);

  void parse(const std::string_view &message);
  void parse_helper(const std::string_view &message);

  void operator()(const Trace<json::CancelAllAfter> &) override;
  void operator()(const Trace<json::Error> &) override;
  void operator()(const Trace<json::Handshake> &) override;
  void operator()(const Trace<json::Subscribe> &) override;
  void operator()(const Trace<json::Unsubscribe> &) override;

  void operator()(const Trace<json::Execution> &, json::Action) override;
  void operator()(const Trace<json::Margin> &, json::Action) override;
  void operator()(const Trace<json::Order> &, json::Action) override;
  void operator()(const Trace<json::Position> &, json::Action) override;
  // ... unexpected
  void operator()(const Trace<json::Funding> &, json::Action) override;
  void operator()(const Trace<json::Instrument> &, json::Action) override;
  void operator()(const Trace<json::Liquidation> &, json::Action) override;
  void operator()(const Trace<json::OrderBookL2> &, json::Action) override;
  void operator()(const Trace<json::Quote> &, json::Action) override;
  void operator()(const Trace<json::Settlement> &, json::Action) override;
  void operator()(const Trace<json::Trade> &, json::Action) override;

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
        create_order, modify_order, cancel_order, cancel_all_orders, cancel_all_after, error,
        execution, handshake, margin, order, position;
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
