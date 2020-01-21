/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <limits>
#include <string_view>

#include "roq/core/json/parser.h"

#include "roq/bitmex/api/enums.h"

namespace roq {
namespace bitmex {
namespace json {

struct Product final {
  std::string_view base_currency;
  double base_increment = std::numeric_limits<double>::quiet_NaN();
  double base_max_size = std::numeric_limits<double>::quiet_NaN();
  double base_min_size = std::numeric_limits<double>::quiet_NaN();
  bool cancel_only = false;
  std::string_view display_name;
  std::string_view id;
  bool limit_only = false;
  bool margin_enabled = false;
  double max_market_funds = std::numeric_limits<double>::quiet_NaN();
  double min_market_funds = std::numeric_limits<double>::quiet_NaN();
  bool post_only = false;
  std::string_view quote_currency;
  double quote_increment = std::numeric_limits<double>::quiet_NaN();
  std::string_view status_message;
  api::XStatus status = api::XStatus::UNKNOWN;

  static Product parse(const std::string_view& message);
  static void parse(Product&, core::json::object_t&&);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::Product> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::Product& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "base_currenc=\"{}\", "
        "base_increment={}, "
        "base_max_size={}, "
        "base_min_size={}, "
        "cancel_only={}, "
        "display_name=\"{}\", "
        "id=\"{}\", "
        "limit_only={}, "
        "margin_enabled={}, "
        "max_market_funds={}, "
        "min_market_funds={}, "
        "post_only={}, "
        "quote_currency=\"{}\", "
        "quote_increment={}, "
        "status_message=\"{}\", "
        "status={}"
        "}}",
        value.base_currency,
        value.base_increment,
        value.base_max_size,
        value.base_min_size,
        value.cancel_only,
        value.display_name,
        value.id,
        value.limit_only,
        value.margin_enabled,
        value.max_market_funds,
        value.min_market_funds,
        value.post_only,
        value.quote_currency,
        value.quote_increment,
        value.status_message,
        value.status);
  }
};
