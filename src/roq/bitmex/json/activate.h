/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <chrono>
#include <limits>

#include "roq/bitmex/api/enums.h"

namespace roq {
namespace bitmex {
namespace json {

struct Activate final {
  double funds = std::numeric_limits<double>::quiet_NaN();
  std::string_view order_id;
  bool private_ = false;
  std::string_view product_id;
  std::string_view profile_id;
  api::Side side = api::Side::UNKNOWN;
  double size = std::numeric_limits<double>::quiet_NaN();
  double stop_price = std::numeric_limits<double>::quiet_NaN();
  api::StopType stop_type = api::StopType::UNKNOWN;
  double taker_fee_rate = std::numeric_limits<double>::quiet_NaN();
  std::chrono::nanoseconds timestamp = {};
  uint32_t user_id = 0;

  static Activate parse(const std::string_view& message);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::Activate> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::Activate& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "funds={}, "
        "order_id=\"{}\", "
        "private={}, "
        "product_id=\"{}\", "
        "profile_id=\"{}\", "
        "side={}, "
        "stop_price={}, "
        "stop_type={}, "
        "taker_fee_rate={}, "
        "timestamp={}, "
        "user_id={}"
        "}}",
        value.funds,
        value.order_id,
        value.private_,
        value.product_id,
        value.profile_id,
        value.side,
        value.stop_price,
        value.stop_type,
        value.taker_fee_rate,
        value.timestamp,
        value.user_id);
  }
};
