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
    _product_cache.clear();
    _symbols.clear();
  }
}

void Gateway::operator()(const json::Error&) {
}

void Gateway::operator()(const json::Heartbeat&) {
}

void Gateway::operator()(const json::Subscriptions&) {
  // check_download();  // TODO(thraneh): can also happen after download
}

void Gateway::operator()(const json::Status& status) {
  roq::span products(
      status.products.items,
      status.products.length);
  for (auto& product : products) {
    if (_dispatcher.discard_symbol(product.id))  // FIXME(thraneh): NOT AT RUNTIME !!!
      continue;
    (*this)(product);
  }
}

void Gateway::operator()(const json::Received&) {
  // note!
  //   order has been received by exchange but is not working yet,
  //   i.e. it is not *in* the order book
}

void Gateway::operator()(const json::Open& open) {
  MBOUpdate update {
    .price = open.price,
    .remaining_quantity = open.remaining_size,
    .action = OrderUpdateAction::NEW,
    .priority = 0,
    .order_id = {},
  };
  core::copy_to(open.order_id, update.order_id);
  MarketByOrder market_by_order {
    .exchange = FLAGS_exchange,
    .symbol = open.product_id,
    .bids = {
      &update,
      open.side == api::Side::BUY ? size_t{1} : size_t{0},
    },
    .asks = {
      &update,
      open.side == api::Side::SELL ? size_t{1} : size_t{0},
    },
    .snapshot = false,
    .exchange_time_utc = open.time,
  };
  enqueue(
      market_by_order,
      true);
}

// note! will receive same sequence number several times
// (1) match *without* maker/taker
// (2) ticker
// (3) match *with* maker/taker
// (4) match *with* maker/taker (duplicate)

void Gateway::operator()(
    const json::Match& match,
    bool trade_summary,
    bool trade_update) {
  auto maker_side = api::map(match.side);
  auto taker_side = invert(maker_side);
  Trade trade {
    .side = taker_side,  // our convention: aggressor (taker)
    .price = match.price,
    .quantity = match.size,
    .trade_id = {},
  };
  // convert trade_id (back) to string
  core::view_t view(
      trade.trade_id,
      trade.trade_id + sizeof(trade.trade_id));
  core::charconv::to_string(
      std::back_inserter(view),
      match.trade_id);
  if (trade_summary) {
    TradeSummary trade_summary {
      .exchange = FLAGS_exchange,
      .symbol = match.product_id,
      .trades = {
        .items = &trade,
        .length = 1,
      },
      .exchange_time_utc = match.time,
    };
    enqueue(
        trade_summary,
        true);
  }
  if (trade_update) {
    // fill (as maker)
    if (unlikely(match.maker_user_id.empty() == false)) {
      auto iter = find_order_mapping(match.maker_order_id);
      if (unlikely(iter == _order_mapping.end())) {
        LOG(WARNING)("*** EXTERNAL ORDER ***");
      } else {
        auto& order_mapping = (*iter).second;
        auto trade_id = _dispatcher.next_trade_id();
        LOG_IF(WARNING, match.product_id.compare(order_mapping.symbol()))(
            "Wrong symbol: received=\"{}\", expected=\"{}\"",
            match.product_id,
            order_mapping.symbol());
        LOG_IF(WARNING, maker_side != order_mapping.side())(
            "Wrong side: received={}, expected={}",
            maker_side,
            order_mapping.side());
        TradeUpdate trade_update {
          .account = _account,
          .trade_id = trade_id,
          .order_id = order_mapping.user_order_id(),
          .exchange = FLAGS_exchange,
          .symbol = order_mapping.symbol(),
          .side = order_mapping.side(),
          .quantity = match.size,
          .price = match.price,
          .position_effect = PositionEffect::UNDEFINED,
          .order_template = std::string(),
          .create_time_utc = match.time,
          .update_time_utc = match.time,
          .gateway_order_id = order_mapping.gateway_order_id(),
          .gateway_trade_id = trade_id,
          .external_order_id = order_mapping.exchange_order_id(),
          .external_trade_id = trade.trade_id,
        };
        enqueue(
            order_mapping.user_id(),
            trade_update,
            true);
      }
    }
    // fill (as taker)
    if (unlikely(match.taker_user_id.empty() == false)) {
      auto iter = find_order_mapping(match.taker_order_id);
      if (unlikely(iter == _order_mapping.end())) {
        LOG(WARNING)("*** EXTERNAL ORDER ***");
      } else {
        auto& order_mapping = (*iter).second;
        auto trade_id = _dispatcher.next_trade_id();
        LOG_IF(WARNING, match.product_id.compare(order_mapping.symbol()))(
            "Wrong symbol: received=\"{}\", expected=\"{}\"",
            match.product_id,
            order_mapping.symbol());
        LOG_IF(WARNING, taker_side != order_mapping.side())(
            "Wrong side: received={}, expected={}",
            taker_side,
            order_mapping.side());
        TradeUpdate trade_update {
          .account = _account,
          .trade_id = trade_id,
          .order_id = order_mapping.user_order_id(),
          .exchange = FLAGS_exchange,
          .symbol = order_mapping.symbol(),
          .side = order_mapping.side(),
          .quantity = match.size,
          .price = match.price,
          .position_effect = PositionEffect::UNDEFINED,
          .order_template = std::string(),
          .create_time_utc = match.time,
          .update_time_utc = match.time,
          .gateway_order_id = order_mapping.gateway_order_id(),
          .gateway_trade_id = trade_id,
          .external_order_id = order_mapping.exchange_order_id(),
          .external_trade_id = trade.trade_id,
        };
        enqueue(
            order_mapping.user_id(),
            trade_update,
            true);
      }
    }
  }
}

