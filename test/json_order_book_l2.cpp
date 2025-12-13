/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::bitmex;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::OrderBookL2;

// note! truncated
TEST_CASE("simple", "[json_order_book_l2]") {
  auto const message = R"({)"
                       R"("table":"orderBookL2",)"
                       R"("action":"partial",)"
                       R"("keys":[)"
                       R"("symbol",)"
                       R"("id",)"
                       R"("side")"
                       R"(],)"
                       R"("types":{)"
                       R"("symbol":"symbol",)"
                       R"("id":"long",)"
                       R"("side":"symbol",)"
                       R"("size":"long",)"
                       R"("price":"float",)"
                       R"("timestamp":"timestamp",)"
                       R"("transactTime":"timestamp")"
                       R"(},)"
                       R"("filter":{},)"
                       R"("data":[{)"
                       R"("symbol":"INJUSDT",)"
                       R"("id":79097967026,)"
                       R"("side":"Sell",)"
                       R"("size":2,)"
                       R"("price":137.9536,)"
                       R"("timestamp":"2025-09-30T14:09:00.537Z",)"
                       R"("transactTime":"2025-09-30T06:34:17.326Z")"
                       R"(},{)"
                       R"("symbol":"INJUSDT",)"
                       R"("id":79098105366,)"
                       R"("side":"Sell",)"
                       R"("size":4,)"
                       R"("price":85.6190,)"
                       R"("timestamp":"2025-09-30T14:09:00.537Z",)"
                       R"("transactTime":"2025-09-30T06:34:17.326Z")"
                       R"(})"
                       R"(])"
                       R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.table == json::Table::ORDER_BOOK_L2);
    CHECK(obj.action == json::Action::PARTIAL);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
