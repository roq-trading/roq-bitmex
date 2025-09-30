/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch_all.hpp>

#include "roq/core/datetime.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitmex/json/welcome.hpp"

using namespace roq;
using namespace roq::bitmex;

using namespace std::literals;

using namespace Catch::literals;

// note! truncated
TEST_CASE("simple", "[json_welcome]") {
  auto const message = R"({)"
                       R"("info":"Welcome to the BitMEX Realtime API.",)"
                       R"("version":"2.0.0",)"
                       R"("timestamp":"2025-09-30T12:48:45.419Z",)"
                       R"("docs":"https://www.bitmex.com/app/wsAPI",)"
                       R"("heartbeatEnabled":false,)"
                       R"("limit":{)"
                       R"("remaining":719)"
                       R"(},)"
                       R"("appName":"ws-feedhandler-76f8766675-2w4vj")"
                       R"(})"sv;
  core::json::BufferStack buffer{8192, 1};
  json::Welcome obj{message, buffer};
}
