/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <fmt/compile.h>
#include <fmt/format.h>

#include "roq/server/flags/settings.hpp"

#include "roq/bitmex/flags/common.hpp"
#include "roq/bitmex/flags/flags.hpp"
#include "roq/bitmex/flags/rest.hpp"
#include "roq/bitmex/flags/ws.hpp"

namespace roq {
namespace bitmex {

struct Settings final : public server::flags::Settings, public flags::Flags {
  explicit Settings(args::Parser const &);

  flags::Common common;
  flags::REST rest;
  flags::WS ws;
};

}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::Settings> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(roq::bitmex::Settings const &value, Context &context) const {
    using namespace fmt::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(common={}, )"
        R"(rest={}, )"
        R"(ws={}, )"
        R"(server={})"
        R"(}})"_cf,
        value.common,
        value.rest,
        value.ws,
        static_cast<roq::server::Settings const &>(value));
  }
};
