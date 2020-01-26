/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/trade_item.h"

#include "roq/bitmex/json/utils.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  FOREIGN_NOTIONAL,
  GROSS_VALUE,
  HOME_NOTIONAL,
  PRICE,
  SIDE,
  SIZE,
  SYMBOL,
  TICK_DIRECTION,
  TIMESTAMP,
  TRD_MATCH_ID,
};

constexpr Field parse_f(auto& name) {
  if (name.compare("foreignNotional") == 0)
    return Field::FOREIGN_NOTIONAL;
  return Field::UNKNOWN;
}

constexpr Field parse_g(auto& name) {
  if (name.compare("grossValue") == 0)
    return Field::GROSS_VALUE;
  return Field::UNKNOWN;
}

constexpr Field parse_h(auto& name) {
  if (name.compare("homeNotional") == 0)
    return Field::HOME_NOTIONAL;
  return Field::UNKNOWN;
}

constexpr Field parse_p(auto& name) {
  if (name.compare("price") == 0)
    return Field::PRICE;
  return Field::UNKNOWN;
}

constexpr Field parse_s(auto& name) {
  if (name.length() >= 3) {
    switch (name.data()[2]) {
      case 'd': {
        if (name.compare("side") == 0)
          return Field::SIDE;
        break;
      }
      case 'z': {
        if (name.compare("size") == 0)
          return Field::SIZE;
        break;
      }
      case 'm': {
        if (name.compare("symbol") == 0)
          return Field::SYMBOL;
        break;
      }
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_t(auto& name) {
  if (name.length() >= 3) {
    switch (name.data()[2]) {
      case 'c': {
        if (name.compare("tickDirection") == 0)
          return Field::TICK_DIRECTION;
        break;
      }
      case 'm': {
        if (name.compare("timestamp") == 0)
          return Field::TIMESTAMP;
        break;
      }
      case 'd': {
        if (name.compare("trdMatchID") == 0)
          return Field::TRD_MATCH_ID;
        break;
      }
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  assert(name.empty() == false);
  switch (name.data()[0]) {
    case 'f':
      return parse_f(name);
    case 'g':
      return parse_g(name);
    case 'h':
      return parse_h(name);
    case 'p':
      return parse_p(name);
    case 's':
      return parse_s(name);
    case 't':
      return parse_t(name);
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("foreignNotional") == Field::FOREIGN_NOTIONAL);
static_assert(parse_name("grossValue") == Field::GROSS_VALUE);
static_assert(parse_name("homeNotional") == Field::HOME_NOTIONAL);
static_assert(parse_name("price") == Field::PRICE);
static_assert(parse_name("side") == Field::SIDE);
static_assert(parse_name("size") == Field::SIZE);
static_assert(parse_name("symbol") == Field::SYMBOL);
static_assert(parse_name("tickDirection") == Field::TICK_DIRECTION);
static_assert(parse_name("timestamp") == Field::TIMESTAMP);
static_assert(parse_name("trdMatchID") == Field::TRD_MATCH_ID);

inline void update_field(auto& result, auto& field, auto& value) {
  switch (field) {
    case Field::UNKNOWN: {
      break;
    }
    case Field::FOREIGN_NOTIONAL: {
      update(result.foreign_notional, value);
      break;
    }
    case Field::GROSS_VALUE: {
      update(result.gross_value, value);
      break;
    }
    case Field::HOME_NOTIONAL: {
      update(result.home_notional, value);
      break;
    }
    case Field::PRICE: {
      update(result.price, value);
      break;
    }
    case Field::SIDE: {
      update(result.side, value);
      break;
    }
    case Field::SIZE: {
      update(result.size, value);
      break;
    }
    case Field::SYMBOL: {
      update(result.symbol, value);
      break;
    }
    case Field::TICK_DIRECTION: {
      update(result.tick_direction, value);
      break;
    }
    case Field::TIMESTAMP: {
      update(result.timestamp, value);
      break;
    }
    case Field::TRD_MATCH_ID: {
      update(result.trd_match_id, value);
      break;
    }
  }
}
}  // namespace

TradeItem TradeItem::parse(const std::string_view& message) {
  core::json::Parser parser(message);
  auto root = parser.root();
  TradeItem result;
  result.parse(std::get<core::json::object_t>(root));
  return result;
}

void TradeItem::parse(core::json::object_t& object) {
  (*this) = {};  // XXX not the right place to reset
  for (auto [key, value] : object) {
    auto field = parse_name(key);
    update_field(*this, field, value);
  }
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
