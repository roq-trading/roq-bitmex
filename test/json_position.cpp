/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::bitmex;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::Position;

TEST_CASE("simple", "[json_position]") {
  auto const message = R"({)"
                       R"("table":"position",)"
                       R"("action":"partial",)"
                       R"("keys":[)"
                       R"("account",)"
                       R"("symbol",)"
                       R"("strategy")"
                       R"(],)"
                       R"("types":{)"
                       R"("account":"long",)"
                       R"("symbol":"symbol",)"
                       R"("strategy":"symbol",)"
                       R"("currency":"symbol",)"
                       R"("underlying":"symbol",)"
                       R"("quoteCurrency":"symbol",)"
                       R"("commission":"float",)"
                       R"("initMarginReq":"float",)"
                       R"("maintMarginReq":"float",)"
                       R"("riskLimit":"long",)"
                       R"("leverage":"float",)"
                       R"("crossMargin":"boolean",)"
                       R"("deleveragePercentile":"float",)"
                       R"("rebalancedPnl":"long",)"
                       R"("prevRealisedPnl":"long",)"
                       R"("prevUnrealisedPnl":"long",)"
                       R"("openingQty":"long",)"
                       R"("openOrderBuyQty":"long",)"
                       R"("openOrderBuyCost":"long",)"
                       R"("openOrderBuyPremium":"long",)"
                       R"("openOrderSellQty":"long",)"
                       R"("openOrderSellCost":"long",)"
                       R"("openOrderSellPremium":"long",)"
                       R"("currentQty":"long",)"
                       R"("currentCost":"long",)"
                       R"("currentComm":"long",)"
                       R"("realisedCost":"long",)"
                       R"("unrealisedCost":"long",)"
                       R"("grossOpenPremium":"long",)"
                       R"("isOpen":"boolean",)"
                       R"("markPrice":"float",)"
                       R"("markValue":"long",)"
                       R"("riskValue":"long",)"
                       R"("homeNotional":"float",)"
                       R"("foreignNotional":"float",)"
                       R"("posState":"symbol",)"
                       R"("posCost":"long",)"
                       R"("posCross":"long",)"
                       R"("posComm":"long",)"
                       R"("posLoss":"long",)"
                       R"("posMargin":"long",)"
                       R"("posMaint":"long",)"
                       R"("posInit":"long",)"
                       R"("initMargin":"long",)"
                       R"("maintMargin":"long",)"
                       R"("realisedPnl":"long",)"
                       R"("unrealisedPnl":"long",)"
                       R"("unrealisedPnlPcnt":"float",)"
                       R"("unrealisedRoePcnt":"float",)"
                       R"("avgCostPrice":"float",)"
                       R"("avgEntryPrice":"float",)"
                       R"("breakEvenPrice":"float",)"
                       R"("marginCallPrice":"float",)"
                       R"("liquidationPrice":"float",)"
                       R"("bankruptPrice":"float",)"
                       R"("timestamp":"timestamp")"
                       R"(},)"
                       R"("filter":{)"
                       R"("account":273093)"
                       R"(},)"
                       R"("data":[])"
                       R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.table == json::Table::POSITION);
    CHECK(obj.action == json::Action::PARTIAL);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
