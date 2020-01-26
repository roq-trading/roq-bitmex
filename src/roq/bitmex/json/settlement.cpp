/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/settlement.h"

#include <utility>

#include "roq/core/json/array.h"
#include "roq/core/json/parser.h"


namespace roq {
namespace bitmex {
namespace json {

Settlement Settlement::parse(
    const std::string_view& message,
    core::json::Buffer& buffer,
    Action action) {
  core::json::Parser parser(message);
  auto root = parser.root();
  return parse(
      std::get<core::json::array_t>(root),
      buffer,
      action);
}

Settlement Settlement::parse(
    core::json::array_t& array,
    core::json::Buffer& buffer,
    Action action) {
  Settlement result {
    .action = action,
    .data = {},
  };
  core::json::Array data(
      buffer,
      result.data,
      array);
  for (auto [item, value] : data) {
    item.parse(std::get<core::json::object_t>(value));
  }
  return result;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
