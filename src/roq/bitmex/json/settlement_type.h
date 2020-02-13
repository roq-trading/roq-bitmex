/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <string_view>

namespace roq {
namespace bitmex {
namespace json {

enum class SettlementType {
  UNDEFINED,
  UNKNOWN,
  REBALANCE,
  SETTLEMENT,
};

extern SettlementType parse_settlement_type(const std::string_view& name);

inline auto EnumNamesSettlementType() {
  static const std::string_view names[] = {
    "UNDEFINED",
    "UNKNOWN",
    "REBALANCE",
    "SETTLEMENT",
  };
  return names;
}

inline auto EnumNameSettlementType(SettlementType e) {
  using value_type = std::underlying_type_t<decltype(e)>;
  return EnumNamesSettlementType()[static_cast<value_type>(e)];
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::SettlementType> {
  template <typename Context>
  constexpr auto parse(Context& context) {
    return context.begin();
  }
  template <typename Context>
  auto format(
      const roq::bitmex::json::SettlementType value,
      Context& context) {
    return format_to(
        context.out(),
        "{}",
        roq::bitmex::json::EnumNameSettlementType(value));
  }
};
