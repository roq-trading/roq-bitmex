/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/instruments.h"

#include <utility>

#include "roq/core/json/array.h"
#include "roq/core/json/parser.h"


namespace roq {
namespace bitmex {
namespace json {

Instruments Instruments::parse(
    const std::string_view& message,
    core::json::Buffer& buffer) {
  core::json::Parser parser(message);
  return parse(
      parser.root<core::json::array_t>(),
      buffer);
}

Instruments Instruments::parse(
    core::json::array_t&& array,
    core::json::Buffer& buffer) {
  Instruments result;
  core::json::Array products(
      buffer,
      result,
      std::move(array));
  for (auto [item, value] : products) {
    Instrument::parse(
        item,
        std::move(std::get<core::json::object_t>(value)));
  }
  return result;
}

Instruments Instruments::parse(
    core::json::array_t& array,
    core::json::Buffer& buffer) {
  Instruments result;
  core::json::Array products(
      buffer,
      result,
      std::move(array));
  for (auto [item, value] : products) {
    Instrument::parse(
        item,
        std::move(std::get<core::json::object_t>(value)));
  }
  return result;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
