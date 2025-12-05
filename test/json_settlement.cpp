/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::bitmex;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::Settlement;

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
  auto helper = [](value_type const &obj) {
    CHECK(obj.table == json::Table::SETTLEMENT);
    CHECK(obj.action == json::Action::PARTIAL);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
