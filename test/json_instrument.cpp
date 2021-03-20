/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include <cmath>

#include "roq/core/datetime.h"

#include "roq/bitmex/json/instrument.h"

using namespace roq;
using namespace roq::bitmex;

constexpr double TOLERANCE = 1.0e-10;

TEST(json_instrument_item, unlisted) {
  const auto message = R"({)"
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
                       R"(})"_sv;
  auto obj = core::json::Parser::create<json::InstrumentItem>(message);
  EXPECT_EQ(obj.symbol, ".EVOL7D");
  EXPECT_EQ(obj.root_symbol, "EVOL");
  EXPECT_EQ(obj.state, json::State::UNLISTED);
  EXPECT_EQ(obj.typ, json::Typ::MRIXXX);
  EXPECT_EQ(obj.underlying, "ETH");
  EXPECT_EQ(obj.quote_currency, "XXX");
  EXPECT_EQ(obj.reference, "BMEX");
  EXPECT_EQ(obj.reference_symbol, ".BETHXBT");
  EXPECT_EQ(obj.calc_interval, core::datetime(2000, 1u, 8u, 0u, 0u, 0u));
  EXPECT_EQ(obj.publish_interval, core::datetime(2000, 1u, 1u, 0u, 5u, 0u));
  EXPECT_NEAR(obj.tick_size, 0.01, TOLERANCE);
  EXPECT_EQ(obj.is_quanto, false);
  EXPECT_EQ(obj.is_inverse, false);
  EXPECT_EQ(obj.capped, false);
  EXPECT_EQ(obj.taxed, false);
  EXPECT_EQ(obj.deleverage, false);
  EXPECT_NEAR(obj.prev_price24h, 7.34, TOLERANCE);
  EXPECT_NEAR(obj.last_price, 6.93, TOLERANCE);
  EXPECT_EQ(obj.last_tick_direction, "ZeroMinusTick");
  EXPECT_NEAR(obj.last_change_pcnt, -0.0559, TOLERANCE);
  EXPECT_EQ(obj.has_liquidity, false);
  EXPECT_NEAR(obj.open_value, 0.0, TOLERANCE);
  EXPECT_NEAR(obj.mark_price, 6.93, TOLERANCE);
  EXPECT_EQ(obj.timestamp, core::datetime(2020, 1u, 23u, 4u, 50u, 0u));
}

