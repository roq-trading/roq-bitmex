/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <limits>
#include <string_view>

#include "roq/core/json/parser.h"

namespace roq {
namespace bitmex {
namespace json {

struct Account final {
  std::string_view id;
  std::string_view currency;
  double balance = std::numeric_limits<double>::quiet_NaN();
  double available = std::numeric_limits<double>::quiet_NaN();
  double hold = std::numeric_limits<double>::quiet_NaN();
  std::string_view profile_id;

  static Account parse(const std::string_view& message);
  static void parse(Account&, core::json::object_t&&);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::Account> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::Account& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "id=\"{}\", "
        "currency=\"{}\", "
        "balance={}, "
        "available={}, "
        "hold={}, "
        "profile_id=\"{}\""
        "}}",
        value.id,
        value.currency,
        value.balance,
        value.available,
        value.hold,
        value.profile_id);
  }
};

