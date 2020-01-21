/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <string_view>

namespace roq {
namespace bitmex {
namespace json {

class Parser final {
 public:
  static std::string_view find_type(const std::string_view& message);

  enum class Type {
    UNKNOWN,
    ERROR,
    HEARTBEAT,
    SUBSCRIPTIONS,
    STATUS,
    RECEIVED,
    OPEN,
    MATCH,
    DONE,
    CHANGE,
    ACTIVATE,
    TICKER,
    SNAPSHOT,
    L2UPDATE,
    LAST_MATCH,
  };
  static Type parse_type(const std::string_view& type);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq
