/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch_all.hpp>

#include "roq/core/datetime.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitmex/json/trade.hpp"

using namespace roq;
using namespace roq::bitmex;

using namespace std::literals;

using namespace Catch::literals;

TEST_CASE("simple", "[json_trade]") {
  auto const message = R"({)"
                       R"("table":"trade",)"
                       R"("action":"partial",)"
                       R"("keys":[],)"
                       R"("types":{)"
                       R"("timestamp":"timestamp",)"
                       R"("symbol":"symbol",)"
                       R"("side":"symbol",)"
                       R"("size":"long",)"
                       R"("price":"float",)"
                       R"("tickDirection":"symbol",)"
                       R"("trdMatchID":"guid",)"
                       R"("grossValue":"long",)"
                       R"("homeNotional":"float",)"
                       R"("foreignNotional":"float",)"
                       R"("trdType":"symbol")"
                       R"(},)"
                       R"("filter":{},)"
                       R"("data":[{)"
                       R"("timestamp":"2025-09-30T14:08:00.000Z",)"
                       R"("symbol":".BPENGUT_NEXT",)"
                       R"("side":"Buy",)"
                       R"("size":0,)"
                       R"("price":0.027023,)"
                       R"("tickDirection":"MinusTick",)"
                       R"("trdType":"Referential")"
                       R"(},{)"
                       R"("timestamp":"2025-09-30T14:08:00.000Z",)"
                       R"("symbol":".BBIGTIMET_NEXT",)"
                       R"("side":"Buy",)"
                       R"("size":0,)"
                       R"("price":0.0461,)"
                       R"("tickDirection":"MinusTick",)"
                       R"("trdType":"Referential")"
                       R"(})"
                       R"(])"
                       R"(})"sv;
  core::json::BufferStack buffer{8192, 1};
  json::Trade obj{message, buffer};
}
