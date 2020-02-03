/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <chrono>
#include <limits>
#include <string_view>

#include "roq/core/json/parser.h"

#include "roq/bitmex/json/state.h"
#include "roq/bitmex/json/typ.h"

namespace roq {
namespace bitmex {
namespace json {

struct InstrumentItem final {
  explicit InstrumentItem(core::json::value_t&);

  InstrumentItem(const InstrumentItem&) = delete;
  InstrumentItem(InstrumentItem&&) = delete;

  double ask_price = std::numeric_limits<double>::quiet_NaN();
  double bankrupt_limit_down_price = std::numeric_limits<double>::quiet_NaN();
  double bankrupt_limit_up_price = std::numeric_limits<double>::quiet_NaN();
  double bid_price = std::numeric_limits<double>::quiet_NaN();
  std::string_view buy_leg;
  std::chrono::nanoseconds calc_interval = {};
  bool capped = false;
  std::chrono::nanoseconds closing_timestamp = {};
  bool deleverage = false;
  std::chrono::nanoseconds expiry = {};
  double fair_basis = std::numeric_limits<double>::quiet_NaN();
  double fair_basis_rate = std::numeric_limits<double>::quiet_NaN();
  std::string_view fair_method;
  double fair_price = std::numeric_limits<double>::quiet_NaN();
  double foreign_notional_24h = std::numeric_limits<double>::quiet_NaN();
  std::chrono::nanoseconds front = {};
  std::string_view funding_base_symbol;
  std::chrono::nanoseconds funding_interval = {};
  std::string_view funding_premium_symbol;
  std::string_view funding_quote_symbol;
  double funding_rate = std::numeric_limits<double>::quiet_NaN();
  std::chrono::nanoseconds funding_timestamp = {};
  bool has_liquidity = false;
  double high_price = std::numeric_limits<double>::quiet_NaN();
  double home_notional_24h = std::numeric_limits<double>::quiet_NaN();
  double impact_ask_price = std::numeric_limits<double>::quiet_NaN();
  double impact_bid_price = std::numeric_limits<double>::quiet_NaN();
  double impact_mid_price = std::numeric_limits<double>::quiet_NaN();
  double indicative_funding_rate = std::numeric_limits<double>::quiet_NaN();
  double indicative_settle_price = std::numeric_limits<double>::quiet_NaN();
  double indicative_tax_rate = std::numeric_limits<double>::quiet_NaN();
  double init_margin = std::numeric_limits<double>::quiet_NaN();
  double insurance_fee = std::numeric_limits<double>::quiet_NaN();
  std::string_view inverse_leg;
  bool is_inverse = false;
  bool is_quanto = false;
  double last_change_pcnt = std::numeric_limits<double>::quiet_NaN();
  double last_price = std::numeric_limits<double>::quiet_NaN();
  double last_price_protected = std::numeric_limits<double>::quiet_NaN();
  std::string_view last_tick_direction;
  double limit = std::numeric_limits<double>::quiet_NaN();
  double limit_down_price = std::numeric_limits<double>::quiet_NaN();
  double limit_up_price = std::numeric_limits<double>::quiet_NaN();
  std::chrono::nanoseconds listing = {};
  double lot_size = std::numeric_limits<double>::quiet_NaN();
  double low_price = std::numeric_limits<double>::quiet_NaN();
  double maint_margin = std::numeric_limits<double>::quiet_NaN();
  double maker_fee = std::numeric_limits<double>::quiet_NaN();
  std::string_view mark_method;
  double mark_price = std::numeric_limits<double>::quiet_NaN();
  double max_order_qty = std::numeric_limits<double>::quiet_NaN();
  double max_price = std::numeric_limits<double>::quiet_NaN();
  double mid_price = std::numeric_limits<double>::quiet_NaN();
  double multiplier = std::numeric_limits<double>::quiet_NaN();
  std::chrono::nanoseconds opening_timestamp = {};
  double open_interest = std::numeric_limits<double>::quiet_NaN();
  double open_value = std::numeric_limits<double>::quiet_NaN();
  double option_multiplier = std::numeric_limits<double>::quiet_NaN();
  double option_strike_pcnt = std::numeric_limits<double>::quiet_NaN();
  double option_strike_price = std::numeric_limits<double>::quiet_NaN();
  double option_strike_round = std::numeric_limits<double>::quiet_NaN();
  double option_underlying_price = std::numeric_limits<double>::quiet_NaN();
  std::string_view position_currency;
  double prev_close_price = std::numeric_limits<double>::quiet_NaN();
  double prev_price_24h = std::numeric_limits<double>::quiet_NaN();
  double prev_total_turnover = std::numeric_limits<double>::quiet_NaN();
  double prev_total_volume = std::numeric_limits<double>::quiet_NaN();
  std::chrono::nanoseconds publish_interval = {};
  std::chrono::nanoseconds publish_time = {};
  std::string_view quote_currency;
  double quote_to_settle_multiplier = std::numeric_limits<double>::quiet_NaN();
  std::chrono::nanoseconds rebalance_interval = {};
  std::chrono::nanoseconds rebalance_timestamp = {};
  std::string_view reference;
  std::string_view reference_symbol;
  std::chrono::nanoseconds relist_interval = {};
  double risk_limit = std::numeric_limits<double>::quiet_NaN();
  double risk_step = std::numeric_limits<double>::quiet_NaN();
  std::string_view root_symbol;
  std::string_view sell_leg;
  std::chrono::nanoseconds session_interval = {};
  std::string_view settl_currency;
  std::chrono::nanoseconds settle = {};
  double settled_price = std::numeric_limits<double>::quiet_NaN();
  double settlement_fee = std::numeric_limits<double>::quiet_NaN();
  State state = State::UNDEFINED;
  std::string_view symbol;
  double taker_fee = std::numeric_limits<double>::quiet_NaN();
  bool taxed = false;
  double tick_size = std::numeric_limits<double>::quiet_NaN();
  std::chrono::nanoseconds timestamp = {};
  double total_turnover = std::numeric_limits<double>::quiet_NaN();
  double total_volume = std::numeric_limits<double>::quiet_NaN();
  double turnover = std::numeric_limits<double>::quiet_NaN();
  double turnover_24h = std::numeric_limits<double>::quiet_NaN();
  Typ typ = Typ::UNDEFINED;
  std::string_view underlying;
  std::string_view underlying_symbol;
  double underlying_to_position_multiplier = std::numeric_limits<double>::quiet_NaN();
  double underlying_to_settle_multiplier = std::numeric_limits<double>::quiet_NaN();
  double volume = std::numeric_limits<double>::quiet_NaN();
  double volume_24h = std::numeric_limits<double>::quiet_NaN();
  double vwap = std::numeric_limits<double>::quiet_NaN();
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::InstrumentItem> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::InstrumentItem& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "ask_price={}, "
        "bankrupt_limit_down_price={}, "
        "bankrupt_limit_up_price={}, "
        "bid_price={}, "
        "buy_leg=\"{}\", "
        "calc_interval={}, "
        "capped={}, "
        "closing_timestamp={}, "
        "deleverage={}, "
        "expiry={}, "
        "fair_basis={}, "
        "fair_basis_rate={}, "
        "fair_method=\"{}\", "
        "fair_price={}, "
        "foreign_notional_24h={}, "
        "front={}, "
        "funding_base_symbol=\"{}\", "
        "funding_interval={}, "
        "funding_premium_symbol=\"{}\", "
        "funding_quote_symbol=\"{}\", "
        "funding_rate={}, "
        "funding_timestamp={}, "
        "has_liquidity={}, "
        "high_price={}, "
        "home_notional_24h={}, "
        "impact_ask_price={}, "
        "impact_bid_price={}, "
        "impact_mid_price={}, "
        "indicative_funding_rate={}, "
        "indicative_settle_price={}, "
        "indicative_tax_rate={}, "
        "init_margin={}, "
        "insurance_fee={}, "
        "inverse_leg=\"{}\", "
        "is_inverse={}, "
        "is_quanto={}, "
        "last_change_pcnt={}, "
        "last_price={}, "
        "last_price_protected={}, "
        "last_tick_direction=\"{}\", "
        "limit={}, "
        "limit_down_price={}, "
        "limit_up_price={}, "
        "listing={}, "
        "lot_size={}, "
        "low_price={}, "
        "maint_margin={}, "
        "maker_fee={}, "
        "mark_method=\"{}\", "
        "mark_price={}, "
        "max_order_qty={}, "
        "max_price={}, "
        "mid_price={}, "
        "multiplier={}, "
        "opening_timestamp={}, "
        "open_interest={}, "
        "open_value={}, "
        "option_multiplier={}, "
        "option_strike_pcnt={}, "
        "option_strike_price={}, "
        "option_strike_round={}, "
        "option_underlying_price={}, "
        "position_currency=\"{}\", "
        "prev_close_price={}, "
        "prev_price_24h={}, "
        "prev_total_turnover={}, "
        "prev_total_volume={}, "
        "publish_interval={}, "
        "publish_time={}, "
        "quote_currency=\"{}\", "
        "quote_to_settle_multiplier={}, "
        "rebalance_interval={}, "
        "rebalance_timestamp={}, "
        "reference=\"{}\", "
        "reference_symbol=\"{}\", "
        "relist_interval={}, "
        "risk_limit={}, "
        "risk_step={}, "
        "root_symbol=\"{}\", "
        "sell_leg=\"{}\", "
        "session_interval={}, "
        "settl_currency=\"{}\", "
        "settle={}, "
        "settled_price={}, "
        "settlement_fee={}, "
        "state={}, "
        "symbol=\"{}\", "
        "taker_fee={}, "
        "taxed={}, "
        "tick_size={}, "
        "timestamp={}, "
        "total_turnover={}, "
        "total_volume={}, "
        "turnover={}, "
        "turnover_24h={}, "
        "typ={}, "
        "underlying=\"{}\", "
        "underlying_symbol=\"{}\", "
        "underlying_to_position_multiplier={}, "
        "underlying_to_settle_multiplier={}, "
        "volume={}, "
        "volume_24h={}, "
        "vwap={}"
        "}}",
        value.ask_price,
        value.bankrupt_limit_down_price,
        value.bankrupt_limit_up_price,
        value.bid_price,
        value.buy_leg,
        value.calc_interval,
        value.capped,
        value.closing_timestamp,
        value.deleverage,
        value.expiry,
        value.fair_basis,
        value.fair_basis_rate,
        value.fair_method,
        value.fair_price,
        value.foreign_notional_24h,
        value.front,
        value.funding_base_symbol,
        value.funding_interval,
        value.funding_premium_symbol,
        value.funding_quote_symbol,
        value.funding_rate,
        value.funding_timestamp,
        value.has_liquidity,
        value.high_price,
        value.home_notional_24h,
        value.impact_ask_price,
        value.impact_bid_price,
        value.impact_mid_price,
        value.indicative_funding_rate,
        value.indicative_settle_price,
        value.indicative_tax_rate,
        value.init_margin,
        value.insurance_fee,
        value.inverse_leg,
        value.is_inverse,
        value.is_quanto,
        value.last_change_pcnt,
        value.last_price,
        value.last_price_protected,
        value.last_tick_direction,
        value.limit,
        value.limit_down_price,
        value.limit_up_price,
        value.listing,
        value.lot_size,
        value.low_price,
        value.maint_margin,
        value.maker_fee,
        value.mark_method,
        value.mark_price,
        value.max_order_qty,
        value.max_price,
        value.mid_price,
        value.multiplier,
        value.opening_timestamp,
        value.open_interest,
        value.open_value,
        value.option_multiplier,
        value.option_strike_pcnt,
        value.option_strike_price,
        value.option_strike_round,
        value.option_underlying_price,
        value.position_currency,
        value.prev_close_price,
        value.prev_price_24h,
        value.prev_total_turnover,
        value.prev_total_volume,
        value.publish_interval,
        value.publish_time,
        value.quote_currency,
        value.quote_to_settle_multiplier,
        value.rebalance_interval,
        value.rebalance_timestamp,
        value.reference,
        value.reference_symbol,
        value.relist_interval,
        value.risk_limit,
        value.risk_step,
        value.root_symbol,
        value.sell_leg,
        value.session_interval,
        value.settl_currency,
        value.settle,
        value.settled_price,
        value.settlement_fee,
        value.state,
        value.symbol,
        value.taker_fee,
        value.taxed,
        value.tick_size,
        value.timestamp,
        value.total_turnover,
        value.total_volume,
        value.turnover,
        value.turnover_24h,
        value.typ,
        value.underlying,
        value.underlying_symbol,
        value.underlying_to_position_multiplier,
        value.underlying_to_settle_multiplier,
        value.volume,
        value.volume_24h,
        value.vwap);
  }
};
