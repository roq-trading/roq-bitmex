/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/state.h"

#include <cassert>

#ifndef NDEBUG
#include "roq/logging.h"
#endif

namespace roq {
namespace bitmex {
namespace json {

namespace {

constexpr auto parse_helper(const std::string_view& name) {
  assert(name.empty() == false);
  switch (name.data()[0]) {
    case 'C':
      if (name.compare("Closed") == 0)
        return State::CLOSED;
      break;
    case 'O':
      if (name.compare("Open") == 0)
        return State::OPEN;
      break;
    case 'U':
      if (name.compare("Unlisted") == 0)
        return State::UNLISTED;
      break;
  }
  return State::UNKNOWN;
}

static_assert(parse_helper("Closed") == State::CLOSED);
static_assert(parse_helper("Open") == State::OPEN);
static_assert(parse_helper("Unlisted") == State::UNLISTED);
}  // namespace

State parse_state(const std::string_view& name) {
  auto result = parse_helper(name);
#ifndef NDEBUG
  LOG_IF(FATAL, result == State::UNKNOWN)(
      "Unknown name=\"{}\"", name);
#endif
  return result;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
