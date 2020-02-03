/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <chrono>
#include <string_view>

#include "roq/core/json/parser.h"

namespace roq {
namespace bitmex {
namespace json {

struct Handshake final {
  Handshake() = default;

  Handshake(const Handshake&) = delete;
  Handshake(Handshake&&) = default;

  std::string_view docs;
  std::string_view info;
  std::chrono::nanoseconds timestamp = {};
  std::string_view version;
  // missing:
  // - limit { remaining }
  // - heartbeat_enabled (?)

  static Handshake parse(core::json::value_t&);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::json::Handshake> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::json::Handshake& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{{"
        "docs=\"{}\", "
        "info=\"{}\", "
        "timestamp={}, "
        "version=\"{}\""
        "}}",
        value.docs,
        value.info,
        value.timestamp,
        value.version);
  }
};

