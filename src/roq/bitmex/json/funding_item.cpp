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

constexpr Field parse_f(auto& name) {
  if (name.length() >= 8) {
    switch (name.data()[7]) {
      case 'I': {
        if (name.compare("fundingInterval") == 0)
          return Field::FUNDING_INTERVAL;
        break;
      }
      case 'R': {
        if (name.length() > 11) {
          if (name.compare("fundingRateDaily") == 0)
            return Field::FUNDING_RATE_DAILY;
        } else {
          if (name.compare("fundingRate") == 0)
            return Field::FUNDING_RATE;
        }
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
    case 'f':
      return parse_f(name);
    case 's':
      return parse_s(name);
    case 't':
      return parse_t(name);
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("fundingInterval") == Field::FUNDING_INTERVAL);
static_assert(parse_name("fundingRate") == Field::FUNDING_RATE);
static_assert(parse_name("fundingRateDaily") == Field::FUNDING_RATE_DAILY);
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
    case Field::FUNDING_INTERVAL: {
      update(result.funding_interval, value);
      break;
    }
    case Field::FUNDING_RATE: {
      update(result.funding_rate, value);
      break;
    }
    case Field::FUNDING_RATE_DAILY: {
      update(result.funding_rate_daily, value);
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

FundingItem::FundingItem(core::json::value_t& value) {
  for (auto [key, value] : std::get<core::json::object_t>(value)) {
    auto field = parse_name(key);
    update_field(*this, field, key, value);
  }
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
