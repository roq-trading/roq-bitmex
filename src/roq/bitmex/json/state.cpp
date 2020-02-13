/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/state.h"

#include <cassert>

#include "roq/logging.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
constexpr State parse_C(const std::string_view& name) {
  if (name.length() == 6 &&
      name[1] == 'l' &&
      name[2] == 'o' &&
      name[3] == 's' &&
      name[4] == 'e' &&
      name[5] == 'd') {
    return State::CLOSED;
  }
  return State::UNKNOWN;
}

constexpr State parse_O(const std::string_view& name) {
  if (name.length() == 4 &&
      name[1] == 'p' &&
      name[2] == 'e' &&
      name[3] == 'n') {
    return State::OPEN;
  }
  return State::UNKNOWN;
}

constexpr State parse_S(const std::string_view& name) {
  if (name.length() == 7 &&
      name[1] == 'e' &&
      name[2] == 't' &&
      name[3] == 't' &&
      name[4] == 'l' &&
      name[5] == 'e' &&
      name[6] == 'd') {
    return State::SETTLED;
  }
  return State::UNKNOWN;
}

constexpr State parse_U(const std::string_view& name) {
  if (name.length() == 8 &&
      name[1] == 'n' &&
      name[2] == 'l' &&
      name[3] == 'i' &&
      name[4] == 's' &&
      name[5] == 't' &&
      name[6] == 'e' &&
      name[7] == 'd') {
    return State::UNLISTED;
  }
  return State::UNKNOWN;
}

constexpr State parse_name(const std::string_view& name) {
  if (name.length() > 0) {
    switch (name[0]) {
      case 'C':
        return parse_C(name);
      case 'O':
        return parse_O(name);
      case 'S':
        return parse_S(name);
      case 'U':
        return parse_U(name);
    }
  }
  return State::UNKNOWN;
}

static_assert(parse_name("Closed") == State::CLOSED);
static_assert(parse_name("Open") == State::OPEN);
static_assert(parse_name("Settled") == State::SETTLED);
static_assert(parse_name("Unlisted") == State::UNLISTED);
}  // namespace

State parse_state(const std::string_view& name) {
  auto result = parse_name(name);
  DLOG_IF(FATAL, result == State::UNKNOWN)(
      FMT_STRING("Unknown name=\"{}\""),
      name);
  return result;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
