/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/status.h"

#include <cassert>
#include <utility>

#include "roq/core/json/array.h"
#include "roq/core/json/parser.h"

#include "roq/bitmex/json/currency.h"
#include "roq/bitmex/json/product.h"
#include "roq/bitmex/json/utils.h"


namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  CURRENCIES,
  PRODUCTS,
};

constexpr Field parse_c(auto& name) {
  if (name.compare("currencies") == 0)
    return Field::CURRENCIES;
  return Field::UNKNOWN;
}

constexpr Field parse_p(auto& name) {
  if (name.compare("products") == 0)
    return Field::PRODUCTS;
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  assert(name.empty() == false);
  switch (name.data()[0]) {
    case 'c':
      return parse_c(name);
    case 'p':
      return parse_p(name);
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("currencies") == Field::CURRENCIES);
static_assert(parse_name("products") == Field::PRODUCTS);
}  // namespace

Status Status::parse(
    const std::string_view& message,
    core::json::Buffer& buffer) {
  Status result;
  core::json::Parser parser(message);
  for (auto [key, value] : parser.root<core::json::object_t>()) {
    auto field = parse_name(key);
    switch (field) {
      case Field::UNKNOWN: {
        break;
      }
      case Field::CURRENCIES: {
        core::json::Array currencies(
            buffer,
            result.currencies,
            value);
        for (auto [item, value] : currencies) {
          Currency::parse(
              item,
              std::move(std::get<core::json::object_t>(value)));
        }
        break;
      }
      case Field::PRODUCTS: {
        core::json::Array products(
            buffer,
            result.products,
            value);
        for (auto [item, value] : products) {
          Product::parse(
              item,
              std::move(std::get<core::json::object_t>(value)));
        }
        break;
      }
    }
  }
  return result;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
