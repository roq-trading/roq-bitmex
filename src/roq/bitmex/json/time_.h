/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <chrono>
#include <string_view>

namespace roq {
namespace bitmex {
namespace json {

struct Time final {
  std::chrono::nanoseconds epoch = {};
  std::chrono::nanoseconds iso = {};

  static Time parse(const std::string_view& message);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::Time> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::Time& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "epoch={}, "
        "iso={}"
        "}}",
        value.epoch,
        value.iso);
  }
};
