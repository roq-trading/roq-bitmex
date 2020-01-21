/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include "roq/core/json/buffer.h"

#include "roq/bitmex/json/fill.h"

namespace roq {
namespace bitmex {
namespace json {

struct Fills final {
  Fill *items = nullptr;
  size_t length = 0;

  static Fills parse(
      const std::string_view& message,
      core::json::Buffer& buffer);

  using fill_t = std::remove_pointer<decltype(items)>::type;
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::Fills> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::Fills& value, C& ctx) {
    return format_to(
        ctx.out(),
        "[{}]",
        fmt::join(value.items, value.items + value.length, ", "));
  }
};
