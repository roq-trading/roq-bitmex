/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include "roq/core/json/buffer.h"

#include "roq/bitmex/json/action.h"
#include "roq/bitmex/json/settlement_item.h"

namespace roq {
namespace bitmex {
namespace json {

struct Settlement final {
  Action action = Action::UNKNOWN;
  struct {
    SettlementItem *items = nullptr;
    size_t length = 0;
  } data;

  Settlement *items = nullptr;
  size_t length = 0;

  static Settlement parse(
      const std::string_view& message,
      core::json::Buffer& buffer,
      Action action);

  static Settlement parse(
      core::json::array_t& array,
      core::json::Buffer& buffer,
      Action action);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::Settlement> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::Settlement& value, C& ctx) {
    roq::span data(
        value.data.items,
        value.data.length);
    return format_to(
        ctx.out(),
        "action={}, "
        "data=[{}]",
        value.action,
        fmt::join(data, ", "));
  }
};
