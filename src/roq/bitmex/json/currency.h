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

struct Currency {
  // TODO(thraneh): convertible_to, details
  std::string_view display_name;
  std::string_view funding_account_id;
  std::string_view id;
  double max_precision = std::numeric_limits<double>::quiet_NaN();
  double min_size = std::numeric_limits<double>::quiet_NaN();
  std::string_view name;
  uint32_t network_confirmations = 0;
  int32_t sort_order = 0;
  std::string_view status_message;
  api::XStatus status = api::XStatus::UNKNOWN;
  std::string_view symbol;
  std::string_view type;

  static Currency parse(const std::string_view& message);
  static void parse(Currency&, core::json::object_t&&);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::Currency> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::Currency& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "display_name=\"{}\", "
        "funding_account_id=\"{}\", "
        "id=\"{}\", "
        "max_precision={}, "
        "min_precision={}, "
        "name=\"{}\", "
        "network_confirmations={}, "
        "sort_order={}, "
        "status_message=\"{}\", "
        "status={}, "
        "symbol=\"{}\", "
        "type=\"{}\""
        "}}",
        value.display_name,
        value.funding_account_id,
        value.id,
        value.max_precision,
        value.min_size,
        value.name,
        value.network_confirmations,
        value.sort_order,
        value.status_message,
        value.status,
        value.symbol,
        value.type);
  }
};
