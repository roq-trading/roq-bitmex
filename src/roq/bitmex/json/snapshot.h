/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <string_view>

#include "roq/core/json/buffer.h"

namespace roq {
namespace bitmex {
namespace json {

struct Snapshot final {
  struct {
    struct {
      double price;
      double size;
    } *items = nullptr;
    size_t length = 0;
  } bids, asks;
  std::string_view product_id;

  static Snapshot parse(
      const std::string_view& message,
      core::json::Buffer& buffer);

  using bid_t = std::remove_pointer<decltype(bids.items)>::type;
  using ask_t = std::remove_pointer<decltype(asks.items)>::type;
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::Snapshot> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::Snapshot& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "bids.length={}, "
        "asks.length={}, "
        "product_id=\"{}\""
        "}}",
        value.bids.length,
        value.asks.length,
        value.product_id);
  }
};
