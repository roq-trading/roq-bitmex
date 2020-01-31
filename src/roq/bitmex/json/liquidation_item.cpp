/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/liquidation_item.h"

#include "roq/bitmex/json/utils.h"

#ifndef NDEBUG
#include "roq/logging.h"
#endif

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  LEAVES_QTY,
  ORDER_ID,
  PRICE,
  SIDE,
  SYMBOL,
};

constexpr Field parse_l(auto& name) {
  if (name.compare("leavesQty") == 0)
    return Field::LEAVES_QTY;
  return Field::UNKNOWN;
}

constexpr Field parse_o(auto& name) {
  if (name.compare("orderID") == 0)
    return Field::ORDER_ID;
  return Field::UNKNOWN;
}

constexpr Field parse_p(auto& name) {
  if (name.compare("price") == 0)
    return Field::PRICE;
  return Field::UNKNOWN;
}

constexpr Field parse_s(auto& name) {
  if (name.length() >= 2) {
    switch (name.data()[1]) {
      case 'i': {
        if (name.compare("side") == 0)
          return Field::SIDE;
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

constexpr Field parse_name(const std::string_view& name) {
  assert(name.empty() == false);
  switch (name.data()[0]) {
    case 'l':
      return parse_l(name);
    case 'o':
      return parse_o(name);
    case 'p':
      return parse_p(name);
    case 's':
      return parse_s(name);
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("leavesQty") == Field::LEAVES_QTY);
static_assert(parse_name("orderID") == Field::ORDER_ID);
static_assert(parse_name("price") == Field::PRICE);
static_assert(parse_name("side") == Field::SIDE);
static_assert(parse_name("symbol") == Field::SYMBOL);

inline void update_field(
    auto& result,
    auto& field,
    auto& key,
    auto& value) {
  switch (field) {
    case Field::UNKNOWN: {
#ifndef NDEBUG
      LOG(FATAL)("Unknown key=\"{}\"", key);
#endif
      break;
    }
    case Field::LEAVES_QTY: {
      update(result.leaves_qty, value);
      break;
    }
    case Field::ORDER_ID: {
      update(result.order_id, value);
      break;
    }
    case Field::PRICE: {
      update(result.price, value);
      break;
    }
    case Field::SIDE: {
      update(result.side, value);
      break;
    }
    case Field::SYMBOL: {
      update(result.symbol, value);
      break;
    }
  }
}
}  // namespace

LiquidationItem::LiquidationItem(core::json::value_t& value) {
  for (auto [key, value] : std::get<core::json::object_t>(value)) {
    auto field = parse_name(key);
    update_field(*this, field, key, value);
  }
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
