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

struct FundingItem final {
  std::chrono::nanoseconds funding_interval = {};
  double funding_rate = std::numeric_limits<double>::quiet_NaN();
  double funding_rate_daily = std::numeric_limits<double>::quiet_NaN();
  std::string_view symbol;
  std::chrono::nanoseconds timestamp = {};

  static FundingItem parse(const std::string_view& message);

  void parse(core::json::object_t& object);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::FundingItem> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::FundingItem& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "funding_interval={}, "
        "funding_rate={}, "
        "funding_rate_daily={}, "
        "symbol=\"{}\", "
        "timestamp={}"
        "}}",
        value.funding_interval,
        value.funding_rate,
        value.funding_rate_daily,
        value.symbol,
        value.timestamp);
  }
};
