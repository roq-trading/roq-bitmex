/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/api.hpp"

#include "roq/flags/args.hpp"

#include "roq/logging/flags/settings.hpp"

#include "roq/bitmex/bridge/application.hpp"

using namespace std::literals;

// === CONSTANTS ===

namespace {
auto const INFO = roq::Service::Info{
    .description = "Roq's BitMEX gateway (Protobuf interface)"sv,
    .package_name = ROQ_PACKAGE_NAME,
    .build_version = ROQ_BUILD_VERSION,
    .build_number = ROQ_BUILD_NUMBER,
    .build_type = ROQ_BUILD_TYPE,
    .git_hash = ROQ_GIT_DESCRIBE_HASH,
};
}  // namespace

// === IMPLEMENTATION ===

int main(int argc, char **argv) {
  roq::flags::Args args{argc, argv, INFO.description, INFO.build_version};
  roq::logging::flags::Settings settings{args};
  return roq::bitmex::bridge::Application{args, settings, INFO}.run();
}
