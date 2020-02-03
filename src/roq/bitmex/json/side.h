/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <string_view>

namespace roq {
namespace bitmex {
namespace json {

enum class Side {
  UNDEFINED,
  UNKNOWN,
  BUY,
  SELL,
};

extern Side parse_side(const std::string_view& name);

inline auto EnumNamesSide() {
  static const std::string_view names[] = {
    "UNDEFINED",
    "UNKNOWN",
    "BUY",
    "SELL",
  };
  return names;
}

inline auto EnumNameSide(Side e) {
  using value_type = std::underlying_type_t<decltype(e)>;
  return EnumNamesSide()[static_cast<value_type>(e)];
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::Side> {
  template <typename T>
  constexpr auto parse(T& ctx) {
    return ctx.begin();
  }
  template <typename T>
  auto format(const roq::bitmex::json::Side value, T& ctx) {
    return format_to(
        ctx.out(),
        "{}",
        roq::bitmex::json::EnumNameSide(value));
  }
};

