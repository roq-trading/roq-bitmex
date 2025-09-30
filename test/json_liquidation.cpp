/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch_all.hpp>

#include "roq/core/datetime.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitmex/json/liquidation.hpp"

using namespace roq;
using namespace roq::bitmex;

using namespace std::literals;

using namespace Catch::literals;

// note! truncated
TEST_CASE("empty", "[json_liquidation]") {
  auto const message = R"({)"
                       R"("table":"liquidation",)"
                       R"("action":"partial",)"
                       R"("keys":["orderID"],)"
                       R"("types":{)"
                       R"("orderID":"guid",)"
                       R"("symbol":"symbol",)"
                       R"("side":"symbol",)"
                       R"("price":"float",)"
                       R"("leavesQty":"long")"
                       R"(},)"
                       R"("filter":{},)"
                       R"("data":[])"
                       R"(})"sv;
  core::json::BufferStack buffer{8192, 1};
  json::Liquidation obj{message, buffer};
}
