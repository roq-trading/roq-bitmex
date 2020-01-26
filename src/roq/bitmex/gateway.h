/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "roq/metrics.h"

#include "roq/server.h"

#include "roq/core/hash/map.h"

#include "roq/core/ssl/ssl.h"

#include "roq/core/event/base.h"
#include "roq/core/event/dns_base.h"

#include "roq/bitmex/config.h"

#include "roq/bitmex/order_mapping.h"
#include "roq/bitmex/rest.h"
#include "roq/bitmex/websocket.h"

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
  void operator()(const CreateOrderEvent&) override;
  void operator()(const ModifyOrderEvent&) override;
  void operator()(const CancelOrderEvent&) override;

  void operator()(Metrics& metrics) override;

  // ws
  void operator()(const WebSocket&);
  void operator()(const json::Instrument&);
  void operator()(const json::OrderBookL2&);
  void operator()(const json::Quote&);
  void operator()(const json::Settlement&);
  void operator()(const json::Trade&);

  // rest
  void operator()(const Rest&);

 private:
  void update_market_data(GatewayStatus gateway_status);
  void update_order_manager(GatewayStatus gateway_status);

  void begin_download();
  void check_download();

  void download_accounts();

  void subscribe_instrument();
  void subscribe_order_book_l2();

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

  bool validate(const CreateOrderEvent& event);

  bool validate(
      const ModifyOrderEvent& event,
      uint32_t gateway_order_id,
      const std::string_view& external_order_id);

  bool validate(
      const CancelOrderEvent& event,
      uint32_t gateway_order_id,
      const std::string_view& external_order_id);

 private:
  server::Dispatcher& _dispatcher;
  // config
  const std::string _account;
  // async
  core::event::Base _base;
  core::event::DNSBase _dns_base;
  // crypto
  core::ssl::Context _ssl_context;
  // connections
  WebSocket _websocket;
  Rest _rest;
  // download
  enum class Download {
    NONE,
    PRODUCTS,
    ACCOUNTS,
    ORDER_BOOKS,
    READY,
  } _download = Download::NONE;
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
  std::unordered_map<uint64_t, OrderMapping> _order_mapping;
  core::hash::map<std::string, uint64_t> _order_lookup;

  decltype(_order_mapping)::iterator find_order_mapping(  // XXX move
      const std::string_view& order_id);
  decltype(_order_mapping)::iterator find_order_mapping(  // XXX move
      const std::string_view& order_id,
      const std::string_view& cl_ord_id);
};

}  // namespace bitmex
}  // namespace roq
