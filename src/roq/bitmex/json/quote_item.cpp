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

constexpr Field parse_askP(const std::string_view& name) {
  if (name.length() == 8 &&
      name[4] == 'r' &&
      name[5] == 'i' &&
      name[6] == 'c' &&
      name[7] == 'e') {
    return Field::ASK_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_askS(const std::string_view& name) {
  if (name.length() == 7 &&
      name[4] == 'i' &&
      name[5] == 'z' &&
      name[6] == 'e') {
    return Field::ASK_SIZE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_a(const std::string_view& name) {
  if (name.length() >= 4 &&
      name[1] == 's' &&
      name[2] == 'k') {
    switch (name[3]) {
      case 'P':
        return parse_askP(name);
      case 'S':
        return parse_askS(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_bidP(const std::string_view& name) {
  if (name.length() == 8 &&
      name[4] == 'r' &&
      name[5] == 'i' &&
      name[6] == 'c' &&
      name[7] == 'e') {
    return Field::BID_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_bidS(const std::string_view& name) {
  if (name.length() == 7 &&
      name[4] == 'i' &&
      name[5] == 'z' &&
      name[6] == 'e') {
    return Field::BID_SIZE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_b(const std::string_view& name) {
  if (name.length() >= 4 &&
      name[1] == 'i' &&
      name[2] == 'd') {
    switch (name[3]) {
      case 'P':
        return parse_bidP(name);
      case 'S':
        return parse_bidS(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_s(const std::string_view& name) {
  if (name.length() == 6 &&
      name[1] == 'y' &&
      name[2] == 'm' &&
      name[3] == 'b' &&
      name[4] == 'o' &&
      name[5] == 'l') {
    return Field::SYMBOL;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_t(const std::string_view& name) {
  if (name.length() == 9 &&
      name[1] == 'i' &&
      name[2] == 'm' &&
      name[3] == 'e' &&
      name[4] == 's' &&
      name[5] == 't' &&
      name[6] == 'a' &&
      name[7] == 'm' &&
      name[8] == 'p') {
    return Field::TIMESTAMP;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  if (name.length() > 0) {
    switch (name[0]) {
      case 'a':
        return parse_a(name);
      case 'b':
        return parse_b(name);
      case 's':
        return parse_s(name);
      case 't':
        return parse_t(name);
    }
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("askPrice") == Field::ASK_PRICE);
static_assert(parse_name("askSize") == Field::ASK_SIZE);
static_assert(parse_name("bidPrice") == Field::BID_PRICE);
static_assert(parse_name("bidSize") == Field::BID_SIZE);
static_assert(parse_name("symbol") == Field::SYMBOL);
static_assert(parse_name("timestamp") == Field::TIMESTAMP);

void update_field(
    auto& result,
    auto& key,
    auto& value) {
  auto field = parse_name(key);
  switch (field) {
    case Field::UNKNOWN:
      DLOG(FATAL)(
          FMT_STRING("Unknown key=\"{}\""),
          key);
      break;
    case Field::ASK_PRICE:
      update(result.ask_price, value);
      break;
    case Field::ASK_SIZE:
      update(result.ask_size, value);
      break;
    case Field::BID_PRICE:
      update(result.bid_price, value);
      break;
    case Field::BID_SIZE:
      update(result.bid_size, value);
      break;
    case Field::SYMBOL:
      update(result.symbol, value);
      break;
    case Field::TIMESTAMP:
      update(result.timestamp, value);
      break;
  }
}
}  // namespace

QuoteItem::QuoteItem(core::json::value_t& value) {
  for (auto [key, value] : std::get<core::json::object_t>(value))
    update_field(*this, key, value);
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
