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
    .side = json::map(item.side),  // XXX check
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
      _web_socket {
        .connection = {
          *this,
          config,
          _random,
          _base,
          _dns_base,
          _ssl_context,
        },
        .download = WebSocketDownload(
            std::chrono::seconds { FLAGS_download_timeout_secs }),
      },
      _rest {
        .connection = {
          *this,
          config,
          _random,
          _base,
          _dns_base,
          _ssl_context,
        },
      },
      _bid(FLAGS_max_depth),
      _ask(FLAGS_max_depth),
      _trade(FLAGS_max_trades) {
  LOG_IF(WARNING, FLAGS_cancel_on_disconnect == false)(
      "Orders will *NOT* be cancelled on disconnect");
}

void Gateway::operator()(const StartEvent& event) {
  LOG(INFO)("Starting the gateway...");
  _web_socket.connection(event);
  _rest.connection(event);
}

void Gateway::operator()(const StopEvent& event) {
  LOG(INFO)("Stopping the gateway...");
  _rest.connection(event);
  _web_socket.connection(event);
}

void Gateway::operator()(const TimerEvent& event) {
  // web socket
  _web_socket.connection(event);
  // rest
  if (_web_socket.download.has_expired()) {
    LOG(WARNING)("Rest download has timed out");
    _web_socket.download.reset();
    _rest.connection.close();
  } else {
    _rest.connection(event);
  }
  _base.loop(EVLOOP_NONBLOCK);
}

void Gateway::operator()(const ConnectionStatusEvent&) {
}

void Gateway::operator()(
    const CreateOrderEvent& event,
    const std::string_view& request_id,
    uint32_t gateway_order_id) {
  (void) gateway_order_id;  // avoid warning
  /*
  core::stack::Buffer<char, 36> buffer;
  fmt::format_to(
      std::back_inserter(buffer),
      "roq-{}-{}-{}",
      gateway_order_id,
      message_info.source,
      create_order.order_id);
  std::string_view cl_ord_id(
      buffer.data(),
      buffer.size());
  */
  _rest.connection.create_order(
      event.create_order,
      request_id);
}

void Gateway::operator()(
    const ModifyOrderEvent& event,
    const std::string_view& request_id,
    const server::OMS_Order& order) {
  _rest.connection.modify_order(
      event.modify_order,
      request_id,
      order);
}

void Gateway::operator()(
    const CancelOrderEvent& event,
    const std::string_view& request_id,
    const server::OMS_Order& order) {
  _rest.connection.cancel_order(
      event.cancel_order,
      request_id,
      order);
}

void Gateway::operator()(Metrics& metrics) {
  _web_socket.connection(metrics);
  _rest.connection(metrics);
}

// ws

void Gateway::operator()(const WebSocket&) {
  if (_web_socket.connection.ready()) {
    _web_socket.download.check(
        WebSocketDownload::State::UNDEFINED,
        [this](auto state) -> uint32_t {
          return download(state);
        });
  } else {
    _web_socket.download.reset();  // XXX here?
    _symbols.clear();
    _snapshot = {};
  }
}

void Gateway::operator()(
    const json::Action action,
    const json::Instrument& instrument) {
  switch (action) {
    case json::Action::UNDEFINED:
    case json::Action::UNKNOWN:
      LOG(FATAL)("Unexpected");
      break;
    case json::Action::PARTIAL:
      if (_snapshot.instrument == false) {
        _snapshot.instrument = true;
        // assert(_download != Download::READY);
        size_t security_count = 0;
        for (auto& item : instrument.data) {
          if (_dispatcher.discard_symbol(item.symbol)) {
            VLOG(1)(
                FMT_STRING(R"(Drop symbol="{}")"),
                item.symbol);
            continue;
          }
          ++security_count;
          auto& product = find_product(item);
          auto reference_data = product.create_reference_data(item);
          enqueue(reference_data, false);
          auto market_status = product.create_market_status(item);
          enqueue(market_status, true);
        }
        VLOG(1)(
            FMT_STRING(R"(- securities: {} (/{}))"),
            security_count,
            instrument.data.size());
        _web_socket.download.check_relaxed(
            WebSocketDownload::State::INSTRUMENT,
            [this](auto state) -> uint32_t {
              return download(state);
            });
      }
      break;
    case json::Action::INSERT:
      // drop
      break;
    case json::Action::UPDATE: {
      for (auto& item : instrument.data) {
        if (_dispatcher.discard_symbol(item.symbol))
          continue;
        auto& product = find_product(item);
        // XXX check if dirty
        auto market_status = product.create_market_status(item);
        enqueue(market_status, true);
      }
      break;
    }
    case json::Action::DELETE:
      // drop
      break;
  }
}

