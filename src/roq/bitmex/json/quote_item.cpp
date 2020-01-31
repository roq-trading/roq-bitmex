/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/quote_item.h"

#include "roq/bitmex/json/utils.h"

#include "roq/logging.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  ASK_PRICE,
  ASK_SIZE,
  BID_PRICE,
  BID_SIZE,
  SYMBOL,
  TIMESTAMP,
};

constexpr Field parse_a(auto& name) {
  if (name.length() >= 4) {
    switch (name.data()[3]) {
      case 'P': {
        if (name.compare("askPrice") == 0)
          return Field::ASK_PRICE;
        break;
      }
      case 'S': {
        if (name.compare("askSize") == 0)
          return Field::ASK_SIZE;
        break;
      }
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_b(auto& name) {
  if (name.length() >= 4) {
    switch (name.data()[3]) {
      case 'P': {
        if (name.compare("bidPrice") == 0)
          return Field::BID_PRICE;
        break;
      }
      case 'S': {
        if (name.compare("bidSize") == 0)
          return Field::BID_SIZE;
        break;
      }
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_s(auto& name) {
  if (name.compare("symbol") == 0)
    return Field::SYMBOL;
  return Field::UNKNOWN;
}

constexpr Field parse_t(auto& name) {
  if (name.compare("timestamp") == 0)
    return Field::TIMESTAMP;
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  assert(name.empty() == false);
  switch (name.data()[0]) {
    case 'a':
      return parse_a(name);
    case 'b':
      return parse_b(name);
    case 's':
      return parse_s(name);
    case 't':
      return parse_t(name);
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("askPrice") == Field::ASK_PRICE);
static_assert(parse_name("askSize") == Field::ASK_SIZE);
static_assert(parse_name("bidPrice") == Field::BID_PRICE);
static_assert(parse_name("bidSize") == Field::BID_SIZE);
static_assert(parse_name("symbol") == Field::SYMBOL);
static_assert(parse_name("timestamp") == Field::TIMESTAMP);

inline void update_field(
    auto& result,
    auto& field,
    auto& key,
    auto& value) {
  switch (field) {
    case Field::UNKNOWN: {
      DLOG(FATAL)("Unknown key=\"{}\"", key);
      break;
    }
    case Field::ASK_PRICE: {
      update(result.ask_price, value);
      break;
    }
    case Field::ASK_SIZE: {
      update(result.ask_size, value);
      break;
    }
    case Field::BID_PRICE: {
      update(result.bid_price, value);
      break;
    }
    case Field::BID_SIZE: {
      update(result.bid_size, value);
      break;
    }
    case Field::SYMBOL: {
      update(result.symbol, value);
      break;
    }
    case Field::TIMESTAMP: {
      update(result.timestamp, value);
      break;
    }
  }
}
}  // namespace

QuoteItem::QuoteItem(core::json::value_t& value) {
  for (auto [key, value] : std::get<core::json::object_t>(value)) {
    auto field = parse_name(key);
    update_field(*this, field, key, value);
  }
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
