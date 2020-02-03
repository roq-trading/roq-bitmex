/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <chrono>
#include <limits>
#include <string_view>

#include "roq/core/json/parser.h"

namespace roq {
namespace bitmex {
namespace json {

struct Position final {
  Position() = default;

  Position(const Position&) = delete;
  Position(Position&&) = default;

  uint64_t account = 0;
  double avg_cost_price = std::numeric_limits<double>::quiet_NaN();
  double avg_entry_price = std::numeric_limits<double>::quiet_NaN();
  double bankrupt_price = std::numeric_limits<double>::quiet_NaN();
  double break_even_price = std::numeric_limits<double>::quiet_NaN();
  double commission = std::numeric_limits<double>::quiet_NaN();
  bool cross_margin = false;
  std::string_view currency;
  double current_comm = std::numeric_limits<double>::quiet_NaN();
  double current_cost = std::numeric_limits<double>::quiet_NaN();
  double current_qty = std::numeric_limits<double>::quiet_NaN();
  std::chrono::nanoseconds current_timestamp = {};
  double deleverage_percentile = std::numeric_limits<double>::quiet_NaN();
  double exec_buy_cost = std::numeric_limits<double>::quiet_NaN();
  double exec_buy_qty = std::numeric_limits<double>::quiet_NaN();
  double exec_comm = std::numeric_limits<double>::quiet_NaN();
  double exec_cost = std::numeric_limits<double>::quiet_NaN();
  double exec_qty = std::numeric_limits<double>::quiet_NaN();
  double exec_sell_cost = std::numeric_limits<double>::quiet_NaN();
  double exec_sell_qty = std::numeric_limits<double>::quiet_NaN();
  double foreign_notional = std::numeric_limits<double>::quiet_NaN();
  double gross_exec_cost = std::numeric_limits<double>::quiet_NaN();
  double gross_open_cost = std::numeric_limits<double>::quiet_NaN();
  double gross_open_premium = std::numeric_limits<double>::quiet_NaN();
  double home_notional = std::numeric_limits<double>::quiet_NaN();
  double indicative_tax = std::numeric_limits<double>::quiet_NaN();
  double indicative_tax_rate = std::numeric_limits<double>::quiet_NaN();
  double init_margin = std::numeric_limits<double>::quiet_NaN();
  double init_margin_req = std::numeric_limits<double>::quiet_NaN();
  bool is_open = false;
  double last_price = std::numeric_limits<double>::quiet_NaN();
  double last_value = std::numeric_limits<double>::quiet_NaN();
  double leverage = std::numeric_limits<double>::quiet_NaN();
  double liquidation_price = std::numeric_limits<double>::quiet_NaN();
  double long_bankrupt = std::numeric_limits<double>::quiet_NaN();
  double maint_margin = std::numeric_limits<double>::quiet_NaN();
  double maint_margin_req = std::numeric_limits<double>::quiet_NaN();
  double margin_call_price = std::numeric_limits<double>::quiet_NaN();
  double mark_price = std::numeric_limits<double>::quiet_NaN();
  double mark_value = std::numeric_limits<double>::quiet_NaN();
  double opening_comm = std::numeric_limits<double>::quiet_NaN();
  double opening_cost = std::numeric_limits<double>::quiet_NaN();
  double opening_qty = std::numeric_limits<double>::quiet_NaN();
  std::chrono::nanoseconds opening_timestamp = {};
  double open_order_buy_cost = std::numeric_limits<double>::quiet_NaN();
  double open_order_buy_premium = std::numeric_limits<double>::quiet_NaN();
  double open_order_buy_qty = std::numeric_limits<double>::quiet_NaN();
  double open_order_sell_cost = std::numeric_limits<double>::quiet_NaN();
  double open_order_sell_premium = std::numeric_limits<double>::quiet_NaN();
  double open_order_sell_qty = std::numeric_limits<double>::quiet_NaN();
  double pos_allowance = std::numeric_limits<double>::quiet_NaN();
  double pos_comm = std::numeric_limits<double>::quiet_NaN();
  double pos_cost = std::numeric_limits<double>::quiet_NaN();
  double pos_cost2 = std::numeric_limits<double>::quiet_NaN();
  double pos_cross = std::numeric_limits<double>::quiet_NaN();
  double pos_init = std::numeric_limits<double>::quiet_NaN();
  double pos_loss = std::numeric_limits<double>::quiet_NaN();
  double pos_maint = std::numeric_limits<double>::quiet_NaN();
  double pos_margin = std::numeric_limits<double>::quiet_NaN();
  double pos_state = std::numeric_limits<double>::quiet_NaN();
  double prev_close_price = std::numeric_limits<double>::quiet_NaN();
  double prev_realised_pnl = std::numeric_limits<double>::quiet_NaN();
  double prev_unrealised_pnl = std::numeric_limits<double>::quiet_NaN();
  std::string_view quote_currency;
  double realised_cost = std::numeric_limits<double>::quiet_NaN();
  double realised_gross_pnl = std::numeric_limits<double>::quiet_NaN();
  double realised_pnl = std::numeric_limits<double>::quiet_NaN();
  double realised_tax = std::numeric_limits<double>::quiet_NaN();
  double rebalanced_pnl = std::numeric_limits<double>::quiet_NaN();
  double risk_limit = std::numeric_limits<double>::quiet_NaN();
  double risk_value = std::numeric_limits<double>::quiet_NaN();
  double session_margin = std::numeric_limits<double>::quiet_NaN();
  double short_bankrupt = std::numeric_limits<double>::quiet_NaN();
  double simple_cost = std::numeric_limits<double>::quiet_NaN();
  double simple_pnl = std::numeric_limits<double>::quiet_NaN();
  double simple_pnl_pcnt = std::numeric_limits<double>::quiet_NaN();
  double simple_qty = std::numeric_limits<double>::quiet_NaN();
  double simple_value = std::numeric_limits<double>::quiet_NaN();
  std::string_view symbol;
  double target_excess_margin;
  double taxable_margin = std::numeric_limits<double>::quiet_NaN();
  double tax_base = std::numeric_limits<double>::quiet_NaN();
  std::chrono::nanoseconds timestamp = {};
  std::string_view underlying;
  double unrealised_cost = std::numeric_limits<double>::quiet_NaN();
  double unrealised_gross_pnl = std::numeric_limits<double>::quiet_NaN();
  double unrealised_pnl = std::numeric_limits<double>::quiet_NaN();
  double unrealised_pnl_pcnt = std::numeric_limits<double>::quiet_NaN();
  double unrealised_roq_pcnt = std::numeric_limits<double>::quiet_NaN();
  double unrealised_tax = std::numeric_limits<double>::quiet_NaN();
  double var_margin = std::numeric_limits<double>::quiet_NaN();

