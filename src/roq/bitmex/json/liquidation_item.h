/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <chrono>
#include <limits>
#include <string_view>

#include "roq/core/json/parser.h"

#include "roq/bitmex/json/side.h"

namespace roq {
namespace bitmex {
namespace json {

struct LiquidationItem final {
  explicit LiquidationItem(core::json::value_t& value);

  LiquidationItem(const LiquidationItem&) = delete;
  LiquidationItem(LiquidationItem&&) = delete;

  double leaves_qty = std::numeric_limits<double>::quiet_NaN();
  std::string_view order_id;
  double price = std::numeric_limits<double>::quiet_NaN();
  Side side = Side::UNKNOWN;
  std::string_view symbol;
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::LiquidationItem> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::LiquidationItem& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "leaves_qty={}, "
        "order_id=\"{}\", "
        "price={}, "
        "side={}, "
        "symbol=\"{}\""
        "}}",
        value.leaves_qty,
        value.order_id,
        value.price,
        value.side,
        value.symbol);
  }
};
