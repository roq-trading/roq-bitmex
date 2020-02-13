/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/trade_item.h"

#include "roq/bitmex/json/utils.h"

#include "roq/logging.h"

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

constexpr Field parse_f(const std::string_view& name) {
  if (name.length() == 15 &&
      name[1] == 'o' &&
      name[2] == 'r' &&
      name[3] == 'e' &&
      name[4] == 'i' &&
      name[5] == 'g' &&
      name[6] == 'n' &&
      name[7] == 'N' &&
      name[8] == 'o' &&
      name[9] == 't' &&
      name[10] == 'i' &&
      name[11] == 'o' &&
      name[12] == 'n' &&
      name[13] == 'a' &&
      name[14] == 'l') {
    return Field::FOREIGN_NOTIONAL;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_g(const std::string_view& name) {
  if (name.length() == 10 &&
      name[1] == 'r' &&
      name[2] == 'o' &&
      name[3] == 's' &&
      name[4] == 's' &&
      name[5] == 'V' &&
      name[6] == 'a' &&
      name[7] == 'l' &&
      name[8] == 'u' &&
      name[9] == 'e') {
    return Field::GROSS_VALUE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_h(const std::string_view& name) {
  if (name.length() == 12 &&
      name[1] == 'o' &&
      name[2] == 'm' &&
      name[3] == 'e' &&
      name[4] == 'N' &&
      name[5] == 'o' &&
      name[6] == 't' &&
      name[7] == 'i' &&
      name[8] == 'o' &&
      name[9] == 'n' &&
      name[10] == 'a' &&
      name[11] == 'l') {
    return Field::HOME_NOTIONAL;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_p(const std::string_view& name) {
  if (name.length() == 5 &&
      name[1] == 'r' &&
      name[2] == 'i' &&
      name[3] == 'c' &&
      name[4] == 'e') {
    return Field::PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_sid(const std::string_view& name) {
  if (name.length() == 4 &&
      name[3] == 'e') {
    return Field::SIDE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_siz(const std::string_view& name) {
  if (name.length() == 4 &&
      name[3] == 'e') {
    return Field::SIZE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_si(const std::string_view& name) {
  if (name.length() > 2) {
    switch (name[2]) {
      case 'd':
        return parse_sid(name);
      case 'z':
        return parse_siz(name);
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
      case 'i':
        return parse_si(name);
      case 'y':
        return parse_sy(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_tic(const std::string_view& name) {
  if (name.length() == 13 &&
      name[3] == 'k' &&
      name[4] == 'D' &&
      name[5] == 'i' &&
      name[6] == 'r' &&
      name[7] == 'e' &&
      name[8] == 'c' &&
      name[9] == 't' &&
      name[10] == 'i' &&
      name[11] == 'o' &&
      name[12] == 'n') {
    return Field::TICK_DIRECTION;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_tim(const std::string_view& name) {
  if (name.length() == 9 &&
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

constexpr Field parse_ti(const std::string_view& name) {
  if (name.length() > 2) {
    switch (name[2]) {
      case 'c':
        return parse_tic(name);
      case 'm':
        return parse_tim(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_tr(const std::string_view& name) {
  if (name.length() == 10 &&
      name[2] == 'd' &&
      name[3] == 'M' &&
      name[4] == 'a' &&
      name[5] == 't' &&
      name[6] == 'c' &&
      name[7] == 'h' &&
      name[8] == 'I' &&
      name[9] == 'D') {
    return Field::TRD_MATCH_ID;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_t(const std::string_view& name) {
  if (name.length() > 1) {
    switch (name[1]) {
      case 'i':
        return parse_ti(name);
      case 'r':
        return parse_tr(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  if (name.length() > 0) {
    switch (name[0]) {
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
    case Field::FOREIGN_NOTIONAL:
      update(result.foreign_notional, value);
      break;
    case Field::GROSS_VALUE:
      update(result.gross_value, value);
      break;
    case Field::HOME_NOTIONAL:
      update(result.home_notional, value);
      break;
    case Field::PRICE:
      update(result.price, value);
      break;
    case Field::SIDE:
      update(result.side, value);
      break;
    case Field::SIZE:
      update(result.size, value);
      break;
    case Field::SYMBOL:
      update(result.symbol, value);
      break;
    case Field::TICK_DIRECTION:
      update(result.tick_direction, value);
      break;
    case Field::TIMESTAMP:
      update(result.timestamp, value);
      break;
    case Field::TRD_MATCH_ID:
      update(result.trd_match_id, value);
      break;
  }
}
}  // namespace

TradeItem::TradeItem(core::json::value_t& value) {
  for (auto [key, value] : std::get<core::json::object_t>(value))
    update_field(*this, key, value);
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
