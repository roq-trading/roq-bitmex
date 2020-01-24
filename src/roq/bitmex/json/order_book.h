/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <limits>
#include <string_view>

#include "roq/core/json/parser.h"

namespace roq {
namespace bitmex {
namespace json {

struct OrderBook final {
  uint64_t id = 0;
  double price = std::numeric_limits<double>::quiet_NaN();
  std::string_view side;
  double size = std::numeric_limits<double>::quiet_NaN();
  std::string_view symbol;

  static OrderBook parse(const std::string_view& message);
  static void parse(OrderBook&, core::json::object_t&&);

  static OrderBook parse(core::json::object_t& object);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::OrderBook> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::OrderBook& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "id={}, "
        "price={}, "
        "side=\"{}\", "
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
