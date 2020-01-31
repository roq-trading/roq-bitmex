/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/subscribe.h"

#include "roq/bitmex/json/utils.h"

#ifndef NDEBUG
#include "roq/logging.h"
#endif

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  FAILURE,
  SUBSCRIBE,
  SUCCESS,
  REQUEST,
};

constexpr Field parse_f(auto& name) {
  if (name.compare("failure") == 0)
    return Field::FAILURE;
  return Field::UNKNOWN;
}

constexpr Field parse_s(auto& name) {
  if (name.length() >= 3) {
    switch (name.data()[2]) {
      case 'b':
        if (name.compare("subscribe") == 0)
          return Field::SUBSCRIBE;
        break;
      case 'c':
        if (name.compare("success") == 0)
          return Field::SUCCESS;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_r(auto& name) {
  if (name.compare("request") == 0)
    return Field::REQUEST;
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  assert(name.empty() == false);
  switch (name.data()[0]) {
    case 'f':
      return parse_f(name);
    case 's':
      return parse_s(name);
    case 'r':
      return parse_r(name);
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("failure") == Field::FAILURE);
static_assert(parse_name("subscribe") == Field::SUBSCRIBE);
static_assert(parse_name("success") == Field::SUCCESS);
static_assert(parse_name("request") == Field::REQUEST);

inline void update_field(
    auto& result,
    auto& field,
    auto& key,
    auto& value) {
  switch (field) {
    case Field::UNKNOWN: {
#ifndef NDEBUG
      LOG(FATAL)("Unknown key=\"{}\"", key);
#endif
      break;
    }
    case Field::FAILURE: {
      update(result.failure, value);
      break;
    }
    case Field::SUBSCRIBE: {
      update(result.subscribe, value);
      break;
    }
    case Field::SUCCESS: {
      update(result.success, value);
      break;
    }
    case Field::REQUEST: {
      // do nothing
      break;
    }
  }
}
}  // namespace

Subscribe Subscribe::parse(const std::string_view& message) {
  Subscribe result;
  core::json::Parser parser(message);
  auto root = parser.root();
  for (auto [key, value] : std::get<core::json::object_t>(root)) {
    auto field = parse_name(key);
    update_field(result, field, key, value);
  }
  return result;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
