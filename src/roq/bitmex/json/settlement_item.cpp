/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/settlement_item.h"

#include "roq/bitmex/json/utils.h"

#include "roq/logging.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  BANKRUPT,
  OPTION_STRIKE_PRICE,
  OPTION_UNDERLYING_PRICE,
  SETTLED_PRICE,
  SETTLEMENT_TYPE,
  SYMBOL,
  TAX_BASE,
  TAX_RATE,
  TIMESTAMP,
};

constexpr Field parse_b(const std::string_view& name) {
  if (name.length() == 8 &&
      name[1] == 'a' &&
      name[2] == 'n' &&
      name[3] == 'k' &&
      name[4] == 'r' &&
      name[5] == 'u' &&
      name[6] == 'p' &&
      name[7] == 't') {
    return Field::BANKRUPT;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_optionS(const std::string_view& name) {
  if (name.length() == 17 &&
      name[7] == 't' &&
      name[8] == 'r' &&
      name[9] == 'i' &&
      name[10] == 'k' &&
      name[11] == 'e' &&
      name[12] == 'P' &&
      name[13] == 'r' &&
      name[14] == 'i' &&
      name[15] == 'c' &&
      name[16] == 'e') {
    return Field::OPTION_STRIKE_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_optionU(const std::string_view& name) {
  if (name.length() == 21 &&
      name[7] == 'n' &&
      name[8] == 'd' &&
      name[9] == 'e' &&
      name[10] == 'r' &&
      name[11] == 'l' &&
      name[12] == 'y' &&
      name[13] == 'i' &&
      name[14] == 'n' &&
      name[15] == 'g' &&
      name[16] == 'P' &&
      name[17] == 'r' &&
      name[18] == 'i' &&
      name[19] == 'c' &&
      name[20] == 'e') {
    return Field::OPTION_UNDERLYING_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_o(const std::string_view& name) {
  if (name.length() >= 7 &&
      name[1] == 'p' &&
      name[2] == 't' &&
      name[3] == 'i' &&
      name[4] == 'o' &&
      name[5] == 'n') {
    switch (name[6]) {
      case 'S':
        return parse_optionS(name);
      case 'U':
        return parse_optionU(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_settled(const std::string_view& name) {
  if (name.length() == 12 &&
      name[7] == 'P' &&
      name[8] == 'r' &&
      name[9] == 'i' &&
      name[10] == 'c' &&
      name[11] == 'e') {
    return Field::SETTLED_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_settlem(const std::string_view& name) {
  if (name.length() == 14 &&
      name[7] == 'e' &&
      name[8] == 'n' &&
      name[9] == 't' &&
      name[10] == 'T' &&
      name[11] == 'y' &&
      name[12] == 'p' &&
      name[13] == 'e') {
    return Field::SETTLEMENT_TYPE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_se(const std::string_view& name) {
  if (name.length() >= 7 &&
      name[2] == 't' &&
      name[3] == 't' &&
      name[4] == 'l' &&
      name[5] == 'e') {
    switch (name[6]) {
      case 'd':
        return parse_settled(name);
      case 'm':
        return parse_settlem(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_sy(const std::string_view& name) {
  if (name.length() == 6 &&
      name[2] == 'm' &&
      name[3] == 'b' &&
      name[4] == 'o' &&
      name[5] == 'l') {
    return Field::SYMBOL;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_s(const std::string_view& name) {
  if (name.length() > 1) {
    switch (name[1]) {
      case 'e':
        return parse_se(name);
      case 'y':
        return parse_sy(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_taxB(const std::string_view& name) {
  if (name.length() == 7 &&
      name[4] == 'a' &&
      name[5] == 's' &&
      name[6] == 'e') {
    return Field::TAX_BASE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_taxR(const std::string_view& name) {
  if (name.length() == 7 &&
      name[4] == 'a' &&
      name[5] == 't' &&
      name[6] == 'e') {
    return Field::TAX_RATE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ta(const std::string_view& name) {
  if (name.length() >= 4 &&
      name[2] == 'x') {
    switch (name[3]) {
      case 'B':
        return parse_taxB(name);
      case 'R':
        return parse_taxR(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ti(const std::string_view& name) {
  if (name.length() == 9 &&
      name[2] == 'm' &&
      name[3] == 'e' &&
      name[4] == 's' &&
      name[5] == 't' &&
      name[6] == 'a' &&
      name[7] == 'm' &&
      name[8] == 'p') {
    return Field::TIMESTAMP;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_t(const std::string_view& name) {
  if (name.length() > 1) {
    switch (name[1]) {
      case 'a':
        return parse_ta(name);
      case 'i':
        return parse_ti(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  if (name.length() > 0) {
    switch (name[0]) {
      case 'b':
        return parse_b(name);
      case 'o':
        return parse_o(name);
      case 's':
        return parse_s(name);
      case 't':
        return parse_t(name);
    }
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("bankrupt") == Field::BANKRUPT);
static_assert(parse_name("optionStrikePrice") == Field::OPTION_STRIKE_PRICE);
static_assert(parse_name("optionUnderlyingPrice") == Field::OPTION_UNDERLYING_PRICE);
static_assert(parse_name("settledPrice") == Field::SETTLED_PRICE);
static_assert(parse_name("settlementType") == Field::SETTLEMENT_TYPE);
static_assert(parse_name("symbol") == Field::SYMBOL);
static_assert(parse_name("taxBase") == Field::TAX_BASE);
static_assert(parse_name("taxRate") == Field::TAX_RATE);
static_assert(parse_name("timestamp") == Field::TIMESTAMP);

void update_field(
    auto& result,
    auto& key,
    auto& value) {
  auto field = parse_name(key);
  switch (field) {
    case Field::UNKNOWN:
      DLOG(FATAL)(
          FMT_STRING("Unknown key=\"{}\""),
          key);
      break;
    case Field::BANKRUPT:
      update(result.bankrupt, value);
      break;
    case Field::OPTION_STRIKE_PRICE:
      update(result.option_strike_price, value);
      break;
    case Field::OPTION_UNDERLYING_PRICE:
      update(result.option_underlying_price, value);
      break;
    case Field::SETTLED_PRICE:
      update(result.settled_price, value);
      break;
    case Field::SETTLEMENT_TYPE:
      update(result.settlement_type, value);
      break;
    case Field::SYMBOL:
      update(result.symbol, value);
      break;
    case Field::TAX_BASE:
      update(result.tax_base, value);
      break;
    case Field::TAX_RATE:
      update(result.tax_rate, value);
      break;
    case Field::TIMESTAMP:
      update(result.timestamp, value);
      break;
  }
}
}  // namespace

SettlementItem::SettlementItem(core::json::value_t& value) {
  for (auto [key, value] : std::get<core::json::object_t>(value))
    update_field(*this, key, value);
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
