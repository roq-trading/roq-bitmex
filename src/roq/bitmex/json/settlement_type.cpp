/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/settlement_type.h"

#include <cassert>

#include "roq/logging.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {

constexpr auto parse_helper(const std::string_view& name) {
  assert(name.empty() == false);
  switch (name.data()[0]) {
    case 'R':
      if (name.compare("Rebalance") == 0)
        return SettlementType::REBALANCE;
      break;
    case 'S':
      if (name.compare("Settlement") == 0)
        return SettlementType::SETTLEMENT;
      break;
  }
  return SettlementType::UNKNOWN;
}

static_assert(parse_helper("Settlement") == SettlementType::SETTLEMENT);
}  // namespace

SettlementType parse_settlement_type(const std::string_view& name) {
  auto result = parse_helper(name);
  DLOG_IF(FATAL, result == SettlementType::UNKNOWN)(
      "Unknown name=\"{}\"", name);
  return result;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
