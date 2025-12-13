/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/bitmex/application.hpp"

#include "roq/bitmex/config.hpp"
#include "roq/bitmex/gateway.hpp"
#include "roq/bitmex/settings.hpp"

using namespace std::literals;

namespace roq {
namespace bitmex {

// === CONSTANTS ===

namespace {
uint8_t const API_2 = {};
}

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  Settings settings{args};
  Config config{settings};
  auto context = server::create_io_context(settings);
  server::Trading<Gateway>{settings, config, *context, API_2}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace bitmex
}  // namespace roq
