/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/bitmex/application.h"

#include "roq/bitmex/config.h"
#include "roq/bitmex/flags.h"
#include "roq/bitmex/gateway.h"

using namespace roq::literals;

namespace roq {
namespace bitmex {

int Application::main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  LOG(INFO)(R"(Parse config_file="{}")"_fmt, Flags::config_file());
  Config config(Flags::config_file());
  VLOG(1)("config={}"_fmt, config);
  LOG(INFO)("Starting the gateway"_sv);
  roq::server::Trading<Gateway>(ROQ_PACKAGE_NAME, config, server::RequestIdType::SEQUENTIAL, config)
      .dispatch();
  return EXIT_SUCCESS;
}

}  // namespace bitmex
}  // namespace roq
