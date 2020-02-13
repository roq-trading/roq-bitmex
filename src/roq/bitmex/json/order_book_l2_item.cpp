/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/order_book_l2_item.h"

#include "roq/bitmex/json/utils.h"

#include "roq/logging.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  ID,
  PRICE,
  SIDE,
  SIZE,
  SYMBOL,
};

constexpr Field parse_i(const std::string_view& name) {
  if (name.length() == 2 &&
      name[1] == 'd') {
    return Field::ID;
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

constexpr Field parse_name(const std::string_view& name) {
  if (name.length() > 0) {
    switch (name[0]) {
      case 'i':
        return parse_i(name);
      case 'p':
        return parse_p(name);
      case 's':
        return parse_s(name);
    }
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("id") == Field::ID);
static_assert(parse_name("price") == Field::PRICE);
static_assert(parse_name("side") == Field::SIDE);
static_assert(parse_name("size") == Field::SIZE);
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
    case Field::ID:
      update(result.id, value);
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
  }
}
}  // namespace

OrderBookL2Item::OrderBookL2Item(core::json::value_t& value) {
  for (auto [key, value] : std::get<core::json::object_t>(value))
    update_field(*this, key, value);
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
