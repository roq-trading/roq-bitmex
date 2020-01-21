/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/time_.h"

#include "roq/bitmex/json/utils.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  EPOCH,
  ISO,
};

constexpr Field parse_e(auto& name) {
  if (name.compare("epoch") == 0)
    return Field::EPOCH;
  return Field::UNKNOWN;
}

constexpr Field parse_i(auto& name) {
  if (name.compare("iso") == 0)
    return Field::ISO;
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  assert(name.empty() == false);
  switch (name.data()[0]) {
    case 'e':
      return parse_e(name);
    case 'i':
      return parse_i(name);
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("epoch") == Field::EPOCH);
static_assert(parse_name("iso") == Field::ISO);

inline void update_field(auto& result, auto& field, auto& value) {
  switch (field) {
    case Field::UNKNOWN: {
      break;
    }
    case Field::EPOCH: {
      update(result.epoch, value);
      break;
    }
    case Field::ISO: {
      update(result.iso, value);
      break;
    }
  }
}
}  // namespace

Time Time::parse(const std::string_view& message) {
  Time result;
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
