/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include "roq/bitmex/json/error_response.hpp"

namespace roq {
namespace bitmex {
namespace json {

struct ErrorParser final {
  template <typename F>
  static bool dispatch(std::string_view const &message, F &&callback) {
    using namespace std::literals;
    core::json::Parser parser{message};
    auto root = parser.root();
    for (auto [key, value] : std::get<core::json::Object>(root)) {
      if (key.compare("error"sv) == 0) {
        ErrorResponse error_response{value};
        callback(error_response);
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
