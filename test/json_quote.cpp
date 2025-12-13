/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::bitmex;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::Quote;

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
  auto helper = [](value_type const &obj) {
    CHECK(obj.table == json::Table::QUOTE);
    CHECK(obj.action == json::Action::PARTIAL);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
