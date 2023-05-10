/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/bitmex/application.hpp"

#include "roq/bitmex/config.hpp"
#include "roq/bitmex/gateway.hpp"
#include "roq/bitmex/settings.hpp"

using namespace std::literals;

namespace roq {
namespace bitmex {

// === CONSTANTS ===

namespace {
auto const TYPE = server::Type::ORDER_MANAGEMENT;
}

// === IMPLEMENTATION ===

int Application::main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  Settings settings{TYPE};
  Config config{settings};
  auto context = server::create_io_context(settings);
  server::Trading<Gateway>{settings, config, *context}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace bitmex
}  // namespace roq
