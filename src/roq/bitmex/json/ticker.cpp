/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/ticker.h"

#include "roq/bitmex/json/utils.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  BEST_ASK,
  BEST_BID,
  HIGH_24H,
  LAST_SIZE,
  LOW_24H,
  OPEN_24H,
  PRICE,
  PRODUCT_ID,
  SEQUENCE,
  SIDE,
  TIME,
  TRADE_ID,
  VOLUME_24H,
  VOLUME_30D,
};

constexpr Field parse_b(auto& name) {
  if (name.length() >=6) {
    switch (name.data()[5]) {
      case 'a':
        if (name.compare("best_ask") == 0)
          return Field::BEST_ASK;
        break;
      case 'b':
        if (name.compare("best_bid") == 0)
          return Field::BEST_BID;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_h(auto& name) {
  if (name.compare("high_24h") == 0)
    return Field::HIGH_24H;
  return Field::UNKNOWN;
}

constexpr Field parse_l(auto& name) {
  if (name.length() >= 2) {
    switch (name.data()[1]) {
      case 'a':
        if (name.compare("last_size") == 0)
          return Field::LAST_SIZE;
        break;
      case 'o':
        if (name.compare("low_24h") == 0)
          return Field::LOW_24H;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_o(auto& name) {
  if (name.compare("open_24h") == 0)
    return Field::OPEN_24H;
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
  if (name.length() >= 2) {
    switch (name.data()[1]) {
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

constexpr Field parse_v(auto& name) {
  if (name.length() >= 8) {
    switch (name.data()[7]) {
      case '2':
        if (name.compare("volume_24h") == 0)
          return Field::VOLUME_24H;
        break;
      case '3':
        if (name.compare("volume_30d") == 0)
          return Field::VOLUME_30D;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  assert(name.empty() == false);
  switch (name.data()[0]) {
    case 'b':
      return parse_b(name);
    case 'h':
      return parse_h(name);
    case 'l':
      return parse_l(name);
    case 'o':
      return parse_o(name);
    case 'p':
      return parse_p(name);
    case 's':
      return parse_s(name);
    case 't':
      return parse_t(name);
    case 'v':
      return parse_v(name);
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("best_ask") == Field::BEST_ASK);
static_assert(parse_name("best_bid") == Field::BEST_BID);
static_assert(parse_name("high_24h") == Field::HIGH_24H);
static_assert(parse_name("last_size") == Field::LAST_SIZE);
static_assert(parse_name("low_24h") == Field::LOW_24H);
static_assert(parse_name("open_24h") == Field::OPEN_24H);
static_assert(parse_name("price") == Field::PRICE);
static_assert(parse_name("product_id") == Field::PRODUCT_ID);
static_assert(parse_name("sequence") == Field::SEQUENCE);
static_assert(parse_name("side") == Field::SIDE);
static_assert(parse_name("time") == Field::TIME);
static_assert(parse_name("trade_id") == Field::TRADE_ID);
static_assert(parse_name("volume_24h") == Field::VOLUME_24H);
static_assert(parse_name("volume_30d") == Field::VOLUME_30D);

inline void update_field(auto& result, auto& field, auto& value) {
  switch (field) {
    case Field::UNKNOWN: {
      break;
    }
    case Field::BEST_ASK: {
      update(result.best_ask, value);
      break;
    }
    case Field::BEST_BID: {
      update(result.best_bid, value);
      break;
    }
    case Field::HIGH_24H: {
      update(result.high_24h, value);
      break;
    }
    case Field::LAST_SIZE: {
      update(result.last_size, value);
      break;
    }
    case Field::LOW_24H: {
      update(result.low_24h, value);
      break;
    }
    case Field::OPEN_24H: {
      update(result.open_24h, value);
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
    case Field::TIME: {
      update(result.time, value);
      break;
    }
    case Field::TRADE_ID: {
      update(result.trade_id, value);
      break;
    }
    case Field::VOLUME_24H: {
      update(result.volume_24h, value);
      break;
    }
    case Field::VOLUME_30D: {
      update(result.volume_30d, value);
      break;
    }
  }
}
}  // namespace

Ticker Ticker::parse(const std::string_view& message) {
  Ticker result;
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
