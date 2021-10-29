/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include <cmath>

#include "roq/core/datetime.h"

#include "roq/bitmex/json/error_parser.h"
#include "roq/bitmex/json/error_response.h"

using namespace roq;
using namespace roq::bitmex;

using namespace std::literals;

// "{"error":{"message":"Invalid leavesQty","name":"HTTPError"}}"

TEST(json_error_response, simple_1) {
  const auto message = R"(
  {"error":{"message":"Account has insufficient Available Balance, 5929700 XBt required","name":"CodedHTTPError","details":"19000"}}")"sv;
  auto res = json::ErrorParser::dispatch(message, [](auto &error) {
    EXPECT_EQ(error.message, "Account has insufficient Available Balance, 5929700 XBt required"sv);
  });
  EXPECT_TRUE(res);
}

TEST(json_error_response, simple_2) {
  const auto message = R"({"error":{"message":"Invalid leavesQty","name":"HTTPError"}})"sv;
  auto res = json::ErrorParser::dispatch(
      message, [](auto &error) { EXPECT_EQ(error.message, "Invalid leavesQty"sv); });
  EXPECT_TRUE(res);
}
