/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>

#include "roq/core/metrics/counter.h"
#include "roq/core/metrics/latency.h"
#include "roq/core/metrics/profile.h"

#include "roq/core/io/context.h"

#include "roq/core/web/socket.h"

#include "roq/download.h"
#include "roq/server.h"

#include "roq/bitmex/drop_copy_state.h"
#include "roq/bitmex/security.h"
#include "roq/bitmex/shared.h"

#include "roq/bitmex/json/parser.h"

namespace roq {
namespace bitmex {

class DropCopy final : public core::web::Socket::Handler, public json::Parser::Handler {
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
  void operator()(const core::web::Socket::Connected &) override;
  void operator()(const core::web::Socket::Disconnected &) override;
  void operator()(const core::web::Socket::Ready &) override;
  void operator()(const core::web::Socket::Close &) override;
  void operator()(const core::web::Socket::Latency &) override;
  void operator()(const core::web::Socket::Text &) override;

 private:
  void operator()(ConnectionStatus);

  void send_cancel_all_after(std::chrono::seconds seconds);

  void send_subscribe(const std::string_view &topic);
  void send_subscribe(const roq::span<std::string_view> &topics);

  uint32_t download(DropCopyState);

  void subscribe();

  void parse(const std::string_view &message);
  void parse_helper(const std::string_view &message);

  void operator()(const json::CancelAllAfter &) override;
  void operator()(const json::Error &) override;
  void operator()(const json::Handshake &) override;
  void operator()(const json::Subscribe &) override;

  void operator()(const json::Action, const json::Execution &, const server::TraceInfo &) override;
  void operator()(const json::Action, const json::Margin &, const server::TraceInfo &) override;
  void operator()(const json::Action, const json::Order &, const server::TraceInfo &) override;
  void operator()(const json::Action, const json::Position &, const server::TraceInfo &) override;
  // ... unexpected
  void operator()(const json::Action, const json::Funding &, const server::TraceInfo &) override;
  void operator()(const json::Action, const json::Instrument &, const server::TraceInfo &) override;
  void operator()(
      const json::Action, const json::Liquidation &, const server::TraceInfo &) override;
  void operator()(
      const json::Action, const json::OrderBookL2 &, const server::TraceInfo &) override;
  void operator()(const json::Action, const json::Quote &, const server::TraceInfo &) override;
  void operator()(const json::Action, const json::Settlement &, const server::TraceInfo &) override;
  void operator()(const json::Action, const json::Trade &, const server::TraceInfo &) override;

  // utilities

  std::string create_upgrade_headers();

 private:
  Handler &handler_;
  // config
  const uint16_t stream_id_;
  const std::string name_;
  // connection
  core::web::Socket connection_;
  // buffers
  core::Buffer decode_buffer_;
  // metrics
  struct {
    core::metrics::Counter disconnect;
  } counter_;
  struct {
    core::metrics::Profile parse, cancel_all_after, error, execution, handshake, margin, order,
        position, subscribe;
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
  server::Download<DropCopyState> download_;
  struct {
    bool order = false;
    // XXX maybe everything else too?
  } partial_received_;
};

}  // namespace bitmex
}  // namespace roq
