/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/received.h"

#include "roq/bitmex/json/utils.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  CLIENT_OID,
  ORDER_ID,
  ORDER_TYPE,
  PRICE,
  PRODUCT_ID,
  SEQUENCE,
  SIDE,
  SIZE,
  TIME,
};

constexpr Field parse_c(auto& name) {
  if (name.compare("client_oid") == 0)
    return Field::CLIENT_OID;
  return Field::UNKNOWN;
}

constexpr Field parse_o(auto& name) {
  if (name.length() >= 7) {
    switch (name.data()[6]) {
      case 'i':
        if (name.compare("order_id") == 0)
          return Field::ORDER_ID;
        break;
      case 't':
        if (name.compare("order_type") == 0)
          return Field::ORDER_TYPE;
        break;
    }
  }
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
  if (name.compare("time") == 0)
    return Field::TIME;
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  assert(name.empty() == false);
  switch (name.data()[0]) {
    case 'c':
      return parse_c(name);
    case 'o':
      return parse_o(name);
    case 'p':
      return parse_p(name);
    case 's':
      return parse_s(name);
    case 't':
      return parse_t(name);
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("client_oid") == Field::CLIENT_OID);
static_assert(parse_name("order_id") == Field::ORDER_ID);
static_assert(parse_name("order_type") == Field::ORDER_TYPE);
static_assert(parse_name("price") == Field::PRICE);
static_assert(parse_name("product_id") == Field::PRODUCT_ID);
static_assert(parse_name("sequence") == Field::SEQUENCE);
static_assert(parse_name("side") == Field::SIDE);
static_assert(parse_name("size") == Field::SIZE);
static_assert(parse_name("time") == Field::TIME);

inline void update_field(auto& result, auto& field, auto& value) {
  switch (field) {
    case Field::UNKNOWN: {
      break;
    }
    case Field::CLIENT_OID: {
      update(result.client_oid, value);
      break;
    }
    case Field::ORDER_ID: {
      update(result.order_id, value);
      break;
    }
    case Field::ORDER_TYPE: {
      update(result.order_type, value);
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
    case Field::TIME: {
      update(result.time, value);
      break;
    }
  }
}
}  // namespace

Received Received::parse(const std::string_view& message) {
  Received result;
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
