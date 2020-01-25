/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/core/json/buffer.h"

#include "roq/bitmex/json/instrument.h"
#include "roq/bitmex/json/order_book_l2.h"

#undef VERSION

namespace roq {
namespace bitmex {
namespace json {

struct Parser final {
  struct Handler {
    virtual void operator()(const Instrument&) = 0;
    virtual void operator()(const OrderBookL2&) = 0;
  };

  std::string_view action;
  bool failure = false;
  std::string_view subscribe;
  bool success = false;
  std::string_view table;
  std::string_view timestamp;
  std::string_view version;

  static void dispatch(
      Handler& handler,
      const std::string_view& message,
      core::json::Buffer& buffer);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq
