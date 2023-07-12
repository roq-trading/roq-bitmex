/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/bitmex/settings.hpp"

#include "roq/logging.hpp"

#include "roq/bitmex/flags/flags.hpp"

using namespace std::literals;

namespace roq {
namespace bitmex {

Settings::Settings(args::Parser const &args, server::Type type)
    : server::flags::Settings{args, type, ROQ_PACKAGE_NAME, ROQ_BUILD_NUMBER}, exchange{flags::Flags::exchange()} {
  log::debug("settings={}"sv, *this);
}

}  // namespace bitmex
}  // namespace roq
