/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/application.h"

namespace {
constexpr std::string_view DESCRIPTION = "Roq Coinbase-Pro Gateway";
}  // namespace

int main(int argc, char **argv) {
  return roq::bitmex::Application(
      argc,
      argv,
      DESCRIPTION,
      VERSION).run();
}
