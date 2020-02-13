/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/liquidation_item.h"

#include "roq/bitmex/json/utils.h"

#include "roq/logging.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  LEAVES_QTY,
  ORDER_ID,
  PRICE,
  SIDE,
  SYMBOL,
};

constexpr Field parse_l(const std::string_view& name) {
  if (name.length() == 9 &&
      name[1] == 'e' &&
      name[2] == 'a' &&
      name[3] == 'v' &&
      name[4] == 'e' &&
      name[5] == 's' &&
      name[6] == 'Q' &&
      name[7] == 't' &&
      name[8] == 'y') {
    return Field::LEAVES_QTY;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_o(const std::string_view& name) {
  if (name.length() == 7 &&
      name[1] == 'r' &&
      name[2] == 'd' &&
      name[3] == 'e' &&
      name[4] == 'r' &&
      name[5] == 'I' &&
      name[6] == 'D') {
    return Field::ORDER_ID;
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

constexpr Field parse_si(const std::string_view& name) {
  if (name.length() == 4 &&
      name[2] == 'd' &&
      name[3] == 'e') {
    return Field::SIDE;
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

constexpr Field parse_name(const std::string_view& name) {
  if (name.length() > 0) {
    switch (name[0]) {
      case 'l':
        return parse_l(name);
      case 'o':
        return parse_o(name);
      case 'p':
        return parse_p(name);
      case 's':
        return parse_s(name);
    }
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("leavesQty") == Field::LEAVES_QTY);
static_assert(parse_name("orderID") == Field::ORDER_ID);
static_assert(parse_name("price") == Field::PRICE);
static_assert(parse_name("side") == Field::SIDE);
static_assert(parse_name("symbol") == Field::SYMBOL);

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
    case Field::LEAVES_QTY:
      update(result.leaves_qty, value);
      break;
    case Field::ORDER_ID:
      update(result.order_id, value);
      break;
    case Field::PRICE:
      update(result.price, value);
      break;
    case Field::SIDE:
      update(result.side, value);
      break;
    case Field::SYMBOL:
      update(result.symbol, value);
      break;
  }
}
}  // namespace

LiquidationItem::LiquidationItem(core::json::value_t& value) {
  for (auto [key, value] : std::get<core::json::object_t>(value))
    update_field(*this, key, value);
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
