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

struct Received final {
  std::string_view client_oid;
  std::string_view order_id;
  api::OrderType order_type = api::OrderType::UNKNOWN;
  double price = std::numeric_limits<double>::quiet_NaN();
  std::string_view product_id;
  uint64_t sequence = 0;
  api::Side side = api::Side::UNKNOWN;
  double size = std::numeric_limits<double>::quiet_NaN();
  std::chrono::nanoseconds time = {};

  static Received parse(const std::string_view& message);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::Received> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::Received& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "client_oid=\"{}\", "
        "order_id=\"{}\", "
        "order_type={}, "
        "price={}, "
        "product_id=\"{}\", "
        "sequence={}, "
        "side={}, "
        "size={}, "
        "time={}"
        "}}",
        value.client_oid,
        value.order_id,
        value.order_type,
        value.price,
        value.product_id,
        value.sequence,
        value.side,
        value.size,
        value.time);
  }
};
