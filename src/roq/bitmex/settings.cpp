/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/bitmex/settings.hpp"

#include "roq/logging.hpp"

#include "roq/bitmex/flags/flags.hpp"

using namespace std::literals;

namespace roq {
namespace bitmex {

Settings::Settings(args::Parser const &args)
    : server::flags::Settings{args, ROQ_PACKAGE_NAME, ROQ_BUILD_NUMBER}, flags::Flags{flags::Flags::create()},
      common{flags::Common::create()}, rest{flags::REST::create()}, ws{flags::WS::create()} {
  log::debug("settings={}"sv, *this);
}

}  // namespace bitmex
}  // namespace roq
