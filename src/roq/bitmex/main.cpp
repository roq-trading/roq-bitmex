/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/bitmex/application.hpp"

using namespace std::literals;

// === CONSTANTS ===

namespace {
roq::Service::Info const INFO{
    .description = "Roq BitMEX Gateway"sv,
    .package_name = ROQ_PACKAGE_NAME,
    .host = ROQ_HOST,
    .build_version = ROQ_BUILD_VERSION,
    .build_number = ROQ_BUILD_NUMBER,
    .build_type = ROQ_BUILD_TYPE,
    .git_hash = ROQ_GIT_DESCRIBE_HASH,
};
}  // namespace

// === IMPLEMENTATION ===

int main(int argc, char **argv) {
  return roq::bitmex::Application(argc, argv, INFO).run();
}