auto compute_request_status(
    auto request_type,
    auto ord_status) {
  switch (ord_status) {
    case json::OrdStatus::UNDEFINED:
    case json::OrdStatus::UNKNOWN:
      break;
    case json::OrdStatus::NEW: {
      switch (request_type) {
        case RequestType::UNDEFINED:
          LOG(WARNING)("*** EXTERNAL ACTION ***");
          break;
        case RequestType::CREATE_ORDER:
          return RequestStatus::ACCEPTED;
        case RequestType::MODIFY_ORDER:
        case RequestType::CANCEL_ORDER:
          DLOG(FATAL)("UNEXPECTED");
        break;
      }
      break;
    }
  }
  return RequestStatus::UNDEFINED;
}

void Gateway::operator()(
    const json::Action action,
    const json::Order& order) {
  DLOG(INFO)(
      FMT_STRING(R"(action={} order={})"),
      action,
      order);
  for (auto& iter : order.data) {
    auto order_status = iter.working_indicator
      ? OrderStatus::WORKING
      : OrderStatus::COMPLETED;
    DLOG(INFO)(
        FMT_STRING(R"(order_status={})"),
        order_status);

    server::OMS_Lookup order_lookup {
      .symbol = iter.symbol,
      .side = json::map(iter.side),
      .status = order_status,
      .price = iter.price,
      .remaining_quantity = iter.leaves_qty,
      .traded_quantity = iter.cum_qty,
      .timestamp = iter.timestamp,
      .external_order_id = iter.order_id,
    };

    auto found = _dispatcher.find_order(
        iter.cl_ord_id,
        std::string_view(),  // XXX ?????
        order_lookup,
        [&](const auto& order, auto& result) {
      // constexpr auto origin = Origin::EXCHANGE;
      // auto status = RequestStatus::UNDEFINED;

      // XXX ord_rej_reason
      result.request_status = compute_request_status(
          order.request_type,
          iter.ord_status);

      if (result.request_status != RequestStatus::UNDEFINED) {
        result.origin = Origin::EXCHANGE;
        result.error = Error::UNKNOWN;
        result.text = iter.text;
      }
    });

    if (found == false) {
      LOG(WARNING)("*** EXTERNAL ORDER ***");
    }
  }
}

void Gateway::operator()(
    const json::Action action,
    const json::OrderBookL2& order_book_l2) {
  assert(action != json::Action::UNKNOWN);
  auto snapshot = action == json::Action::PARTIAL;
  // note!
  //   first partial update will include *all* instruments
  //   drop everything received before partial (see: API documentation)
  if (snapshot == false &&
      _snapshot.order_book_l2 == false)
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
        action,
        item.id,
        item.price,
        item.size);
    if (std::isfinite(price_size.first)) {
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
    } else {
      LOG(WARNING)(
          FMT_STRING(
            R"(Closing web-socket: )"
            R"(unexpected action={} id={} price={} size={})"),
          action,
          item.id,
          item.price,
          item.size);
      _web_socket.connection.close();
      return;
    }
  }
  if (unlikely(success == false)) {
    LOG(FATAL)(
        FMT_STRING(
          R"(Insufficient bid/ask array size(s): )"
          R"(len(bid)={}/{}, len(ask)={}/{})"),
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
  if (snapshot) {
      _snapshot.order_book_l2 = true;
      _web_socket.download.check_relaxed(
          WebSocketDownload::State::ORDER_BOOK_L2,
          [this](auto state) -> uint32_t {
            return download(state);
          });
  }
}

void Gateway::operator()(
    const json::Action,
    const json::Position&) {
}

void Gateway::operator()(
    const json::Action,
    const json::Quote& quote) {
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
    VLOG(1)(
        FMT_STRING(R"(top_of_book={})"),
        top_of_book);
    enqueue(
        top_of_book,
        true);  // XXX not always correct
  }
}

void Gateway::operator()(
    const json::Action,
    const json::Settlement&) {
}

