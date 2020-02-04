/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/action.h"

#include "roq/logging.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
constexpr auto parse_d(const std::string_view& name) {
  if (name.compare("delete") == 0)
    return Action::DELETE;
  return Action::UNKNOWN;
}

constexpr auto parse_i(const std::string_view& name) {
  if (name.compare("insert") == 0)
    return Action::INSERT;
  return Action::UNKNOWN;
}

constexpr auto parse_p(const std::string_view& name) {
  if (name.compare("partial") == 0)
    return Action::PARTIAL;
  return Action::UNKNOWN;
}

constexpr auto parse_u(const std::string_view& name) {
  if (name.compare("update") == 0)
    return Action::UPDATE;
  return Action::UNKNOWN;
}

constexpr auto parse_helper(const std::string_view& name) {
  if (name.empty())
    return Action::UNDEFINED;
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
  return Action::UNKNOWN;
}

static_assert(parse_helper("") == Action::UNDEFINED);
static_assert(parse_helper("delete") == Action::DELETE);
static_assert(parse_helper("insert") == Action::INSERT);
static_assert(parse_helper("partial") == Action::PARTIAL);
static_assert(parse_helper("update") == Action::UPDATE);
}  // namespace

Action parse_action(const std::string_view& name) {
  auto result = parse_helper(name);
  DLOG_IF(FATAL, result == Action::UNKNOWN)(
      FMT_STRING("Unknown name=\"{}\""),
      name);
  return result;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
