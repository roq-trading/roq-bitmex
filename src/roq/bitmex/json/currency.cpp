/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/currency.h"

#include <cassert>

#include "roq/bitmex/json/utils.h"


namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  CONVERTIBLE_TO,
  CRYPTO_ADDRESS_LINK,
  CRYPTO_TRANSACTION_LINK,
  DETAILS,
  DISPLAY_NAME,
  FUNDING_ACCOUNT_ID,
  GROUP_TYPES,
  ID,
  MAX_PRECISION,
  MIN_SIZE,
  NAME,
  NETWORK_CONFIRMATIONS,
  PUSH_PAYMENT_METHODS,
  SORT_ORDER,
  STATUS,
  STATUS_MESSAGE,
  SYMBOL,
  TYPE,
};

constexpr Field parse_c(auto& name) {
  if (name.length() >= 8) {
    switch (name.data()[7]) {
      case 'i': {
        if (name.compare("convertible_to") == 0)
          return Field::CONVERTIBLE_TO;
        break;
      }
      case 'a': {
        if (name.compare("crypto_address_link") == 0)
          return Field::CRYPTO_ADDRESS_LINK;
        break;
      }
      case 't': {
        if (name.compare("crypto_transaction_link") == 0)
          return Field::CRYPTO_TRANSACTION_LINK;
        break;
      }
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_d(auto& name) {
  if (name.length() >= 2) {
    switch (name.data()[1]) {
      case 'e': {
        if (name.compare("details") == 0)
          return Field::DETAILS;
        break;
      }
      case 'i': {
        if (name.compare("display_name") == 0)
          return Field::DISPLAY_NAME;
        break;
      }
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_f(auto& name) {
  if (name.compare("funding_account_id") == 0)
    return Field::FUNDING_ACCOUNT_ID;
  return Field::UNKNOWN;
}

constexpr Field parse_g(auto& name) {
  if (name.compare("group_types") == 0)
    return Field::GROUP_TYPES;
  return Field::UNKNOWN;
}

constexpr Field parse_i(auto& name) {
  if (name.compare("id") == 0)
    return Field::ID;
  return Field::UNKNOWN;
}

constexpr Field parse_m(auto& name) {
  if (name.length() >= 2) {
    switch (name.data()[1]) {
      case 'a': {
        if (name.compare("max_precision") == 0)
          return Field::MAX_PRECISION;
        break;
      }
      case 'i': {
        if (name.compare("min_size") == 0)
          return Field::MIN_SIZE;
        break;
      }
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_n(auto& name) {
  if (name.length() >= 2) {
    switch (name.data()[1]) {
      case 'a': {
        if (name.compare("name") == 0)
          return Field::NAME;
        break;
      }
      case 'e': {
        if (name.compare("network_confirmations") == 0)
          return Field::NETWORK_CONFIRMATIONS;
        break;
      }
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_p(auto& name) {
  if (name.compare("push_payment_methods") == 0)
    return Field::PUSH_PAYMENT_METHODS;
  return Field::UNKNOWN;
}

constexpr Field parse_s(auto& name) {
  if (name.length() >= 2) {
    switch (name.data()[1]) {
      case 'o': {
        if (name.compare("sort_order") == 0)
          return Field::SORT_ORDER;
        break;
      }
      case 't': {
        if (name.length() > 6) {
          if (name.compare("status_message") == 0)
            return Field::STATUS_MESSAGE;
        } else {
          if (name.compare("status") == 0)
            return Field::STATUS;
        }
        break;
      }
      case 'y': {
        if (name.compare("symbol") == 0)
          return Field::SYMBOL;
        break;
      }
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_t(auto& name) {
  if (name.compare("type") == 0)
    return Field::TYPE;
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  assert(name.empty() == false);
  switch (name.data()[0]) {
    case 'c':
      return parse_c(name);
    case 'd':
      return parse_d(name);
    case 'f':
      return parse_f(name);
    case 'g':
      return parse_g(name);
    case 'i':
      return parse_i(name);
    case 'm':
      return parse_m(name);
    case 'n':
      return parse_n(name);
    case 'p':
      return parse_p(name);
    case 's':
      return parse_s(name);
    case 't':
      return parse_t(name);
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("convertible_to") == Field::CONVERTIBLE_TO);
static_assert(parse_name("crypto_address_link") == Field::CRYPTO_ADDRESS_LINK);
static_assert(parse_name("crypto_transaction_link") == Field::CRYPTO_TRANSACTION_LINK);
static_assert(parse_name("details") == Field::DETAILS);
static_assert(parse_name("display_name") == Field::DISPLAY_NAME);
static_assert(parse_name("funding_account_id") == Field::FUNDING_ACCOUNT_ID);
static_assert(parse_name("group_types") == Field::GROUP_TYPES);
static_assert(parse_name("id") == Field::ID);
static_assert(parse_name("max_precision") == Field::MAX_PRECISION);
static_assert(parse_name("min_size") == Field::MIN_SIZE);
static_assert(parse_name("name") == Field::NAME);
static_assert(parse_name("network_confirmations") == Field::NETWORK_CONFIRMATIONS);
static_assert(parse_name("push_payment_methods") == Field::PUSH_PAYMENT_METHODS);
static_assert(parse_name("sort_order") == Field::SORT_ORDER);
static_assert(parse_name("status") == Field::STATUS);
static_assert(parse_name("status_message") == Field::STATUS_MESSAGE);
static_assert(parse_name("symbol") == Field::SYMBOL);
static_assert(parse_name("type") == Field::TYPE);

inline void update_field(auto& result, auto& field, auto& value) {
  switch (field) {
    case Field::UNKNOWN: {
      break;
    }
    case Field::CONVERTIBLE_TO: {
      // not parsed
      break;
    }
    case Field::CRYPTO_ADDRESS_LINK: {
      // not parsed
      break;
    }
    case Field::CRYPTO_TRANSACTION_LINK: {
      // not parsed
      break;
    }
    case Field::DETAILS: {
      // not parsed
      break;
    }
    case Field::DISPLAY_NAME: {
      update(result.display_name, value);
      break;
    }
    case Field::FUNDING_ACCOUNT_ID: {
      update(result.funding_account_id, value);
      break;
    }
    case Field::GROUP_TYPES: {
      // not parsed
      break;
    }
    case Field::ID: {
      update(result.id, value);
      break;
    }
    case Field::MAX_PRECISION: {
      update(result.max_precision, value);
      break;
    }
    case Field::MIN_SIZE: {
      update(result.min_size, value);
      break;
    }
    case Field::NAME: {
      update(result.name, value);
      break;
    }
    case Field::NETWORK_CONFIRMATIONS: {
      update(result.network_confirmations, value);
      break;
    }
    case Field::PUSH_PAYMENT_METHODS: {
      // not parsed
      break;
    }
    case Field::SORT_ORDER: {
      update(result.sort_order, value);
      break;
    }
    case Field::STATUS: {
      update(result.status, value);
      break;
    }
    case Field::STATUS_MESSAGE: {
      update(result.status_message, value);
      break;
    }
    case Field::SYMBOL: {
      update(result.symbol, value);
      break;
    }
    case Field::TYPE: {
      update(result.type, value);
      break;
    }
  }
}
}  // namespace

Currency Currency::parse(
    const std::string_view& message) {
  core::json::Parser parser(message);
  Currency result;
  parse(result, parser.root<core::json::object_t>());
  return result;
}

void Currency::parse(Currency& result, core::json::object_t&& object) {
  new (&result) std::remove_reference<decltype(result)>::type {};
  for (auto [key, value] : object) {
    auto field = parse_name(key);
    update_field(result, field, value);
  }
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