void Gateway::operator()(const json::Done& done) {
  MBOUpdate update {
    .price = done.price,
    .remaining_quantity = done.remaining_size,
    .action = OrderUpdateAction::REMOVE,
    .priority = 0,
    .order_id = {},
  };
  core::copy_to(done.order_id, update.order_id);
  MarketByOrder market_by_order {
    .exchange = FLAGS_exchange,
    .symbol = done.product_id,
    .bids = {
      &update,
      done.side == api::Side::BUY ? size_t{1} : size_t{0},
    },
    .asks = {
      &update,
      done.side == api::Side::SELL ? size_t{1} : size_t{0},
    },
    .snapshot = false,
    .exchange_time_utc = done.time,
  };
  enqueue(
      market_by_order,
      true);
}

void Gateway::operator()(const json::Change& change) {
  MBOUpdate update {
    .price = change.price,
    .remaining_quantity = change.new_size,
    .action = OrderUpdateAction::MODIFY,
    .priority = 0,
    .order_id = {},
  };
  core::copy_to(change.order_id, update.order_id);
  MarketByOrder market_by_order {
    .exchange = FLAGS_exchange,
    .symbol = change.product_id,
    .bids = {
      &update,
      change.side == api::Side::BUY ? size_t{1} : size_t{0},
    },
    .asks = {
      &update,
      change.side == api::Side::SELL ? size_t{1} : size_t{0},
    },
    .snapshot = false,
    .exchange_time_utc = change.time,
  };
  enqueue(
      market_by_order,
      true);
}

void Gateway::operator()(const json::Activate&) {
}

void Gateway::operator()(const json::Ticker& ticker) {
  auto& cache = _product_cache[ticker.product_id];
  if (cache.update_daily_statistics(ticker)) {
    DailyStatistics daily_statistics {
      .exchange = FLAGS_exchange,
      .symbol = ticker.product_id,
      .open_price = cache.open_price,
      .settlement_price = std::numeric_limits<double>::quiet_NaN(),
      .close_price = std::numeric_limits<double>::quiet_NaN(),
      .open_interest = std::numeric_limits<double>::quiet_NaN(),
      .exchange_time_utc = {},
    };
    enqueue(
        daily_statistics,
        true);
  }
  if (cache.update_session_statistics(ticker)) {
    SessionStatistics session_statistics {
      .exchange = FLAGS_exchange,
      .symbol = ticker.product_id,
      .pre_open_interest = std::numeric_limits<double>::quiet_NaN(),
      .pre_settlement_price = std::numeric_limits<double>::quiet_NaN(),
      .pre_close_price = std::numeric_limits<double>::quiet_NaN(),
      .highest_traded_price = cache.highest_traded_price,
      .lowest_traded_price = cache.lowest_traded_price,
      .upper_limit_price = std::numeric_limits<double>::quiet_NaN(),
      .lower_limit_price = std::numeric_limits<double>::quiet_NaN(),
      .exchange_time_utc = {},
    };
    enqueue(
        session_statistics,
        true);
  }
}

void Gateway::operator()(const json::Snapshot& snapshot) {
  LOG(INFO)(
      "Market data snapshot symbol=\"{}\"",
      snapshot.product_id);
  size_t bid_length = 0, ask_length = 0;
  roq::span bids(
      snapshot.bids.items,
      snapshot.bids.length);
  for (auto& item : bids) {
    if (std::fabs(item.size) < TOLERANCE)
      throw std::runtime_error("Unexpected size");
    mbp_update(_bid, bid_length, item);
  }
  roq::span asks(
      snapshot.asks.items,
      snapshot.asks.length);
  for (auto& item : asks) {
    if (std::fabs(item.size) < TOLERANCE)
      throw std::runtime_error("Unexpected size");
    mbp_update(_ask, ask_length, item);
  }
  if (bid_length > 0 || ask_length > 0) {
    MarketByPrice market_by_price {
      .exchange = FLAGS_exchange,
      .symbol = snapshot.product_id,
      .bids = {
        .items = _bid.data(),
        .length = bid_length,
      },
      .asks = {
        .items = _ask.data(),
        .length = ask_length,
      },
      .snapshot = true,  // reset
      .exchange_time_utc = {},
    };
    enqueue(
        market_by_price,
        true);
  }
}

