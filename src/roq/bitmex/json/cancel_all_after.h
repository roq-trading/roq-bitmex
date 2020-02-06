/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>
#include <fmt/chrono.h>

namespace roq {
namespace bitmex {
namespace json {

struct CancelAllAfter final {
  CancelAllAfter() = default;

  CancelAllAfter(const CancelAllAfter&) = delete;
  CancelAllAfter(CancelAllAfter&&) = default;

  std::chrono::nanoseconds cancel_time = {};
  std::chrono::nanoseconds now = {};
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::CancelAllAfter> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::CancelAllAfter& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "cancel_time={}, "
        "now={}"
        "}}",
        value.cancel_time,
        value.now);
  }
};

