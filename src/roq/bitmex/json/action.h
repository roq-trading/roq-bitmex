/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <string_view>

namespace roq {
namespace bitmex {
namespace json {

enum class Action {
  UNDEFINED,
  UNKNOWN,
  DELETE,
  INSERT,
  PARTIAL,
  UPDATE,
};

extern Action parse_action(const std::string_view& name);

inline auto EnumNamesAction() {
  static const std::string_view names[] = {
    "UNDEFINED",
    "UNKNOWN",
    "DELETE",
    "INSERT",
    "PARTIAL",
    "UPDATE",
  };
  return names;
}

inline auto EnumNameAction(Action e) {
  using value_type = std::underlying_type_t<decltype(e)>;
  return EnumNamesAction()[static_cast<value_type>(e)];
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::Action> {
  template <typename T>
  constexpr auto parse(T& ctx) {
    return ctx.begin();
  }
  template <typename T>
  auto format(const roq::bitmex::json::Action value, T& ctx) {
    return format_to(
        ctx.out(),
        "{}",
        roq::bitmex::json::EnumNameAction(value));
  }
};

