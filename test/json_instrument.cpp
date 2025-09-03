/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch_all.hpp>

#include "roq/core/datetime.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitmex/json/instrument.hpp"

using namespace roq;
using namespace roq::bitmex;

using namespace std::literals;

using namespace Catch::literals;

TEST_CASE("json_instrument_item_unlisted", "[json_instrument_item]") {
  auto const message = R"({)"
                       R"("symbol":".EVOL7D",)"
                       R"("rootSymbol":"EVOL",)"
                       R"("state":"Unlisted",)"
                       R"("typ":"MRIXXX",)"
                       R"("listing":null,)"
                       R"("front":null,)"
                       R"("expiry":null,)"
                       R"("settle":null,)"
                       R"("relistInterval":null,)"
                       R"("inverseLeg":"",)"
                       R"("sellLeg":"",)"
                       R"("buyLeg":"",)"
                       R"("optionStrikePcnt":null,)"
                       R"("optionStrikeRound":null,)"
                       R"("optionStrikePrice":null,)"
                       R"("optionMultiplier":null,)"
                       R"("positionCurrency":"",)"
                       R"("underlying":"ETH",)"
                       R"("quoteCurrency":"XXX",)"
                       R"("underlyingSymbol":".EVOL7D",)"
                       R"("reference":"BMEX",)"
                       R"("referenceSymbol":".BETHXBT",)"
                       R"("calcInterval":"2000-01-08T00:00:00.000Z",)"
                       R"("publishInterval":"2000-01-01T00:05:00.000Z",)"
                       R"("publishTime":null,)"
                       R"("maxOrderQty":null,)"
                       R"("maxPrice":null,)"
                       R"("lotSize":null,)"
                       R"("tickSize":0.01,)"
                       R"("multiplier":null,)"
                       R"("settlCurrency":"",)"
                       R"("underlyingToPositionMultiplier":null,)"
                       R"("underlyingToSettleMultiplier":null,)"
                       R"("quoteToSettleMultiplier":null,)"
                       R"("isQuanto":false,)"
                       R"("isInverse":false,)"
                       R"("initMargin":null,)"
                       R"("maintMargin":null,)"
                       R"("riskLimit":null,)"
                       R"("riskStep":null,)"
                       R"("limit":null,)"
                       R"("capped":false,)"
                       R"("taxed":false,)"
                       R"("deleverage":false,)"
                       R"("makerFee":null,)"
                       R"("takerFee":null,)"
                       R"("settlementFee":null,)"
                       R"("insuranceFee":null,)"
                       R"("fundingBaseSymbol":"",)"
                       R"("fundingQuoteSymbol":"",)"
                       R"("fundingPremiumSymbol":"",)"
                       R"("fundingTimestamp":null,)"
                       R"("fundingInterval":null,)"
                       R"("fundingRate":null,)"
                       R"("indicativeFundingRate":null,)"
                       R"("rebalanceTimestamp":null,)"
                       R"("rebalanceInterval":null,)"
                       R"("openingTimestamp":null,)"
                       R"("closingTimestamp":null,)"
                       R"("sessionInterval":null,)"
                       R"("prevClosePrice":null,)"
                       R"("limitDownPrice":null,)"
                       R"("limitUpPrice":null,)"
                       R"("bankruptLimitDownPrice":null,)"
                       R"("bankruptLimitUpPrice":null,)"
                       R"("prevTotalVolume":null,)"
                       R"("totalVolume":null,)"
                       R"("volume":null,)"
                       R"("volume24h":null,)"
                       R"("prevTotalTurnover":null,)"
                       R"("totalTurnover":null,)"
                       R"("turnover":null,)"
                       R"("turnover24h":null,)"
                       R"("homeNotional24h":null,)"
                       R"("foreignNotional24h":null,)"
                       R"("prevPrice24h":7.34,)"
                       R"("vwap":null,)"
                       R"("highPrice":null,)"
                       R"("lowPrice":null,)"
                       R"("lastPrice":6.93,)"
                       R"("lastPriceProtected":null,)"
                       R"("lastTickDirection":"ZeroMinusTick",)"
                       R"("lastChangePcnt":-0.0559,)"
                       R"("bidPrice":null,)"
                       R"("midPrice":null,)"
                       R"("askPrice":null,)"
                       R"("impactBidPrice":null,)"
                       R"("impactMidPrice":null,)"
                       R"("impactAskPrice":null,)"
                       R"("hasLiquidity":false,)"
                       R"("openInterest":null,)"
                       R"("openValue":0,)"
                       R"("fairMethod":"",)"
                       R"("fairBasisRate":null,)"
                       R"("fairBasis":null,)"
                       R"("fairPrice":null,)"
                       R"("markMethod":"LastPrice",)"
                       R"("markPrice":6.93,)"
                       R"("indicativeTaxRate":null,)"
                       R"("indicativeSettlePrice":null,)"
                       R"("optionUnderlyingPrice":null,)"
                       R"("settledPrice":null,)"
                       R"("timestamp":"2020-01-23T04:50:00.000Z")"
                       R"(})"sv;
  json::InstrumentItem obj{message};
  CHECK(obj.symbol == ".EVOL7D");
  CHECK(obj.root_symbol == "EVOL");
  CHECK(obj.state == json::State::UNLISTED);
  CHECK(obj.typ == json::Typ::MRIXXX);
  CHECK(obj.underlying == "ETH");
  CHECK(obj.quote_currency == "XXX");
  CHECK(obj.reference == "BMEX");
  CHECK(obj.reference_symbol == ".BETHXBT");
  CHECK(obj.calc_interval == core::datetime(2000, 1u, 8u, 0u, 0u, 0u));
  CHECK(obj.publish_interval == core::datetime(2000, 1u, 1u, 0u, 5u, 0u));
  CHECK(obj.tick_size == 0.01_a);
  CHECK(obj.is_quanto == false);
  CHECK(obj.is_inverse == false);
  CHECK(obj.capped == false);
  CHECK(obj.taxed == false);
  CHECK(obj.deleverage == false);
  CHECK(obj.prev_price24h == 7.34_a);
  CHECK(obj.last_price == 6.93_a);
  CHECK(obj.last_tick_direction == "ZeroMinusTick");
  CHECK(obj.last_change_pcnt == -0.0559_a);
  CHECK(obj.has_liquidity == false);
  CHECK(obj.open_value == 0.0_a);
  CHECK(obj.mark_price == 6.93_a);
  CHECK(obj.timestamp == core::datetime(2020, 1u, 23u, 4u, 50u, 0u));
}

