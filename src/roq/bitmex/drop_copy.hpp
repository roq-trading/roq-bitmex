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

#include "roq/bitmex/drop_copy_state.hpp"
#include "roq/bitmex/security.hpp"
#include "roq/bitmex/shared.hpp"

#include "roq/bitmex/json/stream_parser.hpp"

namespace roq {
namespace bitmex {

class DropCopy final : public core::web::ClientSocket::Handler, public json::StreamParser::Handler {
 public:
  struct Handler {
    virtual void operator()(const Trace<StreamStatus const> &) = 0;
    virtual void operator()(const Trace<ExternalLatency const> &) = 0;
    virtual void operator()(const Trace<TradeUpdate const> &, bool is_last, uint8_t user_id) = 0;
    virtual void operator()(const Trace<PositionUpdate const> &, bool is_last) = 0;
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

  void operator()(const Trace<json::CancelAllAfter const> &) override;
  void operator()(const Trace<json::Error const> &) override;
  void operator()(const Trace<json::Handshake const> &) override;
  void operator()(const Trace<json::Subscribe const> &) override;
  void operator()(const Trace<json::Unsubscribe const> &) override;

  void operator()(const Trace<json::Execution const> &, json::Action) override;
  void operator()(const Trace<json::Margin const> &, json::Action) override;
  void operator()(const Trace<json::Order const> &, json::Action) override;
  void operator()(const Trace<json::Position const> &, json::Action) override;
  // ... unexpected
  void operator()(const Trace<json::Funding const> &, json::Action) override;
  void operator()(const Trace<json::Instrument const> &, json::Action) override;
  void operator()(const Trace<json::Liquidation const> &, json::Action) override;
  void operator()(const Trace<json::OrderBookL2 const> &, json::Action) override;
  void operator()(const Trace<json::Quote const> &, json::Action) override;
  void operator()(const Trace<json::Settlement const> &, json::Action) override;
  void operator()(const Trace<json::Trade const> &, json::Action) override;

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
