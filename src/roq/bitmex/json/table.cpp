/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/table.h"

#include <cassert>

#include "roq/logging.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
constexpr auto parse_helper(const std::string_view& name) {
  assert(name.empty() == false);
  switch (name.data()[0]) {
    case 'f':
      if (name.compare("funding") == 0)
        return Table::FUNDING;
      break;
    case 'i':
      if (name.compare("instrument") == 0)
        return Table::INSTRUMENT;
      break;
    case 'l':
      if (name.compare("liquidation") == 0)
        return Table::LIQUIDATION;
      break;
    case 'o':
      if (name.compare("orderBookL2") == 0)
        return Table::ORDER_BOOK_L2;
      break;
    case 'q':
      if (name.compare("quote") == 0)
        return Table::QUOTE;
      break;
    case 's':
      if (name.compare("settlement") == 0)
        return Table::SETTLEMENT;
      break;
    case 't':
      if (name.compare("trade") == 0)
        return Table::TRADE;
      break;
  }
  return Table::UNKNOWN;
}

static_assert(parse_helper("funding") == Table::FUNDING);
static_assert(parse_helper("instrument") == Table::INSTRUMENT);
static_assert(parse_helper("liquidation") == Table::LIQUIDATION);
static_assert(parse_helper("orderBookL2") == Table::ORDER_BOOK_L2);
static_assert(parse_helper("quote") == Table::QUOTE);
static_assert(parse_helper("settlement") == Table::SETTLEMENT);
static_assert(parse_helper("trade") == Table::TRADE);
}  // namespace

Table parse_table(const std::string_view& name) {
  auto result = parse_helper(name);
  DLOG_IF(FATAL, result == Table::UNKNOWN)(
      "Can't parse name=\"{}\"", name);
  return result;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
