/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include "roq/bitmex/json/error_response.h"

namespace roq {
namespace bitmex {
namespace json {

struct ErrorParser final {
  template <typename F>
  static bool dispatch(const std::string_view &message, F &&callback) {
    core::json::Parser parser(message);
    auto root = parser.root();
    for (auto [key, value] : std::get<core::json::object_t>(root)) {
      if (key.compare("error"_sv) == 0) {
        ErrorResponse error(value);
        callback(error);
        return true;
      } else {
        return false;
      }
    }
    return false;
  }
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq
