/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/activate.h"

#include "roq/bitmex/json/utils.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  FUNDS,
  ORDER_ID,
  PRIVATE,
  PRODUCT_ID,
  PROFILE_ID,
  SIDE,
  SIZE,
  STOP_PRICE,
  STOP_TYPE,
  TAKER_FEE_RATE,
  TIMESTAMP,
  USER_ID,
};

constexpr Field parse_f(auto& name) {
  if (name.compare("funds") == 0)
    return Field::FUNDS;
  return Field::UNKNOWN;
}

constexpr Field parse_o(auto& name) {
  if (name.compare("order_id") == 0)
    return Field::ORDER_ID;
  return Field::UNKNOWN;
}

constexpr Field parse_p(auto& name) {
  if (name.length() >= 4) {
    switch (name.data()[3]) {
      case 'v':
        if (name.compare("private") == 0)
          return Field::PRIVATE;
        break;
      case 'd':
        if (name.compare("product_id") == 0)
          return Field::PRODUCT_ID;
        break;
      case 'f':
        if (name.compare("profile_id") == 0)
          return Field::PROFILE_ID;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_s(auto& name) {
  if (name.length() >= 3) {
    switch (name.data()[2]) {
      case 'd':
        if (name.compare("side") == 0)
          return Field::SIDE;
        break;
      case 'z':
        if (name.compare("size") == 0)
          return Field::SIZE;
        break;
      case 'o':
        if (name.length() >= 6) {
          switch (name.data()[5]) {
            case 'p':
              if (name.compare("stop_price") == 0)
                return Field::STOP_PRICE;
              break;
            case 't':
              if (name.compare("stop_type") == 0)
                return Field::STOP_TYPE;
              break;
          }
        }
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_t(auto& name) {
  if (name.length() >= 2) {
    switch (name.data()[1]) {
      case 'a':
        if (name.compare("taker_fee_rate") == 0)
          return Field::TAKER_FEE_RATE;
        break;
      case 'i':
        if (name.compare("timestamp") == 0)
          return Field::TIMESTAMP;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_u(auto& name) {
  if (name.compare("user_id") == 0)
    return Field::USER_ID;
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  assert(name.empty() == false);
  switch (name.data()[0]) {
    case 'f':
      return parse_f(name);
    case 'o':
      return parse_o(name);
    case 'p':
      return parse_p(name);
    case 's':
      return parse_s(name);
    case 't':
      return parse_t(name);
    case 'u':
      return parse_u(name);
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("funds") == Field::FUNDS);
static_assert(parse_name("order_id") == Field::ORDER_ID);
static_assert(parse_name("private") == Field::PRIVATE);
static_assert(parse_name("product_id") == Field::PRODUCT_ID);
static_assert(parse_name("profile_id") == Field::PROFILE_ID);
static_assert(parse_name("side") == Field::SIDE);
static_assert(parse_name("size") == Field::SIZE);
static_assert(parse_name("stop_price") == Field::STOP_PRICE);
static_assert(parse_name("stop_type") == Field::STOP_TYPE);
static_assert(parse_name("taker_fee_rate") == Field::TAKER_FEE_RATE);
static_assert(parse_name("timestamp") == Field::TIMESTAMP);
static_assert(parse_name("user_id") == Field::USER_ID);

inline void update_field(auto& result, auto& field, auto& value) {
  switch (field) {
    case Field::UNKNOWN: {
      break;
    }
    case Field::FUNDS: {
      update(result.funds, value);
      break;
    }
    case Field::ORDER_ID: {
      update(result.order_id, value);
      break;
    }
    case Field::PRIVATE: {
      update(result.private_, value);
      break;
    }
    case Field::PRODUCT_ID: {
      update(result.product_id, value);
      break;
    }
    case Field::PROFILE_ID: {
      update(result.profile_id, value);
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
    case Field::STOP_PRICE: {
      update(result.stop_price, value);
      break;
    }
    case Field::STOP_TYPE: {
      update(result.stop_type, value);
      break;
    }
    case Field::TAKER_FEE_RATE: {
      update(result.taker_fee_rate, value);
      break;
    }
    case Field::TIMESTAMP: {
      update(result.timestamp, value);
      break;
    }
    case Field::USER_ID: {
      update(result.user_id, value);
      break;
    }
  }
}
}  // namespace

Activate Activate::parse(const std::string_view& message) {
  Activate result;
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
