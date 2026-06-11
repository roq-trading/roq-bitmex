/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/bitmex/flags/settings.hpp"

#include "roq/logging.hpp"

#include "roq/bitmex/flags/flags.hpp"

using namespace std::literals;

namespace roq {
namespace bitmex {
namespace flags {

Settings::Settings(args::Parser const &args)
    : server::flags::Settings{args, ROQ_PACKAGE_NAME, ROQ_BUILD_NUMBER, ROQ_GIT_DESCRIBE_HASH, {}}, flags::Flags{flags::Flags::create()},
      misc{flags::Misc::create()}, rest{flags::REST::create()}, ws{flags::WS::create()} {
  log::info("settings={}"sv, *this);
}

}  // namespace flags
}  // namespace bitmex
}  // namespace roq
