/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitmex/protocol/json/cancel_all_orders_ack.hpp"

using namespace roq;
using namespace roq::bitmex;

using namespace std::literals;

using value_type = protocol::json::CancelAllOrdersAck;

TEST_CASE("simple", "[json_cancel_all_orders_ack]") {
  auto const message = R"([{)"
                       R"("account":273093,)"
                       R"("clOrdID":"1wACBqHY0z8AAQAAAAAA",)"
                       R"("cumQty":0,)"
                       R"("currency":"USDT",)"
                       R"("leavesQty":0,)"
                       R"("ordStatus":"Canceled",)"
                       R"("ordType":"Limit",)"
                       R"("orderID":"57375492-e737-4e7a-b2be-4f7e7259f527",)"
                       R"("orderQty":1000,)"
                       R"("price":32000.0,)"
                       R"("side":"Buy",)"
                       R"("strategy":"OneWay",)"
                       R"("symbol":"XBT_USDT",)"
                       R"("text":"Canceled\nCanceled via API.",)"
                       R"("timeInForce":"GoodTillCancel",)"
                       R"("timestamp":"2025-11-14T06:55:25.578Z",)"
                       R"("transactTime":"2025-11-14T06:55:25.578Z",)"
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
