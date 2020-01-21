/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/order.h"

#include "roq/bitmex/json/utils.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  CREATED_AT,
  EXECUTED_VALUE,
  FILLED_SIZE,
  FILL_FEES,
  ID,
  POST_ONLY,
  PRICE,
  PRODUCT_ID,
  SETTLED,
  SIDE,
  SIZE,
  STATUS,
  STP,
  TIME_IN_FORCE,
  TYPE,
};

constexpr Field parse_c(auto& name) {
  if (name.compare("created_at") == 0)
    return Field::CREATED_AT;
  return Field::UNKNOWN;
}

constexpr Field parse_e(auto& name) {
  if (name.compare("executed_value") == 0)
    return Field::EXECUTED_VALUE;
  return Field::UNKNOWN;
}

constexpr Field parse_f(auto& name) {
  if (name.length() >= 5) {
    switch (name.data()[4]) {
      case 'e':
        if (name.compare("filled_size") == 0)
          return Field::FILLED_SIZE;
        break;
      case '_':
        if (name.compare("fill_fees") == 0)
          return Field::FILL_FEES;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_i(auto& name) {
  if (name.compare("id") == 0)
    return Field::ID;
  return Field::UNKNOWN;
}

constexpr Field parse_p(auto& name) {
  if (name.length() >= 3) {
    switch (name.data()[2]) {
      case 's':
        if (name.compare("post_only") == 0)
          return Field::POST_ONLY;
        break;
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
      case 't':
        if (name.compare("settled") == 0)
          return Field::SETTLED;
        break;
      case 'd':
        if (name.compare("side") == 0)
          return Field::SIDE;
        break;
      case 'z':
        if (name.compare("size") == 0)
          return Field::SIZE;
        break;
      case 'a':
        if (name.compare("status") == 0)
          return Field::STATUS;
        break;
      case 'p':
        if (name.compare("stp") == 0)
          return Field::STP;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_t(auto& name) {
  if (name.length() >= 2) {
    switch (name.data()[1]) {
      case 'i':
        if (name.compare("time_in_force") == 0)
          return Field::TIME_IN_FORCE;
        break;
      case 'y':
        if (name.compare("type") == 0)
          return Field::TYPE;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  assert(name.empty() == false);
  switch (name.data()[0]) {
    case 'c':
      return parse_c(name);
    case 'e':
      return parse_e(name);
    case 'f':
      return parse_f(name);
    case 'i':
      return parse_i(name);
    case 'p':
      return parse_p(name);
    case 's':
      return parse_s(name);
    case 't':
      return parse_t(name);
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("created_at") == Field::CREATED_AT);
static_assert(parse_name("executed_value") == Field::EXECUTED_VALUE);
static_assert(parse_name("filled_size") == Field::FILLED_SIZE);
static_assert(parse_name("fill_fees") == Field::FILL_FEES);
static_assert(parse_name("id") == Field::ID);
static_assert(parse_name("post_only") == Field::POST_ONLY);
static_assert(parse_name("price") == Field::PRICE);
static_assert(parse_name("product_id") == Field::PRODUCT_ID);
static_assert(parse_name("settled") == Field::SETTLED);
static_assert(parse_name("side") == Field::SIDE);
static_assert(parse_name("size") == Field::SIZE);
static_assert(parse_name("status") == Field::STATUS);
static_assert(parse_name("stp") == Field::STP);
static_assert(parse_name("time_in_force") == Field::TIME_IN_FORCE);
static_assert(parse_name("type") == Field::TYPE);

inline void update_field(auto& result, auto& field, auto& value) {
  switch (field) {
    case Field::UNKNOWN: {
      break;
    }
    case Field::CREATED_AT: {
      update(result.created_at, value);
      break;
    }
    case Field::EXECUTED_VALUE: {
      update(result.executed_value, value);
      break;
    }
    case Field::FILLED_SIZE: {
      update(result.filled_size, value);
      break;
    }
    case Field::FILL_FEES: {
      update(result.fill_fees, value);
      break;
    }
    case Field::ID: {
      update(result.id, value);
      break;
    }
    case Field::POST_ONLY: {
      update(result.post_only, value);
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
    case Field::SETTLED: {
      update(result.settled, value);
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
    case Field::STATUS: {
      update(result.status, value);
      break;
    }
    case Field::STP: {
      update(result.stp, value);
      break;
    }
    case Field::TIME_IN_FORCE: {
      update(result.time_in_force, value);
      break;
    }
    case Field::TYPE: {
      update(result.type, value);
      break;
    }
  }
}
}  // namespace

Order Order::parse(const std::string_view& message) {
  core::json::Parser parser(message);
  Order result;
  parse(result, parser.root<core::json::object_t>());
  return result;
}

void Order::parse(Order& result, core::json::object_t&& object) {
  new (&result) std::remove_reference<decltype(result)>::type {};
  for (auto [key, value] : object) {
    auto field = parse_name(key);
    update_field(result, field, value);
  }
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
