/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/bitmex/application.hpp"

#include "roq/bitmex/config.hpp"
#include "roq/bitmex/gateway.hpp"

using namespace std::literals;

namespace roq {
namespace bitmex {

// === CONSTANTS ===

namespace {
auto const SETTINGS = server::Settings{
    .package_name = ROQ_PACKAGE_NAME,
    .build_number = ROQ_BUILD_NUMBER,
    .api = {},
    .type = server::Type::ORDER_MANAGEMENT,
};
}

// === IMPLEMENTATION ===

int Application::main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  Config config;
  auto context = server::create_io_context();
  server::Trading<Gateway>{SETTINGS, config, *context}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace bitmex
}  // namespace roq
