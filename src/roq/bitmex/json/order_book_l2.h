/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include "roq/core/json/buffer.h"

#include "roq/bitmex/json/action.h"
#include "roq/bitmex/json/order_book_l2_item.h"

namespace roq {
namespace bitmex {
namespace json {

struct OrderBookL2 final {
  Action action = Action::UNKNOWN;
  struct {
    OrderBookL2Item *items = nullptr;
    size_t length = 0;
  } data;

  static OrderBookL2 parse(
      const std::string_view& message,
      core::json::Buffer& buffer,
      Action action);

  static OrderBookL2 parse(
      core::json::array_t& array,
      core::json::Buffer& buffer,
      Action action);
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
        fmt::join(
          value.data.items,
          value.data.items + value.data.length,
          ", "));
  }
};
