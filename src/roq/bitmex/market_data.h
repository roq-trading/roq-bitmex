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

#include "roq/bitmex/market_data_state.h"
#include "roq/bitmex/product.h"
#include "roq/bitmex/security.h"
#include "roq/bitmex/shared.h"

#include "roq/bitmex/json/parser.h"

namespace roq {
namespace bitmex {

class MarketData final : public core::web::Socket::Handler, public json::Parser::Handler {
 public:
  struct Handler {
    virtual void operator()(const server::Trace<StreamUpdate> &) = 0;
    virtual void operator()(const server::Trace<ExternalLatency> &) = 0;
    virtual void operator()(const server::Trace<ReferenceData> &, bool is_last) = 0;
    virtual void operator()(const server::Trace<MarketStatus> &, bool is_last) = 0;
    virtual void operator()(const server::Trace<TopOfBook> &, bool is_last) = 0;
    virtual void operator()(const server::Trace<MarketByPriceUpdate> &, bool is_last) = 0;
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
  void operator()(const core::web::Socket::Connected &) override;
  void operator()(const core::web::Socket::Disconnected &) override;
  void operator()(const core::web::Socket::Ready &) override;
  void operator()(const core::web::Socket::Close &) override;
  void operator()(const core::web::Socket::Latency &) override;
  void operator()(const core::web::Socket::Text &) override;

 private:
  void operator()(GatewayStatus);

  void send_subscribe(const std::string_view &topic);
  void send_subscribe(const roq::span<std::string_view> &topics);

  uint32_t download(MarketDataState);

  void subscribe_instrument();
  void subscribe_order_book_l2();

  void parse(const std::string_view &message);
  void parse_helper(const std::string_view &message);

  void operator()(const json::CancelAllAfter &) override;
  void operator()(const json::Error &) override;
  void operator()(const json::Handshake &) override;
  void operator()(const json::Subscribe &) override;

  void operator()(const json::Action, const json::Funding &, const server::TraceInfo &) override;
  void operator()(const json::Action, const json::Instrument &, const server::TraceInfo &) override;
  void operator()(
      const json::Action, const json::Liquidation &, const server::TraceInfo &) override;
  void operator()(
      const json::Action, const json::OrderBookL2 &, const server::TraceInfo &) override;
  void operator()(const json::Action, const json::Quote &, const server::TraceInfo &) override;
  void operator()(const json::Action, const json::Settlement &, const server::TraceInfo &) override;
  void operator()(const json::Action, const json::Trade &, const server::TraceInfo &) override;
  // ... unexpected
  void operator()(const json::Action, const json::Execution &, const server::TraceInfo &) override;
  void operator()(const json::Action, const json::Margin &, const server::TraceInfo &) override;
  void operator()(const json::Action, const json::Order &, const server::TraceInfo &) override;
  void operator()(const json::Action, const json::Position &, const server::TraceInfo &) override;

  // utilities

  Product &find_product(const json::InstrumentItem &);

 private:
  Handler &handler_;
  // config
  const uint16_t stream_id_;
  const std::string name_;
  // connection
  core::web::Socket connection_;
  // buffers
  core::utils::Buffer decode_buffer_;
  // metrics
  struct {
    core::metrics::Counter disconnect;
  } counter_;
  struct {
    core::metrics::Profile parse, cancel_all_after, error, funding, handshake, instrument,
        liquidation, order_book_l2, quote, settlement, subscribe, trade;
  } profile_;
  struct {
    core::metrics::Latency ping, heartbeat;
  } latency_;
  // cache
  Shared &shared_;
  absl::flat_hash_map<std::string, Product> product_cache_;
  // state
  bool ready_ = false;
  GatewayStatus status_ = {};
  server::Download<MarketDataState> download_;
  struct {
    bool instrument = false;
    bool order_book_l2 = false;
  } partial_received_;
};

}  // namespace bitmex
}  // namespace roq
