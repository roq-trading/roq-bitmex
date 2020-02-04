/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/settlement_type.h"

#include <cassert>

#include "roq/logging.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
constexpr auto parse_r(const std::string_view& name) {
  if (name.compare("Rebalance") == 0)
    return SettlementType::REBALANCE;
  return SettlementType::UNKNOWN;
}

constexpr auto parse_s(const std::string_view& name) {
  if (name.compare("Settlement") == 0)
    return SettlementType::SETTLEMENT;
  return SettlementType::UNKNOWN;
}

constexpr auto parse_helper(const std::string_view& name) {
  if (name.empty())
    return SettlementType::UNDEFINED;
  switch (name[0]) {
    case 'R':
      return parse_r(name);
    case 'S':
      return parse_s(name);
  }
  return SettlementType::UNKNOWN;
}

static_assert(parse_helper("Rebalance") == SettlementType::REBALANCE);
static_assert(parse_helper("Settlement") == SettlementType::SETTLEMENT);
}  // namespace

SettlementType parse_settlement_type(const std::string_view& name) {
  auto result = parse_helper(name);
  DLOG_IF(FATAL, result == SettlementType::UNKNOWN)(
      FMT_STRING("Unknown name=\"{}\""),
      name);
  return result;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
