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

struct TradeItem final {
  double foreign_notional = std::numeric_limits<double>::quiet_NaN();
  double gross_value = std::numeric_limits<double>::quiet_NaN();
  double home_notional = std::numeric_limits<double>::quiet_NaN();
  double price = std::numeric_limits<double>::quiet_NaN();
  Side side = Side::UNKNOWN;
  double size = std::numeric_limits<double>::quiet_NaN();
  std::string_view symbol;
  std::string_view tick_direction;
  std::chrono::nanoseconds timestamp = {};
  std::string_view trd_match_id;

  static TradeItem parse(const std::string_view& message);

  void parse(core::json::object_t& object);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::TradeItem> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::TradeItem& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "foreign_notional={}, "
        "gross_value={}, "
        "home_notional={}, "
        "price={}, "
        "side={}, "
        "size={}, "
        "symbol=\"{}\", "
        "tick_direction=\"{}\", "
        "timestamp={}, "
        "trd_match_id=\"{}\""
        "}}",
        value.foreign_notional,
        value.gross_value,
        value.home_notional,
        value.price,
        value.side,
        value.size,
        value.symbol,
        value.tick_direction,
        value.timestamp,
        value.trd_match_id);
  }
};
