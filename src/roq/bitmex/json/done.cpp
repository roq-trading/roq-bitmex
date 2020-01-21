/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/done.h"

#include "roq/bitmex/json/utils.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  ORDER_ID,
  PRICE,
  PRODUCT_ID,
  REASON,
  REMAINING_SIZE,
  SEQUENCE,
  SIDE,
  TIME,
};

constexpr Field parse_o(auto& name) {
  if (name.compare("order_id") == 0)
    return Field::ORDER_ID;
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

constexpr Field parse_r(auto& name) {
  if (name.length() >= 3) {
    switch (name.data()[2]) {
      case 'a':
        if (name.compare("reason") == 0)
          return Field::REASON;
        break;
      case 'm':
        if (name.compare("remaining_size") == 0)
          return Field::REMAINING_SIZE;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_s(auto& name) {
  if (name.length() >= 2) {
    switch (name.data()[1]) {
      case 'e':
        if (name.compare("sequence") == 0)
          return Field::SEQUENCE;
        break;
      case 'i':
        if (name.compare("side") == 0)
          return Field::SIDE;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_t(auto& name) {
  if (name.compare("time") == 0)
    return Field::TIME;
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  assert(name.empty() == false);
  switch (name.data()[0]) {
    case 'o':
      return parse_o(name);
    case 'p':
      return parse_p(name);
    case 'r':
      return parse_r(name);
    case 's':
      return parse_s(name);
    case 't':
      return parse_t(name);
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("order_id") == Field::ORDER_ID);
static_assert(parse_name("price") == Field::PRICE);
static_assert(parse_name("product_id") == Field::PRODUCT_ID);
static_assert(parse_name("reason") == Field::REASON);
static_assert(parse_name("remaining_size") == Field::REMAINING_SIZE);
static_assert(parse_name("sequence") == Field::SEQUENCE);
static_assert(parse_name("side") == Field::SIDE);
static_assert(parse_name("time") == Field::TIME);

inline void update_field(auto& result, auto& field, auto& value) {
  switch (field) {
    case Field::UNKNOWN: {
      break;
    }
    case Field::ORDER_ID: {
      update(result.order_id, value);
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
    case Field::REASON: {
      update(result.reason, value);
      break;
    }
    case Field::REMAINING_SIZE: {
      update(result.remaining_size, value);
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
    case Field::TIME: {
      update(result.time, value);
      break;
    }
  }
}
}  // namespace

Done Done::parse(const std::string_view& message) {
  Done result;
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
