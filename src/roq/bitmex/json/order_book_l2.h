/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include "roq/core/json/buffer.h"

#include "roq/bitmex/json/action.h"
#include "roq/bitmex/json/order_book_l2_item.h"

namespace roq {
namespace bitmex {
namespace json {

struct OrderBookL2 final {
  OrderBookL2(
      core::json::value_t& value,
      core::json::Buffer& buffer,
      Action action);

  OrderBookL2(const OrderBookL2&) = delete;
  OrderBookL2(OrderBookL2&&) = delete;

  Action action = Action::UNDEFINED;
  roq::span<OrderBookL2Item const> data;
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::OrderBookL2> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::OrderBookL2& value, C& ctx) {
    return format_to(
        ctx.out(),
        "action={}, "
        "data=[{}]",
        value.action,
        fmt::join(value.data, ", "));
  }
};
