/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch_all.hpp>

#include "roq/core/datetime.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitmex/json/subscribe.hpp"

using namespace roq;
using namespace roq::bitmex;

using namespace std::literals;

using namespace Catch::literals;

TEST_CASE("simple", "[json_subscribe]") {
  auto const message = R"({)"
                       R"("success":true,)"
                       R"("subscribe":"instrument",)"
                       R"("request":{)"
                       R"("op":"subscribe",)"
                       R"("args":"instrument")"
                       R"(})"
                       R"(})"sv;
  core::json::BufferStack buffer{8192, 1};
  json::Subscribe obj{message, buffer};
}
