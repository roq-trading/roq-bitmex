/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/settlement_type.h"

#include <cassert>

#include "roq/logging.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
constexpr SettlementType parse_R(const std::string_view& name) {
  if (name.length() == 9 &&
      name[1] == 'e' &&
      name[2] == 'b' &&
      name[3] == 'a' &&
      name[4] == 'l' &&
      name[5] == 'a' &&
      name[6] == 'n' &&
      name[7] == 'c' &&
      name[8] == 'e') {
    return SettlementType::REBALANCE;
  }
  return SettlementType::UNKNOWN;
}

constexpr SettlementType parse_S(const std::string_view& name) {
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
    return SettlementType::SETTLEMENT;
  }
  return SettlementType::UNKNOWN;
}

constexpr SettlementType parse_name(const std::string_view& name) {
  if (name.length() > 0) {
    switch (name[0]) {
      case 'R':
        return parse_R(name);
      case 'S':
        return parse_S(name);
    }
  }
  return SettlementType::UNKNOWN;
}

static_assert(parse_name("Rebalance") == SettlementType::REBALANCE);
static_assert(parse_name("Settlement") == SettlementType::SETTLEMENT);
}  // namespace

SettlementType parse_settlement_type(const std::string_view& name) {
  auto result = parse_name(name);
  DLOG_IF(FATAL, result == SettlementType::UNKNOWN)(
      FMT_STRING("Unknown name=\"{}\""),
      name);
  return result;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
