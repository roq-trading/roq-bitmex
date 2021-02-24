/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include <string>
#include <utility>
#include <vector>

#include "roq/metrics.h"

#include "roq/download.h"
#include "roq/server.h"

#include "roq/core/io/context.h"

#include "roq/bitmex/config.h"
#include "roq/bitmex/random.h"

#include "roq/bitmex/product.h"
#include "roq/bitmex/rest.h"
#include "roq/bitmex/web_socket.h"

#include "roq/bitmex/web_socket_state.h"

// json (inbound)

namespace roq {
namespace bitmex {

class WebSocket;

class Gateway final : public server::Handler, public Rest::Handler, public WebSocket::Handler {
 public:
  Gateway(server::Dispatcher &dispatcher, const Config &config);

 protected:
  // server::Handler

  void operator()(const Event<Start> &) override;
  void operator()(const Event<Stop> &) override;
  void operator()(const Event<Timer> &) override;
  void operator()(const Event<Connection> &) override;

  void operator()(
      const Event<CreateOrder> &event,
      const std::string_view &request_id,
      uint32_t gateway_order_id) override;
  void operator()(
      const Event<ModifyOrder> &event,
      const std::string_view &request_id,
      const server::OMS_Order &order) override;
  void operator()(
      const Event<CancelOrder> &event,
      const std::string_view &request_id,
      const server::OMS_Order &order) override;

  void operator()(metrics::Writer &writer) override;

  // all
  void operator()(const ExternalLatency &, const server::TraceInfo &) override;

  // WebSocket::Handler

  void operator()(const WebSocket &) override;
  void operator()(const json::Action, const json::Execution &, const server::TraceInfo &) override;
  void operator()(const json::Action, const json::Instrument &, const server::TraceInfo &) override;
  void operator()(const json::Action, const json::Order &, const server::TraceInfo &) override;
  void operator()(
      const json::Action, const json::OrderBookL2 &, const server::TraceInfo &) override;
  void operator()(const json::Action, const json::Position &, const server::TraceInfo &) override;
  void operator()(const json::Action, const json::Quote &, const server::TraceInfo &) override;
  void operator()(const json::Action, const json::Settlement &, const server::TraceInfo &) override;
  void operator()(const json::Action, const json::Trade &, const server::TraceInfo &) override;

  // Rest::Handler

  void operator()(const Rest &) override;

 private:
  void update_market_data(GatewayStatus gateway_status);
  void update_order_manager(GatewayStatus gateway_status);

  void download_accounts();

  void subscribe_instrument();
  void subscribe_order_book_l2();

  const Product &find_product(const json::InstrumentItem &);

  std::pair<double, double> find_price(json::Action action, uint64_t id, double price, double size);

  void publish_market_by_price(
      const std::string_view &symbol,
      size_t bid_length,
      size_t ask_length,
      bool snapshot,
      const server::TraceInfo &trace_info,
      bool is_last);

  void operator()(const json::OrderItem &);
  void operator()(const json::Order &);

  using WebSocketDownload = server::Download<WebSocketState>;

  int32_t download(WebSocketDownload::State state);

 private:
  server::Dispatcher &dispatcher_;
  // config
  const std::string account_;
  // authentication
  Random random_;
  // io
  core::io::Context context_;
  // connections
  struct {
    WebSocket connection;
    WebSocketDownload download;
  } web_socket_;
  struct {
    Rest connection;
  } rest_;
  // reference data
  absl::flat_hash_map<std::string, Product> product_cache_;
  std::vector<std::string> symbols_;
  struct {
    bool instrument = false;
    bool order_book_l2 = false;
  } snapshot_;
  // market data
  GatewayStatus market_data_status_ = GatewayStatus::DISCONNECTED;
  core::page_aligned_vector<MBPUpdate> bid_, ask_;
  core::page_aligned_vector<Trade> trade_;
  core::page_aligned_vector<Fill> fill_;
  absl::flat_hash_map<uint64_t, double> price_lookup_;
  // order manager
  GatewayStatus order_manager_status_ = GatewayStatus::DISCONNECTED;
};

}  // namespace bitmex
}  // namespace roq
