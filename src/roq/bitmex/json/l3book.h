/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <limits>
#include <string_view>

#include "roq/core/json/buffer.h"

namespace roq {
namespace bitmex {
namespace json {

struct L3Book final {
  struct {
    struct {
      std::string_view order_id;
      double price = std::numeric_limits<double>::quiet_NaN();
      double size = std::numeric_limits<double>::quiet_NaN();
    } *items = nullptr;
    size_t length = 0;
  } bids, asks;
  uint64_t sequence = 0;

  static L3Book parse(
      const std::string_view& message,
      core::json::Buffer& buffer);

  using bid_t = std::remove_pointer<decltype(bids.items)>::type;
  using ask_t = std::remove_pointer<decltype(asks.items)>::type;
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::L3Book> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::L3Book& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "}}");
  }
};

