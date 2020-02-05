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

#include "roq/bitmex/json/utils.h"

namespace roq {
namespace bitmex {

constexpr auto DEFAULT_MULTIPLIER = double{1.0};

constexpr auto TOLERANCE = double{1.0e-10};

template <typename T>
static bool mbp_update(
    auto& data,
    size_t& offset,
    const T& item) {
  auto& obj = data[offset];
  new (&obj) MBPUpdate {
    .price = item.first,
    .quantity = item.second,
  };
  ++offset;
  return offset < data.size();
}

template <typename T>
static bool trade_update(
    auto& data,
    size_t& offset,
    const T& item) {
  auto& obj = data[offset];
  new (&obj) Trade {
    .side = json::convert(item.side),  // XXX check
    .price = item.price,
    .quantity = item.size,
    .trade_id = {},
  };
  core::copy_to(
      item.trd_match_id,
      obj.trade_id);
  ++offset;
  return offset < data.size();
}

Gateway::Gateway(
    server::Dispatcher& dispatcher,
    const Config& config)
    : _dispatcher(dispatcher),
      _account(config.get_account()),
      _random(
          config.get_api_key(),
          config.get_secret()),
      _dns_base(_base, true),
      _websocket(
          *this,
          config,
          _random,
          _base,
          _dns_base,
          _ssl_context),
      _rest(
          *this,
          config,
          _random,
          _base,
          _dns_base,
          _ssl_context),
      _bid(FLAGS_max_depth),
      _ask(FLAGS_max_depth),
      _trade(FLAGS_max_trades) {
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
  VLOG(1)("WebSocket");
  if (websocket.ready()) {
    if (_download == Download::NONE) {
      // pretend the (automatic) upgrade request is the login
      update_order_manager(GatewayStatus::LOGIN_SENT);
      update_market_data(GatewayStatus::LOGIN_SENT);
      begin_download();
    }
  } else {
    update_market_data(GatewayStatus::DISCONNECTED);
    _download = Download::NONE;
    _symbols.clear();
    _snapshot = {};
  }
}

void Gateway::operator()(const json::Instrument& instrument) {
  switch (instrument.action) {
    case json::Action::UNDEFINED:
    case json::Action::UNKNOWN:
      LOG(FATAL)("Unexpected");
      break;
    case json::Action::PARTIAL:
      if (_snapshot.instrument == false) {
        _snapshot.instrument = true;
        assert(_download != Download::READY);
        size_t security_count = 0;
        for (auto& item : instrument.data) {
          if (_dispatcher.discard_symbol(item.symbol)) {
            VLOG(1)(
                FMT_STRING("Drop symbol=\"{}\""),
                item.symbol);
            continue;
          }
          ++security_count;
          ReferenceData reference_data {
            .exchange = FLAGS_exchange,
            .symbol = item.symbol,
            .security_type = SecurityType::UNDEFINED,  // XXX typ?
            .currency = item.quote_currency,  // XXX or position_currency?
            .settlement_currency = item.settl_currency,
            .commission_currency = std::string_view(),
            .tick_size = item.tick_size,
            .limit_up = item.limit_up_price,
            .limit_down = item.limit_down_price,
            .multiplier = item.multiplier,
            .min_trade_vol = item.lot_size,  // XXX correct?
            .option_type = OptionType::UNDEFINED,  // XXX typ?
            .strike_currency = std::string_view(),
            .strike_price = item.option_strike_price,
          };
          enqueue(reference_data, false);
          // XXX market status <-- state (but need caching)
        }
        VLOG(1)(
            FMT_STRING("- securities: {} (/{})"),
            security_count,
            instrument.data.size());
        check_download();
      }
      break;
    case json::Action::INSERT:
      // drop
      break;
    case json::Action::UPDATE:
      // drop
      break;
    case json::Action::DELETE:
      // drop
      break;
  }
}

void Gateway::operator()(const json::Order&) {
}

void Gateway::operator()(const json::OrderBookL2& order_book_l2) {
  assert(order_book_l2.action != json::Action::UNKNOWN);
  auto snapshot = order_book_l2.action == json::Action::PARTIAL;
  // note!
  //   first partial update will include *all* instruments
  //   drop everything received before partial (see: API documentation)
  if (snapshot == false &&
      _download != Download::READY)
    return;
  std::string_view previous;
  bool success = true;
  size_t bid_length = 0, ask_length = 0;
  for (auto& item : order_book_l2.data) {
    if (success == false)
      break;
    if (previous.empty()) {
      previous = item.symbol;
    } else if (previous.compare(item.symbol) != 0) {
      publish_market_by_price(
          previous,
          bid_length,
          ask_length,
          snapshot,
          false);
      previous = item.symbol;
      bid_length = ask_length = 0;
    }
    auto price_size = find_price(
        order_book_l2.action,
        item.id,
        item.price,
        item.size);
    switch (item.side) {
      case json::Side::BUY:
        success = mbp_update(
            _bid,
            bid_length,
            price_size);
        break;
      case json::Side::SELL:
        success = mbp_update(
            _ask,
            ask_length,
            price_size);
        break;
      default:
        LOG(FATAL)("Unexpected");
    }
  }
  if (unlikely(success == false)) {
    LOG(FATAL)(
        FMT_STRING(
          "Insufficient bid/ask array size(s): "
          "len(bid)={}/{}, len(ask)={}/{}"),
        bid_length,
        _bid.size(),
        ask_length,
        _ask.size());
  }
  assert(previous.empty() == false);
  publish_market_by_price(
      previous,
      bid_length,
      ask_length,
      snapshot,
      true);
  // download complete?
  if (snapshot && _download != Download::READY)
    check_download();
}

void Gateway::operator()(const json::Position&) {
}

void Gateway::operator()(const json::Quote& quote) {
  for (auto& item : quote.data) {
    TopOfBook top_of_book {
      .exchange = FLAGS_exchange,
      .symbol = item.symbol,
      .layer = {
        .bid_price = item.bid_price,
        .bid_quantity = item.bid_size,
        .ask_price = item.ask_price,
        .ask_quantity = item.ask_size,
      },
      .snapshot = false,  // XXX ???
      .exchange_time_utc = item.timestamp,
    };
    VLOG(1)(FMT_STRING("top_of_book={}"), top_of_book);
    enqueue(
        top_of_book,
        true);  // XXX not always correct
  }
}

void Gateway::operator()(const json::Settlement&) {
}

void Gateway::operator()(const json::Trade& trade) {
  if (trade.action != json::Action::INSERT)
    return;
  std::string_view previous;
  bool success = true;
  size_t trade_length = 0;
  std::chrono::nanoseconds timestamp = {};
  for (auto& item : trade.data) {
    if (success == false)
      break;
    if (timestamp.count() == 0) {
      timestamp = item.timestamp;
    } else {
      assert(timestamp == item.timestamp);
    }
    if (item.symbol.compare(previous) != 0) {
      if (previous.empty() == false && trade_length > 0) {
        TradeSummary trade_summary {
          .exchange = FLAGS_exchange,
          .symbol = previous,
          .trades = {
            .items = _trade.data(),
            .length = trade_length,
          },
          .exchange_time_utc = timestamp,
          };
        VLOG(1)(FMT_STRING("trade_summary={}"), trade_summary);
        enqueue(
            trade_summary,
            false);
      }
      previous = item.symbol;
      trade_length = 0;
    }
    success = trade_update(
        _trade,
        trade_length,
        item);
  }
  if (unlikely(success == false)) {
    LOG(FATAL)(
        FMT_STRING(
          "Insufficient trade array size: "
          "len(trade)={}/{}"),
        trade_length,
        _trade.size());
  }
  if (previous.empty() == false && trade_length > 0) {
    TradeSummary trade_summary {
      .exchange = FLAGS_exchange,
      .symbol = previous,
      .trades = {
        .items = _trade.data(),
        .length = trade_length,
      },
      .exchange_time_utc = timestamp,
      };
    VLOG(1)(FMT_STRING("trade_summary={}"), trade_summary);
    enqueue(
        trade_summary,
        true);
  }
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
  LOG(INFO)(FMT_STRING("market_data_status={}"), _market_data_status);
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
  LOG(INFO)(FMT_STRING("order_manager_status={}"), _order_manager_status);
}

void Gateway::begin_download() {
  assert(_download == Download::NONE);
  assert(_order_manager_status == GatewayStatus::LOGIN_SENT);
  assert(_market_data_status == GatewayStatus::LOGIN_SENT);
  update_order_manager(GatewayStatus::DOWNLOADING);
  update_market_data(GatewayStatus::DOWNLOADING);
  LOG(INFO)("Download:");
  subscribe_instrument();
}

void Gateway::check_download() {
  assert(_download != Download::NONE);
  assert(_download != Download::READY);
  switch (_download) {
    case Download::NONE:
      assert(false);
      break;
    case Download::ACCOUNTS: {
      LOG(INFO)("Download accounts COMPLETED");
      update_order_manager(GatewayStatus::READY);
      subscribe_order_book_l2();
      break;
    }
    case Download::PRODUCTS: {
      LOG(INFO)("Download products COMPLETED");
      // download_accounts();
      update_order_manager(GatewayStatus::READY);
      subscribe_order_book_l2();
      break;
    }
    case Download::ORDER_BOOKS: {
      LOG(INFO)("Download order books COMPLETED");
      update_market_data(GatewayStatus::READY);
      LOG(INFO)("Download COMPLETED");
      _download = Download::READY;
      LOG(INFO)("********************************************");
      LOG(INFO)("***   DEFAULT LOGGING IS NOW MINIMAL     ***");
      LOG(INFO)("***                                      ***");
      LOG(INFO)("***   verbose logging can be enabled     ***");
      LOG(INFO)("***   by setting the ROQ_v environment   ***");
      LOG(INFO)("***   variable to a non-zero value       ***");
      LOG(INFO)("********************************************");
      break;
    }
    case Download::READY:
      assert(false);
      break;
  }
}

void Gateway::download_accounts() {
  assert(_download != Download::READY);
  LOG(INFO)("Download accounts...");
  // _rest.get_accounts();
  _download = Download::ACCOUNTS;
}

void Gateway::subscribe_instrument() {
  assert(_download != Download::READY);
  LOG(INFO)("Download products...");
  // XXX _rest.get_products();
  _websocket.subscribe("instrument");
  _download = Download::PRODUCTS;
}

void Gateway::subscribe_order_book_l2() {
  assert(_download != Download::READY);
  LOG(INFO)("Download order books");
  _websocket.subscribe("orderBookL2", _symbols);
  _download = Download::ORDER_BOOKS;

  // XXX not here
  _websocket.subscribe("funding", _symbols);
  _websocket.subscribe("liquidation", _symbols);
  _websocket.subscribe("quote", _symbols);
  _websocket.subscribe("settlement", _symbols);
  _websocket.subscribe("trade", _symbols);
  // XXX private
  _websocket.subscribe("execution", _symbols);
  _websocket.subscribe("order", _symbols);
  _websocket.subscribe("margin", _symbols);
  _websocket.subscribe("position", _symbols);
  // XXX other
  // cancelAllAfter
  // authKeyExpires
}

std::pair<double, double> Gateway::find_price(
    json::Action action,
    uint64_t id,
    double price,
    double size) {
  auto result = std::numeric_limits<double>::quiet_NaN();
  auto iter = _price_lookup.find(id);
  switch (action) {
    case json::Action::UNDEFINED:
    case json::Action::UNKNOWN:
      LOG(FATAL)("Unexpected");
      break;
    case json::Action::PARTIAL:
      LOG_IF(FATAL, std::isfinite(price) == false)(
          FMT_STRING("id={} price={}: expected finite"), id, price);
      if (iter == _price_lookup.end()) {
        iter = _price_lookup.emplace(id, price).first;
      } else {
        auto diff = std::fabs(price - (*iter).second);
        LOG_IF(FATAL, diff > TOLERANCE)(
            FMT_STRING("id={} price={}: id already exists as price={}"),
            id, price, (*iter).second);
      }
      result = (*iter).second;
      break;
    case json::Action::INSERT:
      LOG_IF(FATAL, std::isfinite(price) == false)(
          FMT_STRING("id={} price={}"), id, price);
      if (iter == _price_lookup.end()) {
        iter = _price_lookup.emplace(id, price).first;
      } else {
        auto diff = std::fabs(price - (*iter).second);
        LOG_IF(FATAL, diff > TOLERANCE)(
            FMT_STRING("id={} price={}: id already exists as price={}"),
            id, price, (*iter).second);
      }
      result = (*iter).second;
      break;
    case json::Action::UPDATE:
      LOG_IF(FATAL, iter == _price_lookup.end())(
            FMT_STRING("id={} price={}: doesn't exist"), id, price);
      LOG_IF(FATAL, std::isnan(price) == false)(
          FMT_STRING("id={} price={}: expected nan"), id, price);
      LOG_IF(FATAL, std::isnan(size) || std::fabs(size) < TOLERANCE)(
          FMT_STRING("id={} price={} : expected size"), id, price);
      result = (*iter).second;
      break;
    case json::Action::DELETE:
      LOG_IF(FATAL, iter == _price_lookup.end())(
            FMT_STRING("id={} price={}: id doesn't exist"), id, price);
      LOG_IF(FATAL, std::isnan(size) == false && std::fabs(size) > TOLERANCE)(
          FMT_STRING("id={} price={} : expected size"), id, price);
      result = (*iter).second;
      _price_lookup.erase(iter);
      break;
  }
  assert(std::isnan(result) == false);
  return std::make_pair(
      result,
      std::isnan(size) ? 0.0 : size);
}

void Gateway::publish_market_by_price(
    const std::string_view& symbol,
    size_t bid_length,
    size_t ask_length,
    bool snapshot,
    bool is_last) {
  assert(symbol.empty() == false);
  if ((bid_length + ask_length) == 0)
    return;
  LOG_IF(INFO, snapshot)(
      FMT_STRING("Received market data snapshot for symbol=\"{}\""),
      symbol);
  MarketByPrice market_by_price {
    .exchange = FLAGS_exchange,
    .symbol = symbol,
    .bids = {
      .items = _bid.data(),
      .length = bid_length,
    },
    .asks = {
      .items = _ask.data(),
      .length = ask_length,
    },
    .snapshot = snapshot,
    .exchange_time_utc = {},
    };
  VLOG(1)(FMT_STRING("market_by_price={}"), market_by_price);
  enqueue(
      market_by_price,
      is_last);
}

template <typename T>
void Gateway::enqueue(
    const T& event,
    bool is_last) {
  auto now = core::get_system_clock();
  _dispatcher(
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
  _dispatcher(
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