TEST(json_instrument_item, open) {
  const auto message = R"({)"
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
                       R"(})"_sv;
  auto obj = core::json::Parser::create<json::InstrumentItem>(message);
  EXPECT_EQ(obj.symbol, "XRPH20");
  EXPECT_EQ(obj.root_symbol, "XRP");
  EXPECT_EQ(obj.state, json::State::OPEN);
  EXPECT_EQ(obj.typ, json::Typ::FFCCSX);
  EXPECT_EQ(obj.listing, core::datetime(2019, 12u, 6u, 4u, 0u, 0u));
  EXPECT_EQ(obj.front, core::datetime(2020, 2u, 28u, 12u, 0u, 0u));
  EXPECT_EQ(obj.expiry, core::datetime(2020, 3u, 27u, 12u, 0u, 0u));
  EXPECT_EQ(obj.settle, core::datetime(2020, 3u, 27u, 12u, 0u, 0u));
  EXPECT_EQ(obj.position_currency, "XRP");
  EXPECT_EQ(obj.underlying, "XRP");
  EXPECT_EQ(obj.quote_currency, "XBT");
  EXPECT_EQ(obj.underlying_symbol, "XRPXBT=");
  EXPECT_EQ(obj.reference, "BMEX");
  EXPECT_EQ(obj.reference_symbol, ".BXRPXBT30M");
  EXPECT_NEAR(obj.max_order_qty, 100000000.0, TOLERANCE);
  EXPECT_NEAR(obj.max_price, 1.0, TOLERANCE);
  EXPECT_NEAR(obj.lot_size, 1.0, TOLERANCE);
  EXPECT_NEAR(obj.tick_size, 1.0e-8, TOLERANCE);
  EXPECT_NEAR(obj.multiplier, 100000000.0, TOLERANCE);
  EXPECT_EQ(obj.settl_currency, "XBt");
  EXPECT_NEAR(obj.underlying_to_position_multiplier, 1.0, TOLERANCE);
  EXPECT_NEAR(obj.quote_to_settle_multiplier, 100000000.0, TOLERANCE);
  EXPECT_EQ(obj.is_quanto, false);
  EXPECT_EQ(obj.is_inverse, false);
  EXPECT_NEAR(obj.init_margin, 0.05, TOLERANCE);
  EXPECT_NEAR(obj.maint_margin, 0.025, TOLERANCE);
  EXPECT_NEAR(obj.risk_limit, 5000000000.0, TOLERANCE);
  EXPECT_NEAR(obj.risk_step, 5000000000.0, TOLERANCE);
  EXPECT_EQ(obj.capped, false);
  EXPECT_EQ(obj.taxed, true);
  EXPECT_EQ(obj.deleverage, true);
  EXPECT_NEAR(obj.maker_fee, -0.0005, TOLERANCE);
  EXPECT_NEAR(obj.taker_fee, 0.0025, TOLERANCE);
  EXPECT_NEAR(obj.settlement_fee, 0.0, TOLERANCE);
  EXPECT_NEAR(obj.insurance_fee, 0.0, TOLERANCE);
  EXPECT_EQ(obj.opening_timestamp, core::datetime(2020, 1u, 22u, 19u, 0u, 0u));
  EXPECT_EQ(obj.closing_timestamp, core::datetime(2020, 1u, 22u, 20u, 0u, 0u));
  EXPECT_EQ(obj.session_interval, core::datetime(2000, 1u, 1u, 1u, 0u, 0u));
  EXPECT_NEAR(obj.prev_close_price, 0.00002701, TOLERANCE);
  EXPECT_NEAR(obj.prev_total_volume, 26517101.0, TOLERANCE);
  EXPECT_NEAR(obj.total_volume, 26517103.0, TOLERANCE);
  EXPECT_NEAR(obj.volume, 2.0, TOLERANCE);
  EXPECT_NEAR(obj.volume24h, 84425.0, TOLERANCE);
  EXPECT_NEAR(obj.prev_total_turnover, 72134564731.0, TOLERANCE);
  EXPECT_NEAR(obj.total_turnover, 72134570075.0, TOLERANCE);
  EXPECT_NEAR(obj.turnover, 5344.0, TOLERANCE);
  EXPECT_NEAR(obj.turnover24h, 228958667.0, TOLERANCE);
  EXPECT_NEAR(obj.home_notional24h, 84425.0, TOLERANCE);
  EXPECT_NEAR(obj.foreign_notional24h, 2.28958667, TOLERANCE);
  EXPECT_NEAR(obj.prev_price24h, 0.00002725, TOLERANCE);
  EXPECT_NEAR(obj.vwap, 0.00002712, TOLERANCE);
  EXPECT_NEAR(obj.high_price, 0.00002775, TOLERANCE);
  EXPECT_NEAR(obj.low_price, 0.00002672, TOLERANCE);
  EXPECT_NEAR(obj.last_price, 0.00002672, TOLERANCE);
  EXPECT_NEAR(obj.last_price_protected, 0.00002672, TOLERANCE);
  EXPECT_EQ(obj.last_tick_direction, "ZeroMinusTick");
  EXPECT_NEAR(obj.last_change_pcnt, -0.0194, TOLERANCE);
  EXPECT_NEAR(obj.bid_price, 0.00002672, TOLERANCE);
  EXPECT_NEAR(obj.mid_price, 0.00002685, TOLERANCE);
  EXPECT_NEAR(obj.ask_price, 0.00002698, TOLERANCE);
  EXPECT_NEAR(obj.impact_bid_price, 0.00002661, TOLERANCE);
  EXPECT_NEAR(obj.impact_mid_price, 0.00002681, TOLERANCE);
  EXPECT_NEAR(obj.impact_ask_price, 0.00002701, TOLERANCE);
  EXPECT_EQ(obj.has_liquidity, true);
  EXPECT_NEAR(obj.open_interest, 17614667.0, TOLERANCE);
  EXPECT_NEAR(obj.open_value, 47207307560.0, TOLERANCE);
  EXPECT_EQ(obj.fair_method, "ImpactMidPrice");
  EXPECT_NEAR(obj.fair_basis_rate, -0.09, TOLERANCE);
  EXPECT_NEAR(obj.fair_basis, -4.3e-7, TOLERANCE);
  EXPECT_NEAR(obj.fair_price, 0.0000268, TOLERANCE);
  EXPECT_EQ(obj.mark_method, "FairPrice");
  EXPECT_NEAR(obj.mark_price, 0.0000268, TOLERANCE);
  EXPECT_NEAR(obj.indicative_settle_price, 0.00002723, TOLERANCE);
  EXPECT_EQ(obj.timestamp, core::datetime(2020, 1u, 22u, 19u, 9u, 30u));
}

TEST(json_instrument, empty) {
  const auto message = "[]"_sv;
  core::utils::Buffer buffer(8192);
  core::json::Buffer decode_buffer(buffer);
  auto obj = core::json::Parser::create<json::Instrument>(message, decode_buffer);
  EXPECT_EQ(obj.data.size(), size_t{0});
}

TEST(json_instrument, simple) {
  const auto message = R"([)"
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
                       R"(])"_sv;
  core::utils::Buffer buffer(8192);
  core::json::Buffer decode_buffer(buffer);
  auto obj = core::json::Parser::create<json::Instrument>(message, decode_buffer);
  EXPECT_EQ(obj.data.size(), size_t{2});
  // item #0
  EXPECT_EQ(obj.data[0].symbol, ".EVOL7D");
  // item #1
  EXPECT_EQ(obj.data[1].symbol, "XRPH20");
}
