/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include "roq/core/json/buffer.h"

#include "roq/bitmex/json/action.h"
#include "roq/bitmex/json/liquidation_item.h"

namespace roq {
namespace bitmex {
namespace json {

struct Liquidation final {
  Liquidation(
      core::json::value_t& value,
      core::json::Buffer& buffer,
      Action action);

  Liquidation(const Liquidation&) = delete;
  Liquidation(Liquidation&&) = delete;

  Action action = Action::UNDEFINED;
  roq::span<LiquidationItem const> data;
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::Liquidation> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::Liquidation& value, C& ctx) {
    return format_to(
        ctx.out(),
        "action={}, "
        "data=[{}]",
        value.action,
        fmt::join(value.data, ", "));
  }
};