void Gateway::operator()(const json::L2Update& l2update) {
  size_t bid_length = 0, ask_length = 0;
  roq::span changes(
      l2update.changes.items,
      l2update.changes.length);
  for (auto& item : changes) {
    switch (item.side) {
      case api::Side::BUY:
        mbp_update(_bid, bid_length, item);
        break;
      case api::Side::SELL:
        mbp_update(_ask, ask_length, item);
        break;
      default:
        LOG(WARNING)("Unsupported: {}", item);
        break;
    }
  }
  if (bid_length > 0 || ask_length > 0) {
    MarketByPrice market_by_price {
      .exchange = FLAGS_exchange,
      .symbol = l2update.product_id,
      .bids = {
        .items = _bid.data(),
        .length = bid_length,
      },
      .asks = {
        .items = _ask.data(),
        .length = ask_length,
      },
      .snapshot = false,  // incremental
      .exchange_time_utc = l2update.time,
    };
    enqueue(
        market_by_price,
        true);
  }
}

void Gateway::operator()(const json::LastMatch&) {
  // note!
  //   this looks like a snapshot (confirm?)
}

// rest

void Gateway::operator()(const Rest&) {
}

void Gateway::operator()(const json::Products& products) {
  size_t product_count = 0;
  roq::span products_(
      products.items,
      products.length);
  if (products_.empty() == false) {
    assert(_symbols.empty());
    _symbols.reserve(products_.size());
    for (auto& product : products_) {
      if (_dispatcher.discard_symbol(product.id))
        continue;
      _symbols.emplace_back(product.id);
      (*this)(product);
      ++product_count;
    }
  }
  LOG(INFO)(
      "- products: {} (/{})",
      product_count,
      products.length);
  check_download();
}

void Gateway::operator()(const json::Accounts& accounts) {
  size_t account_count = 0;
  roq::span accounts_(
      accounts.items,
      accounts.length);
  for (auto& account : accounts_) {
    // TODO(thraneh): check currency...
    (*this)(account);
    ++account_count;
  }
  LOG(INFO)(
      "- accounts: {} (/{})",
      account_count,
      accounts.length);
  check_download();
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
  _rest.get_products();
  _download = Download::PRODUCTS;
}

void Gateway::download_accounts() {
  assert(_market_data_status == GatewayStatus::DOWNLOADING);
  LOG(INFO)("Download accounts...");
  _rest.get_accounts();
  _download = Download::ACCOUNTS;
}

void Gateway::subscribe() {
  assert(_market_data_status == GatewayStatus::READY);
  LOG(INFO)("Subscribe channels");
  _websocket.subscribe(_symbols);
}

void Gateway::operator()(const json::Product& product) {
  auto& cache = _product_cache[product.id];
  if (cache.update_reference_data(product)) {
    ReferenceData reference_data {
      .exchange = FLAGS_exchange,
      .symbol = product.id,
      .security_type = SecurityType::FX_SPOT,
      .currency = product.base_currency,
      .settlement_currency = product.quote_currency,
      .commission_currency = product.base_currency,  // TODO(thraneh): double-check
      .tick_size = cache.tick_size,
      .limit_up = std::numeric_limits<double>::quiet_NaN(),
      .limit_down = std::numeric_limits<double>::quiet_NaN(),
      .multiplier = DEFAULT_MULTIPLIER,
      .min_trade_vol = product.base_min_size,
      .option_type = OptionType::UNDEFINED,
      .strike_currency = std::string_view(),
      .strike_price = std::numeric_limits<double>::quiet_NaN(),
    };
    enqueue(
        reference_data,
        true);
  }
  if (cache.update_market_status(product)) {
    MarketStatus market_status {
      .exchange = FLAGS_exchange,
      .symbol = product.id,
      .trading_status = cache.trading_status,
    };
    enqueue(
        market_status,
        true);
  }
}

void Gateway::operator()(const json::Account& account) {
  VLOG(1)("account={}", account);
  FundsUpdate funds_update {
    .account = _account,
    .currency = account.currency,
    .balance = account.available,
    .hold = account.hold,
  };
  enqueue(
      funds_update,
      true);
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
