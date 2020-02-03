/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <chrono>
#include <limits>
#include <string_view>

#include "roq/core/json/parser.h"

#include "roq/bitmex/json/settlement_type.h"

namespace roq {
namespace bitmex {
namespace json {

struct SettlementItem final {
  explicit SettlementItem(core::json::value_t& value);

  SettlementItem(const SettlementItem&) = delete;
  SettlementItem(SettlementItem&&) = delete;

  double bankrupt = std::numeric_limits<double>::quiet_NaN();
  double option_strike_price = std::numeric_limits<double>::quiet_NaN();
  double option_underlying_price = std::numeric_limits<double>::quiet_NaN();
  double settled_price = std::numeric_limits<double>::quiet_NaN();
  SettlementType settlement_type = SettlementType::UNDEFINED;
  std::string_view symbol;
  double tax_base = std::numeric_limits<double>::quiet_NaN();
  double tax_rate = std::numeric_limits<double>::quiet_NaN();
  std::chrono::nanoseconds timestamp = {};
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
        "settlement_type={}, "
        "symbol=\"{}\", "
        "tax_base={}, "
        "tax_rate={}, "
        "timestamp={}"
        "}}",
        value.bankrupt,
        value.option_strike_price,
        value.option_underlying_price,
        value.settled_price,
        value.settlement_type,
        value.symbol,
        value.tax_base,
        value.tax_rate,
        value.timestamp);
  }
};