TEST_CASE("json_instrument_item_open", "[json_instrument_item]") {
  auto const message = R"({)"
                       R"("symbol":"XRPH20",)"
                       R"("rootSymbol":"XRP",)"
                       R"("state":"Open",)"
                       R"("typ":"FFCCSX",)"
                       R"("listing":"2019-12-06T04:00:00.000Z",)"
                       R"("front":"2020-02-28T12:00:00.000Z",)"
                       R"("expiry":"2020-03-27T12:00:00.000Z",)"
                       R"("settle":"2020-03-27T12:00:00.000Z",)"
                       R"("relistInterval":null,)"
                       R"("inverseLeg":"",)"
                       R"("sellLeg":"",)"
                       R"("buyLeg":"",)"
                       R"("optionStrikePcnt":null,)"
                       R"("optionStrikeRound":null,)"
                       R"("optionStrikePrice":null,)"
                       R"("optionMultiplier":null,)"
                       R"("positionCurrency":"XRP",)"
                       R"("underlying":"XRP",)"
                       R"("quoteCurrency":"XBT",)"
                       R"("underlyingSymbol":"XRPXBT=",)"
                       R"("reference":"BMEX",)"
                       R"("referenceSymbol":".BXRPXBT30M",)"
                       R"("calcInterval":null,)"
                       R"("publishInterval":null,)"
                       R"("publishTime":null,)"
                       R"("maxOrderQty":100000000,)"
                       R"("maxPrice":1,)"
                       R"("lotSize":1,)"
                       R"("tickSize":1e-8,)"
                       R"("multiplier":100000000,)"
                       R"("settlCurrency":"XBt",)"
                       R"("underlyingToPositionMultiplier":1,)"
                       R"("underlyingToSettleMultiplier":null,)"
                       R"("quoteToSettleMultiplier":100000000,)"
                       R"("isQuanto":false,)"
                       R"("isInverse":false,)"
                       R"("initMargin":0.05,)"
                       R"("maintMargin":0.025,)"
                       R"("riskLimit":5000000000,)"
                       R"("riskStep":5000000000,)"
                       R"("limit":null,)"
                       R"("capped":false,)"
                       R"("taxed":true,)"
                       R"("deleverage":true,)"
                       R"("makerFee":-0.0005,)"
                       R"("takerFee":0.0025,)"
                       R"("settlementFee":0,)"
                       R"("insuranceFee":0,)"
                       R"("fundingBaseSymbol":"",)"
                       R"("fundingQuoteSymbol":"",)"
                       R"("fundingPremiumSymbol":"",)"
                       R"("fundingTimestamp":null,)"
                       R"("fundingInterval":null,)"
                       R"("fundingRate":null,)"
                       R"("indicativeFundingRate":null,)"
                       R"("rebalanceTimestamp":null,)"
                       R"("rebalanceInterval":null,)"
                       R"("openingTimestamp":"2020-01-22T19:00:00.000Z",)"
                       R"("closingTimestamp":"2020-01-22T20:00:00.000Z",)"
                       R"("sessionInterval":"2000-01-01T01:00:00.000Z",)"
                       R"("prevClosePrice":0.00002701,)"
                       R"("limitDownPrice":null,)"
                       R"("limitUpPrice":null,)"
                       R"("bankruptLimitDownPrice":null,)"
                       R"("bankruptLimitUpPrice":null,)"
                       R"("prevTotalVolume":26517101,)"
                       R"("totalVolume":26517103,)"
                       R"("volume":2,)"
                       R"("volume24h":84425,)"
                       R"("prevTotalTurnover":72134564731,)"
                       R"("totalTurnover":72134570075,)"
                       R"("turnover":5344,)"
                       R"("turnover24h":228958667,)"
                       R"("homeNotional24h":84425,)"
                       R"("foreignNotional24h":2.28958667,)"
                       R"("prevPrice24h":0.00002725,)"
                       R"("vwap":0.00002712,)"
                       R"("highPrice":0.00002775,)"
                       R"("lowPrice":0.00002672,)"
                       R"("lastPrice":0.00002672,)"
                       R"("lastPriceProtected":0.00002672,)"
                       R"("lastTickDirection":"ZeroMinusTick",)"
                       R"("lastChangePcnt":-0.0194,)"
                       R"("bidPrice":0.00002672,)"
                       R"("midPrice":0.00002685,)"
                       R"("askPrice":0.00002698,)"
                       R"("impactBidPrice":0.00002661,)"
                       R"("impactMidPrice":0.00002681,)"
                       R"("impactAskPrice":0.00002701,)"
                       R"("hasLiquidity":true,)"
                       R"("openInterest":17614667,)"
                       R"("openValue":47207307560,)"
                       R"("fairMethod":"ImpactMidPrice",)"
                       R"("fairBasisRate":-0.09,)"
                       R"("fairBasis":-4.3e-7,)"
                       R"("fairPrice":0.0000268,)"
                       R"("markMethod":"FairPrice",)"
                       R"("markPrice":0.0000268,)"
                       R"("indicativeTaxRate":null,)"
                       R"("indicativeSettlePrice":0.00002723,)"
                       R"("optionUnderlyingPrice":null,)"
                       R"("settledPrice":null,)"
                       R"("timestamp":"2020-01-22T19:09:30.000Z")"
                       R"(})"sv;
  json::InstrumentItem obj{message};
  CHECK(obj.symbol == "XRPH20");
  CHECK(obj.root_symbol == "XRP");
  CHECK(obj.state == json::State::OPEN);
  CHECK(obj.typ == json::Typ::FFCCSX);
  CHECK(obj.listing == core::datetime(2019, 12u, 6u, 4u, 0u, 0u));
  CHECK(obj.front == core::datetime(2020, 2u, 28u, 12u, 0u, 0u));
  CHECK(obj.expiry == core::datetime(2020, 3u, 27u, 12u, 0u, 0u));
  CHECK(obj.settle == core::datetime(2020, 3u, 27u, 12u, 0u, 0u));
  CHECK(obj.position_currency == "XRP");
  CHECK(obj.underlying == "XRP");
  CHECK(obj.quote_currency == "XBT");
  CHECK(obj.underlying_symbol == "XRPXBT=");
  CHECK(obj.reference == "BMEX");
  CHECK(obj.reference_symbol == ".BXRPXBT30M");
  CHECK(obj.max_order_qty == 100000000.0_a);
  CHECK(obj.max_price == 1.0_a);
  CHECK(obj.lot_size == 1.0_a);
  CHECK(obj.tick_size == 1.0e-8_a);
  CHECK(obj.multiplier == 100000000.0_a);
  CHECK(obj.settl_currency == "XBt");
  CHECK(obj.underlying_to_position_multiplier == 1.0_a);
  CHECK(obj.quote_to_settle_multiplier == 100000000.0_a);
  CHECK(obj.is_quanto == false);
  CHECK(obj.is_inverse == false);
  CHECK(obj.init_margin == 0.05_a);
  CHECK(obj.maint_margin == 0.025_a);
  CHECK(obj.risk_limit == 5000000000.0_a);
  CHECK(obj.risk_step == 5000000000.0_a);
  CHECK(obj.capped == false);
  CHECK(obj.taxed == true);
  CHECK(obj.deleverage == true);
  CHECK(obj.maker_fee == -0.0005_a);
  CHECK(obj.taker_fee == 0.0025_a);
  CHECK(obj.settlement_fee == 0.0_a);
  CHECK(obj.insurance_fee == 0.0_a);
  CHECK(obj.opening_timestamp == core::datetime(2020, 1u, 22u, 19u, 0u, 0u));
  CHECK(obj.closing_timestamp == core::datetime(2020, 1u, 22u, 20u, 0u, 0u));
  CHECK(obj.session_interval == core::datetime(2000, 1u, 1u, 1u, 0u, 0u));
  CHECK(obj.prev_close_price == 0.00002701_a);
  CHECK(obj.prev_total_volume == 26517101.0_a);
  CHECK(obj.total_volume == 26517103.0_a);
  CHECK(obj.volume == 2.0_a);
  CHECK(obj.volume24h == 84425.0_a);
  CHECK(obj.prev_total_turnover == 72134564731.0_a);
  CHECK(obj.total_turnover == 72134570075.0_a);
  CHECK(obj.turnover == 5344.0_a);
  CHECK(obj.turnover24h == 228958667.0_a);
  CHECK(obj.home_notional24h == 84425.0_a);
  CHECK(obj.foreign_notional24h == 2.28958667_a);
  CHECK(obj.prev_price24h == 0.00002725_a);
  CHECK(obj.vwap == 0.00002712_a);
  CHECK(obj.high_price == 0.00002775_a);
  CHECK(obj.low_price == 0.00002672_a);
  CHECK(obj.last_price == 0.00002672_a);
  CHECK(obj.last_price_protected == 0.00002672_a);
  CHECK(obj.last_tick_direction == "ZeroMinusTick");
  CHECK(obj.last_change_pcnt == -0.0194_a);
  CHECK(obj.bid_price == 0.00002672_a);
  CHECK(obj.mid_price == 0.00002685_a);
  CHECK(obj.ask_price == 0.00002698_a);
  CHECK(obj.impact_bid_price == 0.00002661_a);
  CHECK(obj.impact_mid_price == 0.00002681_a);
  CHECK(obj.impact_ask_price == 0.00002701_a);
  CHECK(obj.has_liquidity == true);
  CHECK(obj.open_interest == 17614667.0_a);
  CHECK(obj.open_value == 47207307560.0_a);
  CHECK(obj.fair_method == "ImpactMidPrice");
  CHECK(obj.fair_basis_rate == -0.09_a);
  CHECK(obj.fair_basis == -4.3e-7_a);
  CHECK(obj.fair_price == 0.0000268_a);
  CHECK(obj.mark_method == "FairPrice");
  CHECK(obj.mark_price == 0.0000268_a);
  CHECK(obj.indicative_settle_price == 0.00002723_a);
  CHECK(obj.timestamp == core::datetime(2020, 1u, 22u, 19u, 9u, 30u));
}

