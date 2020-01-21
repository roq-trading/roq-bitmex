/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <string_view>

namespace roq {
namespace bitmex {
namespace json {

struct Error final {
  std::string_view message;

  static Error parse(const std::string_view& message);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::Error> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::Error& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "message=\"{}\""
        "}}",
        value.message);
  }
};

