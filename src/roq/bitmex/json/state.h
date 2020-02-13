/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <string_view>

namespace roq {
namespace bitmex {
namespace json {

enum class State {
  UNDEFINED,
  UNKNOWN,
  CLOSED,
  OPEN,
  SETTLED,
  UNLISTED,
};

extern State parse_state(const std::string_view& name);

inline auto EnumNamesState() {
  static const std::string_view names[] = {
    "UNDEFINED",
    "UNKNOWN",
    "CLOSED",
    "OPEN",
    "SETTLED",
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
  template <typename Context>
  constexpr auto parse(Context& context) {
    return context.begin();
  }
  template <typename Context>
  auto format(
      const roq::bitmex::json::State value,
      Context& context) {
    return format_to(
        context.out(),
        "{}",
        roq::bitmex::json::EnumNameState(value));
  }
};
