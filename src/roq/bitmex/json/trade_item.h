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
  explicit TradeItem(core::json::value_t& value);

  TradeItem(const TradeItem&) = delete;
  TradeItem(TradeItem&&) = delete;

  double foreign_notional = std::numeric_limits<double>::quiet_NaN();
  double gross_value = std::numeric_limits<double>::quiet_NaN();
  double home_notional = std::numeric_limits<double>::quiet_NaN();
  double price = std::numeric_limits<double>::quiet_NaN();
  Side side = Side::UNDEFINED;
  double size = std::numeric_limits<double>::quiet_NaN();
  std::string_view symbol;
  std::string_view tick_direction;
  std::chrono::nanoseconds timestamp = {};
  std::string_view trd_match_id;
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::TradeItem> {
  template <typename Context>
  constexpr auto parse(Context& context) {
    return context.begin();
  }
  template <typename Context>
  auto format(
      const roq::bitmex::json::TradeItem& value,
      Context& context) {
    return format_to(
        context.out(),
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
