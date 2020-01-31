/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/quote.h"

#include <utility>

#include "roq/core/json/array.h"
#include "roq/core/json/parser.h"

namespace roq {
namespace bitmex {
namespace json {

Quote Quote::parse(
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

Quote Quote::parse(
    core::json::array_t& array,
    core::json::Buffer& buffer,
    Action action) {
  return Quote {
    .action = action,
    .data = core::json::Array<decltype(data)>::parse(buffer, array),
  };
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
