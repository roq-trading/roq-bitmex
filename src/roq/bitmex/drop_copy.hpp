/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>

#include "roq/core/download.hpp"

#include "roq/core/metrics/counter.hpp"
#include "roq/core/metrics/latency.hpp"
#include "roq/core/metrics/profile.hpp"

#include "roq/io/context.hpp"

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
    virtual void operator()(Trace<StreamStatus const> const &) = 0;
    virtual void operator()(Trace<ExternalLatency const> const &) = 0;
    virtual void operator()(Trace<TradeUpdate const> const &, bool is_last, uint8_t user_id) = 0;
    virtual void operator()(Trace<PositionUpdate const> const &, bool is_last) = 0;
  };

  DropCopy(Handler &, io::Context &, uint16_t stream_id, Security &, Shared &);

  DropCopy(DropCopy &&) = delete;
  DropCopy(DropCopy const &) = delete;

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &);

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

  void send_cancel_all_after(std::chrono::nanoseconds timeout);

  void send_subscribe(std::string_view const &topic);
  void send_subscribe(std::span<std::string_view> const &topics);

  uint32_t download(DropCopyState);

  void subscribe();

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
    core::metrics::Profile parse, cancel_all_after, error, execution, handshake, margin, order, position, subscribe,
        unsubscribe;
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
