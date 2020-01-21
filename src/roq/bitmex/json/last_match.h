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

struct LastMatch final {
  std::string_view maker_order_id;
  double price = std::numeric_limits<double>::quiet_NaN();
  std::string_view product_id;
  uint64_t sequence = 0;
  api::Side side = api::Side::UNKNOWN;
  double size = std::numeric_limits<double>::quiet_NaN();
  std::string_view taker_order_id;
  std::chrono::nanoseconds time = {};
  uint64_t trade_id = 0;

  static LastMatch parse(const std::string_view& message);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::LastMatch> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::LastMatch& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "maker_order_id=\"{}\", "
        "price={}, "
        "product_id=\"{}\", "
        "sequence={}, "
        "side={}, "
        "size={}, "
        "taker_order_id=\"{}\", "
        "time={}, "
        "trade_id={}"
        "}}",
        value.maker_order_id,
        value.price,
        value.product_id,
        value.sequence,
        value.side,
        value.size,
        value.taker_order_id,
        value.time,
        value.trade_id);
  }
};
