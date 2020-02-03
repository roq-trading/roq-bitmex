/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/error.h"

#include "roq/bitmex/json/utils.h"

#include "roq/logging.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  ERROR,
  META,
  REQUEST,
  STATUS,
};

constexpr Field parse_e(auto& name) {
  if (name.compare("error") == 0)
    return Field::ERROR;
  return Field::UNKNOWN;
}

constexpr Field parse_m(auto& name) {
  if (name.compare("meta") == 0)
    return Field::META;
  return Field::UNKNOWN;
}

constexpr Field parse_r(auto& name) {
  if (name.compare("request") == 0)
    return Field::REQUEST;
  return Field::UNKNOWN;
}

constexpr Field parse_s(auto& name) {
  if (name.compare("status") == 0)
    return Field::STATUS;
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  if (name.empty())
    return Field::UNKNOWN;
  switch (name[0]) {
    case 'e':
      return parse_e(name);
    case 'm':
      return parse_m(name);
    case 'r':
      return parse_r(name);
    case 's':
      return parse_s(name);
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("") == Field::UNKNOWN);
static_assert(parse_name("error") == Field::ERROR);
static_assert(parse_name("meta") == Field::META);
static_assert(parse_name("request") == Field::REQUEST);
static_assert(parse_name("status") == Field::STATUS);

inline void update_field(
    auto& result,
    auto& key,
    auto& value) {
  auto field = parse_name(key);
  switch (field) {
    case Field::UNKNOWN:
      DLOG(FATAL)("Unknown key=\"{}\"", key);
      break;
    case Field::ERROR:
      update(result.error, value);
      break;
    case Field::META:
      // do nothing
      break;
    case Field::STATUS:
      update(result.status, value);
      break;
    case Field::REQUEST:
      // do nothing
      break;
  }
}
}  // namespace

Error Error::parse(core::json::value_t& value) {
  Error result;
  for (auto [key, value] : std::get<core::json::object_t>(value))
    update_field(result, key, value);
  return result;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
