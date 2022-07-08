/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include <memory>
#include <string>

#include "roq/server.hpp"

#include "roq/core/io/event_context.hpp"

#include "roq/bitmex/config.hpp"
#include "roq/bitmex/security.hpp"
#include "roq/bitmex/shared.hpp"

#include "roq/bitmex/drop_copy.hpp"
#include "roq/bitmex/market_data.hpp"
#include "roq/bitmex/order_entry.hpp"
#include "roq/bitmex/web_socket.hpp"

namespace roq {
namespace bitmex {

class MarketData;

class Gateway final : public server::Handler,
                      public OrderEntry::Handler,
                      public WebSocket::Handler,
                      public DropCopy::Handler,
                      public MarketData::Handler {
 public:
  Gateway(server::Dispatcher &, Config const &);

 protected:
  // server::Handler

  void operator()(Event<Start> const &) override;
  void operator()(Event<Stop> const &) override;
  void operator()(Event<Timer> const &) override;
  void operator()(Event<Connected> const &) override;
  void operator()(Event<Disconnected> const &) override;

  void operator()(Trace<StreamStatus const> const &) override;
  void operator()(Trace<ExternalLatency const> const &) override;
  void operator()(Trace<ReferenceData const> const &, bool is_last) override;
  void operator()(Trace<MarketStatus const> const &, bool is_last) override;
  void operator()(Trace<TopOfBook const> const &, bool is_last) override;
  void operator()(Trace<MarketByPriceUpdate const> const &, bool is_last, bool refresh) override;
  void operator()(Trace<TradeSummary const> const &, bool is_last) override;
  void operator()(Trace<StatisticsUpdate const> const &, bool is_last) override;
  void operator()(Trace<TradeUpdate const> const &, bool is_last, uint8_t user_id) override;
  void operator()(Trace<PositionUpdate const> const &, bool is_last) override;

  uint16_t operator()(Event<CreateOrder> const &, oms::Order const &, std::string_view const &request_id) override;
  uint16_t operator()(
      Event<ModifyOrder> const &,
      oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;
  uint16_t operator()(
      Event<CancelOrder> const &,
      oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;
  uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id) override;

  void operator()(metrics::Writer &) override;

 private:
  OrderEntry &get_order_entry(std::string_view const &account);
  WebSocket &get_web_socket(std::string_view const &account);

 private:
  server::Dispatcher &dispatcher_;
  // config
  // security
  absl::flat_hash_map<Account, std::unique_ptr<Security>> security_;
  // io
  core::io::EventContext context_;
  // shared
  Shared shared_;
  // seed
  uint16_t stream_id_ = {};
  // streams
  absl::flat_hash_map<Account, std::unique_ptr<OrderEntry>> order_entry_;
  absl::flat_hash_map<Account, std::unique_ptr<WebSocket>> web_socket_;
  absl::flat_hash_map<Account, std::unique_ptr<DropCopy>> drop_copy_;
  MarketData market_data_;
};

}  // namespace bitmex
}  // namespace roq
