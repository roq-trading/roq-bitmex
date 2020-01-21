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

struct Ticker final {
  double best_ask = std::numeric_limits<double>::quiet_NaN();
  double best_bid = std::numeric_limits<double>::quiet_NaN();
  double high_24h = std::numeric_limits<double>::quiet_NaN();
  double last_size = std::numeric_limits<double>::quiet_NaN();
  double low_24h = std::numeric_limits<double>::quiet_NaN();
  double open_24h = std::numeric_limits<double>::quiet_NaN();
  double price = std::numeric_limits<double>::quiet_NaN();
  std::string_view product_id;
  uint64_t sequence = 0;
  api::Side side = api::Side::UNKNOWN;
  double volume_24h = std::numeric_limits<double>::quiet_NaN();
  double volume_30d = std::numeric_limits<double>::quiet_NaN();
  // optional fields
  std::chrono::nanoseconds time = {};
  uint32_t trade_id = 0;

  static Ticker parse(const std::string_view& message);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::Ticker> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::Ticker& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "best_ask={}, "
        "best_bid={}, "
        "high_24h={}, "
        "last_size={}, "
        "low_24h={}, "
        "open_24h={}, "
        "price={}, "
        "product_id=\"{}\", "
        "sequence={}, "
        "side={}, "
        "volume_24h={}, "
        "volume_30d={}, "
        "time={}, "
        "trade_id={}"
        "}}",
        value.best_ask,
        value.best_bid,
        value.high_24h,
        value.last_size,
        value.low_24h,
        value.open_24h,
        value.price,
        value.product_id,
        value.sequence,
        value.side,
        value.volume_24h,
        value.volume_30d,
        value.time,
        value.trade_id);
  }
};
