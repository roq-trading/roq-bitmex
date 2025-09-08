/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include "roq/compat/fmt.hpp"

#include <fmt/format.h>

#include "roq/server/flags/settings.hpp"

#include "roq/bitmex/flags/flags.hpp"
#include "roq/bitmex/flags/misc.hpp"
#include "roq/bitmex/flags/rest.hpp"
#include "roq/bitmex/flags/ws.hpp"

namespace roq {
namespace bitmex {

struct Settings final : public server::flags::Settings, public flags::Flags {
  explicit Settings(args::Parser const &);

  flags::Misc misc;
  flags::REST rest;
  flags::WS ws;
};

}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::Settings> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::bitmex::Settings const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(misc={}, )"
        R"(rest={}, )"
        R"(ws={}, )"
        R"(server={})"
        R"(}})"sv,
        value.misc,
        value.rest,
        value.ws,
        static_cast<roq::server::Settings const &>(value));
  }
};
