/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/account.h"

#include "roq/bitmex/json/utils.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  AVAILABLE,
  BALANCE,
  CURRENCY,
  HOLD,
  ID,
  PROFILE_ID,
};

constexpr Field parse_a(auto& name) {
  if (name.compare("available") == 0)
    return Field::AVAILABLE;
  return Field::UNKNOWN;
}

constexpr Field parse_b(auto& name) {
  if (name.compare("balance") == 0)
    return Field::BALANCE;
  return Field::UNKNOWN;
}

constexpr Field parse_c(auto& name) {
  if (name.compare("currency") == 0)
    return Field::CURRENCY;
  return Field::UNKNOWN;
}

constexpr Field parse_h(auto& name) {
  if (name.compare("hold") == 0)
    return Field::HOLD;
  return Field::UNKNOWN;
}

constexpr Field parse_i(auto& name) {
  if (name.compare("id") == 0)
    return Field::ID;
  return Field::UNKNOWN;
}

constexpr Field parse_p(auto& name) {
  if (name.compare("profile_id") == 0)
    return Field::PROFILE_ID;
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  assert(name.empty() == false);
  switch (name.data()[0]) {
    case 'a':
      return parse_a(name);
    case 'b':
      return parse_b(name);
    case 'c':
      return parse_c(name);
    case 'h':
      return parse_h(name);
    case 'i':
      return parse_i(name);
    case 'p':
      return parse_p(name);
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("available") == Field::AVAILABLE);
static_assert(parse_name("balance") == Field::BALANCE);
static_assert(parse_name("currency") == Field::CURRENCY);
static_assert(parse_name("hold") == Field::HOLD);
static_assert(parse_name("id") == Field::ID);
static_assert(parse_name("profile_id") == Field::PROFILE_ID);

inline void update_field(auto& result, auto& field, auto& value) {
  switch (field) {
    case Field::UNKNOWN: {
      break;
    }
    case Field::AVAILABLE: {
      update(result.available, value);
      break;
    }
    case Field::BALANCE: {
      update(result.balance, value);
      break;
    }
    case Field::CURRENCY: {
      update(result.currency, value);
      break;
    }
    case Field::HOLD: {
      update(result.hold, value);
      break;
    }
    case Field::ID: {
      update(result.id, value);
      break;
    }
    case Field::PROFILE_ID: {
      update(result.profile_id, value);
      break;
    }
  }
}
}  // namespace

Account Account::parse(const std::string_view& message) {
  Account result;
  core::json::Parser parser(message);
  parse(result, parser.root<core::json::object_t>());
  return result;
}

void Account::parse(Account& result, core::json::object_t&& object) {
  for (auto [key, value] : object) {
    auto field = parse_name(key);
    update_field(result, field, value);
  }
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
