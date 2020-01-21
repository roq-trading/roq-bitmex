/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once


#include <fmt/chrono.h>
#include <fmt/format.h>

#include <chrono>
#include <limits>
#include <string_view>
#include <type_traits>

#include "roq/core/json/buffer.h"

#include "roq/bitmex/api/enums.h"

namespace roq {
namespace bitmex {
namespace json {

struct L2Update final {
  struct {
    struct {
      double price = std::numeric_limits<double>::quiet_NaN();
      api::Side side = api::Side::UNKNOWN;
      double size = std::numeric_limits<double>::quiet_NaN();
    } *items = nullptr;
    size_t length = 0;
  } changes;
  std::string_view product_id;
  std::chrono::nanoseconds time;

  static L2Update parse(
      const std::string_view& message,
      core::json::Buffer& buffer);

  using change_t = std::remove_pointer<decltype(changes.items)>::type;
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::L2Update::change_t> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::L2Update::change_t& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "price={}, "
        "side={}, "
        "size={}"
        "}}",
        value.price,
        value.side,
        value.size);
  }
};

template <>
struct fmt::formatter<roq::bitmex::json::L2Update> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::L2Update& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "changes=[{}], "
        "product_id=\"{}\", "
        "time={}"
        "}}",
        fmt::join(
          value.changes.items,
          value.changes.items + value.changes.length,
          ", "),
        value.product_id,
        value.time);
  }
};
