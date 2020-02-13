/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/side.h"

#include <cassert>

#include "roq/logging.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
constexpr Side parse_B(const std::string_view& name) {
  if (name.length() == 3 &&
      name[1] == 'u' &&
      name[2] == 'y') {
    return Side::BUY;
  }
  return Side::UNKNOWN;
}

constexpr Side parse_S(const std::string_view& name) {
  if (name.length() == 4 &&
      name[1] == 'e' &&
      name[2] == 'l' &&
      name[3] == 'l') {
    return Side::SELL;
  }
  return Side::UNKNOWN;
}

constexpr Side parse_name(const std::string_view& name) {
  if (name.length() > 0) {
    switch (name[0]) {
      case 'B':
        return parse_B(name);
      case 'S':
        return parse_S(name);
    }
  }
  return Side::UNKNOWN;
}

static_assert(parse_name("Buy") == Side::BUY);
static_assert(parse_name("Sell") == Side::SELL);
}  // namespace

Side parse_side(const std::string_view& name) {
  auto result = parse_name(name);
  DLOG_IF(FATAL, result == Side::UNKNOWN)(
      FMT_STRING("Unknown name=\"{}\""),
      name);
  return result;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
