/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/last_match.h"

#include "roq/bitmex/json/utils.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  MAKER_ORDER_ID,
  PRICE,
  PRODUCT_ID,
  SEQUENCE,
  SIDE,
  SIZE,
  TAKER_ORDER_ID,
  TIME,
  TRADE_ID,
};

constexpr Field parse_m(auto& name) {
  if (name.compare("maker_order_id") == 0)
    return Field::MAKER_ORDER_ID;
  return Field::UNKNOWN;
}

constexpr Field parse_p(auto& name) {
  if (name.length() >= 3) {
    switch (name.data()[2]) {
      case 'i':
        if (name.compare("price") == 0)
          return Field::PRICE;
        break;
      case 'o':
        if (name.compare("product_id") == 0)
          return Field::PRODUCT_ID;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_s(auto& name) {
  if (name.length() >= 3) {
    switch (name.data()[2]) {
      case 'q':
        if (name.compare("sequence") == 0)
          return Field::SEQUENCE;
        break;
      case 'd':
        if (name.compare("side") == 0)
          return Field::SIDE;
        break;
      case 'z':
        if (name.compare("size") == 0)
          return Field::SIZE;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_t(auto& name) {
  if (name.length() >= 2) {
    switch (name.data()[1]) {
      case 'a':
        if (name.compare("taker_order_id") == 0)
          return Field::TAKER_ORDER_ID;
        break;
      case 'i':
        if (name.compare("time") == 0)
          return Field::TIME;
        break;
      case 'r':
        if (name.compare("trade_id") == 0)
          return Field::TRADE_ID;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  assert(name.empty() == false);
  switch (name.data()[0]) {
    case 'm':
      return parse_m(name);
    case 'p':
      return parse_p(name);
    case 's':
      return parse_s(name);
    case 't':
      return parse_t(name);
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("maker_order_id") == Field::MAKER_ORDER_ID);
static_assert(parse_name("price") == Field::PRICE);
static_assert(parse_name("product_id") == Field::PRODUCT_ID);
static_assert(parse_name("sequence") == Field::SEQUENCE);
static_assert(parse_name("side") == Field::SIDE);
static_assert(parse_name("size") == Field::SIZE);
static_assert(parse_name("taker_order_id") == Field::TAKER_ORDER_ID);
static_assert(parse_name("time") == Field::TIME);
static_assert(parse_name("trade_id") == Field::TRADE_ID);

inline void update_field(auto& result, auto& field, auto& value) {
  switch (field) {
    case Field::UNKNOWN: {
      break;
    }
    case Field::MAKER_ORDER_ID: {
      update(result.maker_order_id, value);
      break;
    }
    case Field::PRICE: {
      update(result.price, value);
      break;
    }
    case Field::PRODUCT_ID: {
      update(result.product_id, value);
      break;
    }
    case Field::SEQUENCE: {
      update(result.sequence, value);
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
    case Field::TAKER_ORDER_ID: {
      update(result.taker_order_id, value);
      break;
    }
    case Field::TIME: {
      update(result.time, value);
      break;
    }
    case Field::TRADE_ID: {
      update(result.trade_id, value);
      break;
    }
  }
}
}  // namespace

LastMatch LastMatch::parse(const std::string_view& message) {
  LastMatch result;
  core::json::Parser parser(message);
  for (auto [key, value] : parser.root<core::json::object_t>()) {
    auto field = parse_name(key);
    update_field(result, field, value);
  }
  return result;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
