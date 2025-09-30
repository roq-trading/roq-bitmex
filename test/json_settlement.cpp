/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch_all.hpp>

#include "roq/core/datetime.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitmex/json/settlement.hpp"

using namespace roq;
using namespace roq::bitmex;

using namespace std::literals;

using namespace Catch::literals;

// note! truncated
TEST_CASE("simple", "[json_settlement]") {
  auto const message = R"({)"
                       R"("table":"settlement",)"
                       R"("action":"partial",)"
                       R"("keys":[)"
                       R"("timestamp",)"
                       R"("symbol"],)"
                       R"("types":{)"
                       R"("timestamp":"timestamp",)"
                       R"("symbol":"symbol",)"
                       R"("settlementType":"symbol",)"
                       R"("settledPrice":"float",)"
                       R"("optionStrikePrice":"float",)"
                       R"("optionUnderlyingPrice":"float",)"
                       R"("bankrupt":"long",)"
                       R"("taxBase":"long",)"
                       R"("taxRate":"float")"
                       R"(},)"
                       R"("filter":{},)"
                       R"("data":[{)"
                       R"("timestamp":"2024-05-06T12:00:00.000Z",)"
                       R"("symbol":"SNTUSDT",)"
                       R"("settlementType":"Settlement",)"
                       R"("settledPrice":0.03969)"
                       R"(},{)"
                       R"("timestamp":"2021-11-02T12:00:00.000Z",)"
                       R"("symbol":"SUSHIUSDT",)"
                       R"("settlementType":"Settlement",)"
                       R"("settledPrice":12.475)"
                       R"(})"
                       R"(])"
                       R"(})"sv;
  core::json::BufferStack buffer{8192, 1};
  json::Settlement obj{message, buffer};
}
