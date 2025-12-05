/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::bitmex;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::Trade;

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
  auto helper = [](value_type const &obj) {
    CHECK(obj.table == json::Table::TRADE);
    CHECK(obj.action == json::Action::PARTIAL);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
