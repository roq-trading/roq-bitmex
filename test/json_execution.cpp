/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::bitmex;

using namespace std::literals;

using namespace Catch::literals;

using value_type = protocol::json::Execution;

TEST_CASE("simple", "[json_execution]") {
  auto const message = R"({)"
                       R"("table":"execution",)"
                       R"("action":"partial",)"
                       R"("keys":[],)"
                       R"("types":{)"
                       R"("execID":"guid",)"
                       R"("orderID":"guid",)"
                       R"("clOrdID":"",)"
                       R"("clOrdLinkID":"",)"
                       R"("account":"long",)"
                       R"("symbol":"symbol",)"
                       R"("strategy":"symbol",)"
                       R"("side":"symbol",)"
                       R"("lastQty":"long",)"
                       R"("lastPx":"float",)"
                       R"("lastLiquidityInd":"symbol",)"
                       R"("orderQty":"long",)"
                       R"("price":"float",)"
                       R"("displayQty":"long",)"
                       R"("stopPx":"float",)"
                       R"("pegOffsetValue":"float",)"
                       R"("pegPriceType":"symbol",)"
                       R"("currency":"symbol",)"
                       R"("settlCurrency":"symbol",)"
                       R"("execType":"symbol",)"
                       R"("ordType":"symbol",)"
                       R"("timeInForce":"symbol",)"
                       R"("execInst":"symbol",)"
                       R"("contingencyType":"symbol",)"
                       R"("ordStatus":"symbol",)"
                       R"("triggered":"symbol",)"
                       R"("workingIndicator":"boolean",)"
                       R"("ordRejReason":"",)"
                       R"("leavesQty":"long",)"
                       R"("cumQty":"long",)"
                       R"("avgPx":"float",)"
                       R"("commission":"float",)"
                       R"("brokerCommission":"float",)"
                       R"("feeType":"symbol",)"
                       R"("tradePublishIndicator":"symbol",)"
                       R"("text":"",)"
                       R"("trdMatchID":"guid",)"
                       R"("execCost":"long",)"
                       R"("execComm":"long",)"
                       R"("brokerExecComm":"long",)"
                       R"("homeNotional":"float",)"
                       R"("foreignNotional":"float",)"
                       R"("transactTime":"timestamp",)"
                       R"("timestamp":"timestamp",)"
                       R"("realisedPnl":"long",)"
                       R"("trdType":"symbol")"
                       R"(},)"
                       R"("filter":{)"
                       R"("account":273093)"
                       R"(},)"
                       R"("data":[])"
                       R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.table == protocol::json::Table::EXECUTION);
    CHECK(obj.action == protocol::json::Action::PARTIAL);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
