/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/quote.h"

#include <utility>

#include "roq/core/json/array.h"

namespace roq {
namespace bitmex {
namespace json {

Quote::Quote(
    core::json::value_t& value,
    core::json::Buffer& buffer,
    Action action)
    : action(action),
      data(core::json::Array<decltype(data)>::parse(buffer, value)) {
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
