/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/table.h"

#include <cassert>

#include "roq/logging.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
constexpr auto parse_e(const std::string_view& name) {
  if (name.compare("execution") == 0)
    return Table::EXECUTION;
  return Table::UNKNOWN;
}

constexpr auto parse_f(const std::string_view& name) {
  if (name.compare("funding") == 0)
    return Table::FUNDING;
  return Table::UNKNOWN;
}

constexpr auto parse_i(const std::string_view& name) {
  if (name.compare("instrument") == 0)
    return Table::INSTRUMENT;
  return Table::UNKNOWN;
}

constexpr auto parse_l(const std::string_view& name) {
  if (name.compare("liquidation") == 0)
    return Table::LIQUIDATION;
  return Table::UNKNOWN;
}

constexpr auto parse_m(const std::string_view& name) {
  if (name.compare("margin") == 0)
    return Table::MARGIN;
  return Table::UNKNOWN;
}

constexpr auto parse_o(const std::string_view& name) {
  if (name.length() >= 6) {
    if (name.compare("orderBookL2") == 0)
      return Table::ORDER_BOOK_L2;
  } else {
    if (name.compare("order") == 0)
      return Table::ORDER;
  }
  return Table::UNKNOWN;
}

constexpr auto parse_p(const std::string_view& name) {
  if (name.compare("position") == 0)
    return Table::POSITION;
  return Table::UNKNOWN;
}

constexpr auto parse_q(const std::string_view& name) {
  if (name.compare("quote") == 0)
    return Table::QUOTE;
  return Table::UNKNOWN;
}

constexpr auto parse_s(const std::string_view& name) {
  if (name.compare("settlement") == 0)
    return Table::SETTLEMENT;
  return Table::UNKNOWN;
}

constexpr auto parse_t(const std::string_view& name) {
  if (name.compare("trade") == 0)
    return Table::TRADE;
  return Table::UNKNOWN;
}

constexpr auto parse_helper(const std::string_view& name) {
  if (name.empty())
    return Table::UNKNOWN;
  switch (name[0]) {
    case 'e': return parse_e(name);
    case 'f': return parse_f(name);
    case 'i': return parse_i(name);
    case 'l': return parse_l(name);
    case 'm': return parse_m(name);
    case 'o': return parse_o(name);
    case 'p': return parse_p(name);
    case 'q': return parse_q(name);
    case 's': return parse_s(name);
    case 't': return parse_t(name);
    default:
      return Table::UNKNOWN;
  }
}

static_assert(parse_helper("execution") == Table::EXECUTION);
static_assert(parse_helper("funding") == Table::FUNDING);
static_assert(parse_helper("instrument") == Table::INSTRUMENT);
static_assert(parse_helper("liquidation") == Table::LIQUIDATION);
static_assert(parse_helper("margin") == Table::MARGIN);
static_assert(parse_helper("order") == Table::ORDER);
static_assert(parse_helper("orderBookL2") == Table::ORDER_BOOK_L2);
static_assert(parse_helper("position") == Table::POSITION);
static_assert(parse_helper("quote") == Table::QUOTE);
static_assert(parse_helper("settlement") == Table::SETTLEMENT);
static_assert(parse_helper("trade") == Table::TRADE);
}  // namespace

Table parse_table(const std::string_view& name) {
  auto result = parse_helper(name);
  DLOG_IF(FATAL, result == Table::UNKNOWN)(
      FMT_STRING("Unknown name=\"{}\""),
      name);
  return result;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
