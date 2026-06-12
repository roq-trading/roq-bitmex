/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/bitmex/application.hpp"

#include "roq/bitmex/flags/settings.hpp"

#include "roq/bitmex/gateway/config.hpp"
#include "roq/bitmex/gateway/controller.hpp"

using namespace std::literals;

namespace roq {
namespace bitmex {

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  flags::Settings settings{args};
  gateway::Config config{settings};
  auto context = server::create_io_context(settings);
  server::Trading<gateway::Controller>{settings, config, *context}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace bitmex
}  // namespace roq
