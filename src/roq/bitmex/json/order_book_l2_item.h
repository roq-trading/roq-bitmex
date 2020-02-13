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

struct OrderBookL2Item final {
  explicit OrderBookL2Item(core::json::value_t& value);

  OrderBookL2Item(const OrderBookL2Item&) = delete;
  OrderBookL2Item(OrderBookL2Item&&) = delete;

  uint64_t id = 0;
  double price = std::numeric_limits<double>::quiet_NaN();
  Side side = Side::UNDEFINED;
  double size = std::numeric_limits<double>::quiet_NaN();
  std::string_view symbol;
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::OrderBookL2Item> {
  template <typename Context>
  constexpr auto parse(Context& context) {
    return context.begin();
  }
  template <typename Context>
  auto format(
      const roq::bitmex::json::OrderBookL2Item& value,
      Context& context) {
    return format_to(
        context.out(),
        "{{"
        "id={}, "
        "price={}, "
        "side={}, "
        "size={}, "
        "symbol=\"{}\""
        "}}",
        value.id,
        value.price,
        value.side,
        value.size,
        value.symbol);
  }
};
