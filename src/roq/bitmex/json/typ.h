/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <string_view>

namespace roq {
namespace bitmex {
namespace json {

// note! is this really an enum? looks more like a bitmap

enum class Typ {
  UNKNOWN,
  FFCCSX,
  FFWCSX,
  MRCXXX,
  MRIXXX,
  MRRXXX,
  OCECCS,
  OPECCS,
};

extern Typ parse_typ(const std::string_view& name);

inline auto EnumNamesTyp() {
  static const std::string_view names[] = {
    "UNKNOWN",
    "FFCCSX",
    "FFWCSX",
    "MRCXXX",
    "MRIXXX",
    "MRRXXX",
    "OCECCS",
    "OPECCS",
  };
  return names;
}

inline auto EnumNameTyp(Typ e) {
  using value_type = std::underlying_type_t<decltype(e)>;
  return EnumNamesTyp()[static_cast<value_type>(e)];
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::Typ> {
  template <typename T>
  constexpr auto parse(T& ctx) {
    return ctx.begin();
  }
  template <typename T>
  auto format(const roq::bitmex::json::Typ value, T& ctx) {
    return format_to(
        ctx.out(),
        "{}",
        roq::bitmex::json::EnumNameTyp(value));
  }
};

