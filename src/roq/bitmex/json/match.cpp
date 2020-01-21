/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/match.h"

#include "roq/bitmex/json/utils.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  MAKER_ORDER_ID,
  MAKER_PROFILE_ID,
  MAKER_USER_ID,
  PRICE,
  PRODUCT_ID,
  PROFILE_ID,
  SEQUENCE,
  SIDE,
  SIZE,
  TAKER_ORDER_ID,
  TAKER_PROFILE_ID,
  TAKER_USER_ID,
  TIME,
  TRADE_ID,
  USER_ID,
};

constexpr Field parse_m(auto& name) {
  if (name.length() >= 7) {
    switch (name.data()[6]) {
      case 'o':
        if (name.compare("maker_order_id") == 0)
          return Field::MAKER_ORDER_ID;
        break;
      case 'p':
        if (name.compare("maker_profile_id") == 0)
          return Field::MAKER_PROFILE_ID;
        break;
      case 'u':
        if (name.compare("maker_user_id") == 0)
          return Field::MAKER_USER_ID;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_p(auto& name) {
  if (name.length() >= 4) {
    switch (name.data()[3]) {
      case 'c':
        if (name.compare("price") == 0)
          return Field::PRICE;
        break;
      case 'd':
        if (name.compare("product_id") == 0)
          return Field::PRODUCT_ID;
        break;
      case 'f':
        if (name.compare("profile_id") == 0)
          return Field::PROFILE_ID;
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
        if (name.length() >= 7) {
          switch (name.data()[6]) {
            case 'o':
              if (name.compare("taker_order_id") == 0)
                return Field::TAKER_ORDER_ID;
              break;
            case 'p':
              if (name.compare("taker_profile_id") == 0)
                return Field::TAKER_PROFILE_ID;
              break;
            case 'u':
              if (name.compare("taker_user_id") == 0)
                return Field::TAKER_USER_ID;
              break;
          }
        }
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

constexpr Field parse_u(auto& name) {
  if (name.compare("user_id") == 0)
    return Field::USER_ID;
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
    case 'u':
      return parse_u(name);
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("maker_order_id") == Field::MAKER_ORDER_ID);
static_assert(parse_name("maker_profile_id") == Field::MAKER_PROFILE_ID);
static_assert(parse_name("maker_user_id") == Field::MAKER_USER_ID);
static_assert(parse_name("price") == Field::PRICE);
static_assert(parse_name("product_id") == Field::PRODUCT_ID);
static_assert(parse_name("profile_id") == Field::PROFILE_ID);
static_assert(parse_name("sequence") == Field::SEQUENCE);
static_assert(parse_name("side") == Field::SIDE);
static_assert(parse_name("size") == Field::SIZE);
static_assert(parse_name("taker_order_id") == Field::TAKER_ORDER_ID);
static_assert(parse_name("taker_profile_id") == Field::TAKER_PROFILE_ID);
static_assert(parse_name("taker_user_id") == Field::TAKER_USER_ID);
static_assert(parse_name("time") == Field::TIME);
static_assert(parse_name("trade_id") == Field::TRADE_ID);
static_assert(parse_name("user_id") == Field::USER_ID);

inline void update_field(auto& result, auto& field, auto& value) {
  switch (field) {
    case Field::UNKNOWN: {
      break;
    }
    case Field::MAKER_ORDER_ID: {
      update(result.maker_order_id, value);
      break;
    }
    case Field::MAKER_PROFILE_ID: {
      update(result.maker_profile_id, value);
      break;
    }
    case Field::MAKER_USER_ID: {
      update(result.maker_user_id, value);
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
    case Field::PROFILE_ID: {
      update(result.profile_id, value);
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
    case Field::TAKER_PROFILE_ID: {
      update(result.taker_profile_id, value);
      break;
    }
    case Field::TAKER_USER_ID: {
      update(result.taker_user_id, value);
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
    case Field::USER_ID: {
      update(result.user_id, value);
      break;
    }
  }
}
}  // namespace

Match Match::parse(const std::string_view& message) {
  Match result;
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
