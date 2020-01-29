/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include "roq/core/json/buffer.h"

#include "roq/bitmex/json/action.h"
#include "roq/bitmex/json/funding_item.h"

namespace roq {
namespace bitmex {
namespace json {

struct Funding final {
  Action action = Action::UNKNOWN;
  roq::span<FundingItem const> data;

  static Funding parse(
      const std::string_view& message,
      core::json::Buffer& buffer,
      Action action);

  static Funding parse(
      core::json::array_t& array,
      core::json::Buffer& buffer,
      Action action);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::Funding> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::Funding& value, C& ctx) {
    return format_to(
        ctx.out(),
        "action={}, "
        "data=[{}]",
        value.action,
        fmt::join(value.data, ", "));
  }
};
