/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::bitmex;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::Liquidation;

// note! truncated
TEST_CASE("empty", "[json_liquidation]") {
  auto const message = R"({)"
                       R"("table":"liquidation",)"
                       R"("action":"partial",)"
                       R"("keys":["orderID"],)"
                       R"("types":{)"
                       R"("orderID":"guid",)"
                       R"("symbol":"symbol",)"
                       R"("side":"symbol",)"
                       R"("price":"float",)"
                       R"("leavesQty":"long")"
                       R"(},)"
                       R"("filter":{},)"
                       R"("data":[])"
                       R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.table == json::Table::LIQUIDATION);
    CHECK(obj.action == json::Action::PARTIAL);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
