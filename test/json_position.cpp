/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch.hpp>

#include "roq/core/datetime.hpp"

#include "roq/bitmex/json/position.hpp"

using namespace roq;
using namespace roq::bitmex;

using namespace std::literals;

TEST_CASE("json_position_item_item_unlisted", "[json_position_item_item]") {
  const auto message =
      R"({)"
      R"("account":359347,"symbol":"XBTUSD","openOrderBuyPremium":0,"openOrderSellPremium":0,)"
      R"("currentTimestamp":"2021-06-24T17:16:55.376Z","grossOpenPremium":0,"markPrice":34734.8,)"
      R"("markValue":-5182128,"riskValue":5182128,"homeNotional":0.05182128,"foreignNotional":-1800,)"
      R"("posState":"Liquidation","posCross":18335,"posComm":3910,"posMargin":73673,"posMaint":36567,)"
      R"("initMargin":0,"maintMargin":34324,"unrealisedGrossPnl":-39349,"unrealisedPnl":-39349,)"
      R"("unrealisedPnlPcnt":-0.0077,"unrealisedRoePcnt":-0.7651,"marginCallPrice":34750,)"
      R"("liquidationPrice":34750,"bankruptPrice":34532.5,"timestamp":"2021-06-24T17:16:55.376Z",)"
      R"("lastPrice":34734.8,"lastValue":-5182128,"currency":"XBt","currentQty":1800})";
  auto obj = core::json::Parser::create<json::PositionItem>(message);
  CHECK(obj.account == 359347);
  CHECK(obj.symbol == "XBTUSD"sv);
}
