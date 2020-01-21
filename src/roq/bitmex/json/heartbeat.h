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

struct Heartbeat final {
  uint64_t last_trade_id = 0;
  std::string_view product_id;
  uint64_t sequence = 0;
  std::chrono::nanoseconds time = {};

  static Heartbeat parse(const std::string_view& message);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::Heartbeat> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::Heartbeat& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "last_trade_id={}, "
        "product_id=\"{}\", "
        "sequence={}, "
        "time={}"
        "}}",
        value.last_trade_id,
        value.product_id,
        value.sequence,
        value.time);
  }
};
