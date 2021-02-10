/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/bitmex/application.h"

using namespace std::literals;  // NOLINT

namespace {
static const auto DESCRIPTION = "Roq BitMEX Gateway"sv;
}  // namespace

int main(int argc, char **argv) {
  return roq::bitmex::Application(
             argc, argv, DESCRIPTION, ROQ_BUILD_VERSION, ROQ_BUILD_TYPE, ROQ_GIT_DESCRIBE_HASH)
      .run();
}
