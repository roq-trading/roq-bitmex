/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitmex/protocol/json/order_data_item.hpp"

using namespace roq;
using namespace roq::bitmex;

using namespace std::literals;

using value_type = protocol::json::OrderDataItem;

TEST_CASE("create_ack", "[json_order_data_item]") {
  auto message = R"({)"
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
  auto helper = [&](value_type &obj) {
    CHECK(obj.account == 273093);
    CHECK(obj.cl_ord_id == "OAACO4sn1D8AAQAAAAAA"sv);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}

TEST_CASE("modify_ack", "[json_order_data_item]") {
  auto message = R"({)"
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
  auto helper = [&](value_type &obj) {
    CHECK(obj.account == 273093);
    CHECK(obj.cl_ord_id == "EQACG_dR1D8AAgAAAAAA"sv);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
