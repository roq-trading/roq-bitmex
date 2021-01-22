/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/gateway.h"

#include <absl/flags/flag.h>

#include <algorithm>
#include <limits>
#include <utility>

#include "roq/logging.h"

#include "roq/core/charconv.h"
#include "roq/core/clock.h"
#include "roq/core/utils.h"
#include "roq/core/view.h"

#include "roq/core/http/exceptions.h"

#include "roq/bitmex/options.h"

#include "roq/bitmex/json/utils.h"

namespace roq {
namespace bitmex {

constexpr auto DEFAULT_MULTIPLIER = double{1.0};

constexpr auto TOLERANCE = double{1.0e-10};

template <typename C, typename T>
static bool mbp_update(C &data, size_t &offset, const T &item) {
  auto &obj = data[offset];
  new (&obj) MBPUpdate{
      .price = item.first,
      .quantity = item.second,
  };
  ++offset;
  return offset < data.size();
}

template <typename C, typename T>
static bool trade_update(C &data, size_t &offset, const T &item) {
  auto &obj = data[offset];
  new (&obj) Trade{
      .side = json::map(item.side),  // XXX check
      .price = item.price,
      .quantity = item.size,
      .trade_id = item.trd_match_id,
  };
  ++offset;
  return offset < data.size();
}

template <typename C, typename T>
static bool fill_update(
    server::Dispatcher &dispatcher, C &data, size_t &offset, const T &item) {
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

Gateway::Gateway(server::Dispatcher &dispatcher, const Config &config)
    : dispatcher_(dispatcher), account_(config.get_account()),
      random_(config.get_api_key(), config.get_secret()),
      dns_base_(base_, true),
      web_socket_{
          .connection =
              {
                  *this,
                  config,
                  random_,
                  base_,
                  dns_base_,
                  ssl_context_,
              },
          .download = WebSocketDownload(
              std::chrono::seconds{
                  absl::GetFlag(FLAGS_ws_request_timeout_secs)},
              [this](auto state) { return download(state); }),
      },
      rest_{
          .connection =
              {
                  *this,
                  config,
                  random_,
                  base_,
                  dns_base_,
                  ssl_context_,
              },
      },
      bid_(absl::GetFlag(FLAGS_cache_mbp_max_depth)),
      ask_(absl::GetFlag(FLAGS_cache_mbp_max_depth)),
      trade_(absl::GetFlag(FLAGS_cache_trades_max_depth)),
      fill_(absl::GetFlag(FLAGS_cache_fills_max_depth)) {
  LOG_IF(WARNING, absl::GetFlag(FLAGS_ws_cancel_on_disconnect) == false)
  ("Orders will *NOT* be cancelled on disconnect");
}

void Gateway::operator()(const Event<Start> &event) {
  LOG(INFO)("Starting the gateway...");
  web_socket_.connection(event);
  rest_.connection(event);
}

void Gateway::operator()(const Event<Stop> &event) {
  LOG(INFO)("Stopping the gateway...");
  rest_.connection(event);
  web_socket_.connection(event);
}

void Gateway::operator()(const Event<Timer> &event) {
  // web socket
  web_socket_.connection(event);
  // rest
  /*
  if (web_socket_.download.has_expired()) {
    LOG(WARNING)("Rest download has timed out");
    web_socket_.download.reset();
    rest_.connection.close();
  } else {
    rest_.connection(event);
  }
  */
  rest_.connection(event);
  base_.loop(EVLOOP_NONBLOCK);
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
      case core::http::Status::NOT_FOUND: {   // 404
      */
    } catch (NetworkError &e) {
      // XXX send ack failure
      LOG(FATAL)(R"(Unexpected what="{}")", e.what());
    }
  });
}

void Gateway::operator()(
    const Event<ModifyOrder> &event,
    const std::string_view &request_id,
    const server::OMS_Order &order) {
  rest_.connection.modify_order(
      event.value, request_id, order, [this](auto &promise) {
        try {
          (*this)(promise.get());
          /*
          case core::http::Status::BAD_REQUEST:   // 400
          case core::http::Status::UNAUTHORIZED:  // 401
          case core::http::Status::FORBIDDEN:     // 403
          case core::http::Status::NOT_FOUND: {   // 404
          */
        } catch (NetworkError &e) {
          // XXX send ack failure
          LOG(FATAL)(R"(Unexpected what="{}")", e.what());
        }
      });
}

void Gateway::operator()(
    const Event<CancelOrder> &event,
    const std::string_view &request_id,
    const server::OMS_Order &order) {
  rest_.connection.cancel_order(
      event.value, request_id, order, [this](auto &promise) {
        try {
          (*this)(promise.get());
          /*
          case core::http::Status::BAD_REQUEST:   // 400
          case core::http::Status::UNAUTHORIZED:  // 401
          case core::http::Status::FORBIDDEN:     // 403
          case core::http::Status::NOT_FOUND: {   // 404
          */
        } catch (NetworkError &e) {
          // XXX send ack failure
          LOG(FATAL)(R"(Unexpected what="{}")", e.what());
        }
      });
}

void Gateway::operator()(metrics::Writer &writer) {
  web_socket_.connection(writer);
  rest_.connection(writer);
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

auto compute_request_status_2(
    RequestType request_type, json::ExecType exec_type) {
  switch (exec_type) {
    case json::ExecType::UNDEFINED:
    case json::ExecType::UNKNOWN:
      break;
    case json::ExecType::NEW: {
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
    case json::ExecType::REPLACED: {
      switch (request_type) {
        case RequestType::UNDEFINED:
          LOG(WARNING)("*** EXTERNAL ACTION ***");
          break;
        case RequestType::MODIFY_ORDER:
          return RequestStatus::ACCEPTED;
        case RequestType::CREATE_ORDER:
        case RequestType::CANCEL_ORDER:
          DLOG(FATAL)("UNEXPECTED");
          break;
      }
      break;
    }
    case json::ExecType::CANCELED: {
      switch (request_type) {
        case RequestType::UNDEFINED:
          LOG(WARNING)("*** EXTERNAL ACTION ***");
          break;
        case RequestType::CANCEL_ORDER:
          return RequestStatus::ACCEPTED;
        case RequestType::CREATE_ORDER:
        case RequestType::MODIFY_ORDER:
          DLOG(FATAL)("UNEXPECTED");
          break;
      }
      break;
    }
    case json::ExecType::TRADE:
      break;
  }
  return RequestStatus::UNDEFINED;
}

void Gateway::operator()(
    [[maybe_unused]] json::Action action,
    const json::Execution &execution,
    const server::TraceInfo &trace_info) {
  DLOG(INFO)("execution={}", execution);
  size_t fill_length = 0, index = 0;
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
          result.request_status =
              compute_request_status_2(order.request_type, item.exec_type);

          if (result.request_status != RequestStatus::UNDEFINED) {
            result.origin = Origin::EXCHANGE;
            result.error =
                item.ord_rej_reason.empty() ? Error::UNDEFINED : Error::UNKNOWN,
            result.text = item.ord_rej_reason;
          }

          if (item.exec_type == json::ExecType::TRADE) {
            success = fill_update(dispatcher_, fill_, fill_length, item);
            if (ROQ_UNLIKELY(success == false)) {
              LOG(FATAL)
              (R"(Insufficient fill array size: )"
               R"(len(trade)={}/{})",
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
               R"(len(fill)={}/{})",
               fill_length,
               fill_.size());
            }
          }
        });
    if (found == false) {
      LOG(WARNING)("*** EXTERNAL ORDER ***");
      LOG(WARNING)("action={}, execution={}", action, execution);
    }
  }
  /*
  {
  account=273093
  avg_px=0.0
  cl_ord_id="roq-1586872864-4"
  cl_ord_link_id=""
  commission=0.0
  contingency_type=""
  cum_qty=0.0
  currency="USD"
  display_qty=0.0
  ex_destination="XBME"
  exec_comm=0.0
  exec_cost=0.0
  exec_id="0cf1e0f7-89a7-2b16-0413-77d3443750b8"
  exec_inst=""
  exec_type="New"
  foreign_notional=0.0
  home_notional=0.0
  last_liquidity_ind=""
  last_mkt=""
  last_px=0.0
  last_qty=0.0
  leaves_qty=1.0
  multi_leg_reporting_type="SingleSecurity"
  order_id="3a174d58-1d40-19d4-71fd-1040eb2a35be"
  order_qty=1.0
  ord_rej_reason=""
  ord_status="New"
  ord_type="Limit"
  peg_offset_value=0.0
  peg_price_type=""
  price=6883.0
  settl_currency="XBt"
  side="Buy"
  simple_cum_qty=0.0
  simple_leaves_qty=0.0
  simple_order_qty=0.0
  stop_px=0.0
  symbol="XBTUSD"
  text="Submitted via API."
  time_in_force="GoodTillCancel"
  timestamp=1586873125585000000ns
  trade_publish_indicator=""
  transact_time=1586873125585000000ns
  trd_match_id="00000000-0000-0000-0000-000000000000"
  triggered=""
  underlying_last_px=0.0
  working_indicator=true
  }

  {
  account=273093
  avg_px=0.0
  cl_ord_id="roq-1586872864-4"
  cl_ord_link_id=""
  commission=0.0
  contingency_type=""
  cum_qty=0.0
  currency="USD"
  display_qty=0.0
  ex_destination="XBME"
  exec_comm=0.0
  exec_cost=0.0
  exec_id="e66711c6-2f86-4df0-39e5-4f4faa5cf57e"
  exec_inst=""
  exec_type="Replaced"
  foreign_notional=0.0
  home_notional=0.0
  last_liquidity_ind=""
  last_mkt=""
  last_px=0.0
  last_qty=0.0
  leaves_qty=1.0
  multi_leg_reporting_type="SingleSecurity"
  order_id="3a174d58-1d40-19d4-71fd-1040eb2a35be"
  order_qty=1.0
  ord_rej_reason=""
  ord_status="New"
  ord_type="Limit"
  peg_offset_value=0.0
  peg_price_type=""
  price=6883.5
  settl_currency="XBt"
  side="Buy"
  simple_cum_qty=0.0
  simple_leaves_qty=0.0
  simple_order_qty=0.0
  stop_px=0.0
  symbol="XBTUSD"
  text="Amended orderQty price: Amended via API.\nSubmitted via API."
  time_in_force="GoodTillCancel"
  timestamp=1586873245657000000ns
  trade_publish_indicator=""
  transact_time=1586873245657000000ns
  trd_match_id="00000000-0000-0000-0000-000000000000"
  triggered=""
  underlying_last_px=0.0
  working_indicator=true
  }


  {
  account=273093
  avg_px=6883.75
  cl_ord_id="roq-1586872864-4"
  cl_ord_link_id=""
  commission=0.0007500000000000002
  contingency_type=""
  cum_qty=1.0
  currency="USD"
  display_qty=0.0
  ex_destination="XBME"
  exec_comm=10.0
  exec_cost=-14527.0
  exec_id="66f13afe-5e57-4d37-ad43-fe8d0f8c603c"
  exec_inst=""
  exec_type="Trade"
  foreign_notional=-1.0
  home_notional=0.00014527000000000005
  last_liquidity_ind="RemovedLiquidity"
  last_mkt="XBME"
  last_px=6883.5
  last_qty=1.0
  leaves_qty=0.0
  multi_leg_reporting_type="SingleSecurity"
  order_id="3a174d58-1d40-19d4-71fd-1040eb2a35be"
  order_qty=1.0
  ord_rej_reason=""
  ord_status="Filled"
  ord_type="Limit"
  peg_offset_value=0.0
  peg_price_type=""
  price=6883.5
  settl_currency="XBt"
  side="Buy"
  simple_cum_qty=0.0
  simple_leaves_qty=0.0
  simple_order_qty=0.0
  stop_px=0.0
  symbol="XBTUSD"
  text="Amended orderQty price: Amended via API.\nSubmitted via API."
  time_in_force="GoodTillCancel"
  timestamp=1586873245657000000ns
  trade_publish_indicator="PublishTrade"
  transact_time=1586873245657000000ns
  trd_match_id="15b03a0f-03de-6826-4c63-3f49ba0c8190"
  triggered=""
  underlying_last_px=0.0
  working_indicator=false
  }

  {
  account=273093
  avg_px=6885.0
  cl_ord_id=""
  cl_ord_link_id=""
  commission=-0.0002500000000000001
  contingency_type=""
  cum_qty=3.0
  currency="USD"
  display_qty=0.0
  ex_destination="XBME"
  exec_comm=-10.0
  exec_cost=43572.0
  exec_id="8c2a2682-04aa-ff1e-ef44-027062134e22"
  exec_inst=""
  exec_type="Trade"
  foreign_notional=3.0
  home_notional=-0.00043572000000000017
  last_liquidity_ind="AddedLiquidity"
  last_mkt="XBME"
  last_px=6885.0
  last_qty=3.0
  leaves_qty=0.0
  multi_leg_reporting_type="SingleSecurity"
  order_id="018c5318-0442-eafd-b4df-b2d261323d1a"
  order_qty=3.0
  ord_rej_reason=""
  ord_status="Filled"
  ord_type="Limit"
  peg_offset_value=0.0
  peg_price_type=""
  price=6885.0
  settl_currency="XBt"
  side="Sell"
  simple_cum_qty=0.0
  simple_leaves_qty=0.0
  simple_order_qty=0.0
  stop_px=0.0
  symbol="XBTUSD"
  text="Submission from testnet.bitmex.com"
  time_in_force="GoodTillCancel"
  timestamp=1586873368899000000ns
  trade_publish_indicator="PublishTrade"
  transact_time=1586873368899000000ns
  trd_match_id="77db132c-3b26-0bc9-b750-5fbeb04d5ff6"
  triggered=""
  underlying_last_px=0.0
  working_indicator=false
  }

  {
  account=273093
  avg_px=0.0
  cl_ord_id="roq-1586872864-3"
  cl_ord_link_id=""
  commission=0.0
  contingency_type=""
  cum_qty=0.0
  currency="USD"
  display_qty=0.0
  ex_destination="XBME"
  exec_comm=0.0
  exec_cost=0.0
  exec_id="a2455064-a337-d36f-1008-21c648f49d12"
  exec_inst=""
  exec_type="Canceled"
  foreign_notional=0.0
  home_notional=0.0
  last_liquidity_ind=""
  last_mkt=""
  last_px=0.0
  last_qty=0.0
  leaves_qty=0.0
  multi_leg_reporting_type="SingleSecurity"
  order_id="73c01791-d59f-a4a6-6b65-1ceb561f7c8c"
  order_qty=1.0
  ord_rej_reason=""
  ord_status="Canceled"
  ord_type="Limit"
  peg_offset_value=0.0
  peg_price_type=""
  price=6883.0
  settl_currency="XBt"
  side="Buy"
  simple_cum_qty=0.0
  simple_leaves_qty=0.0
  simple_order_qty=0.0
  stop_px=0.0
  symbol="XBTUSD"
  text="Canceled: Cancel from testnet.bitmex.com\nSubmitted via API."
  time_in_force="GoodTillCancel"
  timestamp=1586873099436000000ns
  trade_publish_indicator=""
  transact_time=1586873099436000000ns
  trd_match_id="00000000-0000-0000-0000-000000000000"
  triggered=""
  underlying_last_px=0.0
  working_indicator=false
  }
  */
}

