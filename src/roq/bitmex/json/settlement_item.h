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

struct SettlementItem final {
  double bankrupt = std::numeric_limits<double>::quiet_NaN();
  double option_strike_price = std::numeric_limits<double>::quiet_NaN();
  double option_underlying_price = std::numeric_limits<double>::quiet_NaN();
  double settled_price = std::numeric_limits<double>::quiet_NaN();
  std::string_view symbol;
  double tax_base = std::numeric_limits<double>::quiet_NaN();
  double tax_rate = std::numeric_limits<double>::quiet_NaN();
  std::chrono::nanoseconds timestamp = {};

  static SettlementItem parse(const std::string_view& message);

  void parse(core::json::object_t& object);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::SettlementItem> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::SettlementItem& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "bankrupt={}, "
        "option_strike_price={}, "
        "option_underlying_price={}, "
        "settled_price={}, "
        "symbol=\"{}\", "
        "tax_base={}, "
        "tax_rate={}, "
        "timestamp={}"
        "}}",
        value.bankrupt,
        value.option_strike_price,
        value.option_underlying_price,
        value.settled_price,
        value.symbol,
        value.tax_base,
        value.tax_rate,
        value.timestamp);
  }
};
