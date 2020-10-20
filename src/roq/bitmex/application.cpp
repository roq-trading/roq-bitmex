/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/application.h"

#include "roq/bitmex/config.h"
#include "roq/bitmex/gateway.h"
#include "roq/bitmex/options.h"

namespace roq {
namespace bitmex {

int Application::main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  LOG(INFO)("Parse configuration");
  Config config(FLAGS_config_file);
  VLOG(1)("config={}", config);
  LOG(INFO)("Starting the gateway");
  roq::server::Trading<Gateway>(
      ROQ_PACKAGE_NAME, config, server::RequestIdType::SEQUENTIAL, config)
      .dispatch();
  return EXIT_SUCCESS;
}

}  // namespace bitmex
}  // namespace roq
