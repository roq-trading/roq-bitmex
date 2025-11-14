/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitmex/json/cancel_order_ack.hpp"

using namespace roq;
using namespace roq::bitmex;

using namespace std::literals;

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
  core::json::BufferStack buffer{8192, 1};
  json::CancelOrderAck obj{message, buffer};
}
