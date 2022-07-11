/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

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

#include "roq/bitmex/market_data_state.hpp"
#include "roq/bitmex/product.hpp"
#include "roq/bitmex/security.hpp"
#include "roq/bitmex/shared.hpp"

#include "roq/bitmex/json/stream_parser.hpp"

namespace roq {
namespace bitmex {

class MarketData final : public core::web::ClientSocket::Handler, public json::StreamParser::Handler {
 public:
  struct Handler {
    virtual void operator()(Trace<StreamStatus const> const &) = 0;
    virtual void operator()(Trace<ExternalLatency const> const &) = 0;
    virtual void operator()(Trace<ReferenceData const> const &, bool is_last) = 0;
    virtual void operator()(Trace<MarketStatus const> const &, bool is_last) = 0;
    virtual void operator()(Trace<TopOfBook const> const &, bool is_last) = 0;
    virtual void operator()(Trace<MarketByPriceUpdate const> const &, bool is_last, bool refresh) = 0;
    virtual void operator()(Trace<TradeSummary const> const &, bool is_last) = 0;
    virtual void operator()(Trace<StatisticsUpdate const> const &, bool is_last) = 0;
  };

  MarketData(Handler &, io::Context &, uint16_t stream_id, Shared &);

  MarketData(MarketData &&) = delete;
  MarketData(MarketData const &) = delete;

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

  void send_subscribe(std::string_view const &topic);
  void send_unsubscribe(std::string_view const &topic);

  void send_subscribe(std::span<std::string_view> const &topics);

  void send_subscribe(std::string_view const &topic, std::string_view const &symbol);
  void send_unsubscribe(std::string_view const &topic, std::string_view const &symbol);

  uint32_t download(MarketDataState);

  void subscribe_instrument();
  void subscribe_order_book_l2();

  void parse(std::string_view const &message);
  void parse_helper(std::string_view const &message);

  void operator()(Trace<json::CancelAllAfter const> const &) override;
  void operator()(Trace<json::Error const> const &) override;
  void operator()(Trace<json::Handshake const> const &) override;
  void operator()(Trace<json::Subscribe const> const &) override;
  void operator()(Trace<json::Unsubscribe const> const &) override;

  void operator()(Trace<json::Funding const> const &, json::Action) override;
  void operator()(Trace<json::Instrument const> const &, json::Action) override;
  void operator()(Trace<json::Liquidation const> const &, json::Action) override;
  void operator()(Trace<json::OrderBookL2 const> const &, json::Action) override;
  void operator()(Trace<json::Quote const> const &, json::Action) override;
  void operator()(Trace<json::Settlement const> const &, json::Action) override;
  void operator()(Trace<json::Trade const> const &, json::Action) override;
  // ... unexpected
  void operator()(Trace<json::Execution const> const &, json::Action) override;
  void operator()(Trace<json::Margin const> const &, json::Action) override;
  void operator()(Trace<json::Order const> const &, json::Action) override;
  void operator()(Trace<json::Position const> const &, json::Action) override;

  // utilities

  Product &find_product(json::InstrumentItem const &);
  Product &find_product(json::FundingItem const &);

  // experimental

  void publish_market_by_price(
      TraceInfo const &,
      bool is_last,
      std::string_view const &symbol,
      std::span<MBPUpdate> const &bids,
      std::span<MBPUpdate> const &asks,
      bool snapshot,
      const std::chrono::nanoseconds exchange_time_utc);
  void resubscribe_order_book_l2(std::string_view const &symbol);

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
    core::metrics::Profile parse, cancel_all_after, error, funding, handshake, instrument, liquidation, order_book_l2,
        quote, settlement, subscribe, unsubscribe, trade;
  } profile_;
  struct {
    core::metrics::Latency ping, heartbeat;
  } latency_;
  // cache
  Shared &shared_;
  absl::flat_hash_map<Symbol, Product> product_cache_;
  // state
  bool ready_ = false;
  ConnectionStatus status_ = {};
  core::Download<MarketDataState> download_;
  struct {
    bool instrument = false;
    bool order_book_l2 = false;
  } partial_received_;
};

}  // namespace bitmex
}  // namespace roq