void Gateway::operator()(
    json::Action action,
    const json::Instrument &instrument,
    const server::TraceInfo &trace_info) {
  switch (action) {
    case json::Action::UNDEFINED:
    case json::Action::UNKNOWN:
      LOG(FATAL)("Unexpected");
      break;
    case json::Action::PARTIAL:
      if (snapshot_.instrument == false) {
        snapshot_.instrument = true;
        // assert(_download != Download::READY);
        size_t security_count = 0;
        for (auto &item : instrument.data) {
          if (dispatcher_.discard_symbol(item.symbol)) {
            VLOG(1)(R"(Drop symbol="{}")", item.symbol);
            continue;
          }
          ++security_count;
          auto &product = find_product(item);
          auto reference_data = product.create_reference_data(item);
          server::create_trace_and_dispatch(
              trace_info, reference_data, dispatcher_, false);
          auto market_status = product.create_market_status(item);
          server::create_trace_and_dispatch(
              trace_info, market_status, dispatcher_, true);
          auto statistics_update = product.create_statistics_update(item);
          server::create_trace_and_dispatch(
              trace_info, statistics_update, dispatcher_, false);
        }
        VLOG(2)
        (R"(- securities: {} (/{}))", security_count, instrument.data.size());
        web_socket_.download.check_relaxed(
            WebSocketDownload::State::INSTRUMENT);
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
        server::create_trace_and_dispatch(
            trace_info, market_status, dispatcher_, true);
      }
      break;
    }
    case json::Action::DELETE:
      // drop
      break;
  }
}

