/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/product.h"

#include <cassert>

#include "roq/bitmex/json/utils.h"


namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  BASE_CURRENCY,
  BASE_INCREMENT,
  BASE_MAX_SIZE,
  BASE_MIN_SIZE,
  CANCEL_ONLY,
  DISPLAY_NAME,
  ID,
  LIMIT_ONLY,
  MARGIN_ENABLED,
  MAX_MARKET_FUNDS,
  MIN_MARKET_FUNDS,
  POST_ONLY,
  QUOTE_CURRENCY,
  QUOTE_INCREMENT,
  STATUS_MESSAGE,
  STATUS,
};

constexpr Field parse_b(auto& name) {
  if (name.length() >= 7) {
    switch (name.data()[6]) {
      case 'u': {
        if (name.compare("base_currency") == 0)
          return Field::BASE_CURRENCY;
        break;
      }
      case 'n': {
        if (name.compare("base_increment") == 0)
          return Field::BASE_INCREMENT;
        break;
      }
      case 'a': {
        if (name.compare("base_max_size") == 0)
          return Field::BASE_MAX_SIZE;
        break;
      }
      case 'i': {
        if (name.compare("base_min_size") == 0)
          return Field::BASE_MIN_SIZE;
        break;
      }
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_c(auto& name) {
  if (name.compare("cancel_only") == 0)
    return Field::CANCEL_ONLY;
  return Field::UNKNOWN;
}

constexpr Field parse_d(auto& name) {
  if (name.compare("display_name") == 0)
    return Field::DISPLAY_NAME;
  return Field::UNKNOWN;
}

constexpr Field parse_i(auto& name) {
  if (name.compare("id") == 0)
    return Field::ID;
  return Field::UNKNOWN;
}

constexpr Field parse_l(auto& name) {
  if (name.compare("limit_only") == 0)
    return Field::LIMIT_ONLY;
  return Field::UNKNOWN;
}

constexpr Field parse_m(auto& name) {
  if (name.length() >= 3) {
    switch (name.data()[2]) {
      case 'r': {
        if (name.compare("margin_enabled") == 0)
          return Field::MARGIN_ENABLED;
        break;
      }
      case 'x': {
        if (name.compare("max_market_funds") == 0)
          return Field::MAX_MARKET_FUNDS;
        break;
      }
      case 'n': {
        if (name.compare("min_market_funds") == 0)
          return Field::MIN_MARKET_FUNDS;
        break;
      }
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_p(auto& name) {
  if (name.compare("post_only") == 0)
    return Field::POST_ONLY;
  return Field::UNKNOWN;
}

constexpr Field parse_q(auto& name) {
  if (name.length() >= 7) {
    switch (name.data()[6]) {
      case 'c': {
        if (name.compare("quote_currency") == 0)
          return Field::QUOTE_CURRENCY;
        break;
      }
      case 'i': {
        if (name.compare("quote_increment") == 0)
          return Field::QUOTE_INCREMENT;
        break;
      }
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_s(auto& name) {
  if (name.length() > 6) {
    if (name.compare("status_message") == 0)
      return Field::STATUS_MESSAGE;
  } else {
    if (name.compare("status") == 0)
      return Field::STATUS;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  assert(name.empty() == false);
  switch (name.data()[0]) {
    case 'b':
      return parse_b(name);
    case 'c':
      return parse_c(name);
    case 'd':
      return parse_d(name);
    case 'i':
      return parse_i(name);
    case 'l':
      return parse_l(name);
    case 'm':
      return parse_m(name);
    case 'p':
      return parse_p(name);
    case 'q':
      return parse_q(name);
    case 's':
      return parse_s(name);
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("base_currency") == Field::BASE_CURRENCY);
static_assert(parse_name("base_increment") == Field::BASE_INCREMENT);
static_assert(parse_name("base_max_size") == Field::BASE_MAX_SIZE);
static_assert(parse_name("base_min_size") == Field::BASE_MIN_SIZE);
static_assert(parse_name("cancel_only") == Field::CANCEL_ONLY);
static_assert(parse_name("display_name") == Field::DISPLAY_NAME);
static_assert(parse_name("id") == Field::ID);
static_assert(parse_name("limit_only") == Field::LIMIT_ONLY);
static_assert(parse_name("margin_enabled") == Field::MARGIN_ENABLED);
static_assert(parse_name("max_market_funds") == Field::MAX_MARKET_FUNDS);
static_assert(parse_name("min_market_funds") == Field::MIN_MARKET_FUNDS);
static_assert(parse_name("post_only") == Field::POST_ONLY);
static_assert(parse_name("quote_currency") == Field::QUOTE_CURRENCY);
static_assert(parse_name("quote_increment") == Field::QUOTE_INCREMENT);
static_assert(parse_name("status_message") == Field::STATUS_MESSAGE);
static_assert(parse_name("status") == Field::STATUS);

inline void update_field(auto& result, auto& field, auto& value) {
  switch (field) {
    case Field::UNKNOWN: {
      break;
    }
    case Field::BASE_CURRENCY: {
      update(result.base_currency, value);
      break;
    }
    case Field::BASE_INCREMENT: {
      update(result.base_increment, value);
      break;
    }
    case Field::BASE_MAX_SIZE: {
      update(result.base_max_size, value);
      break;
    }
    case Field::BASE_MIN_SIZE: {
      update(result.base_min_size, value);
      break;
    }
    case Field::CANCEL_ONLY: {
      update(result.cancel_only, value);
      break;
    }
    case Field::DISPLAY_NAME: {
      update(result.display_name, value);
      break;
    }
    case Field::ID: {
      update(result.id, value);
      break;
    }
    case Field::LIMIT_ONLY: {
      update(result.limit_only, value);
      break;
    }
    case Field::MARGIN_ENABLED: {
      update(result.margin_enabled, value);
      break;
    }
    case Field::MAX_MARKET_FUNDS: {
      update(result.max_market_funds, value);
      break;
    }
    case Field::MIN_MARKET_FUNDS: {
      update(result.min_market_funds, value);
      break;
    }
    case Field::POST_ONLY: {
      update(result.post_only, value);
      break;
    }
    case Field::QUOTE_CURRENCY: {
      update(result.quote_currency, value);
      break;
    }
    case Field::QUOTE_INCREMENT: {
      update(result.quote_increment, value);
      break;
    }
    case Field::STATUS_MESSAGE: {
      update(result.status_message, value);
      break;
    }
    case Field::STATUS: {
      update(result.status, value);
      break;
    }
  }
}
}  // namespace

Product Product::parse(const std::string_view& message) {
  core::json::Parser parser(message);
  Product result;
  parse(result, parser.root<core::json::object_t>());
  return result;
}

void Product::parse(Product& result, core::json::object_t&& object) {
  new (&result) std::remove_reference<decltype(result)>::type {};
  for (auto [key, value] : object) {
    auto field = parse_name(key);
    update_field(result, field, value);
  }
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