TEST_CASE("json_instrument_empty", "[json_instrument]") {
  auto const message = "[]"sv;
  core::json::BufferStack buffer{8192, 1};
  json::Instrument obj{message, buffer};
  CHECK(std::size(obj.data) == size_t{0});
}

TEST_CASE("json_instrument_simple", "[json_instrument]") {
  auto const message = R"([)"
                       R"({)"
                       R"("symbol":".EVOL7D",)"
                       R"("rootSymbol":"EVOL",)"
                       R"("state":"Unlisted",)"
                       R"("typ":"MRIXXX",)"
                       R"("listing":null,)"
                       R"("front":null,)"
                       R"("expiry":null,)"
                       R"("settle":null,)"
                       R"("relistInterval":null,)"
                       R"("inverseLeg":"",)"
                       R"("sellLeg":"",)"
                       R"("buyLeg":"",)"
                       R"("optionStrikePcnt":null,)"
                       R"("optionStrikeRound":null,)"
                       R"("optionStrikePrice":null,)"
                       R"("optionMultiplier":null,)"
                       R"("positionCurrency":"",)"
                       R"("underlying":"ETH",)"
                       R"("quoteCurrency":"XXX",)"
                       R"("underlyingSymbol":".EVOL7D",)"
                       R"("reference":"BMEX",)"
                       R"("referenceSymbol":".BETHXBT",)"
                       R"("calcInterval":"2000-01-08T00:00:00.000Z",)"
                       R"("publishInterval":"2000-01-01T00:05:00.000Z",)"
                       R"("publishTime":null,)"
                       R"("maxOrderQty":null,)"
                       R"("maxPrice":null,)"
                       R"("lotSize":null,)"
                       R"("tickSize":0.01,)"
                       R"("multiplier":null,)"
                       R"("settlCurrency":"",)"
                       R"("underlyingToPositionMultiplier":null,)"
                       R"("underlyingToSettleMultiplier":null,)"
                       R"("quoteToSettleMultiplier":null,)"
                       R"("isQuanto":false,)"
                       R"("isInverse":false,)"
                       R"("initMargin":null,)"
                       R"("maintMargin":null,)"
                       R"("riskLimit":null,)"
                       R"("riskStep":null,)"
                       R"("limit":null,)"
                       R"("capped":false,)"
                       R"("taxed":false,)"
                       R"("deleverage":false,)"
                       R"("makerFee":null,)"
                       R"("takerFee":null,)"
                       R"("settlementFee":null,)"
                       R"("insuranceFee":null,)"
                       R"("fundingBaseSymbol":"",)"
                       R"("fundingQuoteSymbol":"",)"
                       R"("fundingPremiumSymbol":"",)"
                       R"("fundingTimestamp":null,)"
                       R"("fundingInterval":null,)"
                       R"("fundingRate":null,)"
                       R"("indicativeFundingRate":null,)"
                       R"("rebalanceTimestamp":null,)"
                       R"("rebalanceInterval":null,)"
                       R"("openingTimestamp":null,)"
                       R"("closingTimestamp":null,)"
                       R"("sessionInterval":null,)"
                       R"("prevClosePrice":null,)"
                       R"("limitDownPrice":null,)"
                       R"("limitUpPrice":null,)"
                       R"("bankruptLimitDownPrice":null,)"
                       R"("bankruptLimitUpPrice":null,)"
                       R"("prevTotalVolume":null,)"
                       R"("totalVolume":null,)"
                       R"("volume":null,)"
                       R"("volume24h":null,)"
                       R"("prevTotalTurnover":null,)"
                       R"("totalTurnover":null,)"
                       R"("turnover":null,)"
                       R"("turnover24h":null,)"
                       R"("homeNotional24h":null,)"
                       R"("foreignNotional24h":null,)"
                       R"("prevPrice24h":7.34,)"
                       R"("vwap":null,)"
                       R"("highPrice":null,)"
                       R"("lowPrice":null,)"
                       R"("lastPrice":6.93,)"
                       R"("lastPriceProtected":null,)"
                       R"("lastTickDirection":"ZeroMinusTick",)"
                       R"("lastChangePcnt":-0.0559,)"
                       R"("bidPrice":null,)"
                       R"("midPrice":null,)"
                       R"("askPrice":null,)"
                       R"("impactBidPrice":null,)"
                       R"("impactMidPrice":null,)"
                       R"("impactAskPrice":null,)"
                       R"("hasLiquidity":false,)"
                       R"("openInterest":null,)"
                       R"("openValue":0,)"
                       R"("fairMethod":"",)"
                       R"("fairBasisRate":null,)"
                       R"("fairBasis":null,)"
                       R"("fairPrice":null,)"
                       R"("markMethod":"LastPrice",)"
                       R"("markPrice":6.93,)"
                       R"("indicativeTaxRate":null,)"
                       R"("indicativeSettlePrice":null,)"
                       R"("optionUnderlyingPrice":null,)"
                       R"("settledPrice":null,)"
                       R"("timestamp":"2020-01-23T04:50:00.000Z")"
                       R"(},{)"
                       R"("symbol":"XRPH20",)"
                       R"("rootSymbol":"XRP",)"
                       R"("state":"Open",)"
                       R"("typ":"FFCCSX",)"
                       R"("listing":"2019-12-06T04:00:00.000Z",)"
                       R"("front":"2020-02-28T12:00:00.000Z",)"
                       R"("expiry":"2020-03-27T12:00:00.000Z",)"
                       R"("settle":"2020-03-27T12:00:00.000Z",)"
                       R"("relistInterval":null,)"
                       R"("inverseLeg":"",)"
                       R"("sellLeg":"",)"
                       R"("buyLeg":"",)"
                       R"("optionStrikePcnt":null,)"
                       R"("optionStrikeRound":null,)"
                       R"("optionStrikePrice":null,)"
                       R"("optionMultiplier":null,)"
                       R"("positionCurrency":"XRP",)"
                       R"("underlying":"XRP",)"
                       R"("quoteCurrency":"XBT",)"
                       R"("underlyingSymbol":"XRPXBT=",)"
                       R"("reference":"BMEX",)"
                       R"("referenceSymbol":".BXRPXBT30M",)"
                       R"("calcInterval":null,)"
                       R"("publishInterval":null,)"
                       R"("publishTime":null,)"
                       R"("maxOrderQty":100000000,)"
                       R"("maxPrice":1,)"
                       R"("lotSize":1,)"
                       R"("tickSize":1e-8,)"
                       R"("multiplier":100000000,)"
                       R"("settlCurrency":"XBt",)"
                       R"("underlyingToPositionMultiplier":1,)"
                       R"("underlyingToSettleMultiplier":null,)"
                       R"("quoteToSettleMultiplier":100000000,)"
                       R"("isQuanto":false,)"
                       R"("isInverse":false,)"
                       R"("initMargin":0.05,)"
                       R"("maintMargin":0.025,)"
                       R"("riskLimit":5000000000,)"
                       R"("riskStep":5000000000,)"
                       R"("limit":null,)"
                       R"("capped":false,)"
                       R"("taxed":true,)"
                       R"("deleverage":true,)"
                       R"("makerFee":-0.0005,)"
                       R"("takerFee":0.0025,)"
                       R"("settlementFee":0,)"
                       R"("insuranceFee":0,)"
                       R"("fundingBaseSymbol":"",)"
                       R"("fundingQuoteSymbol":"",)"
                       R"("fundingPremiumSymbol":"",)"
                       R"("fundingTimestamp":null,)"
                       R"("fundingInterval":null,)"
                       R"("fundingRate":null,)"
                       R"("indicativeFundingRate":null,)"
                       R"("rebalanceTimestamp":null,)"
                       R"("rebalanceInterval":null,)"
                       R"("openingTimestamp":"2020-01-22T19:00:00.000Z",)"
                       R"("closingTimestamp":"2020-01-22T20:00:00.000Z",)"
                       R"("sessionInterval":"2000-01-01T01:00:00.000Z",)"
                       R"("prevClosePrice":0.00002701,)"
                       R"("limitDownPrice":null,)"
                       R"("limitUpPrice":null,)"
                       R"("bankruptLimitDownPrice":null,)"
                       R"("bankruptLimitUpPrice":null,)"
                       R"("prevTotalVolume":26517101,)"
                       R"("totalVolume":26517103,)"
                       R"("volume":2,)"
                       R"("volume24h":84425,)"
                       R"("prevTotalTurnover":72134564731,)"
                       R"("totalTurnover":72134570075,)"
                       R"("turnover":5344,)"
                       R"("turnover24h":228958667,)"
                       R"("homeNotional24h":84425,)"
                       R"("foreignNotional24h":2.28958667,)"
                       R"("prevPrice24h":0.00002725,)"
                       R"("vwap":0.00002712,)"
                       R"("highPrice":0.00002775,)"
                       R"("lowPrice":0.00002672,)"
                       R"("lastPrice":0.00002672,)"
                       R"("lastPriceProtected":0.00002672,)"
                       R"("lastTickDirection":"ZeroMinusTick",)"
                       R"("lastChangePcnt":-0.0194,)"
                       R"("bidPrice":0.00002672,)"
                       R"("midPrice":0.00002685,)"
                       R"("askPrice":0.00002698,)"
                       R"("impactBidPrice":0.00002661,)"
                       R"("impactMidPrice":0.00002681,)"
                       R"("impactAskPrice":0.00002701,)"
                       R"("hasLiquidity":true,)"
                       R"("openInterest":17614667,)"
                       R"("openValue":47207307560,)"
                       R"("fairMethod":"ImpactMidPrice",)"
                       R"("fairBasisRate":-0.09,)"
                       R"("fairBasis":-4.3e-7,)"
                       R"("fairPrice":0.0000268,)"
                       R"("markMethod":"FairPrice",)"
                       R"("markPrice":0.0000268,)"
                       R"("indicativeTaxRate":null,)"
                       R"("indicativeSettlePrice":0.00002723,)"
                       R"("optionUnderlyingPrice":null,)"
                       R"("settledPrice":null,)"
                       R"("timestamp":"2020-01-22T19:09:30.000Z")"
                       R"(})"
                       R"(])"sv;
  core::json::BufferStack buffer{8192, 1};
  json::Instrument obj{message, buffer};
  CHECK(std::size(obj.data) == size_t{2});
  // item #0
  CHECK(obj.data[0].symbol == ".EVOL7D");
  // item #1
  CHECK(obj.data[1].symbol == "XRPH20");
}
