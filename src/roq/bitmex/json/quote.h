/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include "roq/core/json/buffer.h"

#include "roq/bitmex/json/action.h"
#include "roq/bitmex/json/quote_item.h"

namespace roq {
namespace bitmex {
namespace json {

struct Quote final {
  Action action = Action::UNKNOWN;
  struct {
    QuoteItem *items = nullptr;
    size_t length = 0;
  } data;

  Quote *items = nullptr;
  size_t length = 0;

  static Quote parse(
      const std::string_view& message,
      core::json::Buffer& buffer,
      Action action);

  static Quote parse(
      core::json::array_t& array,
      core::json::Buffer& buffer,
      Action action);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::Quote> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::Quote& value, C& ctx) {
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
