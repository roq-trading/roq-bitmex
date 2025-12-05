/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::bitmex;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::Funding;

// note! truncated
TEST_CASE("simple", "[json_funding]") {
  auto const message = R"({)"
                       R"("table":"funding",)"
                       R"("action":"partial",)"
                       R"("keys":[)"
                       R"("timestamp",)"
                       R"("symbol")"
                       R"(],)"
                       R"("types":{)"
                       R"("timestamp":"timestamp",)"
                       R"("symbol":"symbol",)"
                       R"("fundingInterval":"timespan",)"
                       R"("fundingRate":"float",)"
                       R"("fundingRateDaily":"float"},)"
                       R"("filter":{},)"
                       R"("data":[{)"
                       R"("timestamp":"2024-05-06T12:00:00.000Z",)"
                       R"("symbol":"SNTUSDT",)"
                       R"("fundingInterval":"2000-01-01T08:00:00.000Z",)"
                       R"("fundingRate":1.0E-4,)"
                       R"("fundingRateDaily":3.0000000000000003E-4)"
                       R"(})"
                       R"(])"
                       R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.table == json::Table::FUNDING);
    CHECK(obj.action == json::Action::PARTIAL);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
