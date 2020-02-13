/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/action.h"

#include <cassert>

#include "roq/logging.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
constexpr Action parse_d(const std::string_view& name) {
  if (name.length() == 6 &&
      name[1] == 'e' &&
      name[2] == 'l' &&
      name[3] == 'e' &&
      name[4] == 't' &&
      name[5] == 'e') {
    return Action::DELETE;
  }
  return Action::UNKNOWN;
}

constexpr Action parse_i(const std::string_view& name) {
  if (name.length() == 6 &&
      name[1] == 'n' &&
      name[2] == 's' &&
      name[3] == 'e' &&
      name[4] == 'r' &&
      name[5] == 't') {
    return Action::INSERT;
  }
  return Action::UNKNOWN;
}

constexpr Action parse_p(const std::string_view& name) {
  if (name.length() == 7 &&
      name[1] == 'a' &&
      name[2] == 'r' &&
      name[3] == 't' &&
      name[4] == 'i' &&
      name[5] == 'a' &&
      name[6] == 'l') {
    return Action::PARTIAL;
  }
  return Action::UNKNOWN;
}

constexpr Action parse_u(const std::string_view& name) {
  if (name.length() == 6 &&
      name[1] == 'p' &&
      name[2] == 'd' &&
      name[3] == 'a' &&
      name[4] == 't' &&
      name[5] == 'e') {
    return Action::UPDATE;
  }
  return Action::UNKNOWN;
}

constexpr Action parse_name(const std::string_view& name) {
  if (name.length() > 0) {
    switch (name[0]) {
      case 'd':
        return parse_d(name);
      case 'i':
        return parse_i(name);
      case 'p':
        return parse_p(name);
      case 'u':
        return parse_u(name);
    }
  }
  return Action::UNKNOWN;
}

static_assert(parse_name("delete") == Action::DELETE);
static_assert(parse_name("insert") == Action::INSERT);
static_assert(parse_name("partial") == Action::PARTIAL);
static_assert(parse_name("update") == Action::UPDATE);
}  // namespace

Action parse_action(const std::string_view& name) {
  auto result = parse_name(name);
  DLOG_IF(FATAL, result == Action::UNKNOWN)(
      FMT_STRING("Unknown name=\"{}\""),
      name);
  return result;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
