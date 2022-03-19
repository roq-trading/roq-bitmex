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

#include "roq/core/io/context.hpp"

#include "roq/core/web/client_socket.hpp"

#include "roq/server.hpp"

#include "roq/bitmex/market_data_state.hpp"
#include "roq/bitmex/product.hpp"
#include "roq/bitmex/security.hpp"
#include "roq/bitmex/shared.hpp"

#include "roq/bitmex/json/stream_parser.hpp"

namespace roq {
namespace bitmex {

class MarketData final : public core::web::ClientSocket::Handler,
                         public json::StreamParser::Handler {
 public:
  struct Handler {
    virtual void operator()(const server::Trace<StreamStatus> &) = 0;
    virtual void operator()(const server::Trace<ExternalLatency> &) = 0;
    virtual void operator()(const server::Trace<ReferenceData> &, bool is_last) = 0;
    virtual void operator()(const server::Trace<MarketStatus> &, bool is_last) = 0;
    virtual void operator()(const server::Trace<TopOfBook> &, bool is_last) = 0;
    virtual void operator()(
        const server::Trace<MarketByPriceUpdate> &, bool is_last, bool refresh) = 0;
    virtual void operator()(const server::Trace<TradeSummary> &, bool is_last) = 0;
    virtual void operator()(const server::Trace<StatisticsUpdate> &, bool is_last) = 0;
  };

  MarketData(Handler &, core::io::Context &, uint16_t stream_id, Shared &);

  MarketData(MarketData &&) = delete;
  MarketData(const MarketData &) = delete;

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

  void send_subscribe(const std::string_view &topic);
  void send_unsubscribe(const std::string_view &topic);

  void send_subscribe(const std::span<std::string_view> &topics);

  void send_subscribe(const std::string_view &topic, const std::string_view &symbol);
  void send_unsubscribe(const std::string_view &topic, const std::string_view &symbol);

  uint32_t download(MarketDataState);

  void subscribe_instrument();
  void subscribe_order_book_l2();

  void parse(const std::string_view &message);
  void parse_helper(const std::string_view &message);

  void operator()(const server::Trace<json::CancelAllAfter> &) override;
  void operator()(const server::Trace<json::Error> &) override;
  void operator()(const server::Trace<json::Handshake> &) override;
  void operator()(const server::Trace<json::Subscribe> &) override;
  void operator()(const server::Trace<json::Unsubscribe> &) override;

  void operator()(const server::Trace<json::Funding> &, json::Action) override;
  void operator()(const server::Trace<json::Instrument> &, json::Action) override;
  void operator()(const server::Trace<json::Liquidation> &, json::Action) override;
  void operator()(const server::Trace<json::OrderBookL2> &, json::Action) override;
  void operator()(const server::Trace<json::Quote> &, json::Action) override;
  void operator()(const server::Trace<json::Settlement> &, json::Action) override;
  void operator()(const server::Trace<json::Trade> &, json::Action) override;
  // ... unexpected
  void operator()(const server::Trace<json::Execution> &, json::Action) override;
  void operator()(const server::Trace<json::Margin> &, json::Action) override;
  void operator()(const server::Trace<json::Order> &, json::Action) override;
  void operator()(const server::Trace<json::Position> &, json::Action) override;

  // utilities

  Product &find_product(const json::InstrumentItem &);
  Product &find_product(const json::FundingItem &);

  // experimental

  void publish_market_by_price(
      const server::TraceInfo &,
      bool is_last,
      const std::string_view &symbol,
      const std::span<MBPUpdate> &bids,
      const std::span<MBPUpdate> &asks,
      bool snapshot);
  void resubscribe_order_book_l2(const std::string_view &symbol);

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
    core::metrics::Profile parse, cancel_all_after, error, funding, handshake, instrument,
        liquidation, order_book_l2, quote, settlement, subscribe, unsubscribe, trade;
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
