/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch_all.hpp>

#include "roq/core/datetime.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitmex/json/quote.hpp"

using namespace roq;
using namespace roq::bitmex;

using namespace std::literals;

using namespace Catch::literals;

TEST_CASE("simple", "[json_quote]") {
  auto const message = R"({)"
                       R"("table":"quote",)"
                       R"("action":"partial",)"
                       R"("keys":[],)"
                       R"("types":{)"
                       R"("timestamp":"timestamp",)"
                       R"("symbol":"symbol",)"
                       R"("bidSize":"long",)"
                       R"("bidPrice":"float",)"
                       R"("askPrice":"float",)"
                       R"("askSize":"long")"
                       R"(},)"
                       R"("filter":{},)"
                       R"("data":[{)"
                       R"("timestamp":"2025-09-30T14:09:00.382Z",)"
                       R"("symbol":"AXSUSD",)"
                       R"("bidSize":126397,)"
                       R"("bidPrice":2.06,)"
                       R"("askPrice":2.07,)"
                       R"("askSize":125941)"
                       R"(},{)"
                       R"("timestamp":"2025-09-30T14:08:54.083Z",)"
                       R"("symbol":"INJUSDT",)"
                       R"("bidSize":5,)"
                       R"("bidPrice":11.6971,)"
                       R"("askPrice":11.7087,)"
                       R"("askSize":5)"
                       R"(})"
                       R"(])"
                       R"(})"sv;
  core::json::BufferStack buffer{8192, 1};
  json::Quote obj{message, buffer};
}
