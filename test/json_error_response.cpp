/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include <cmath>

#include "roq/core/datetime.h"

#include "roq/bitmex/json/error.h"

using namespace roq;
using namespace roq::bitmex;

TEST(json_error_response, simple) {
  const auto message = R"(
  {"error":{"message":"Account has insufficient Available Balance, 5929700 XBt required","name":"CodedHTTPError","details":"19000"}}")"_sv;
  // auto obj = core::json::Parser::create<json::Error>(message);
  /*
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
  */
}
