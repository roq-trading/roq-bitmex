/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/products.h"

#include <utility>

#include "roq/core/json/array.h"
#include "roq/core/json/parser.h"


namespace roq {
namespace bitmex {
namespace json {

Products Products::parse(
    const std::string_view& message,
    core::json::Buffer& buffer) {
  Products result;
  core::json::Parser parser(message);
  core::json::Array products(
      buffer,
      result,
      parser.root<core::json::array_t>());
  for (auto [item, value] : products) {
    Product::parse(
        item,
        std::move(std::get<core::json::object_t>(value)));
  }
  return result;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
