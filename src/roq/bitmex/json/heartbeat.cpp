/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/heartbeat.h"

#include "roq/bitmex/json/utils.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  LAST_TRADE_ID,
  PRODUCT_ID,
  SEQUENCE,
  TIME,
};

constexpr Field parse_l(auto& name) {
  if (name.compare("last_trade_id") == 0)
    return Field::LAST_TRADE_ID;
  return Field::UNKNOWN;
}

constexpr Field parse_p(auto& name) {
  if (name.compare("product_id") == 0)
    return Field::PRODUCT_ID;
  return Field::UNKNOWN;
}

constexpr Field parse_s(auto& name) {
  if (name.compare("sequence") == 0)
    return Field::SEQUENCE;
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

static_assert(parse_name("last_trade_id") == Field::LAST_TRADE_ID);
static_assert(parse_name("product_id") == Field::PRODUCT_ID);
static_assert(parse_name("sequence") == Field::SEQUENCE);
static_assert(parse_name("time") == Field::TIME);

inline void update_field(auto& result, auto& field, auto& value) {
  switch (field) {
    case Field::UNKNOWN: {
      break;
    }
    case Field::LAST_TRADE_ID: {
      update(result.last_trade_id, value);
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
    case Field::TIME: {
      update(result.time, value);
      break;
    }
  }
}
}  // namespace

Heartbeat Heartbeat::parse(const std::string_view& message) {
  Heartbeat result;
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
