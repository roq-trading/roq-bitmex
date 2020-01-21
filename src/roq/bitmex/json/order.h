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

struct Order final {
  std::chrono::nanoseconds created_at = {};
  double executed_value = std::numeric_limits<double>::quiet_NaN();
  double filled_size = std::numeric_limits<double>::quiet_NaN();
  double fill_fees = std::numeric_limits<double>::quiet_NaN();
  std::string_view id;
  bool post_only = false;
  double price = std::numeric_limits<double>::quiet_NaN();
  std::string_view product_id;
  bool settled = false;
  api::Side side = api::Side::UNKNOWN;
  double size = std::numeric_limits<double>::quiet_NaN();
  api::OrderStatus status = api::OrderStatus::UNKNOWN;
  std::string_view stp;  // FIXME(thraneh): enum?
  api::TimeInForce time_in_force = api::TimeInForce::UNKNOWN;
  api::OrderType type = api::OrderType::UNKNOWN;

  static Order parse(const std::string_view& message);
  static void parse(Order&, core::json::object_t&&);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::Order> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::Order& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "}}");
  }
};

