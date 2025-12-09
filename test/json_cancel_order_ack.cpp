/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitmex/json/cancel_order_ack.hpp"

using namespace roq;
using namespace roq::bitmex;

using namespace std::literals;

using value_type = json::CancelOrderAck;

TEST_CASE("simple", "[json_cancel_order_ack]") {
  auto const message = R"([{)"
                       R"("account":273093,)"
                       R"("clOrdID":"hgACGVJh1D8AAgAAAAAA",)"
                       R"("cumQty":0,)"
                       R"("currency":"USDT",)"
                       R"("leavesQty":0,)"
                       R"("ordStatus":"Canceled",)"
                       R"("ordType":"Limit",)"
                       R"("orderID":"d1505385-b942-4055-a349-24cab74581b2",)"
                       R"("orderQty":1000,)"
                       R"("price":32323.0,)"
                       R"("side":"Buy",)"
                       R"("strategy":"OneWay",)"
                       R"("symbol":"XBT_USDT",)"
                       R"("text":"Canceled\nCanceled via API.",)"
                       R"("timeInForce":"GoodTillCancel",)"
                       R"("timestamp":"2025-11-14T07:10:26.748Z",)"
                       R"("transactTime":"2025-11-14T07:10:26.747Z",)"
                       R"("workingIndicator":false)"
                       R"(})"
                       R"(])"sv;
  auto helper = [&](value_type &obj) {
    REQUIRE(std::size(obj.data) == 1);
    CHECK(obj.data[0].account == 273093);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}

TEST_CASE("failure", "[json_cancel_order_ack]") {
  auto const message = R"([{)"
                       R"("account":424266,)"
                       R"("error":"Unable to cancel order",)"
                       R"("orderID":"e119ebc6-c1c9-4108-88e0-c819bb688646",)"
                       R"("timestamp":"2025-12-09T08:21:26.122Z",)"
                       R"("transactTime":"2025-12-09T08:21:26.122Z",)"
                       R"("workingIndicator":false)"
                       R"(})"
                       R"(])"sv;
  auto helper = [&](value_type &obj) {
    REQUIRE(std::size(obj.data) == 1);
    CHECK(obj.data[0].account == 424266);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
