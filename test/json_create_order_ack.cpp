/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/bitmex/json/order_data_item.hpp"

using namespace roq;
using namespace roq::bitmex;

using namespace std::literals;

TEST_CASE("simple", "[json_create_order_ack]") {
  auto const message = R"({)"
                       R"("account":273093,)"
                       R"("clOrdID":"OAACO4sn1D8AAQAAAAAA",)"
                       R"("cumQty":0,)"
                       R"("currency":"USDT",)"
                       R"("leavesQty":1000,)"
                       R"("ordStatus":"New",)"
                       R"("ordType":"Limit",)"
                       R"("orderID":"07bb442e-5362-4a01-bd9a-664a1da3d646",)"
                       R"("orderQty":1000,)"
                       R"("price":32000.0,)"
                       R"("side":"Buy",)"
                       R"("strategy":"OneWay",)"
                       R"("symbol":"XBT_USDT",)"
                       R"("text":"Submitted via API.",)"
                       R"("timeInForce":"GoodTillCancel",)"
                       R"("timestamp":"2025-11-14T07:03:58.114Z",)"
                       R"("transactTime":"2025-11-14T07:03:58.113Z",)"
                       R"("workingIndicator":true)"
                       R"(})"sv;
  json::OrderDataItem obj{message};
}