void Gateway::operator()(
    const json::Action action,
    const json::Order &order,
    const server::TraceInfo &) {
  DLOG(INFO)(R"(action={} order={})", action, order);
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
  size_t bid_length = 0, ask_length = 0;
  for (auto &item : order_book_l2.data) {
    if (success == false)
      break;
    if (previous.empty()) {
      previous = item.symbol;
    } else if (previous.compare(item.symbol) != 0) {
      publish_market_by_price(
          previous, bid_length, ask_length, snapshot, trace, false);
      previous = item.symbol;
      bid_length = ask_length = 0;
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
          LOG(FATAL)("Unexpected");
      }
    } else {
      LOG(WARNING)
      (R"(Closing web-socket: )"
       R"(unexpected action={} id={} price={} size={})",
       action,
       item.id,
       item.price,
       item.size);
      web_socket_.connection.close();
      return;
    }
  }
  if (ROQ_UNLIKELY(success == false)) {
    LOG(FATAL)
    (R"(Insufficient bid/ask array size(s): )"
     R"(len(bid)={}/{}, len(ask)={}/{})",
     bid_length,
     bid_.size(),
     ask_length,
     ask_.size());
  }
  assert(previous.empty() == false);
  publish_market_by_price(
      previous, bid_length, ask_length, snapshot, trace, true);
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
  DLOG(INFO)("action={}, position={}", action, position);
  for (auto &item : position.data) {
    PositionUpdate position_update{
        .account = account_,
        .exchange = absl::GetFlag(FLAGS_exchange),
        .symbol = item.symbol,
        .side = Side::UNDEFINED,
        .position = item.current_qty,
        .last_trade_id = 0,
        .position_cost = 0.0,
        .position_yesterday = 0.0,
        .position_cost_yesterday = 0.0,
        .external_account = {},
    };
    server::create_trace_and_dispatch(
        trace_info, position_update, dispatcher_, false);
  }
  /*
  {
  account=273093
  avg_cost_price=6885.0
  avg_entry_price=6885.0
  bankrupt_price=100000000.0
  break_even_price=6926.0
  commission=0.0007500000000000002
  cross_margin=true
  currency="XBt"
  current_comm=186.0
  current_cost=29421.0
  current_qty=-1.0   ///////////////////////////
  current_timestamp=1586929105385000000ns
  deleverage_percentile=1.0
  exec_buy_cost=29091.0
  exec_buy_qty=2.0
  exec_comm=20.0
  exec_cost=-29091.0
  exec_qty=2.0
  exec_sell_cost=0.0
  exec_sell_qty=0.0
  foreign_notional=1.0
  gross_exec_cost=0.0
  gross_open_cost=0.0
  gross_open_premium=0.0
  home_notional=-0.00014558000000000008
  indicative_tax=0.0
  indicative_tax_rate=0.0
  init_margin=0.0
  init_margin_req=0.010000000000000002
  is_open=true
  last_price=6869.2
  last_value=14558.0
  leverage=100.0
  liquidation_price=100000000.0
  long_bankrupt=0.0
  maint_margin=192.0
  maint_margin_req=0.004500000000000002
  margin_call_price=100000000.0
  mark_price=6869.2
  mark_value=14558.0
  opening_comm=166.0
  opening_cost=58512.0
  opening_qty=-3.0   ///////////////////////////
  opening_timestamp=1586926800000000000ns
  open_order_buy_cost=0.0
  open_order_buy_premium=0.0
  open_order_buy_qty=0.0
  open_order_sell_cost=0.0
  open_order_sell_premium=0.0
  open_order_sell_qty=0.0
  pos_allowance=0.0
  pos_comm=12.0
  pos_cost=14524.0
  pos_cost2=14539.0
  pos_cross=15.0
  pos_init=146.0
  pos_loss=15.0
  pos_maint=138.0
  pos_margin=158.0
  pos_state=nan
  prev_close_price=6877.35
  prev_realised_pnl=-21322.0
  prev_unrealised_pnl=0.0
  quote_currency="USD"
  realised_cost=14897.0
  realised_gross_pnl=-14897.0
  realised_pnl=-15083.0
  realised_tax=0.0
  rebalanced_pnl=15169.0
  risk_limit=20000000000.0
  risk_value=14558.0
  session_margin=0.0
  short_bankrupt=0.0
  simple_cost=0.0
  simple_pnl=0.0
  simple_pnl_pcnt=0.0
  simple_qty=0.0
  simple_value=0.0
  symbol="XBTUSD"   ///////////////////////////
  target_excess_margin=0.0
  taxable_margin=0.0
  tax_base=0.0
  timestamp=1586929105385000000ns
  underlying="XBT"
  unrealised_cost=14524.0
  unrealised_gross_pnl=34.0
  unrealised_pnl=34.0
  unrealised_pnl_pcnt=0.0023000000000000004
  unrealised_roe_pcnt=0.2341
  unrealised_tax=0.0
  var_margin=0.0
  }
  */
}

