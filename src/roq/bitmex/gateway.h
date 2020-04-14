/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "roq/metrics.h"

#include "roq/server.h"
#include "roq/download.h"

#include "roq/core/hash/map.h"

#include "roq/core/ssl/ssl.h"

#include "roq/core/event/base.h"
#include "roq/core/event/dns_base.h"

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

class Gateway final : public server::Handler {
 public:
  Gateway(
      server::Dispatcher& dispatcher,
      const Config& config);

  void operator()(const StartEvent&) override;
  void operator()(const StopEvent&) override;
  void operator()(const TimerEvent&) override;
  void operator()(const ConnectionStatusEvent&) override;

  void operator()(
      const CreateOrderEvent& event,
      const std::string_view& request_id,
      uint32_t gateway_order_id) override;
  void operator()(
      const ModifyOrderEvent& event,
      const std::string_view& request_id,
      const server::OMS_Order& order) override;
  void operator()(
      const CancelOrderEvent& event,
      const std::string_view& request_id,
      const server::OMS_Order& order) override;

  void operator()(Metrics& metrics) override;

  // ws
  void operator()(const WebSocket&);
  void operator()(const json::Action, const json::Execution&);
  void operator()(const json::Action, const json::Instrument&);
  void operator()(const json::Action, const json::Order&);
  void operator()(const json::Action, const json::OrderBookL2&);
  void operator()(const json::Action, const json::Position&);
  void operator()(const json::Action, const json::Quote&);
  void operator()(const json::Action, const json::Settlement&);
  void operator()(const json::Action, const json::Trade&);

  // rest
  void operator()(const Rest&);
  void operator()(const json::OrderItem&);
  void operator()(const json::Order&);

 private:
  void update_market_data(GatewayStatus gateway_status);
  void update_order_manager(GatewayStatus gateway_status);

  void download_accounts();

  void subscribe_instrument();
  void subscribe_order_book_l2();

  const Product& find_product(const json::InstrumentItem&);

  std::pair<double, double> find_price(
      json::Action action,
      uint64_t id,
      double price,
      double size);

  void publish_market_by_price(
      const std::string_view& symbol,
      size_t bid_length,
      size_t ask_length,
      bool snapshot,
      bool is_last);

 private:
  template <typename T>
  void enqueue(
      const T& event,
      bool is_last);

  template <typename T>
  void enqueue(
      uint8_t user_id,
      const T& event,
      bool is_last);

 private:
  using WebSocketDownload = server::Download<WebSocketState>;

  uint32_t download(WebSocketDownload::State state);

 private:
  server::Dispatcher& _dispatcher;
  // config
  const std::string _account;
  // authentication
  Random _random;
  // async
  core::event::Base _base;
  core::event::DNSBase _dns_base;
  // crypto
  core::ssl::Context _ssl_context;
  // connections
  struct {
    WebSocket connection;
    WebSocketDownload download;
  } _web_socket;
  struct {
    Rest connection;
  } _rest;
  // reference data
  core::hash::map<std::string, Product> _product_cache;
  std::vector<std::string> _symbols;
  struct {
    bool instrument = false;
    bool order_book_l2 = false;
  } _snapshot;
  // market data
  GatewayStatus _market_data_status = GatewayStatus::DISCONNECTED;
  core::page_aligned_vector<MBPUpdate> _bid, _ask;
  core::page_aligned_vector<Trade> _trade;
  std::unordered_map<uint64_t, double> _price_lookup;
  // order manager
  GatewayStatus _order_manager_status = GatewayStatus::DISCONNECTED;
};

}  // namespace bitmex
}  // namespace roq
