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

struct QuoteItem final {
  double ask_price = std::numeric_limits<double>::quiet_NaN();
  double ask_size = std::numeric_limits<double>::quiet_NaN();
  double bid_price = std::numeric_limits<double>::quiet_NaN();
  double bid_size = std::numeric_limits<double>::quiet_NaN();
  std::string_view symbol;
  std::chrono::nanoseconds timestamp = {};

  static QuoteItem parse(const std::string_view& message);

  void parse(core::json::object_t& object);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::QuoteItem> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::QuoteItem& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "ask_price={}, "
        "ask_size={}, "
        "bid_price={}, "
        "bid_size={}, "
        "symbol=\"{}\", "
        "timestamp={}"
        "}}",
        value.ask_price,
        value.ask_size,
        value.bid_price,
        value.bid_size,
        value.symbol,
        value.timestamp);
  }
};