  static Position parse(core::json::value_t&);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::Position> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::Position& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "ask_price={}, "
        "vwap={}"
        "account={}, "
        "avg_cost_price={}, "
        "avg_entry_price={}, "
        "bankrupt_price={}, "
        "break_even_price={}, "
        "commission={}, "
        "cross_margin={}, "
        "currency=\"{}\", "
        "current_comm={}, "
        "current_cost={}, "
        "current_qty={}, "
        "current_timestamp={}, "
        "deleverage_percentile={}, "
        "exec_buy_cost={}, "
        "exec_buy_qty={}, "
        "exec_comm={}, "
        "exec_cost={}, "
        "exec_qty={}, "
        "exec_sell_cost={}, "
        "exec_sell_qty={}, "
        "foreign_notional={}, "
        "gross_exec_cost={}, "
        "gross_open_cost={}, "
        "gross_open_premium={}, "
        "home_notional={}, "
        "indicative_tax={}, "
        "indicative_tax_rate={}, "
        "init_margin={}, "
        "init_margin_req={}, "
        "is_open={}, "
        "last_price={}, "
        "last_value={}, "
        "leverage={}, "
        "liquidation_price={}, "
        "long_bankrupt={}, "
        "maint_margin={}, "
        "maint_margin_req={}, "
        "margin_call_price={}, "
        "mark_price={}, "
        "mark_value={}, "
        "opening_comm={}, "
        "opening_cost={}, "
        "opening_qty={}, "
        "opening_timestamp={}, "
        "open_order_buy_cost={}, "
        "open_order_buy_premium={}, "
        "open_order_buy_qty={}, "
        "open_order_sell_cost={}, "
        "open_order_sell_premium={}, "
        "open_order_sell_qty={}, "
        "pos_allowance={}, "
        "pos_comm={}, "
        "pos_cost={}, "
        "pos_cost2={}, "
        "pos_cross={}, "
        "pos_init={}, "
        "pos_loss={}, "
        "pos_maint={}, "
        "pos_margin={}, "
        "pos_state={}, "
        "prev_close_price={}, "
        "prev_realised_pnl={}, "
        "prev_unrealised_pnl={}, "
        "quote_currency=\"{}\", "
        "realised_cost={}, "
        "realised_gross_pnl={}, "
        "realised_pnl={}, "
        "realised_tax={}, "
        "rebalanced_pnl={}, "
        "risk_limit={}, "
        "risk_value={}, "
        "session_margin={}, "
        "short_bankrupt={}, "
        "simple_cost={}, "
        "simple_pnl={}, "
        "simple_pnl_pcnt={}, "
        "simple_qty={}, "
        "simple_value={}, "
        "symbol=\"{}\", "
        "target_excess_margin={}, "
        "taxable_margin={}, "
        "tax_base={}, "
        "timestamp={}, "
        "underlying=\"{}\", "
        "unrealised_cost={}, "
        "unrealised_gross_pnl={}, "
        "unrealised_pnl={}, "
        "unrealised_pnl_pcnt={}, "
        "unrealised_roq_pcnt={}, "
        "unrealised_tax={}, "
        "var_margin={}"
        "}}",
        value.account,
        value.avg_cost_price,
        value.avg_entry_price,
        value.bankrupt_price,
        value.break_even_price,
        value.commission,
        value.cross_margin,
        value.currency,
        value.current_comm,
        value.current_cost,
        value.current_qty,
        value.current_timestamp,
        value.deleverage_percentile,
        value.exec_buy_cost,
        value.exec_buy_qty,
        value.exec_comm,
        value.exec_cost,
        value.exec_qty,
        value.exec_sell_cost,
        value.exec_sell_qty,
        value.foreign_notional,
        value.gross_exec_cost,
        value.gross_open_cost,
        value.gross_open_premium,
        value.home_notional,
        value.indicative_tax,
        value.indicative_tax_rate,
        value.init_margin,
        value.init_margin_req,
        value.is_open,
        value.last_price,
        value.last_value,
        value.leverage,
        value.liquidation_price,
        value.long_bankrupt,
        value.maint_margin,
        value.maint_margin_req,
        value.margin_call_price,
        value.mark_price,
        value.mark_value,
        value.opening_comm,
        value.opening_cost,
        value.opening_qty,
        value.opening_timestamp,
        value.open_order_buy_cost,
        value.open_order_buy_premium,
        value.open_order_buy_qty,
        value.open_order_sell_cost,
        value.open_order_sell_premium,
        value.open_order_sell_qty,
        value.pos_allowance,
        value.pos_comm,
        value.pos_cost,
        value.pos_cost2,
        value.pos_cross,
        value.pos_init,
        value.pos_loss,
        value.pos_maint,
        value.pos_margin,
        value.pos_state,
        value.prev_close_price,
        value.prev_realised_pnl,
        value.prev_unrealised_pnl,
        value.quote_currency,
        value.realised_cost,
        value.realised_gross_pnl,
        value.realised_pnl,
        value.realised_tax,
        value.rebalanced_pnl,
        value.risk_limit,
        value.risk_value,
        value.session_margin,
        value.short_bankrupt,
        value.simple_cost,
        value.simple_pnl,
        value.simple_pnl_pcnt,
        value.simple_qty,
        value.simple_value,
        value.symbol,
        value.target_excess_margin,
        value.taxable_margin,
        value.tax_base,
        value.timestamp,
        value.underlying,
        value.unrealised_cost,
        value.unrealised_gross_pnl,
        value.unrealised_pnl,
        value.unrealised_pnl_pcnt,
        value.unrealised_roq_pcnt,
        value.unrealised_tax,
        value.var_margin);
  }
};
