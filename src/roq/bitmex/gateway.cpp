/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/gateway.h"

#include <limits>
#include <utility>

#include "roq/logging.h"
#include "roq/format.h"

#include "roq/core/charconv.h"
#include "roq/core/clock.h"
#include "roq/core/utils.h"
#include "roq/core/view.h"

#include "roq/bitmex/options.h"

#include "roq/bitmex/api/utils.h"

namespace roq {
namespace bitmex {

constexpr auto DEFAULT_MULTIPLIER = double{1.0};

constexpr auto TOLERANCE = double{1.0e-10};

template <typename T>
static inline void mbp_update(
    auto& data,
    size_t& offset,
    const T& item) {
  new (&data[offset++]) MBPUpdate {
    .price = item.price,
    .quantity = item.size,
  };
  if (offset >= data.size())
    throw std::runtime_error("Not enough space");
}

Gateway::Gateway(
    server::Dispatcher& dispatcher,
    const Config& config)
    : _dispatcher(dispatcher),
      _account(config.get_account()),
      _dns_base(_base, true),
      _websocket(
          *this,
          config,
          _base,
          _dns_base,
          _ssl_context),
      _rest(
          *this,
          config,
          _base,
          _dns_base,
          _ssl_context),
      _bid(FLAGS_max_depth),
      _ask(FLAGS_max_depth) {
  LOG_IF(WARNING, FLAGS_cancel_on_disconnect == false)(
      "Orders will *NOT* be cancelled on disconnect");
}

void Gateway::operator()(const StartEvent& event) {
  LOG(INFO)("Starting the gateway...");
  _websocket(event);
  _rest(event);
}

void Gateway::operator()(const StopEvent& event) {
  LOG(INFO)("Stopping the gateway...");
  _websocket(event);
  _rest(event);
}

void Gateway::operator()(const TimerEvent& event) {
  _websocket(event);
  _rest(event);
  _base.loop(EVLOOP_NONBLOCK);
}

void Gateway::operator()(const ConnectionStatusEvent&) {
}

void Gateway::operator()(const CreateOrderEvent& event) {
  if (unlikely(validate(event) == false))
    return;
  auto& message_info = event.message_info;
  auto& create_order = event.create_order;
  // TODO(thraneh): check against max_order_id before continuing
  auto gateway_order_id = _dispatcher.next_order_id();
  OrderMapping order_mapping(
      message_info,
      create_order,
      gateway_order_id);
  auto key = order_mapping.key();
  if (unlikely(_order_mapping.find(key) != _order_mapping.end())) {
    _dispatcher.send_order_ack(
        event,
        Error::INVALID_ORDER_ID);
    return;
  } else {
    auto iter = _order_mapping.emplace(
        key,
        std::move(order_mapping)).first;
    // XXX fix this... replacing previous variable
    auto& order_mapping = (*iter).second;
    LOG(FATAL)("NOT IMPLEMENTED");
  }
}

void Gateway::operator()(const ModifyOrderEvent& event) {
  _dispatcher.send_order_ack(
      event,
      Error::MODIFY_ORDER_NOT_SUPPORTED);
}

void Gateway::operator()(const CancelOrderEvent& event) {
  auto& message_info = event.message_info;
  auto& cancel_order = event.cancel_order;
  auto key = OrderMapping::key(
      message_info.source,
      cancel_order.order_id);
  auto iter = _order_mapping.find(key);
  if (unlikely(iter == _order_mapping.end())) {
    _dispatcher.send_order_ack(
        event,
        Error::UNKNOWN_ORDER_ID);
    return;
  }
  auto& order_mapping = (*iter).second;
  auto gateway_order_id = order_mapping.gateway_order_id();
  auto external_order_id = order_mapping.exchange_order_id();
  if (unlikely(validate(event, gateway_order_id, external_order_id) == false)) {
    return;
  }
  if (unlikely(order_mapping.ready() == false)) {
    _dispatcher.send_order_ack(
        event,
        Error::UNKNOWN_EXCHANGE_ORDER_ID,
        gateway_order_id,
        external_order_id);
    return;
  }
  LOG(FATAL)("NOT IMPLEMENTED");
}

void Gateway::operator()(Metrics& metrics) {
  _websocket(metrics);
  _rest(metrics);
}

// ws

void Gateway::operator()(const WebSocket& websocket) {
  // assert(_market_data_status == GatewayStatus::CONNECTING);
  if (websocket.ready()) {
    // pretend the (automatic) upgrade request is the login
    update_market_data(GatewayStatus::LOGIN_SENT);
    begin_download();
  } else {
    update_market_data(GatewayStatus::DISCONNECTED);
    _download = Download::NONE;
    _symbols.clear();
  }
}

void Gateway::operator()(const json::Instrument&) {
}

void Gateway::operator()(const json::OrderBookL2& order_book_l2) {
  // check partial (we receive just once for all instruments)
  roq::span data(
      order_book_l2.data.items,
      order_book_l2.data.length);
  std::string_view previous;
  for (auto& item : data) {
    if (item.symbol.compare(previous) != 0) {
      previous = item.symbol;
      // XXX enqueue
    }
    auto iter = _price_lookup.find(item.id);
    switch (order_book_l2.action) {
      case json::Action::UNKNOWN:
        LOG(FATAL)("Unexpected");
        break;
      case json::Action::PARTIAL:
      case json::Action::INSERT:
        LOG_IF(FATAL, iter != _price_lookup.end())("FOUND DUPLICATE");
        assert(std::isnan(item.price) == false);
        _price_lookup.emplace(item.id, item.price);
        break;
      case json::Action::UPDATE:
        LOG_IF(FATAL, iter == _price_lookup.end())("NO LOOKUP");
        break;
      case json::Action::DELETE:
        LOG_IF(FATAL, iter == _price_lookup.end())("NO LOOKUP");
        _price_lookup.erase(iter);
        break;
    }
  }
  // XXX enqueue
}

// rest

void Gateway::operator()(const Rest&) {
}


// UTILS:

void Gateway::update_market_data(GatewayStatus gateway_status) {
  if (gateway_status == _market_data_status)
    return;
  _market_data_status = gateway_status;
  MarketDataStatus market_data_status {
    .status = _market_data_status,
  };
  enqueue(
      market_data_status,
      true);
  LOG(INFO)("market_data_status={}", _market_data_status);
}

void Gateway::update_order_manager(GatewayStatus gateway_status) {
  if (gateway_status == _order_manager_status)
    return;
  _order_manager_status = gateway_status;
  OrderManagerStatus order_manager_status {
    .account = _account,
    .status = _order_manager_status,
  };
  enqueue(
      order_manager_status,
      true);
  LOG(INFO)("order_manager_status={}", _order_manager_status);
}

void Gateway::begin_download() {
  assert(_download == Download::NONE);
  assert(_market_data_status == GatewayStatus::LOGIN_SENT);
  update_market_data(GatewayStatus::DOWNLOADING);
  LOG(INFO)("Download:");
  download_products();
}

void Gateway::check_download() {
  assert(_download != Download::NONE);
  assert(_market_data_status == GatewayStatus::DOWNLOADING);
  switch (_download) {
    case Download::NONE:
      assert(false);
      break;
    case Download::PRODUCTS: {
      LOG(INFO)("Download products COMPLETED");
      download_accounts();
      break;
    }
    case Download::ACCOUNTS: {
      LOG(INFO)("Download accounts COMPLETED");
      update_market_data(GatewayStatus::READY);
      LOG(INFO)("Download COMPLETED");
      _download = Download::NONE;
      subscribe();
      break;
    }
  }
}

void Gateway::download_products() {
  assert(_market_data_status == GatewayStatus::DOWNLOADING);
  LOG(INFO)("Download products...");
  // XXX _rest.get_products();
  _websocket.subscribe({});
  _download = Download::PRODUCTS;
}

void Gateway::download_accounts() {
  assert(_market_data_status == GatewayStatus::DOWNLOADING);
  LOG(INFO)("Download accounts...");
  // _rest.get_accounts();
  _download = Download::ACCOUNTS;
}

void Gateway::subscribe() {
  assert(_market_data_status == GatewayStatus::READY);
  LOG(INFO)("Subscribe channels");
  // _websocket.subscribe(_symbols);
}

template <typename T>
void Gateway::enqueue(
    const T& event,
    bool is_last) {
  auto now = core::get_system_clock();
  _dispatcher.enqueue(
      event,
      now,
      now,
      is_last);
}

template <typename T>
void Gateway::enqueue(
    uint8_t user_id,
    const T& event,
    bool is_last) {
  auto now = core::get_system_clock();
  _dispatcher.enqueue(
      user_id,
      event,
      now,
      now,
      is_last);
}

bool Gateway::validate(const CreateOrderEvent& event) {
  auto& create_order = event.create_order;
  auto error = Error::NONE;
  if (unlikely(_order_manager_status != GatewayStatus::READY)) {
    error = Error::GATEWAY_NOT_READY;
  } else if (unlikely(
        create_order.account.compare(_account) != 0)) {
    error = Error::INVALID_ACCOUNT;
  } else if (unlikely(
        create_order.exchange.compare(FLAGS_exchange) != 0)) {
    error = Error::INVALID_EXCHANGE;
  } else if (unlikely(
        create_order.position_effect != PositionEffect::UNDEFINED)) {
    error = Error::INVALID_POSITION_EFFECT;
  } else if (unlikely(
        create_order.order_template.empty() == false &&
        create_order.order_template.compare("default") != 0)) {
    error = Error::INVALID_ORDER_TEMPLATE;
  } else {
    return true;
  }
  assert(error != Error::NONE);
  _dispatcher.send_order_ack(
      event,
      error);
  return false;
}

bool Gateway::validate(
    const ModifyOrderEvent& event,
    uint32_t gateway_order_id,
    const std::string_view& external_order_id) {
  auto& modify_order = event.modify_order;
  auto error = Error::NONE;
  if (unlikely(_order_manager_status != GatewayStatus::READY)) {
    error = Error::GATEWAY_NOT_READY;
  } else if (unlikely(
        modify_order.account.compare(_account) != 0)) {
    error = Error::INVALID_ACCOUNT;
  } else {
    return true;
  }
  assert(error != Error::NONE);
  _dispatcher.send_order_ack(
      event,
      error,
      gateway_order_id,
      external_order_id);
  return false;
}

bool Gateway::validate(
    const CancelOrderEvent& event,
    uint32_t gateway_order_id,
    const std::string_view& external_order_id) {
  auto& cancel_order = event.cancel_order;
  auto error = Error::NONE;
  if (unlikely(_order_manager_status != GatewayStatus::READY)) {
    error = Error::GATEWAY_NOT_READY;
  } else if (unlikely(
        cancel_order.account.compare(_account) != 0)) {
    error = Error::INVALID_ACCOUNT;
  } else {
    return true;
  }
  assert(error != Error::NONE);
  _dispatcher.send_order_ack(
      event,
      error,
      gateway_order_id,
      external_order_id);
  return false;
}


decltype(Gateway::_order_mapping)::iterator
Gateway::find_order_mapping(const std::string_view& order_id) {
  auto iter = _order_lookup.find(order_id);
  if (unlikely(iter == _order_lookup.end())) {
    return _order_mapping.end();
  }
  return _order_mapping.find((*iter).second);
}

decltype(Gateway::_order_mapping)::iterator
Gateway::find_order_mapping(
    const std::string_view& order_id,
    const std::string_view& cl_ord_id) {
  auto iter = _order_lookup.find(order_id);
  if (unlikely(iter == _order_lookup.end())) {
    iter = _order_lookup.find(cl_ord_id);
    if (unlikely(iter == _order_lookup.end())) {
      return _order_mapping.end();
    } else {
      // replace cl_ord_id with order_id
      auto key = (*iter).second;
      _order_lookup.erase(iter);
      iter = _order_lookup.emplace(
          std::string(order_id),
          key).first;
    }
  }
  return _order_mapping.find((*iter).second);
}

}  // namespace bitmex
}  // namespace roq
