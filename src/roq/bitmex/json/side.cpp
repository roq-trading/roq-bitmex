/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/side.h"

#include <cassert>

namespace roq {
namespace bitmex {
namespace json {

namespace {
constexpr auto parse_helper(const std::string_view& name) {
  assert(name.empty() == false);
  switch (name.data()[0]) {
    case 'B':
      if (name.compare("Buy") == 0)
        return Side::BUY;
      break;
    case 'S':
      if (name.compare("Sell") == 0)
        return Side::SELL;
      break;
  }
  return Side::UNKNOWN;
}

static_assert(parse_helper("Buy") == Side::BUY);
static_assert(parse_helper("Sell") == Side::SELL);
}  // namespace

Side parse_side(const std::string_view& name) {
  auto result = parse_helper(name);
#ifndef NDEBUG
  if (result == Side::UNKNOWN) {
    fprintf(stderr, "Can't parse side=\"%.*s\"\n",
        static_cast<int>(name.length()), name.data());
    assert(false);
  }
#endif
  return result;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
