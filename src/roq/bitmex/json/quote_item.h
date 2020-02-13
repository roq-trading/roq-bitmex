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
  explicit QuoteItem(core::json::value_t& value);

  QuoteItem(const QuoteItem&) = delete;
  QuoteItem(QuoteItem&&) = delete;

  double ask_price = std::numeric_limits<double>::quiet_NaN();
  double ask_size = std::numeric_limits<double>::quiet_NaN();
  double bid_price = std::numeric_limits<double>::quiet_NaN();
  double bid_size = std::numeric_limits<double>::quiet_NaN();
  std::string_view symbol;
  std::chrono::nanoseconds timestamp = {};
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::QuoteItem> {
  template <typename Context>
  constexpr auto parse(Context& context) {
    return context.begin();
  }
  template <typename Context>
  auto format(
      const roq::bitmex::json::QuoteItem& value,
      Context& context) {
    return format_to(
        context.out(),
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
