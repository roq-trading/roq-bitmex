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

struct MarginItem final {
  explicit MarginItem(core::json::value_t& value);

  MarginItem(const MarginItem&) = delete;
  MarginItem(MarginItem&&) = delete;

  uint64_t account = 0;
  std::string_view action;
  double amount = std::numeric_limits<double>::quiet_NaN();
  double available_margin = std::numeric_limits<double>::quiet_NaN();
  double commission = std::numeric_limits<double>::quiet_NaN();
  double confirmed_debit = std::numeric_limits<double>::quiet_NaN();
  std::string_view currency;
  double excess_margin = std::numeric_limits<double>::quiet_NaN();
  double excess_margin_pcnt = std::numeric_limits<double>::quiet_NaN();
  double gross_comm = std::numeric_limits<double>::quiet_NaN();
  double gross_exec_cost = std::numeric_limits<double>::quiet_NaN();
  double gross_last_value = std::numeric_limits<double>::quiet_NaN();
  double gross_mark_value = std::numeric_limits<double>::quiet_NaN();
  double gross_open_cost = std::numeric_limits<double>::quiet_NaN();
  double gross_open_premium = std::numeric_limits<double>::quiet_NaN();
  double indicative_tax = std::numeric_limits<double>::quiet_NaN();
  double init_margin = std::numeric_limits<double>::quiet_NaN();
  double maint_margin = std::numeric_limits<double>::quiet_NaN();
  double margin_balance = std::numeric_limits<double>::quiet_NaN();
  double margin_balance_pcnt = std::numeric_limits<double>::quiet_NaN();
  double margin_leverage = std::numeric_limits<double>::quiet_NaN();
  double margin_used_pcnt = std::numeric_limits<double>::quiet_NaN();
  double pending_credit = std::numeric_limits<double>::quiet_NaN();
  double pending_debit = std::numeric_limits<double>::quiet_NaN();
  double prev_realised_pnl = std::numeric_limits<double>::quiet_NaN();
  std::string_view prev_state;
  double prev_unrealised_pnl = std::numeric_limits<double>::quiet_NaN();
  double realised_pnl = std::numeric_limits<double>::quiet_NaN();
  double risk_limit = std::numeric_limits<double>::quiet_NaN();
  double risk_value = std::numeric_limits<double>::quiet_NaN();
  double session_margin = std::numeric_limits<double>::quiet_NaN();
  std::string_view state;
  double synthetic_margin = std::numeric_limits<double>::quiet_NaN();
  double target_excess_margin = std::numeric_limits<double>::quiet_NaN();
  double taxable_margin = std::numeric_limits<double>::quiet_NaN();
  std::chrono::nanoseconds timestamp = {};
  double unrealised_pnl = std::numeric_limits<double>::quiet_NaN();
  double unrealised_profit = std::numeric_limits<double>::quiet_NaN();
  double var_margin = std::numeric_limits<double>::quiet_NaN();
  double wallet_balance = std::numeric_limits<double>::quiet_NaN();
  double withdrawable_margin = std::numeric_limits<double>::quiet_NaN();
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::MarginItem> {
  template <typename Context>
  constexpr auto parse(Context& context) {
    return context.begin();
  }
  template <typename Context>
  auto format(
      const roq::bitmex::json::MarginItem& value,
      Context& context) {
    return format_to(
        context.out(),
        "{{"
        "account={}, "
        "action=\"{}\", "
        "amount={}, "
        "available_margin={}, "
        "commission={}, "
        "confirmed_debit={}, "
        "currency=\"{}\", "
        "excess_margin={}, "
        "excess_margin_pcnt={}, "
        "gross_comm={}, "
        "gross_exec_cost={}, "
        "gross_last_value={}, "
        "gross_mark_value={}, "
        "gross_open_cost={}, "
        "gross_open_premium={}, "
        "indicative_tax={}, "
        "init_margin={}, "
        "maint_margin={}, "
        "margin_balance={}, "
        "margin_balance_pcnt={}, "
        "margin_leverage={}, "
        "margin_used_pcnt={}, "
        "pending_credit={}, "
        "pending_debit={}, "
        "prev_realised_pnl={}, "
        "prev_state=\"{}\", "
        "prev_unrealised_pnl={}, "
        "realised_pnl={}, "
        "risk_limit={}, "
        "risk_value={}, "
        "session_margin={}, "
        "state=\"{}\", "
        "synthetic_margin={}, "
        "target_excess_margin={}, "
        "taxable_margin={}, "
        "timestamp={}, "
        "unrealised_pnl={}, "
        "unrealised_profit={}, "
        "var_margin={}, "
        "wallet_balance={}, "
        "withdrawable_margin={}"
        "}}",
        value.account,
        value.action,
        value.amount,
        value.available_margin,
        value.commission,
        value.confirmed_debit,
        value.currency,
        value.excess_margin,
        value.excess_margin_pcnt,
        value.gross_comm,
        value.gross_exec_cost,
        value.gross_last_value,
        value.gross_mark_value,
        value.gross_open_cost,
        value.gross_open_premium,
        value.indicative_tax,
        value.init_margin,
        value.maint_margin,
        value.margin_balance,
        value.margin_balance_pcnt,
        value.margin_leverage,
        value.margin_used_pcnt,
        value.pending_credit,
        value.pending_debit,
        value.prev_realised_pnl,
        value.prev_state,
        value.prev_unrealised_pnl,
        value.realised_pnl,
        value.risk_limit,
        value.risk_value,
        value.session_margin,
        value.state,
        value.synthetic_margin,
        value.target_excess_margin,
        value.taxable_margin,
        value.timestamp,
        value.unrealised_pnl,
        value.unrealised_profit,
        value.var_margin,
        value.wallet_balance,
        value.withdrawable_margin);
  }
};