void Gateway::operator()(
    const json::Action action,
    const json::Trade& trade) {
  if (action != json::Action::INSERT)
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
        VLOG(1)(
            FMT_STRING(R"(trade_summary={})"),
            trade_summary);
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
          R"(Insufficient trade array size: )"
          R"(len(trade)={}/{})"),
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
    VLOG(1)(
        FMT_STRING(R"(trade_summary={})"),
        trade_summary);
    enqueue(
        trade_summary,
        true);
  }
}

// rest

uint32_t Gateway::download(WebSocketDownload::State state) {
  switch (state) {
    case WebSocketDownload::State::UNDEFINED:
      break;
    case WebSocketDownload::State::ACCOUNTS: {
      // download_accounts();
      // return 1;
      return 0;
    }
    case WebSocketDownload::State::INSTRUMENT: {
      subscribe_instrument();
      return 1;
    }
    case WebSocketDownload::State::ORDER_BOOK_L2: {
      subscribe_order_book_l2();
      return 1;
    }
    case WebSocketDownload::State::DONE:
      update_order_manager(GatewayStatus::READY);
      update_market_data(GatewayStatus::READY);
      return 0;
  }
  assert(false);
  return 0;
}

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
  LOG(INFO)(
      FMT_STRING(R"(market_data_status={})"),
      _market_data_status);
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
  LOG(INFO)(
      FMT_STRING(R"(order_manager_status={})"),
      _order_manager_status);
}

void Gateway::download_accounts() {
  _rest.connection.get_accounts();
}

void Gateway::subscribe_instrument() {
  _web_socket.connection.subscribe("instrument");
}

void Gateway::subscribe_order_book_l2() {
  _web_socket.connection.subscribe("orderBookL2", _symbols);

  // XXX not here
  _web_socket.connection.subscribe("funding", _symbols);
  _web_socket.connection.subscribe("liquidation", _symbols);
  _web_socket.connection.subscribe("quote", _symbols);
  _web_socket.connection.subscribe("settlement", _symbols);
  _web_socket.connection.subscribe("trade", _symbols);
  // XXX private
  _web_socket.connection.subscribe("execution", _symbols);
  _web_socket.connection.subscribe("order", _symbols);
  _web_socket.connection.subscribe("margin", _symbols);
  _web_socket.connection.subscribe("position", _symbols);
  // XXX other
  // cancelAllAfter
  // authKeyExpires
}

const Product& Gateway::find_product(const json::InstrumentItem& item) {
  auto iter = _product_cache.find(item.symbol);
  if (iter == _product_cache.end()) {
    iter = _product_cache.emplace(
        std::string(item.symbol),
        item).first;
  } else {
    (*iter).second.update(item);
  }
  assert(iter != _product_cache.end());
  return (*iter).second;
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
    case json::Action::INSERT:
      if (std::isnan(price) == false &&
          std::isnan(size) == false) {
        if (iter == _price_lookup.end()) {
          iter = _price_lookup.emplace(id, price).first;
          result = (*iter).second;
        } else {
          auto previous = (*iter).second;
          if (std::fabs(price - previous) < TOLERANCE) {
            result = (*iter).second;
          } else {
            // exists as a different price ==> fail
          }
        }
      } else {
        // unexpected price or size ==> fail
        DLOG(FATAL)(
            FMT_STRING(R"(action={} id={} price={} size={})"),
            action,
            id,
            price,
            size);
      }
      break;
    case json::Action::UPDATE:
      if (std::isnan(price) &&
          std::isnan(size) == false &&
          std::fabs(size) > TOLERANCE) {
        if (iter != _price_lookup.end()) {
          result = (*iter).second;
        } else {
          // unable to find the cached price ==> fail
        }
      } else {
        // unexpected price or size ==> fail
      }
      break;
    case json::Action::DELETE:
      if (std::isnan(price) &&
          (std::isnan(size) || std::fabs(size) < TOLERANCE)) {
        if (iter != _price_lookup.end()) {
          result = (*iter).second;
          _price_lookup.erase(iter);
        } else {
          // unable to find the cached price ==> fail
        }
      } else {
        // unexpected price or size ==> fail
      }
  }
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
      FMT_STRING(R"(Received market data snapshot for symbol="{}")"),
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
  VLOG(1)(
      FMT_STRING(R"(market_by_price={})"),
      market_by_price);
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

}  // namespace bitmex
}  // namespace roq
