/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/bitmex/gateway/settings.hpp"

namespace roq {
namespace bitmex {
namespace gateway {

struct API final {
  struct {
    std::string_view exchange_info;
    std::string_view depth;
  } market_data;
  struct {
    std::string_view order;
    std::string_view order_all;
  } order_management;
  struct {
    std::string_view realtime;
  } ws;

  // factory
  static API create(Settings const &);
};

}  // namespace gateway
}  // namespace bitmex
}  // namespace roq
