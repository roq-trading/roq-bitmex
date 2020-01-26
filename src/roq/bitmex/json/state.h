/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <string_view>

namespace roq {
namespace bitmex {
namespace json {

enum class State {
  UNKNOWN,
  OPEN,
  UNLISTED,
};

extern State parse_state(const std::string_view& name);

inline auto EnumNamesState() {
  static const std::string_view names[] = {
    "UNKNOWN",
    "OPEN",
    "UNLISTED",
  };
  return names;
}

inline auto EnumNameState(State e) {
  using value_type = std::underlying_type_t<decltype(e)>;
  return EnumNamesState()[static_cast<value_type>(e)];
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::State> {
  template <typename T>
  constexpr auto parse(T& ctx) {
    return ctx.begin();
  }
  template <typename T>
  auto format(const roq::bitmex::json::State value, T& ctx) {
    return format_to(
        ctx.out(),
        "{}",
        roq::bitmex::json::EnumNameState(value));
  }
};

