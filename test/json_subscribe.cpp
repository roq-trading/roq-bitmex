/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::bitmex;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::Subscribe;

TEST_CASE("simple", "[json_subscribe]") {
  auto const message = R"({)"
                       R"("success":true,)"
                       R"("subscribe":"instrument",)"
                       R"("request":{)"
                       R"("op":"subscribe",)"
                       R"("args":"instrument")"
                       R"(})"
                       R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.success == true);
    CHECK(obj.subscribe == "instrument"sv);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
