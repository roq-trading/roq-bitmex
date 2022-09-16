/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/bitmex/application.hpp"

#include "roq/io/engine/context_factory.hpp"

#include "roq/bitmex/config.hpp"
#include "roq/bitmex/flags.hpp"
#include "roq/bitmex/gateway.hpp"

using namespace std::literals;

namespace roq {
namespace bitmex {

int Application::main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  log::info(R"(Parse config_file="{}")"sv, Flags::config_file());
  Config config(Flags::config_file(), Flags::secrets_file());
  log::info<1>("config={}"sv, config);
  log::info("Prepare environment"sv);
  auto context = io::engine::ContextFactory::create(server::Flags::io_backend());
  log::info("Starting the gateway"sv);
  server::Settings settings{
      .package_name = ROQ_PACKAGE_NAME,
      .build_number = ROQ_BUILD_NUMBER,
      .api = {},
      .type = server::Type::ORDER_MANAGEMENT,
  };
  server::Trading<Gateway>(settings, config, *context).dispatch();
  return EXIT_SUCCESS;
}

}  // namespace bitmex
}  // namespace roq
