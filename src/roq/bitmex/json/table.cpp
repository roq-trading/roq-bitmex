/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/table.h"

#include <cassert>

#include "roq/logging.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
constexpr Table parse_e(const std::string_view& name) {
  if (name.length() == 9 &&
      name[1] == 'x' &&
      name[2] == 'e' &&
      name[3] == 'c' &&
      name[4] == 'u' &&
      name[5] == 't' &&
      name[6] == 'i' &&
      name[7] == 'o' &&
      name[8] == 'n') {
    return Table::EXECUTION;
  }
  return Table::UNKNOWN;
}

constexpr Table parse_f(const std::string_view& name) {
  if (name.length() == 7 &&
      name[1] == 'u' &&
      name[2] == 'n' &&
      name[3] == 'd' &&
      name[4] == 'i' &&
      name[5] == 'n' &&
      name[6] == 'g') {
    return Table::FUNDING;
  }
  return Table::UNKNOWN;
}

constexpr Table parse_i(const std::string_view& name) {
  if (name.length() == 10 &&
      name[1] == 'n' &&
      name[2] == 's' &&
      name[3] == 't' &&
      name[4] == 'r' &&
      name[5] == 'u' &&
      name[6] == 'm' &&
      name[7] == 'e' &&
      name[8] == 'n' &&
      name[9] == 't') {
    return Table::INSTRUMENT;
  }
  return Table::UNKNOWN;
}

constexpr Table parse_l(const std::string_view& name) {
  if (name.length() == 11 &&
      name[1] == 'i' &&
      name[2] == 'q' &&
      name[3] == 'u' &&
      name[4] == 'i' &&
      name[5] == 'd' &&
      name[6] == 'a' &&
      name[7] == 't' &&
      name[8] == 'i' &&
      name[9] == 'o' &&
      name[10] == 'n') {
    return Table::LIQUIDATION;
  }
  return Table::UNKNOWN;
}

constexpr Table parse_m(const std::string_view& name) {
  if (name.length() == 6 &&
      name[1] == 'a' &&
      name[2] == 'r' &&
      name[3] == 'g' &&
      name[4] == 'i' &&
      name[5] == 'n') {
    return Table::MARGIN;
  }
  return Table::UNKNOWN;
}

constexpr Table parse_order(const std::string_view& name) {
  if (name.length() == 11 &&
      name[5] == 'B' &&
      name[6] == 'o' &&
      name[7] == 'o' &&
      name[8] == 'k' &&
      name[9] == 'L' &&
      name[10] == '2') {
    return Table::ORDER_BOOK_L2;
  }
  return Table::UNKNOWN;
}

constexpr Table parse_o(const std::string_view& name) {
  if (name.length() >= 5 &&
      name[1] == 'r' &&
      name[2] == 'd' &&
      name[3] == 'e' &&
      name[4] == 'r') {
    if (name.length() == 5)
      return Table::ORDER;
    return parse_order(name);
  }
  return Table::UNKNOWN;
}

constexpr Table parse_p(const std::string_view& name) {
  if (name.length() == 8 &&
      name[1] == 'o' &&
      name[2] == 's' &&
      name[3] == 'i' &&
      name[4] == 't' &&
      name[5] == 'i' &&
      name[6] == 'o' &&
      name[7] == 'n') {
    return Table::POSITION;
  }
  return Table::UNKNOWN;
}

constexpr Table parse_q(const std::string_view& name) {
  if (name.length() == 5 &&
      name[1] == 'u' &&
      name[2] == 'o' &&
      name[3] == 't' &&
      name[4] == 'e') {
    return Table::QUOTE;
  }
  return Table::UNKNOWN;
}

constexpr Table parse_s(const std::string_view& name) {
  if (name.length() == 10 &&
      name[1] == 'e' &&
      name[2] == 't' &&
      name[3] == 't' &&
      name[4] == 'l' &&
      name[5] == 'e' &&
      name[6] == 'm' &&
      name[7] == 'e' &&
      name[8] == 'n' &&
      name[9] == 't') {
    return Table::SETTLEMENT;
  }
  return Table::UNKNOWN;
}

constexpr Table parse_t(const std::string_view& name) {
  if (name.length() == 5 &&
      name[1] == 'r' &&
      name[2] == 'a' &&
      name[3] == 'd' &&
      name[4] == 'e') {
    return Table::TRADE;
  }
  return Table::UNKNOWN;
}

constexpr Table parse_name(const std::string_view& name) {
  if (name.length() > 0) {
    switch (name[0]) {
      case 'e':
        return parse_e(name);
      case 'f':
        return parse_f(name);
      case 'i':
        return parse_i(name);
      case 'l':
        return parse_l(name);
      case 'm':
        return parse_m(name);
      case 'o':
        return parse_o(name);
      case 'p':
        return parse_p(name);
      case 'q':
        return parse_q(name);
      case 's':
        return parse_s(name);
      case 't':
        return parse_t(name);
    }
  }
  return Table::UNKNOWN;
}

static_assert(parse_name("execution") == Table::EXECUTION);
static_assert(parse_name("funding") == Table::FUNDING);
static_assert(parse_name("instrument") == Table::INSTRUMENT);
static_assert(parse_name("liquidation") == Table::LIQUIDATION);
static_assert(parse_name("margin") == Table::MARGIN);
static_assert(parse_name("order") == Table::ORDER);
static_assert(parse_name("orderBookL2") == Table::ORDER_BOOK_L2);
static_assert(parse_name("position") == Table::POSITION);
static_assert(parse_name("quote") == Table::QUOTE);
static_assert(parse_name("settlement") == Table::SETTLEMENT);
static_assert(parse_name("trade") == Table::TRADE);
}  // namespace

Table parse_table(const std::string_view& name) {
  auto result = parse_name(name);
  DLOG_IF(FATAL, result == Table::UNKNOWN)(
      FMT_STRING("Unknown name=\"{}\""),
      name);
  return result;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
