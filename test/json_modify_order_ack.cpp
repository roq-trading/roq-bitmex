/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/bitmex/json/order_data_item.hpp"

using namespace roq;
using namespace roq::bitmex;

using namespace std::literals;

TEST_CASE("simple", "[json_modify_order_ack]") {
  auto const message = R"({)"
                       R"("account":273093,)"
                       R"("clOrdID":"EQACG_dR1D8AAgAAAAAA",)"
                       R"("cumQty":0,)"
                       R"("currency":"USDT",)"
                       R"("leavesQty":1000,)"
                       R"("ordStatus":"New",)"
                       R"("ordType":"Limit",)"
                       R"("orderID":"ccff62f6-568e-4941-9d21-a4020a5b8901",)"
                       R"("orderQty":1000,)"
                       R"("price":32323.0,)"
                       R"("side":"Buy",)"
                       R"("strategy":"OneWay",)"
                       R"("symbol":"XBT_USDT",)"
                       R"("text":"Amended orderQty price\nSubmitted via API.",)"
                       R"("timeInForce":"GoodTillCancel",)"
                       R"("timestamp":"2025-11-14T07:08:41.454Z",)"
                       R"("transactTime":"2025-11-14T07:08:41.453Z",)"
                       R"("workingIndicator":true)"
                       R"(})"sv;
  json::OrderDataItem obj{message};
}
