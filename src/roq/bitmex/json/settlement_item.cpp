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

constexpr Field parse_b(auto& name) {
  if (name.compare("bankrupt") == 0)
    return Field::BANKRUPT;
  return Field::UNKNOWN;
}

constexpr Field parse_o(auto& name) {
  if (name.length() >= 7) {
    switch (name.data()[6]) {
      case 'S': {
        if (name.compare("optionStrikePrice") == 0)
          return Field::OPTION_STRIKE_PRICE;
        break;
      }
      case 'U': {
        if (name.compare("optionUnderlyingPrice") == 0)
          return Field::OPTION_UNDERLYING_PRICE;
        break;
      }
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_s(auto& name) {
  if (name.length() >= 2) {
    switch (name.data()[1]) {
      case 'e': {
        if (name.length() >= 7) {
          switch (name.data()[6]) {
            case 'd':
              if (name.compare("settledPrice") == 0)
                return Field::SETTLED_PRICE;
              break;
            case 'm':
              if (name.compare("settlementType") == 0)
                return Field::SETTLEMENT_TYPE;
              break;
          }
        }
        break;
      }
      case 'y': {
        if (name.compare("symbol") == 0)
          return Field::SYMBOL;
        break;
      }
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_t(auto& name) {
  if (name.length() >= 4) {
    switch (name.data()[3]) {
      case 'B': {
        if (name.compare("taxBase") == 0)
          return Field::TAX_BASE;
        break;
      }
      case 'R': {
        if (name.compare("taxRate") == 0)
          return Field::TAX_RATE;
        break;
      }
      case 'e': {
        if (name.compare("timestamp") == 0)
          return Field::TIMESTAMP;
        break;
      }
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  assert(name.empty() == false);
  switch (name.data()[0]) {
    case 'b':
      return parse_b(name);
    case 'o':
      return parse_o(name);
    case 's':
      return parse_s(name);
    case 't':
      return parse_t(name);
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

inline void update_field(
    auto& result,
    auto& field,
    auto& key,
    auto& value) {
  switch (field) {
    case Field::UNKNOWN: {
      DLOG(FATAL)("Unknown key=\"{}\"", key);
      break;
    }
    case Field::BANKRUPT: {
      update(result.bankrupt, value);
      break;
    }
    case Field::OPTION_STRIKE_PRICE: {
      update(result.option_strike_price, value);
      break;
    }
    case Field::OPTION_UNDERLYING_PRICE: {
      update(result.option_underlying_price, value);
      break;
    }
    case Field::SETTLED_PRICE: {
      update(result.settled_price, value);
      break;
    }
    case Field::SETTLEMENT_TYPE: {
      update(result.settlement_type, value);
      break;
    }
    case Field::SYMBOL: {
      update(result.symbol, value);
      break;
    }
    case Field::TAX_BASE: {
      update(result.tax_base, value);
      break;
    }
    case Field::TAX_RATE: {
      update(result.tax_rate, value);
      break;
    }
    case Field::TIMESTAMP: {
      update(result.timestamp, value);
      break;
    }
  }
}
}  // namespace

SettlementItem::SettlementItem(core::json::value_t& value) {
  for (auto [key, value] : std::get<core::json::object_t>(value)) {
    auto field = parse_name(key);
    update_field(*this, field, key, value);
  }
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
