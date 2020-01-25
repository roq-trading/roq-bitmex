/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/action.h"

#include <cassert>

namespace roq {
namespace bitmex {
namespace json {

namespace {
constexpr auto parse_helper(const std::string_view& name) {
  assert(name.empty() == false);
  switch (name.data()[0]) {
    case 'd':
      if (name.compare("delete") == 0)
        return Action::DELETE;
      break;
    case 'i':
      if (name.compare("insert") == 0)
        return Action::INSERT;
      break;
    case 'p':
      if (name.compare("partial") == 0)
        return Action::PARTIAL;
      break;
    case 'u':
      if (name.compare("update") == 0)
        return Action::UPDATE;
      break;
  }
  return Action::UNKNOWN;
}

static_assert(parse_helper("delete") == Action::DELETE);
static_assert(parse_helper("insert") == Action::INSERT);
static_assert(parse_helper("partial") == Action::PARTIAL);
static_assert(parse_helper("update") == Action::UPDATE);
}  // namespace

Action parse_action(const std::string_view& name) {
  return parse_helper(name);
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
