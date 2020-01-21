/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/fill.h"

#include "roq/bitmex/json/utils.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  CREATED_AT,
  FEE,
  FILL_ID,
  LIQUIDITY,
  PRICE,
  PRODUCT_ID,
  SETTLED,
  SIDE,
  SIZE,
  TRADE_ID,
};

constexpr Field parse_c(auto& name) {
  if (name.compare("created_at") == 0)
    return Field::CREATED_AT;
  return Field::UNKNOWN;
}

constexpr Field parse_f(auto& name) {
  if (name.length() >= 2) {
    switch (name.data()[1]) {
      case 'e':
        if (name.compare("fee") == 0)
          return Field::FEE;
        break;
      case 'i':
        if (name.compare("fill_id") == 0)
          return Field::FILL_ID;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_l(auto& name) {
  if (name.compare("liquidity") == 0)
    return Field::LIQUIDITY;
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
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_t(auto& name) {
  if (name.compare("trade_id") == 0)
    return Field::TRADE_ID;
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  assert(name.empty() == false);
  switch (name.data()[0]) {
    case 'c':
      return parse_c(name);
    case 'f':
      return parse_f(name);
    case 'l':
      return parse_l(name);
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
static_assert(parse_name("fee") == Field::FEE);
static_assert(parse_name("fill_id") == Field::FILL_ID);
static_assert(parse_name("liquidity") == Field::LIQUIDITY);
static_assert(parse_name("price") == Field::PRICE);
static_assert(parse_name("product_id") == Field::PRODUCT_ID);
static_assert(parse_name("settled") == Field::SETTLED);
static_assert(parse_name("side") == Field::SIDE);
static_assert(parse_name("size") == Field::SIZE);
static_assert(parse_name("trade_id") == Field::TRADE_ID);

inline void update_field(auto& result, auto& field, auto& value) {
  switch (field) {
    case Field::UNKNOWN: {
      break;
    }
    case Field::CREATED_AT: {
      update(result.created_at, value);
      break;
    }
    case Field::FEE: {
      update(result.fee, value);
      break;
    }
    case Field::FILL_ID: {
      update(result.fill_id, value);
      break;
    }
    case Field::LIQUIDITY: {
      update(result.liquidity, value);
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
    case Field::TRADE_ID: {
      update(result.trade_id, value);
      break;
    }
  }
}
}  // namespace

Fill Fill::parse(const std::string_view& message) {
  core::json::Parser parser(message);
  Fill result;
  parse(result, parser.root<core::json::object_t>());
  return result;
}

void Fill::parse(Fill& result, core::json::object_t&& object) {
  new (&result) std::remove_reference<decltype(result)>::type {};
  for (auto [key, value] : object) {
    auto field = parse_name(key);
    update_field(result, field, value);
  }
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
