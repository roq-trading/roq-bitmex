/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <string_view>

namespace roq {
namespace bitmex {
namespace json {

enum class Table {
  UNDEFINED,
  UNKNOWN,
  EXECUTION,
  FUNDING,
  INSTRUMENT,
  LIQUIDATION,
  MARGIN,
  ORDER,
  ORDER_BOOK_L2,
  POSITION,
  QUOTE,
  SETTLEMENT,
  TRADE,
};

extern Table parse_table(const std::string_view& name);

inline auto EnumNamesTable() {
  static const std::string_view names[] = {
    "UNDEFINED",
    "UNKNOWN",
    "EXECUTION",
    "FUNDING",
    "INSTRUMENT",
    "LIQUIDATION",
    "MARGIN",
    "ORDER",
    "ORDER_BOOK_L2",
    "POSITION",
    "QUOTE",
    "SETTLEMENT",
    "TRADE",
  };
  return names;
}

inline auto EnumNameTable(Table e) {
  using value_type = std::underlying_type_t<decltype(e)>;
  return EnumNamesTable()[static_cast<value_type>(e)];
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::Table> {
  template <typename Context>
  constexpr auto parse(Context& context) {
    return context.begin();
  }
  template <typename Context>
  auto format(
      const roq::bitmex::json::Table value,
      Context& context) {
    return format_to(
        context.out(),
        "{}",
        roq::bitmex::json::EnumNameTable(value));
  }
};
