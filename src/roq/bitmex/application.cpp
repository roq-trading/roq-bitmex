/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/bitmex/application.hpp"

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
  log::info("Starting the gateway"sv);
  roq::server::Trading<Gateway>(ROQ_PACKAGE_NAME, ROQ_BUILD_NUMBER, {}, config).dispatch();
  return EXIT_SUCCESS;
}

}  // namespace bitmex
}  // namespace roq
