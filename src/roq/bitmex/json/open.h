/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <chrono>
#include <limits>
#include <string_view>

#include "roq/bitmex/api/enums.h"

namespace roq {
namespace bitmex {
namespace json {

struct Open final {
  std::string_view order_id;
  double price = std::numeric_limits<double>::quiet_NaN();
  std::string_view product_id;
  double remaining_size = std::numeric_limits<double>::quiet_NaN();
  uint64_t sequence = 0;
  api::Side side = api::Side::UNKNOWN;
  std::chrono::nanoseconds time = {};

  static Open parse(const std::string_view& message);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::Open> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::Open& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "order_id=\"{}\", "
        "price={}, "
        "product_id=\"{}\", "
        "remaining_size={}, "
        "sequence={}, "
        "side={}, "
        "time={}"
        "}}",
        value.order_id,
        value.price,
        value.product_id,
        value.remaining_size,
        value.sequence,
        value.side,
        value.time);
  }
};