void Gateway::operator()(
    const json::Action,
    const json::Quote &quote,
    const server::TraceInfo &trace_info) {
  for (auto &item : quote.data) {
    TopOfBook top_of_book{
        .exchange = absl::GetFlag(FLAGS_exchange),
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
    VLOG(3)(R"(top_of_book={})", top_of_book);
    server::create_trace_and_dispatch(
        trace_info,
        top_of_book,
        dispatcher_,
        true);  // XXX not always correct
  }
}

void Gateway::operator()(
    const json::Action, const json::Settlement &, const server::TraceInfo &) {
}

void Gateway::operator()(
    const json::Action action,
    const json::Trade &trade,
    const server::TraceInfo &trace_info) {
  if (action != json::Action::INSERT)
    return;
  std::string_view previous;
  bool success = true;
  size_t trade_length = 0;
  std::chrono::nanoseconds timestamp = {};
  for (auto &item : trade.data) {
    if (success == false)
      break;
    if (timestamp.count() == 0) {
      timestamp = item.timestamp;
    } else {
      assert(timestamp == item.timestamp);
    }
    if (item.symbol.compare(previous) != 0) {
      if (previous.empty() == false && trade_length > 0) {
        TradeSummary trade_summary{
            .exchange = absl::GetFlag(FLAGS_exchange),
            .symbol = previous,
            .trades =
                {
                    .items = trade_.data(),
                    .length = trade_length,
                },
            .exchange_time_utc = timestamp,
        };
        VLOG(3)(R"(trade_summary={})", trade_summary);
        server::create_trace_and_dispatch(
            trace_info, trade_summary, dispatcher_, false);
      }
      previous = item.symbol;
      trade_length = 0;
    }
    success = trade_update(trade_, trade_length, item);
  }
  if (ROQ_UNLIKELY(success == false)) {
    LOG(FATAL)
    (R"(Insufficient trade array size: )"
     R"(len(trade)={}/{})",
     trade_length,
     trade_.size());
  }
  if (previous.empty() == false && trade_length > 0) {
    TradeSummary trade_summary{
        .exchange = absl::GetFlag(FLAGS_exchange),
        .symbol = previous,
        .trades =
            {
                .items = trade_.data(),
                .length = trade_length,
            },
        .exchange_time_utc = timestamp,
    };
    VLOG(3)(R"(trade_summary={})", trade_summary);
    server::create_trace_and_dispatch(
        trace_info, trade_summary, dispatcher_, true);
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

auto compute_request_status(
    RequestType request_type, json::OrdStatus ord_status) {
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
        case RequestType::MODIFY_ORDER:
          return RequestStatus::ACCEPTED;
        case RequestType::CANCEL_ORDER:
          DLOG(FATAL)("UNEXPECTED");
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
          LOG(WARNING)("*** EXTERNAL ACTION ***");
          break;
        case RequestType::CANCEL_ORDER:
          return RequestStatus::ACCEPTED;
        case RequestType::CREATE_ORDER:
        case RequestType::MODIFY_ORDER:
          DLOG(FATAL)("UNEXPECTED");
          break;
      }
      break;
    }
  }
  return RequestStatus::UNDEFINED;
}

