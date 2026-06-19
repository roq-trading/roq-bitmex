/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/download.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/bitmex/gateway/account.hpp"
#include "roq/bitmex/gateway/shared.hpp"

#include "roq/bitmex/protocol/json/parser.hpp"

namespace roq {
namespace bitmex {
namespace gateway {

struct DropCopy final : public web::socket::Client::Handler, public protocol::json::Parser::Handler {
  struct Handler {};

  DropCopy(Handler &, io::Context &, uint16_t stream_id, Account &, Shared &);

  DropCopy(DropCopy const &) = delete;

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &) const;

 protected:
  // web::socket::Client::Handler

  void operator()(web::socket::Client::Connected const &) override;
  void operator()(web::socket::Client::Disconnected const &) override;
  void operator()(web::socket::Client::Ready const &) override;
  void operator()(web::socket::Client::Close const &) override;
  void operator()(web::socket::Client::Latency const &) override;
  void operator()(web::socket::Client::Text const &) override;
  void operator()(web::socket::Client::Binary const &) override;

 private:
  void operator()(ConnectionStatus, std::string_view const &reason = {});

  void send_cancel_all_after(std::chrono::nanoseconds timeout);

  void send_subscribe(std::string_view const &topic);
  void send_subscribe(std::span<std::string_view> const &topics);

  enum class State {
    UNDEFINED = 0,
    SUBSCRIBE,
    DONE,
  };

  uint32_t download(State);

  void subscribe();

  void parse(std::string_view const &message);

  // protocol::json::Parser::Handler

  void operator()(Trace<protocol::json::Welcome> const &) override;
  void operator()(Trace<protocol::json::Error> const &) override;
  void operator()(Trace<protocol::json::Subscribe> const &) override;
  void operator()(Trace<protocol::json::Unsubscribe> const &) override;
  // public
  void operator()(Trace<protocol::json::Instrument> const &) override;
  void operator()(Trace<protocol::json::Quote> const &) override;
  void operator()(Trace<protocol::json::OrderBookL2> const &) override;
  void operator()(Trace<protocol::json::Trade> const &) override;
  void operator()(Trace<protocol::json::Funding> const &) override;
  void operator()(Trace<protocol::json::Liquidation> const &) override;
  void operator()(Trace<protocol::json::Settlement> const &) override;
  // private
  void operator()(Trace<protocol::json::CancelAllAfter> const &) override;
  void operator()(Trace<protocol::json::Order> const &) override;
  void operator()(Trace<protocol::json::Execution> const &) override;
  void operator()(Trace<protocol::json::Margin> const &) override;
  void operator()(Trace<protocol::json::Position> const &) override;

  // utilities

  std::string create_upgrade_headers();

 private:
  Handler &handler_;
  // config
  uint16_t const stream_id_;
  std::string const name_;
  // connection
  std::unique_ptr<web::socket::Client> const connection_;
  // buffers
  core::json::BufferStack decode_buffer_;
  // metrics
  struct {
    utils::metrics::Counter disconnect;
  } counter_;
  struct {
    utils::metrics::Profile parse,  //
        welcome,                    //
        error,                      //
        subscribe, unsubscribe,     //
        cancel_all_after,           //
        order, execution, margin, position;
  } profile_;
  struct {
    utils::metrics::Latency ping, heartbeat;
  } latency_;
  // account
  Account &account_;
  // cache
  Shared &shared_;
  // state
  bool ready_ = false;
  std::chrono::nanoseconds next_cancel_all_after_ = {};
  ConnectionStatus connection_status_ = {};
  core::Download<State> download_;
  struct {
    bool order = false;
    // XXX maybe everything else too?
  } partial_received_;
};

}  // namespace gateway
}  // namespace bitmex
}  // namespace roq
