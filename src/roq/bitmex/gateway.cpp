/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/bitmex/gateway.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "roq/logging.h"

#include "roq/core/charconv.h"
#include "roq/core/clock.h"
#include "roq/core/utils.h"
#include "roq/core/view.h"

#include "roq/core/http/exceptions.h"

#include "roq/bitmex/flags.h"

#include "roq/bitmex/json/utils.h"

using namespace roq::literals;

namespace roq {
namespace bitmex {

namespace {
constexpr const auto DEFAULT_MULTIPLIER = 1.0;
constexpr const auto TOLERANCE = 1.0e-10;

template <typename C, typename T>
static bool mbp_update(C &data, size_t &offset, const T &item) {
  if (offset >= data.size())
    return false;
  auto &obj = data[offset];
  new (&obj) MBPUpdate{
      .price = item.first,
      .quantity = item.second,
  };
  ++offset;
  return offset <= data.size();
}

template <typename C, typename T>
static bool trade_update(C &data, size_t &offset, const T &item) {
  if (offset >= data.size())
    return false;
  auto &obj = data[offset];
  new (&obj) Trade{
      .side = json::map(item.side),  // XXX check
      .price = item.price,
      .quantity = item.size,
      .trade_id = item.trd_match_id,
  };
  ++offset;
  return offset <= data.size();
}

template <typename C, typename T>
static bool fill_update(server::Dispatcher &dispatcher, C &data, size_t &offset, const T &item) {
  auto trade_id = dispatcher.next_trade_id();
  auto &obj = data[offset];
  new (&obj) Fill{
      .quantity = item.last_qty,
      .price = item.last_px,
      .trade_id = trade_id,
      .gateway_trade_id = trade_id,
      .external_trade_id = item.trd_match_id,
  };
  ++offset;
  return offset < data.size();
}
}  // namespace

Gateway::Gateway(server::Dispatcher &dispatcher, const Config &config)
    : dispatcher_(dispatcher), account_(config.get_account()), security_(config),
      web_socket_{
          .connection =
              {
                  *this,
                  config,
                  security_,
                  context_,
              },
          .download = WebSocketDownload(
              std::chrono::seconds{Flags::ws_request_timeout_secs()},
              [this](auto state) { return download(state); }),
      },
      rest_{
          .connection =
              {
                  *this,
                  config,
                  security_,
                  context_,
              },
      },
      bid_(Flags::cache_mbp_max_depth()), ask_(Flags::cache_mbp_max_depth()),
      trade_(Flags::cache_trades_max_depth()), fill_(Flags::cache_fills_max_depth()) {
  LOG_IF(WARNING, Flags::ws_cancel_on_disconnect() == false)
  ("Orders will *NOT* be cancelled on disconnect"_sv);
}

void Gateway::operator()(const Event<Start> &event) {
  LOG(INFO)("Starting the gateway..."_sv);
  web_socket_.connection(event);
  rest_.connection(event);
}

void Gateway::operator()(const Event<Stop> &event) {
  LOG(INFO)("Stopping the gateway..."_sv);
  rest_.connection(event);
  web_socket_.connection(event);
}

void Gateway::operator()(const Event<Timer> &event) {
  // web socket
  web_socket_.connection(event);
  // rest
  rest_.connection(event);
  context_.dispatch(true);
}

void Gateway::operator()(const Event<Connection> &) {
}

void Gateway::operator()(
    const Event<CreateOrder> &event,
    const std::string_view &request_id,
    [[maybe_unused]] uint32_t gateway_order_id) {
  rest_.connection.create_order(event.value, request_id, [this](auto &promise) {
    try {
      (*this)(promise.get());
      /*
      case core::http::Status::BAD_REQUEST:   // 400
      case core::http::Status::UNAUTHORIZED:  // 401
      case core::http::Status::FORBIDDEN:     // 403
      case core::http::Status::NOT_FOUND:     // 404
      */
    } catch (NetworkError &e) {
      // XXX send ack failure
      LOG(FATAL)(R"(Unexpected what="{}")"_fmt, e.what());
    }
  });
}

void Gateway::operator()(
    const Event<ModifyOrder> &event,
    const std::string_view &request_id,
    const server::OMS_Order &order) {
  rest_.connection.modify_order(event.value, request_id, order, [this](auto &promise) {
    try {
      (*this)(promise.get());
      /*
      case core::http::Status::BAD_REQUEST:   // 400
      case core::http::Status::UNAUTHORIZED:  // 401
      case core::http::Status::FORBIDDEN:     // 403
      case core::http::Status::NOT_FOUND:     // 404
      */
    } catch (NetworkError &e) {
      // XXX send ack failure
      LOG(FATAL)(R"(Unexpected what="{}")"_fmt, e.what());
    }
  });
}

void Gateway::operator()(
    const Event<CancelOrder> &event,
    const std::string_view &request_id,
    const server::OMS_Order &order) {
  rest_.connection.cancel_order(event.value, request_id, order, [this](auto &promise) {
    try {
      (*this)(promise.get());
      /*
      case core::http::Status::BAD_REQUEST:   // 400
      case core::http::Status::UNAUTHORIZED:  // 401
      case core::http::Status::FORBIDDEN:     // 403
      case core::http::Status::NOT_FOUND:     // 404
      */
    } catch (NetworkError &e) {
      // XXX send ack failure
      LOG(FATAL)(R"(Unexpected what="{}")"_fmt, e.what());
    }
  });
}

void Gateway::operator()(metrics::Writer &writer) {
  web_socket_.connection(writer);
  rest_.connection(writer);
}

// all

void Gateway::operator()(
    const ExternalLatency &external_latency, const server::TraceInfo &trace_info) {
  create_trace_and_dispatch(trace_info, external_latency, dispatcher_);
}

// ws

void Gateway::operator()(const WebSocket &) {
  if (web_socket_.connection.ready()) {
    web_socket_.download.begin();
  } else {
    web_socket_.download.reset();
    symbols_.clear();
    snapshot_ = {};
  }
}

auto compute_request_status_2(RequestType request_type, json::ExecType exec_type) {
  switch (exec_type) {
    case json::ExecType::UNDEFINED:
    case json::ExecType::UNKNOWN:
      break;
    case json::ExecType::NEW: {
      switch (request_type) {
        case RequestType::UNDEFINED:
          LOG(WARNING)("*** EXTERNAL ACTION ***"_sv);
          break;
        case RequestType::CREATE_ORDER:
          return RequestStatus::ACCEPTED;
        case RequestType::MODIFY_ORDER:
        case RequestType::CANCEL_ORDER:
          DLOG(FATAL)("UNEXPECTED"_sv);
          break;
      }
      break;
    }
    case json::ExecType::REPLACED: {
      switch (request_type) {
        case RequestType::UNDEFINED:
          LOG(WARNING)("*** EXTERNAL ACTION ***"_sv);
          break;
        case RequestType::MODIFY_ORDER:
          return RequestStatus::ACCEPTED;
        case RequestType::CREATE_ORDER:
        case RequestType::CANCEL_ORDER:
          DLOG(FATAL)("UNEXPECTED"_sv);
          break;
      }
      break;
    }
    case json::ExecType::CANCELED: {
      switch (request_type) {
        case RequestType::UNDEFINED:
          LOG(WARNING)("*** EXTERNAL ACTION ***"_sv);
          break;
        case RequestType::CANCEL_ORDER:
          return RequestStatus::ACCEPTED;
        case RequestType::CREATE_ORDER:
        case RequestType::MODIFY_ORDER:
          DLOG(FATAL)("UNEXPECTED"_sv);
          break;
      }
      break;
    }
    case json::ExecType::TRADE:
      break;
    case json::ExecType::FUNDING:
      break;
  }
  return RequestStatus::UNDEFINED;
}

void Gateway::operator()(
    [[maybe_unused]] json::Action action,
    const json::Execution &execution,
    const server::TraceInfo &trace_info) {
  DLOG(INFO)("execution={}"_fmt, execution);
  size_t fill_length = {}, index = {};
  bool success = true;
  for (auto &item : execution.data) {
    auto last = execution.data.size() == ++index;
    server::OMS_Lookup order_lookup{
        .symbol = item.symbol,
        .side = json::map(item.side),
        .status = json::map(item.ord_status),
        .price = item.price,
        .remaining_quantity = item.leaves_qty,
        .traded_quantity = item.cum_qty,
        .timestamp = item.timestamp,  // XXX transact_time?
        .external_account = {},
        .external_order_id = item.order_id,
    };
    auto found = dispatcher_.find_order(
        item.order_id,
        item.cl_ord_id,
        order_lookup,
        trace_info,
        [&](const auto &order, auto &result) {
          result.request_status = compute_request_status_2(order.request_type, item.exec_type);

          if (result.request_status != RequestStatus::UNDEFINED) {
            result.origin = Origin::EXCHANGE;
            result.error = item.ord_rej_reason.empty() ? Error::UNDEFINED : Error::UNKNOWN,
            result.text = item.ord_rej_reason;
          }

          if (item.exec_type == json::ExecType::TRADE) {
            success = fill_update(dispatcher_, fill_, fill_length, item);
            if (ROQ_UNLIKELY(success == false)) {
              LOG(FATAL)
              (R"(Insufficient fill array size: )"
               R"(len(trade)={}/{})"_fmt,
               fill_length,
               fill_.size());
            }
          }

          if (last && fill_length) {
            if (ROQ_LIKELY(success)) {
              TradeUpdate trade_update{
                  .account = order.account,
                  .order_id = order.user_order_id,
                  .exchange = order.exchange,
                  .symbol = order.symbol,
                  .side = order.side,
                  .position_effect = PositionEffect::UNDEFINED,
                  .order_template = {},
                  .create_time_utc = item.timestamp,  // XXX transact_time?
                  .update_time_utc = item.timestamp,  // XXX transact_time?
                  .gateway_order_id = order.gateway_order_id,
                  .external_account = {},
                  .external_order_id = order.external_order_id,
                  .fills = {fill_.data(), fill_length},
              };
              server::create_trace_and_dispatch(
                  trace_info,
                  trade_update,
                  dispatcher_,
                  true,
                  order.user_id);  // HANS
            } else {
              LOG(FATAL)
              (R"(Insufficient fill array size: )"
               R"(len(fill)={}/{})"_fmt,
               fill_length,
               fill_.size());
            }
          }
        });
    if (found == false) {
      LOG(WARNING)("*** EXTERNAL ORDER ***"_sv);
      LOG(WARNING)("action={}, execution={}"_fmt, action, execution);
    }
  }
}

void Gateway::operator()(
    json::Action action, const json::Instrument &instrument, const server::TraceInfo &trace_info) {
  switch (action) {
    case json::Action::UNDEFINED:
    case json::Action::UNKNOWN:
      LOG(FATAL)("Unexpected"_sv);
      break;
    case json::Action::PARTIAL:
      if (snapshot_.instrument == false) {
        snapshot_.instrument = true;
        size_t security_count = {};
        for (auto &item : instrument.data) {
          if (dispatcher_.discard_symbol(item.symbol)) {
            VLOG(1)(R"(Drop symbol="{}")"_fmt, item.symbol);
            continue;
          }
          ++security_count;
          auto &product = find_product(item);
          auto reference_data = product.create_reference_data(item);
          server::create_trace_and_dispatch(trace_info, reference_data, dispatcher_, false);
          auto market_status = product.create_market_status(item);
          server::create_trace_and_dispatch(trace_info, market_status, dispatcher_, true);
          auto statistics_update = product.create_statistics_update(item);
          server::create_trace_and_dispatch(trace_info, statistics_update, dispatcher_, false);
        }
        VLOG(2)
        (R"(- securities: {} (/{}))"_fmt, security_count, instrument.data.size());
        web_socket_.download.check_relaxed(WebSocketDownload::State::INSTRUMENT);
      }
      break;
    case json::Action::INSERT:
      // drop
      break;
    case json::Action::UPDATE: {
      for (auto &item : instrument.data) {
        if (dispatcher_.discard_symbol(item.symbol))
          continue;
        auto &product = find_product(item);
        // XXX check if dirty
        auto market_status = product.create_market_status(item);
        server::create_trace_and_dispatch(trace_info, market_status, dispatcher_, true);
      }
      break;
    }
    case json::Action::DELETE:
      // drop
      break;
  }
}

void Gateway::operator()(
    const json::Action action, const json::Order &order, const server::TraceInfo &) {
  DLOG(INFO)(R"(action={} order={})"_fmt, action, order);
  for (auto &iter : order.data)
    (*this)(iter);
}

void Gateway::operator()(
    const json::Action action,
    const json::OrderBookL2 &order_book_l2,
    const server::TraceInfo &trace) {
  assert(action != json::Action::UNKNOWN);
  auto snapshot = action == json::Action::PARTIAL;
  // note!
  //   first partial update will include *all* instruments
  //   drop everything received before partial (see: API documentation)
  if (snapshot == false && snapshot_.order_book_l2 == false)
    return;
  std::string_view previous;
  bool success = true;
  size_t bid_length = {}, ask_length = {};
  for (auto &item : order_book_l2.data) {
    if (success == false)
      break;
    if (previous.empty()) {
      previous = item.symbol;
    } else if (previous.compare(item.symbol) != 0) {
      publish_market_by_price(previous, bid_length, ask_length, snapshot, trace, false);
      previous = item.symbol;
      bid_length = ask_length = 0u;
    }
    auto price_size = find_price(action, item.id, item.price, item.size);
    if (std::isfinite(price_size.first)) {
      switch (item.side) {
        case json::Side::BUY:
          success = mbp_update(bid_, bid_length, price_size);
          break;
        case json::Side::SELL:
          success = mbp_update(ask_, ask_length, price_size);
          break;
        default:
          LOG(FATAL)("Unexpected"_sv);
      }
    } else {
      LOG(WARNING)
      (R"(Closing web-socket: )"
       R"(unexpected action={} id={} price={} size={})"_fmt,
       action,
       item.id,
       item.price,
       item.size);
      web_socket_.connection.close();
      return;
    }
  }
  LOG_IF(WARNING, !success)
  (R"(Insufficient bid/ask array size(s): )"
   R"(symbol="{}", len(bid)={}/{}, len(ask)={}/{})"_fmt,
   previous,
   bid_length,
   bid_.size(),
   ask_length,
   ask_.size());
  assert(previous.empty() == false);
  publish_market_by_price(previous, bid_length, ask_length, snapshot, trace, true);
  // download complete?
  if (snapshot) {
    snapshot_.order_book_l2 = true;
    web_socket_.download.check_relaxed(WebSocketDownload::State::ORDER_BOOK_L2);
  }
}

void Gateway::operator()(
    const json::Action action,
    const json::Position &position,
    const server::TraceInfo &trace_info) {
  DLOG(INFO)("action={}, position={}"_fmt, action, position);
  for (auto &item : position.data) {
    PositionUpdate position_update{
        .account = account_,
        .exchange = Flags::exchange(),
        .symbol = item.symbol,
        .side = Side::UNDEFINED,
        .position = item.current_qty,
        .last_trade_id = 0,
        .position_cost = 0.0,
        .position_yesterday = 0.0,
        .position_cost_yesterday = 0.0,
        .external_account = {},
    };
    server::create_trace_and_dispatch(trace_info, position_update, dispatcher_, false);
  }
}

void Gateway::operator()(
    const json::Action, const json::Quote &quote, const server::TraceInfo &trace_info) {
  for (auto &item : quote.data) {
    TopOfBook top_of_book{
        .exchange = Flags::exchange(),
        .symbol = item.symbol,
        .layer =
            {
                .bid_price = item.bid_price,
                .bid_quantity = item.bid_size,
                .ask_price = item.ask_price,
                .ask_quantity = item.ask_size,
            },
        .snapshot = false,  // XXX ???
        .exchange_time_utc = item.timestamp,
    };
    VLOG(3)(R"(top_of_book={})"_fmt, top_of_book);
    server::create_trace_and_dispatch(
        trace_info,
        top_of_book,
        dispatcher_,
        true);  // XXX not always correct
  }
}

void Gateway::operator()(const json::Action, const json::Settlement &, const server::TraceInfo &) {
}

void Gateway::operator()(
    const json::Action action, const json::Trade &trade, const server::TraceInfo &trace_info) {
  if (action != json::Action::INSERT)
    return;
  std::string_view previous;
  bool success = true;
  size_t trade_length = {};
  std::chrono::nanoseconds timestamp = {};
  for (auto &item : trade.data) {
    if (success == false)
      break;
    if (timestamp == timestamp.zero()) {
      timestamp = item.timestamp;
    } else {
      assert(timestamp == item.timestamp);
    }
    if (item.symbol.compare(previous) != 0) {
      if (previous.empty() == false && trade_length > 0u) {
        TradeSummary trade_summary{
            .exchange = Flags::exchange(),
            .symbol = previous,
            .trades = {trade_.data(), trade_length},
            .exchange_time_utc = timestamp,
        };
        VLOG(3)(R"(trade_summary={})"_fmt, trade_summary);
        server::create_trace_and_dispatch(trace_info, trade_summary, dispatcher_, false);
      }
      previous = item.symbol;
      trade_length = 0;
    }
    success = trade_update(trade_, trade_length, item);
  }
  LOG_IF(WARNING, !success)
  (R"(Insufficient trade array size: )"
   R"(symbol="{}", len(trade)={}/{})"_fmt,
   trade.data.size(),
   trade_.size());
  if (previous.empty() == false && trade_length > 0u) {
    TradeSummary trade_summary{
        .exchange = Flags::exchange(),
        .symbol = previous,
        .trades = {trade_.data(), trade_length},
        .exchange_time_utc = timestamp,
    };
    VLOG(3)(R"(trade_summary={})"_fmt, trade_summary);
    server::create_trace_and_dispatch(trace_info, trade_summary, dispatcher_, true);
  }
}

// rest

int32_t Gateway::download(WebSocketDownload::State state) {
  if (rest_.connection.ready() == false)
    return -1;
  switch (state) {
    case WebSocketDownload::State::UNDEFINED:
      break;
    case WebSocketDownload::State::ACCOUNTS: {
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

void Gateway::operator()(const Rest &) {
  if (rest_.connection.ready())
    web_socket_.download.bump();
}

auto compute_order_status(json::OrdStatus ord_status, bool working_status) {
  switch (ord_status) {
    case json::OrdStatus::UNDEFINED:
    case json::OrdStatus::UNKNOWN:
      if (working_status)
        return OrderStatus::WORKING;
      break;
    case json::OrdStatus::NEW:
      return working_status ? OrderStatus::WORKING : OrderStatus::ACCEPTED;
    case json::OrdStatus::FILLED:
      return OrderStatus::COMPLETED;
    case json::OrdStatus::CANCELED:
      return OrderStatus::CANCELED;
  }
  return OrderStatus::UNDEFINED;
}

auto compute_request_status(RequestType request_type, json::OrdStatus ord_status) {
  switch (ord_status) {
    case json::OrdStatus::UNDEFINED:
    case json::OrdStatus::UNKNOWN:
      break;
    case json::OrdStatus::NEW: {
      switch (request_type) {
        case RequestType::UNDEFINED:
          LOG(WARNING)("*** EXTERNAL ACTION ***"_sv);
          break;
        case RequestType::CREATE_ORDER:
        case RequestType::MODIFY_ORDER:
          return RequestStatus::ACCEPTED;
        case RequestType::CANCEL_ORDER:
          DLOG(FATAL)("UNEXPECTED"_sv);
          break;
      }
      break;
    }
    case json::OrdStatus::FILLED: {
      break;
    }
    case json::OrdStatus::CANCELED: {
      switch (request_type) {
        case RequestType::UNDEFINED:
          LOG(WARNING)("*** EXTERNAL ACTION ***"_sv);
          break;
        case RequestType::CANCEL_ORDER:
          return RequestStatus::ACCEPTED;
        case RequestType::CREATE_ORDER:
        case RequestType::MODIFY_ORDER:
          DLOG(FATAL)("UNEXPECTED"_sv);
          break;
      }
      break;
    }
  }
  return RequestStatus::UNDEFINED;
}

void Gateway::operator()(const json::OrderItem &order_item) {
  if (Flags::rest_allow_order_updates() == false)
    return;

  server::TraceInfo trace_info;  // XXX not correct (*after* parsing)

  auto order_status = compute_order_status(order_item.ord_status, order_item.working_indicator);
  DLOG(INFO)(R"(order_status={})"_fmt, order_status);

  server::OMS_Lookup order_lookup{
      .symbol = order_item.symbol,
      .side = json::map(order_item.side),
      .status = order_status,
      .price = order_item.price,
      .remaining_quantity = order_item.leaves_qty,
      .traded_quantity = order_item.cum_qty,
      .timestamp = order_item.timestamp,  // XXX transact_time?
      .external_account = {},
      .external_order_id = order_item.order_id,
  };
  auto found = dispatcher_.find_order(
      order_item.order_id,
      order_item.cl_ord_id,
      order_lookup,
      trace_info,
      [&](const auto &order, auto &result) {
        result.request_status = compute_request_status(order.request_type, order_item.ord_status);

        if (result.request_status != RequestStatus::UNDEFINED) {
          result.origin = Origin::EXCHANGE;
          result.error = order_item.ord_rej_reason.empty() ? Error::UNDEFINED : Error::UNKNOWN,
          result.text = order_item.ord_rej_reason;  // XXX text ???
        }
      });

  if (found == false) {
    LOG(WARNING)("*** EXTERNAL ORDER ***"_sv);
    LOG(WARNING)("order_item={}"_fmt, order_item);
  }
}

void Gateway::operator()(const json::Order &order) {
  DLOG(INFO)(R"(order={})"_fmt, order);
  for (auto &iter : order.data)
    (*this)(iter);
}

// UTILS:

void Gateway::update_market_data(GatewayStatus gateway_status) {
  if (gateway_status == market_data_status_)
    return;
  market_data_status_ = gateway_status;
  server::TraceInfo trace_info;
  MarketDataStatus market_data_status{
      .status = market_data_status_,
  };
  server::create_trace_and_dispatch(trace_info, market_data_status, dispatcher_, true);
  LOG(INFO)(R"(market_data_status={})"_fmt, market_data_status_);
}

void Gateway::update_order_manager(GatewayStatus gateway_status) {
  if (gateway_status == order_manager_status_)
    return;
  order_manager_status_ = gateway_status;
  server::TraceInfo trace_info;
  OrderManagerStatus order_manager_status{
      .account = account_,
      .status = order_manager_status_,
  };
  server::create_trace_and_dispatch(trace_info, order_manager_status, dispatcher_, true);
  LOG(INFO)(R"(order_manager_status={})"_fmt, order_manager_status_);
}

void Gateway::subscribe_instrument() {
  web_socket_.connection.subscribe("instrument"_sv);
}

void Gateway::subscribe_order_book_l2() {
  web_socket_.connection.subscribe("orderBookL2"_sv, symbols_);

  // XXX not here
  web_socket_.connection.subscribe("funding"_sv, symbols_);
  web_socket_.connection.subscribe("liquidation"_sv, symbols_);
  web_socket_.connection.subscribe("quote"_sv, symbols_);
  web_socket_.connection.subscribe("settlement"_sv, symbols_);
  web_socket_.connection.subscribe("trade"_sv, symbols_);
  // XXX private
  web_socket_.connection.subscribe("execution"_sv, symbols_);
  web_socket_.connection.subscribe("order"_sv, symbols_);
  web_socket_.connection.subscribe("margin"_sv, symbols_);
  web_socket_.connection.subscribe("position"_sv, symbols_);
  // XXX other
  // cancelAllAfter
  // authKeyExpires
}

const Product &Gateway::find_product(const json::InstrumentItem &item) {
  auto iter = product_cache_.find(item.symbol);
  if (iter == product_cache_.end()) {
    iter = product_cache_.emplace(item.symbol, item).first;
  } else {
    (*iter).second.update(item);
  }
  assert(iter != product_cache_.end());
  return (*iter).second;
}

std::pair<double, double> Gateway::find_price(
    json::Action action, uint64_t id, double price, double size) {
  auto result = NaN;
  auto iter = price_lookup_.find(id);
  switch (action) {
    case json::Action::UNDEFINED:
    case json::Action::UNKNOWN:
      LOG(FATAL)("Unexpected"_sv);
      break;
    case json::Action::PARTIAL:
    case json::Action::INSERT:
      if (std::isnan(price) == false && std::isnan(size) == false) {
        if (iter == price_lookup_.end()) {
          iter = price_lookup_.emplace(id, price).first;
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
        DLOG(FATAL)
        (R"(action={} id={} price={} size={})"_fmt, action, id, price, size);
      }
      break;
    case json::Action::UPDATE:
      if (std::isnan(price) && std::isnan(size) == false && std::fabs(size) > TOLERANCE) {
        if (iter != price_lookup_.end()) {
          result = (*iter).second;
        } else {
          // unable to find the cached price ==> fail
        }
      } else {
        // unexpected price or size ==> fail
      }
      break;
    case json::Action::DELETE:
      if (std::isnan(price) && (std::isnan(size) || std::fabs(size) < TOLERANCE)) {
        if (iter != price_lookup_.end()) {
          result = (*iter).second;
          price_lookup_.erase(iter);
        } else {
          // unable to find the cached price ==> fail
        }
      } else {
        // unexpected price or size ==> fail
      }
  }
  return std::make_pair(result, std::isnan(size) ? 0.0 : size);
}

void Gateway::publish_market_by_price(
    const std::string_view &symbol,
    size_t bid_length,
    size_t ask_length,
    bool snapshot,
    const server::TraceInfo &trace_info,
    bool is_last) {
  assert(symbol.empty() == false);
  if ((bid_length + ask_length) == 0)
    return;
  LOG_IF(INFO, snapshot)
  (R"(Received market data snapshot for symbol="{}")"_fmt, symbol);
  MarketByPriceUpdate market_by_price_update{
      .exchange = Flags::exchange(),
      .symbol = symbol,
      .bids = {bid_.data(), bid_length},
      .asks = {ask_.data(), ask_length},
      .snapshot = snapshot,
      .exchange_time_utc = {},
  };
  VLOG(3)(R"(market_by_price_update={})"_fmt, market_by_price_update);
  server::create_trace_and_dispatch(trace_info, market_by_price_update, dispatcher_, is_last);
}

}  // namespace bitmex
}  // namespace roq
