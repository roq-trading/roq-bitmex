/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/error.h"

#include "roq/bitmex/json/utils.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  MESSAGE,
};

constexpr Field parse_m(auto& name) {
  if (name.compare("message") == 0)
    return Field::MESSAGE;
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  assert(name.empty() == false);
  switch (name.data()[0]) {
    case 'm':
      return parse_m(name);
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("message") == Field::MESSAGE);

inline void update_field(auto& result, auto& field, auto& value) {
  switch (field) {
    case Field::UNKNOWN: {
      break;
    }
    case Field::MESSAGE: {
      update(result.message, value);
      break;
    }
  }
}
}  // namespace

Error Error::parse(const std::string_view& message) {
  Error result;
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
