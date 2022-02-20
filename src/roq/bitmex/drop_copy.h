/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>

#include "roq/core/download.h"

#include "roq/core/metrics/counter.h"
#include "roq/core/metrics/latency.h"
#include "roq/core/metrics/profile.h"

#include "roq/core/io/context.h"

#include "roq/core/web/client_socket.h"

#include "roq/server.h"

#include "roq/bitmex/drop_copy_state.h"
#include "roq/bitmex/security.h"
#include "roq/bitmex/shared.h"

#include "roq/bitmex/json/stream_parser.h"

namespace roq {
namespace bitmex {

class DropCopy final : public core::web::ClientSocket::Handler, public json::StreamParser::Handler {
 public:
  struct Handler {
    virtual void operator()(const server::Trace<StreamStatus> &) = 0;
    virtual void operator()(const server::Trace<ExternalLatency> &) = 0;
    virtual void operator()(const server::Trace<TradeUpdate> &, bool is_last, uint8_t user_id) = 0;
    virtual void operator()(const server::Trace<PositionUpdate> &, bool is_last) = 0;
  };

  DropCopy(Handler &, core::io::Context &, uint16_t stream_id, Security &, Shared &);

  DropCopy(DropCopy &&) = delete;
  DropCopy(const DropCopy &) = delete;

  void operator()(const Event<Start> &);
  void operator()(const Event<Stop> &);
  void operator()(const Event<Timer> &);

  void operator()(metrics::Writer &);

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

  void send_cancel_all_after(std::chrono::nanoseconds timeout);

  void send_subscribe(const std::string_view &topic);
  void send_subscribe(const std::span<std::string_view> &topics);

  uint32_t download(DropCopyState);

  void subscribe();

  void parse(const std::string_view &message);
  void parse_helper(const std::string_view &message);

  void operator()(const server::Trace<json::CancelAllAfter> &) override;
  void operator()(const server::Trace<json::Error> &) override;
  void operator()(const server::Trace<json::Handshake> &) override;
  void operator()(const server::Trace<json::Subscribe> &) override;
  void operator()(const server::Trace<json::Unsubscribe> &) override;

  void operator()(const server::Trace<json::Execution> &, json::Action) override;
  void operator()(const server::Trace<json::Margin> &, json::Action) override;
  void operator()(const server::Trace<json::Order> &, json::Action) override;
  void operator()(const server::Trace<json::Position> &, json::Action) override;
  // ... unexpected
  void operator()(const server::Trace<json::Funding> &, json::Action) override;
  void operator()(const server::Trace<json::Instrument> &, json::Action) override;
  void operator()(const server::Trace<json::Liquidation> &, json::Action) override;
  void operator()(const server::Trace<json::OrderBookL2> &, json::Action) override;
  void operator()(const server::Trace<json::Quote> &, json::Action) override;
  void operator()(const server::Trace<json::Settlement> &, json::Action) override;
  void operator()(const server::Trace<json::Trade> &, json::Action) override;

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
    core::metrics::Profile parse, cancel_all_after, error, execution, handshake, margin, order,
        position, subscribe, unsubscribe;
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
  core::Download<DropCopyState> download_;
  struct {
    bool order = false;
    // XXX maybe everything else too?
  } partial_received_;
};

}  // namespace bitmex
}  // namespace roq
