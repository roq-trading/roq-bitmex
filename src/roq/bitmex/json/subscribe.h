/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <string_view>

namespace roq {
namespace bitmex {
namespace json {

struct Subscribe final {
  bool failure = false;
  std::string_view subscribe;
  bool success = false;
  // missing:
  // - request

  static Subscribe parse(const std::string_view& message);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::Subscribe> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::Subscribe& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "failure={}, "
        "subscribe=\"{}\", "
        "success={}"
        "}}",
        value.failure,
        value.subscribe,
        value.success);
  }
};

