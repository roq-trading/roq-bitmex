/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include "roq/core/json/buffer.h"

#include "roq/bitmex/json/instrument.h"

namespace roq {
namespace bitmex {
namespace json {

struct Instruments final {
  Instrument *items = nullptr;
  size_t length = 0;

  static Instruments parse(
      const std::string_view& message,
      core::json::Buffer& buffer);

  static Instruments parse(
      core::json::array_t&& array,
      core::json::Buffer& buffer);

  static Instruments parse(
      core::json::array_t& array,
      core::json::Buffer& buffer);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::Instruments> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::Instruments& value, C& ctx) {
    return format_to(
        ctx.out(),
        "[{}]",
        fmt::join(
          value.items,
          value.items + value.length,
          ", "));
  }
};
