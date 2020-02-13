/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/funding_item.h"

#include "roq/bitmex/json/utils.h"

#include "roq/logging.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  FUNDING_INTERVAL,
  FUNDING_RATE,
  FUNDING_RATE_DAILY,
  SYMBOL,
  TIMESTAMP,
};

constexpr Field parse_fundingI(const std::string_view& name) {
  if (name.length() == 15 &&
      name[8] == 'n' &&
      name[9] == 't' &&
      name[10] == 'e' &&
      name[11] == 'r' &&
      name[12] == 'v' &&
      name[13] == 'a' &&
      name[14] == 'l') {
    return Field::FUNDING_INTERVAL;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_fundingRate(const std::string_view& name) {
  if (name.length() == 16 &&
      name[11] == 'D' &&
      name[12] == 'a' &&
      name[13] == 'i' &&
      name[14] == 'l' &&
      name[15] == 'y') {
    return Field::FUNDING_RATE_DAILY;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_fundingR(const std::string_view& name) {
  if (name.length() >= 11 &&
      name[8] == 'a' &&
      name[9] == 't' &&
      name[10] == 'e') {
    if (name.length() == 11)
      return Field::FUNDING_RATE;
    return parse_fundingRate(name);
  }
  return Field::UNKNOWN;
}

constexpr Field parse_f(const std::string_view& name) {
  if (name.length() >= 8 &&
      name[1] == 'u' &&
      name[2] == 'n' &&
      name[3] == 'd' &&
      name[4] == 'i' &&
      name[5] == 'n' &&
      name[6] == 'g') {
    switch (name[7]) {
      case 'I':
        return parse_fundingI(name);
      case 'R':
        return parse_fundingR(name);
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
      case 'f':
        return parse_f(name);
      case 's':
        return parse_s(name);
      case 't':
        return parse_t(name);
    }
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("fundingInterval") == Field::FUNDING_INTERVAL);
static_assert(parse_name("fundingRate") == Field::FUNDING_RATE);
static_assert(parse_name("fundingRateDaily") == Field::FUNDING_RATE_DAILY);
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
    case Field::FUNDING_INTERVAL:
      update(result.funding_interval, value);
      break;
    case Field::FUNDING_RATE:
      update(result.funding_rate, value);
      break;
    case Field::FUNDING_RATE_DAILY:
      update(result.funding_rate_daily, value);
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

FundingItem::FundingItem(core::json::value_t& value) {
  for (auto [key, value] : std::get<core::json::object_t>(value))
    update_field(*this, key, value);
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
