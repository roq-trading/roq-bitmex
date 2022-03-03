/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch.hpp>

#include "roq/core/datetime.h"

#include "roq/bitmex/json/error_parser.h"
#include "roq/bitmex/json/error_response.h"

using namespace roq;
using namespace roq::bitmex;

using namespace std::literals;

// "{"error":{"message":"Invalid leavesQty","name":"HTTPError"}}"

TEST_CASE("json_error_response_simple_1", "json_error_response") {
  const auto message = R"(
  {"error":{"message":"Account has insufficient Available Balance, 5929700 XBt required","name":"CodedHTTPError","details":"19000"}}")"sv;
  auto res = json::ErrorParser::dispatch(message, [](auto &error) {
    CHECK(error.message == "Account has insufficient Available Balance, 5929700 XBt required"sv);
  });
  CHECK(res == true);
}

TEST_CASE("json_error_response_simple_2", "json_error_response") {
  const auto message = R"({"error":{"message":"Invalid leavesQty","name":"HTTPError"}})"sv;
  auto res = json::ErrorParser::dispatch(
      message, [](auto &error) { CHECK(error.message == "Invalid leavesQty"sv); });
  CHECK(res == true);
}