void Gateway::operator()(const json::OrderItem &order_item) {
  if (absl::GetFlag(FLAGS_rest_allow_order_updates) == false)
    return;

  server::TraceInfo trace_info;  // XXX not correct (*after* parsing)

  auto order_status =
      compute_order_status(order_item.ord_status, order_item.working_indicator);
  DLOG(INFO)(R"(order_status={})", order_status);

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
        result.request_status =
            compute_request_status(order.request_type, order_item.ord_status);

        if (result.request_status != RequestStatus::UNDEFINED) {
          result.origin = Origin::EXCHANGE;
          result.error = order_item.ord_rej_reason.empty() ? Error::UNDEFINED
                                                           : Error::UNKNOWN,
          result.text = order_item.ord_rej_reason;  // XXX text ???
        }
      });

  if (found == false) {
    LOG(WARNING)("*** EXTERNAL ORDER ***");
    LOG(WARNING)("order_item={}", order_item);
  }
}

void Gateway::operator()(const json::Order &order) {
  DLOG(INFO)(R"(order={})", order);
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
  server::create_trace_and_dispatch(
      trace_info, market_data_status, dispatcher_, true);
  LOG(INFO)(R"(market_data_status={})", market_data_status_);
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
  server::create_trace_and_dispatch(
      trace_info, order_manager_status, dispatcher_, true);
  LOG(INFO)(R"(order_manager_status={})", order_manager_status_);
}

