/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::bitmex;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::Welcome;

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
  auto helper = [](value_type const &obj) {
    CHECK(obj.info == "Welcome to the BitMEX Realtime API."sv);
    CHECK(obj.version == "2.0.0"sv);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
