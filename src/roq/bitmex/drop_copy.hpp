/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>

#include "roq/core/download.hpp"

#include "roq/core/metrics/counter.hpp"
#include "roq/core/metrics/latency.hpp"
#include "roq/core/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/server.hpp"

#include "roq/bitmex/drop_copy_state.hpp"
#include "roq/bitmex/security.hpp"
#include "roq/bitmex/shared.hpp"

#include "roq/bitmex/json/stream_parser.hpp"

namespace roq {
namespace bitmex {

class DropCopy final : public web::socket::Client::Handler, public json::StreamParser::Handler {
 public:
  struct Handler {
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &) = 0;
    virtual void operator()(Trace<TradeUpdate> const &, bool is_last, uint8_t user_id) = 0;
    virtual void operator()(Trace<PositionUpdate> const &, bool is_last) = 0;
  };

  DropCopy(Handler &, io::Context &, uint16_t stream_id, Security &, Shared &);

  DropCopy(DropCopy &&) = delete;
  DropCopy(DropCopy const &) = delete;

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &);

 protected:
  void operator()(web::socket::Client::Connected const &) override;
  void operator()(web::socket::Client::Disconnected const &) override;
  void operator()(web::socket::Client::Ready const &) override;
  void operator()(web::socket::Client::Close const &) override;
  void operator()(web::socket::Client::Latency const &) override;
  void operator()(web::socket::Client::Text const &) override;
  void operator()(web::socket::Client::Binary const &) override;

 private:
  void operator()(ConnectionStatus);

  void send_cancel_all_after(std::chrono::nanoseconds timeout);

  void send_subscribe(std::string_view const &topic);
  void send_subscribe(std::span<std::string_view> const &topics);

  uint32_t download(DropCopyState);

  void subscribe();

  void parse(std::string_view const &message);
  void parse_helper(std::string_view const &message);

  void operator()(Trace<json::CancelAllAfter> const &) override;
  void operator()(Trace<json::Error> const &) override;
  void operator()(Trace<json::Handshake> const &) override;
  void operator()(Trace<json::Subscribe> const &) override;
  void operator()(Trace<json::Unsubscribe> const &) override;

  void operator()(Trace<json::Execution> const &, json::Action) override;
  void operator()(Trace<json::Margin> const &, json::Action) override;
  void operator()(Trace<json::Order> const &, json::Action) override;
  void operator()(Trace<json::Position> const &, json::Action) override;
  // ... unexpected
  void operator()(Trace<json::Funding> const &, json::Action) override;
  void operator()(Trace<json::Instrument> const &, json::Action) override;
  void operator()(Trace<json::Liquidation> const &, json::Action) override;
  void operator()(Trace<json::OrderBookL2> const &, json::Action) override;
  void operator()(Trace<json::Quote> const &, json::Action) override;
  void operator()(Trace<json::Settlement> const &, json::Action) override;
  void operator()(Trace<json::Trade> const &, json::Action) override;

  // utilities

  std::string create_upgrade_headers();

 private:
  Handler &handler_;
  // config
  const uint16_t stream_id_;
  const std::string name_;
  // connection
  std::unique_ptr<web::socket::Client> connection_;
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