void Gateway::download_accounts() {
  assert(false);
  /*
  constexpr auto state = WebSocketDownload::State::ACCOUNTS;
  auto sequence = web_socket_.download.sequence();
  rest_.connection.get<json::Accounts>(
      [this, sequence](auto& promise) {
    try {
      if (web_socket_.download.skip(sequence, state))
        return;
      (*this)(promise.get());
      web_socket_.download.check(state);
    } catch (NetworkError&) {
      web_socket_.download.retry(state);
    }
  });
  */
}

void Gateway::subscribe_instrument() {
  web_socket_.connection.subscribe("instrument");
}

void Gateway::subscribe_order_book_l2() {
  web_socket_.connection.subscribe("orderBookL2", symbols_);

  // XXX not here
  web_socket_.connection.subscribe("funding", symbols_);
  web_socket_.connection.subscribe("liquidation", symbols_);
  web_socket_.connection.subscribe("quote", symbols_);
  web_socket_.connection.subscribe("settlement", symbols_);
  web_socket_.connection.subscribe("trade", symbols_);
  // XXX private
  web_socket_.connection.subscribe("execution", symbols_);
  web_socket_.connection.subscribe("order", symbols_);
  web_socket_.connection.subscribe("margin", symbols_);
  web_socket_.connection.subscribe("position", symbols_);
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
  auto result = std::numeric_limits<double>::quiet_NaN();
  auto iter = price_lookup_.find(id);
  switch (action) {
    case json::Action::UNDEFINED:
    case json::Action::UNKNOWN:
      LOG(FATAL)("Unexpected");
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
        (R"(action={} id={} price={} size={})", action, id, price, size);
      }
      break;
    case json::Action::UPDATE:
      if (std::isnan(price) && std::isnan(size) == false &&
          std::fabs(size) > TOLERANCE) {
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
      if (std::isnan(price) &&
          (std::isnan(size) || std::fabs(size) < TOLERANCE)) {
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
  (R"(Received market data snapshot for symbol="{}")", symbol);
  MarketByPriceUpdate market_by_price_update{
      .exchange = absl::GetFlag(FLAGS_exchange),
      .symbol = symbol,
      .bids =
          {
              .items = bid_.data(),
              .length = bid_length,
          },
      .asks =
          {
              .items = ask_.data(),
              .length = ask_length,
          },
      .snapshot = snapshot,
      .exchange_time_utc = {},
  };
  VLOG(3)(R"(market_by_price_update={})", market_by_price_update);
  server::create_trace_and_dispatch(
      trace_info, market_by_price_update, dispatcher_, is_last);
}

}  // namespace bitmex
}  // namespace roq
