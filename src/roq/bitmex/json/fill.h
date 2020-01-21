/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <chrono>
#include <limits>
#include <string_view>

#include "roq/core/json/parser.h"

#include "roq/bitmex/api/enums.h"


namespace roq {
namespace bitmex {
namespace json {

struct Fill final {
  std::chrono::nanoseconds created_at = {};
  double fee = std::numeric_limits<double>::quiet_NaN();
  std::string_view fill_id;
  std::string_view liquidity;
  double price = std::numeric_limits<double>::quiet_NaN();
  std::string_view product_id;
  bool settled = false;
  api::Side side = api::Side::UNKNOWN;
  double size = std::numeric_limits<double>::quiet_NaN();
  uint64_t trade_id = 0;

  static Fill parse(const std::string_view& message);
  static void parse(Fill&, core::json::object_t&&);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::Fill> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::Fill& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "}}");
  }
};
