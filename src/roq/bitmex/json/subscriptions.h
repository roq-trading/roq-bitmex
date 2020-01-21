/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <string_view>

#include "roq/core/json/buffer.h"

namespace roq {
namespace bitmex {
namespace json {

struct Subscriptions final {
  struct {
    struct {
      std::string_view name;
      // TODO(thraneh): product_ids
    } *items = nullptr;
    size_t length = 0;
  } channels;

  static Subscriptions parse(
      const std::string_view& message,
      core::json::Buffer& buffer);

  using channel_t = std::remove_pointer<decltype(channels.items)>::type;
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::Subscriptions::channel_t> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::Subscriptions::channel_t& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "name=\"{}\""
        "}}",
        value.name);
  }
};

template <>
struct fmt::formatter<roq::bitmex::json::Subscriptions> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::Subscriptions& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "channels=[{}]"
        "}}",
        fmt::join(
          value.channels.items,
          value.channels.items + value.channels.length,
          ", "));
  }
};
