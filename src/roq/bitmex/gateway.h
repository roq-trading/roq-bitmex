/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include <memory>
#include <string>

#include "roq/server.h"

#include "roq/core/io/context.h"

#include "roq/bitmex/config.h"
#include "roq/bitmex/security.h"
#include "roq/bitmex/shared.h"

#include "roq/bitmex/drop_copy.h"
#include "roq/bitmex/market_data.h"
#include "roq/bitmex/order_entry.h"

namespace roq {
namespace bitmex {

class MarketData;

class Gateway final : public server::Handler,
                      public OrderEntry::Handler,
                      public DropCopy::Handler,
                      public MarketData::Handler {
 public:
  Gateway(server::Dispatcher &, const Config &);

 protected:
  // server::Handler

  void operator()(const Event<Start> &) override;
  void operator()(const Event<Stop> &) override;
  void operator()(const Event<Timer> &) override;
  void operator()(const Event<Connected> &) override;
  void operator()(const Event<Disconnected> &) override;

  void operator()(const server::Trace<StreamStatus> &) override;
  void operator()(const server::Trace<ExternalLatency> &) override;
  void operator()(const server::Trace<ReferenceData> &, bool is_last) override;
  void operator()(const server::Trace<MarketStatus> &, bool is_last) override;
  void operator()(const server::Trace<TopOfBook> &, bool is_last) override;
  void operator()(const server::Trace<MarketByPriceUpdate> &, bool is_last) override;
  void operator()(const server::Trace<TradeSummary> &, bool is_last) override;
  void operator()(const server::Trace<StatisticsUpdate> &, bool is_last) override;
  void operator()(const server::Trace<TradeUpdate> &, bool is_last, uint8_t user_id) override;
  void operator()(const server::Trace<PositionUpdate> &, bool is_last) override;

  void operator()(
      const Event<CreateOrder> &,
      const std::string_view &request_id,
      uint32_t gateway_order_id) override;
  void operator()(
      const Event<ModifyOrder> &,
      const std::string_view &request_id,
      const server::OMS_Order &) override;
  void operator()(
      const Event<CancelOrder> &,
      const std::string_view &request_id,
      const server::OMS_Order &) override;

  void operator()(metrics::Writer &) override;

 private:
  OrderEntry &get_order_entry(const std::string_view &account);

 private:
  server::Dispatcher &dispatcher_;
  // config
  const std::string master_account_;
  // security
  absl::flat_hash_map<std::string, std::unique_ptr<Security>> security_;
  // io
  core::io::Context context_;
  // shared
  Shared shared_;
  // seed
  uint16_t stream_id_ = {};
  // streams
  absl::flat_hash_map<std::string, std::unique_ptr<OrderEntry>> order_entry_;
  absl::flat_hash_map<std::string, std::unique_ptr<DropCopy>> drop_copy_;
  MarketData market_data_;
};

}  // namespace bitmex
}  // namespace roq
