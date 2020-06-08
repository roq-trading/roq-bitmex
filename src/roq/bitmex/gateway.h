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

class Gateway final
    : public server::Handler,
      public Rest::Handler,
      public WebSocket::Handler {
 public:
  Gateway(
      server::Dispatcher& dispatcher,
      const Config& config);

 protected:
  // server::Handler

  void operator()(const server::StartEvent&) override;
  void operator()(const server::StopEvent&) override;
  void operator()(const server::TimerEvent&) override;
  void operator()(const server::ConnectionStatusEvent&) override;

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

  void operator()(metrics::Writer& writer) override;

  // WebSocket::Handler

  void operator()(const WebSocket&) override;
  void operator()(const json::Action, const json::Execution&) override;
  void operator()(const json::Action, const json::Instrument&) override;
  void operator()(const json::Action, const json::Order&) override;
  void operator()(const json::Action, const json::OrderBookL2&) override;
  void operator()(const json::Action, const json::Position&) override;
  void operator()(const json::Action, const json::Quote&) override;
  void operator()(const json::Action, const json::Settlement&) override;
  void operator()(const json::Action, const json::Trade&) override;

  // Rest::Handler

  void operator()(const Rest&) override;

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

  void operator()(const json::OrderItem&);
  void operator()(const json::Order&);

  template <typename T>
  void enqueue(
      const T& event,
      bool is_last);

  template <typename T>
  void enqueue(
      uint8_t user_id,
      const T& event,
      bool is_last);

  using WebSocketDownload = server::Download<WebSocketState>;

  int32_t download(WebSocketDownload::State state);

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
  core::page_aligned_vector<Fill> _fill;
  std::unordered_map<uint64_t, double> _price_lookup;
  // order manager
  GatewayStatus _order_manager_status = GatewayStatus::DISCONNECTED;
};

}  // namespace bitmex
}  // namespace roq
