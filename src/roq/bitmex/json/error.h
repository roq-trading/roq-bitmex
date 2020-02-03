/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <string_view>

#include "roq/core/json/parser.h"

namespace roq {
namespace bitmex {
namespace json {

struct Error final {
  Error() = default;

  Error(const Error&) = delete;
  Error(Error&&) = default;

  std::string_view error;
  int32_t status = 0;
  // missing:
  // - meta
  // - request

  static Error parse(core::json::value_t&);
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
        "error=\"{}\", "
        "status={}"
        "}}",
        value.error,
        value.status);
  }
};

