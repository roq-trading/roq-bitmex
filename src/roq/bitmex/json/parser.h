/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string_view>

#include "roq/core/json/buffer.h"

#include "roq/bitmex/json/error.h"
#include "roq/bitmex/json/funding.h"
#include "roq/bitmex/json/handshake.h"
#include "roq/bitmex/json/instrument.h"
#include "roq/bitmex/json/liquidation.h"
#include "roq/bitmex/json/order_book_l2.h"
#include "roq/bitmex/json/quote.h"
#include "roq/bitmex/json/settlement.h"
#include "roq/bitmex/json/subscribe.h"
#include "roq/bitmex/json/trade.h"

#undef VERSION

namespace roq {
namespace bitmex {
namespace json {

struct Parser final {
  struct Handler {
    virtual void operator()(const Error&) = 0;
    virtual void operator()(const Funding&) = 0;
    virtual void operator()(const Handshake&) = 0;
    virtual void operator()(const Instrument&) = 0;
    virtual void operator()(const Liquidation&) = 0;
    virtual void operator()(const OrderBookL2&) = 0;
    virtual void operator()(const Quote&) = 0;
    virtual void operator()(const Settlement&) = 0;
    virtual void operator()(const Subscribe&) = 0;
    virtual void operator()(const Trade&) = 0;
  };

  std::string_view action;
  std::string_view error;
  bool failure = false;
  int32_t status = 0;
  std::string_view subscribe;
  bool success = false;
  std::string_view table;
  std::chrono::nanoseconds timestamp = {};
  std::string_view version;

  static void dispatch(
      Handler& handler,
      const std::string_view& message,
      core::json::Buffer& buffer);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq
