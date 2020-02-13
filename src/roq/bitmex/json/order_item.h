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

struct OrderItem final {
  explicit OrderItem(core::json::value_t& value);

  OrderItem(const OrderItem&) = delete;
  OrderItem(OrderItem&&) = delete;

  uint64_t account = 0;
  double avg_px = std::numeric_limits<double>::quiet_NaN();
  std::string_view cl_ord_id;
  std::string_view cl_ord_link_id;
  std::string_view contingency_type;
  double cum_qty = std::numeric_limits<double>::quiet_NaN();
  std::string_view currency;
  double display_qty = std::numeric_limits<double>::quiet_NaN();
  std::string_view ex_destination;
  std::string_view exec_inst;
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
  double stop_px = std::numeric_limits<double>::quiet_NaN();
  std::string_view symbol;
  std::string_view text;
  std::string_view time_in_force;
  std::chrono::nanoseconds timestamp = {};
  std::chrono::nanoseconds transact_time = {};
  std::string_view triggered;
  bool working_indicator = false;
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::OrderItem> {
  template <typename Context>
  constexpr auto parse(Context& context) {
    return context.begin();
  }
  template <typename Context>
  auto format(
      const roq::bitmex::json::OrderItem& value,
      Context& context) {
    return format_to(
        context.out(),
        "{{"
        "account={}, "
        "avg_px={}, "
        "cl_ord_id=\"{}\", "
        "cl_ord_link_id=\"{}\", "
        "contingency_type=\"{}\", "
        "cum_qty={}, "
        "currency=\"{}\", "
        "display_qty={}, "
        "ex_destination=\"{}\", "
        "exec_inst=\"{}\", "
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
        "stop_px={}, "
        "symbol=\"{}\", "
        "text=\"{}\", "
        "time_in_force=\"{}\", "
        "timestamp={}, "
        "transact_time={}, "
        "triggered=\"{}\", "
        "working_indicator={}"
        "}}",
        value.account,
        value.avg_px,
        value.cl_ord_id,
        value.cl_ord_link_id,
        value.contingency_type,
        value.cum_qty,
        value.currency,
        value.display_qty,
        value.ex_destination,
        value.exec_inst,
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
        value.stop_px,
        value.symbol,
        value.text,
        value.time_in_force,
        value.timestamp,
        value.transact_time,
        value.triggered,
        value.working_indicator);
  }
};
