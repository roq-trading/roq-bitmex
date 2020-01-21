/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include "roq/core/json/buffer.h"

#include "roq/bitmex/json/account.h"

namespace roq {
namespace bitmex {
namespace json {

struct Accounts final {
  Account *items = nullptr;
  size_t length = 0;

  static Accounts parse(
      const std::string_view& message,
      core::json::Buffer& buffer);

  using account_t = std::remove_pointer<decltype(items)>::type;
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::Accounts> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::Accounts& value, C& ctx) {
    return format_to(
        ctx.out(),
        "[{}]",
        fmt::join(value.items, value.items + value.length, ", "));
  }
};
