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

struct ExecutionItem final {
  explicit ExecutionItem(core::json::value_t& value);

  ExecutionItem(const ExecutionItem&) = delete;
  ExecutionItem(ExecutionItem&&) = delete;

  uint64_t account = 0;
  double avg_px = std::numeric_limits<double>::quiet_NaN();
  std::string_view cl_ord_id;
  std::string_view cl_ord_link_id;
  double commission = std::numeric_limits<double>::quiet_NaN();
  std::string_view contingency_type;
  double cum_qty = std::numeric_limits<double>::quiet_NaN();
  std::string_view currency;
  double display_qty = std::numeric_limits<double>::quiet_NaN();
  std::string_view ex_destination;
  double exec_comm = std::numeric_limits<double>::quiet_NaN();
  double exec_cost = std::numeric_limits<double>::quiet_NaN();
  std::string_view exec_id;
  std::string_view exec_inst;
  std::string_view exec_type;
  double foreign_notional = std::numeric_limits<double>::quiet_NaN();
  double home_notional = std::numeric_limits<double>::quiet_NaN();
  std::string_view last_liquidity_ind;
  std::string_view last_mkt;
  double last_px = std::numeric_limits<double>::quiet_NaN();
  double last_qty = std::numeric_limits<double>::quiet_NaN();
  double leaves_qty = std::numeric_limits<double>::quiet_NaN();
  std::string_view multi_leg_reporting_type;
  std::string_view ord_rej_reason;
  std::string_view ord_status;
  std::string_view ord_type;
  std::string_view order_id;
  double order_qty = std::numeric_limits<double>::quiet_NaN();
  double peg_offset_value = std::numeric_limits<double>::quiet_NaN();
  std::string_view peg_price_type;
  double price = std::numeric_limits<double>::quiet_NaN();
  std::string_view settl_currency;
  std::string_view side;
  double simple_cum_qty = std::numeric_limits<double>::quiet_NaN();
  double simple_leaves_qty = std::numeric_limits<double>::quiet_NaN();
  double simple_order_qty = std::numeric_limits<double>::quiet_NaN();
  double stop_px = std::numeric_limits<double>::quiet_NaN();
  std::string_view symbol;
  std::string_view text;
  std::string_view time_in_force;
  std::chrono::nanoseconds timestamp = {};
  std::string_view trade_publish_indicator;
  std::chrono::nanoseconds transact_time = {};
  std::string_view trd_match_id;
  std::string_view triggered;
  double underlying_last_px = std::numeric_limits<double>::quiet_NaN();
  bool working_indicator = false;
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::ExecutionItem> {
  template <typename Context>
  constexpr auto parse(Context& context) {
    return context.begin();
  }
  template <typename Context>
  auto format(
      const roq::bitmex::json::ExecutionItem& value,
      Context& context) {
    return format_to(
        context.out(),
        "{{"
        "account={}, "
        "avg_px={}, "
        "cl_ord_id=\"{}\", "
        "cl_ord_link_id=\"{}\", "
        "commission={}, "
        "contingency_type=\"{}\", "
        "cum_qty={}, "
        "currency=\"{}\", "
        "display_qty={}, "
        "ex_destination=\"{}\", "
        "exec_comm={}, "
        "exec_cost={}, "
        "exec_id=\"{}\", "
        "exec_inst=\"{}\", "
        "exec_type=\"{}\", "
        "foreign_notional={}, "
        "home_notional={}, "
        "last_liquidity_ind=\"{}\", "
        "last_mkt=\"{}\", "
        "last_px={}, "
        "last_qty={}, "
        "leaves_qty={}, "
        "multi_leg_reporting_type=\"{}\", "
        "ord_rej_reason=\"{}\", "
        "ord_status=\"{}\", "
        "ord_type=\"{}\", "
        "order_id=\"{}\", "
        "order_qty={}, "
        "peg_offset_value={}, "
        "peg_price_type=\"{}\", "
        "price={}, "
        "settl_currency=\"{}\", "
        "side=\"{}\", "
        "simple_cum_qty={}, "
        "simple_leaves_qty={}, "
        "simple_order_qty={}, "
        "stop_px={}, "
        "symbol=\"{}\", "
        "text=\"{}\", "
        "time_in_force=\"{}\", "
        "timestamp={}, "
        "trade_publish_indicator=\"{}\", "
        "transact_time={}, "
        "trd_match_id=\"{}\", "
        "triggered=\"{}\", "
        "underlying_last_px={}, "
        "working_indicator={}"
        "}}",
        value.account,
        value.avg_px,
        value.cl_ord_id,
        value.cl_ord_link_id,
        value.commission,
        value.contingency_type,
        value.cum_qty,
        value.currency,
        value.display_qty,
        value.ex_destination,
        value.exec_comm,
        value.exec_cost,
        value.exec_id,
        value.exec_inst,
        value.exec_type,
        value.foreign_notional,
        value.home_notional,
        value.last_liquidity_ind,
        value.last_mkt,
        value.last_px,
        value.last_qty,
        value.leaves_qty,
        value.multi_leg_reporting_type,
        value.ord_rej_reason,
        value.ord_status,
        value.ord_type,
        value.order_id,
        value.order_qty,
        value.peg_offset_value,
        value.peg_price_type,
        value.price,
        value.settl_currency,
        value.side,
        value.simple_cum_qty,
        value.simple_leaves_qty,
        value.simple_order_qty,
        value.stop_px,
        value.symbol,
        value.text,
        value.time_in_force,
        value.timestamp,
        value.trade_publish_indicator,
        value.transact_time,
        value.trd_match_id,
        value.triggered,
        value.underlying_last_px,
        value.working_indicator);
  }
};
