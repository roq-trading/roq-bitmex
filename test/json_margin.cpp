/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::bitmex;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::Margin;

TEST_CASE("simple", "[json_margin]") {
  auto const message = R"({)"
                       R"("table":"margin",)"
                       R"("action":"partial",)"
                       R"("keys":[)"
                       R"("account",)"
                       R"("currency")"
                       R"(],)"
                       R"("types":{)"
                       R"("account":"long",)"
                       R"("currency":"symbol",)"
                       R"("riskLimit":"long",)"
                       R"("state":"symbol",)"
                       R"("amount":"long",)"
                       R"("prevRealisedPnl":"long",)"
                       R"("grossComm":"long",)"
                       R"("grossOpenCost":"long",)"
                       R"("grossOpenPremium":"long",)"
                       R"("grossExecCost":"long",)"
                       R"("grossMarkValue":"long",)"
                       R"("riskValue":"long",)"
                       R"("initMargin":"long",)"
                       R"("maintMargin":"long",)"
                       R"("targetExcessMargin":"long",)"
                       R"("realisedPnl":"long",)"
                       R"("unrealisedPnl":"long",)"
                       R"("walletBalance":"long",)"
                       R"("marginBalance":"long",)"
                       R"("marginLeverage":"float",)"
                       R"("marginUsedPcnt":"float",)"
                       R"("excessMargin":"long",)"
                       R"("availableMargin":"long",)"
                       R"("withdrawableMargin":"long",)"
                       R"("systemWithdrawableMargin":"long",)"
                       R"("makerFeeDiscount":"float",)"
                       R"("takerFeeDiscount":"float",)"
                       R"("timestamp":"timestamp",)"
                       R"("foreignMarginBalance":"long",)"
                       R"("foreignRequirement":"long")"
                       R"(},)"
                       R"("filter":{)"
                       R"("account":273093)"
                       R"(},)"
                       R"("data":[{)"
                       R"("account":273093,)"
                       R"("currency":"XBt",)"
                       R"("riskLimit":1000000000000,)"
                       R"("amount":2,)"
                       R"("grossComm":0,)"
                       R"("grossOpenCost":0,)"
                       R"("grossOpenPremium":0,)"
                       R"("grossMarkValue":0,)"
                       R"("riskValue":0,)"
                       R"("initMargin":0,)"
                       R"("maintMargin":0,)"
                       R"("targetExcessMargin":2,)"
                       R"("realisedPnl":0,)"
                       R"("unrealisedPnl":0,)"
                       R"("walletBalance":2,)"
                       R"("marginBalance":2,)"
                       R"("marginLeverage":0.0,)"
                       R"("marginUsedPcnt":0.0,)"
                       R"("excessMargin":2,)"
                       R"("availableMargin":2,)"
                       R"("withdrawableMargin":2,)"
                       R"("timestamp":"2024-09-27T05:43:39.607Z",)"
                       R"("foreignMarginBalance":0,)"
                       R"("foreignRequirement":0)"
                       R"(})"
                       R"(])"
                       R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.table == json::Table::MARGIN);
    CHECK(obj.action == json::Action::PARTIAL);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
