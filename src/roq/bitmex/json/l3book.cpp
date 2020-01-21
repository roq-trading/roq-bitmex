/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/l3book.h"

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
  ASKS,
  BIDS,
  SEQUENCE,
};

constexpr Field parse_a(auto& name) {
  if (name.compare("asks") == 0)
    return Field::ASKS;
  return Field::UNKNOWN;
}

constexpr Field parse_b(auto& name) {
  if (name.compare("bids") == 0)
    return Field::BIDS;
  return Field::UNKNOWN;
}

constexpr Field parse_s(auto& name) {
  if (name.compare("sequence") == 0)
    return Field::SEQUENCE;
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  assert(name.empty() == false);
  switch (name.data()[0]) {
    case 'a':
      return parse_a(name);
    case 'b':
      return parse_b(name);
    case 's':
      return parse_s(name);
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("asks") == Field::ASKS);
static_assert(parse_name("bids") == Field::BIDS);
static_assert(parse_name("sequence") == Field::SEQUENCE);
}  // namespace

L3Book L3Book::parse(
    const std::string_view& message,
    core::json::Buffer& buffer) {
  L3Book result;
  core::json::Parser parser(message);
  for (auto [key, value] : parser.root<core::json::object_t>()) {
    auto field = parse_name(key);
    switch (field) {
      case Field::UNKNOWN: {
        break;
      }
      case Field::ASKS: {
        core::json::Array asks(
            buffer,
            result.asks,
            value);
        for (auto [item, value] : asks) {
          // TODO(thraneh): initialize ask?
          int offset = 0;
          for (auto iter_2 : std::get<core::json::array_t>(value)) {
            switch (++offset) {
              case 1: {
                update(item.price, iter_2);
                break;
              }
              case 2: {
                update(item.size, iter_2);
                break;
              }
              case 3: {
                update(item.order_id, iter_2);
                break;
              }
            }
          }
        }
        break;
      }
      case Field::BIDS: {
        core::json::Array bids(
            buffer,
            result.bids,
            value);
        for (auto [item, value] : bids) {
          // TODO(thraneh): initialize bid?
          int offset = 0;
          for (auto iter_2 : std::get<core::json::array_t>(value)) {
            switch (++offset) {
              case 1: {
                update(item.price, iter_2);
                break;
              }
              case 2: {
                update(item.size, iter_2);
                break;
              }
              case 3: {
                update(item.order_id, iter_2);
                break;
              }
            }
          }
        }
        break;
      }
      case Field::SEQUENCE: {
        update(result.sequence, value);
        break;
      }
    }
  }
  return result;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
