/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/state.h"

#include <cassert>

#include "roq/logging.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
constexpr auto parse_c(const std::string_view& name) {
  if (name.compare("Closed") == 0)
    return State::CLOSED;
  return State::UNKNOWN;
}
constexpr auto parse_o(const std::string_view& name) {
  if (name.compare("Open") == 0)
    return State::OPEN;
  return State::UNKNOWN;
}
constexpr auto parse_s(const std::string_view& name) {
  if (name.compare("Settled") == 0)
    return State::SETTLED;
  return State::UNKNOWN;
}
constexpr auto parse_u(const std::string_view& name) {
  if (name.compare("Unlisted") == 0)
    return State::UNLISTED;
  return State::UNKNOWN;
}
constexpr auto parse_helper(const std::string_view& name) {
  if (name.empty())
    return State::UNDEFINED;
  switch (name[0]) {
    case 'C':
      return parse_c(name);
    case 'O':
      return parse_o(name);
    case 'S':
      return parse_s(name);
    case 'U':
      return parse_u(name);
  }
  return State::UNKNOWN;
}

static_assert(parse_helper("") == State::UNDEFINED);
static_assert(parse_helper("Closed") == State::CLOSED);
static_assert(parse_helper("Open") == State::OPEN);
static_assert(parse_helper("Settled") == State::SETTLED);
static_assert(parse_helper("Unlisted") == State::UNLISTED);
}  // namespace

State parse_state(const std::string_view& name) {
  auto result = parse_helper(name);
  DLOG_IF(FATAL, result == State::UNKNOWN)(
      "Unknown name=\"{}\"", name);
  return result;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
