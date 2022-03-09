/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include "roq/bitmex/json/error_response.hpp"

namespace roq {
namespace bitmex {
namespace json {

struct ErrorParser final {
  template <typename F>
  static bool dispatch(const std::string_view &message, F &&callback) {
    using namespace std::literals;
    core::json::Parser parser(message);
    auto root = parser.root();
    for (auto [key, value] : std::get<core::json::object_t>(root)) {
      if (key.compare("error"sv) == 0) {
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
