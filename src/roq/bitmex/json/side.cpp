/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/side.h"

#include <cassert>

#include "roq/logging.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
constexpr auto parse_b(const std::string_view& name) {
  if (name.compare("Buy") == 0)
    return Side::BUY;
  return Side::UNKNOWN;
}

constexpr auto parse_s(const std::string_view& name) {
  if (name.compare("Sell") == 0)
    return Side::SELL;
  return Side::UNKNOWN;
}

constexpr auto parse_helper(const std::string_view& name) {
  if (name.empty())
    return Side::UNDEFINED;
  switch (name[0]) {
    case 'B':
      return parse_b(name);
    case 'S':
      return parse_s(name);
  }
  return Side::UNKNOWN;
}

static_assert(parse_helper("") == Side::UNDEFINED);
static_assert(parse_helper("Buy") == Side::BUY);
static_assert(parse_helper("Sell") == Side::SELL);
}  // namespace

Side parse_side(const std::string_view& name) {
  auto result = parse_helper(name);
  DLOG_IF(FATAL, result == Side::UNKNOWN)(
      "Unknown name=\"{}\"", name);
  return result;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
