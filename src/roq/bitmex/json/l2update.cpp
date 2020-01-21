/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/l2update.h"

#include <cassert>

#include "roq/core/json/array.h"
#include "roq/core/json/parser.h"

#include "roq/bitmex/json/utils.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  CHANGES,
  PRODUCT_ID,
  TIME,
};

constexpr Field parse_c(auto& name) {
  if (name.compare("changes") == 0)
    return Field::CHANGES;
  return Field::UNKNOWN;
}

constexpr Field parse_p(auto& name) {
  if (name.compare("product_id") == 0)
    return Field::PRODUCT_ID;
  return Field::UNKNOWN;
}

constexpr Field parse_t(auto& name) {
  if (name.compare("time") == 0)
    return Field::TIME;
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  assert(name.empty() == false);
  switch (name.data()[0]) {
    case 'c':
      return parse_c(name);
    case 'p':
      return parse_p(name);
    case 't':
      return parse_t(name);
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("changes") == Field::CHANGES);
static_assert(parse_name("product_id") == Field::PRODUCT_ID);
static_assert(parse_name("time") == Field::TIME);
}  // namespace

L2Update L2Update::parse(
    const std::string_view& message,
    core::json::Buffer& buffer) {
  L2Update result;
  core::json::Parser parser(message);
  for (auto [key, value] : parser.root<core::json::object_t>()) {
    auto field = parse_name(key);
    switch (field) {
      case Field::UNKNOWN: {
        break;
      }
      case Field::CHANGES: {
        core::json::Array changes(
            buffer,
            result.changes,
            value);
        for (auto [item, value] : changes) {
          // TODO(thraneh): initialize change?
          int offset = 0;
          for (auto iter_2 : std::get<core::json::array_t>(value)) {
            switch (++offset) {
              case 1: {
                update(item.side, iter_2);
                break;
              }
              case 2: {
                update(item.price, iter_2);
                break;
              }
              case 3: {
                update(item.size, iter_2);
                break;
              }
            }
          }
        }
        break;
      }
      case Field::PRODUCT_ID: {
        update(result.product_id, value);
        break;
      }
      case Field::TIME: {
        update(result.time, value);
        break;
      }
    }
  }
  return result;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
