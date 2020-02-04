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

constexpr Field parse_i(auto& name) {
  if (name.compare("id") == 0)
    return Field::ID;
  return Field::UNKNOWN;
}

constexpr Field parse_p(auto& name) {
  if (name.compare("price") == 0)
    return Field::PRICE;
  return Field::UNKNOWN;
}

constexpr Field parse_s(auto& name) {
  if (name.length() >= 3) {
    switch (name[2]) {
      case 'd':
        if (name.compare("side") == 0)
          return Field::SIDE;
        break;
      case 'z':
        if (name.compare("size") == 0)
          return Field::SIZE;
        break;
      case 'm':
        if (name.compare("symbol") == 0)
          return Field::SYMBOL;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  if (name.empty())
    return Field::UNKNOWN;
  switch (name[0]) {
    case 'i':
      return parse_i(name);
    case 'p':
      return parse_p(name);
    case 's':
      return parse_s(name);
    default:
      return Field::UNKNOWN;
  }
}

static_assert(parse_name("id") == Field::ID);
static_assert(parse_name("price") == Field::PRICE);
static_assert(parse_name("side") == Field::SIDE);
static_assert(parse_name("size") == Field::SIZE);
static_assert(parse_name("symbol") == Field::SYMBOL);

inline void update_field(
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
