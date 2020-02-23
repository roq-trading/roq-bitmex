/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/application.h"

#include "roq/bitmex/config.h"
#include "roq/bitmex/gateway.h"
#include "roq/bitmex/options.h"

namespace roq {
namespace bitmex {

int Application::main(int, char **) {
  LOG(INFO)("Parse configuration");
  Config config(FLAGS_config_file);
  VLOG(1)(FMT_STRING("config={}"), config);
  LOG(INFO)("Starting the gateway");
  roq::server::Trading<Gateway>(
      config,
      FLAGS_listen,
      server::RequestIdType::SEQUENTIAL,
      config).dispatch();
  return EXIT_SUCCESS;
}

}  // namespace bitmex
}  // namespace roq
