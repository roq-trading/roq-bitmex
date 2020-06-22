/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string_view>

#include "roq/core/json/buffer.h"

#include "roq/server.h"

#include "roq/bitmex/json/action.h"
#include "roq/bitmex/json/cancel_all_after.h"
#include "roq/bitmex/json/error.h"
#include "roq/bitmex/json/execution.h"
#include "roq/bitmex/json/funding.h"
#include "roq/bitmex/json/handshake.h"
#include "roq/bitmex/json/instrument.h"
#include "roq/bitmex/json/liquidation.h"
#include "roq/bitmex/json/margin.h"
#include "roq/bitmex/json/order.h"
#include "roq/bitmex/json/order_book_l2.h"
#include "roq/bitmex/json/position.h"
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
    virtual void operator()(const CancelAllAfter&) = 0;
    virtual void operator()(const Error&) = 0;
    virtual void operator()(const Handshake&) = 0;
    virtual void operator()(const Subscribe&) = 0;
    // table
    virtual void operator()(
        const Action,
        const Execution&,
        const server::Trace&) = 0;
    virtual void operator()(
        const Action,
        const Funding&,
        const server::Trace&) = 0;
    virtual void operator()(
        const Action,
        const Instrument&,
        const server::Trace&) = 0;
    virtual void operator()(
        const Action,
        const Liquidation&,
        const server::Trace&) = 0;
    virtual void operator()(
        const Action,
        const Margin&,
        const server::Trace&) = 0;
    virtual void operator()(
        const Action,
        const Order&,
        const server::Trace&) = 0;
    virtual void operator()(
        const Action,
        const OrderBookL2&,
        const server::Trace&) = 0;
    virtual void operator()(
        const Action,
        const Position&,
        const server::Trace&) = 0;
    virtual void operator()(
        const Action,
        const Quote&,
        const server::Trace&) = 0;
    virtual void operator()(
        const Action,
        const Settlement&,
        const server::Trace&) = 0;
    virtual void operator()(
        const Action,
        const Trade&,
        const server::Trace&) = 0;
  };

  std::string_view action;
  std::chrono::nanoseconds cancel_time = {};
  std::string_view error;
  bool failure = false;
  std::chrono::nanoseconds now = {};
  int32_t status = 0;
  std::string_view subscribe;
  bool success = false;
  std::string_view table;
  std::chrono::nanoseconds timestamp = {};
  std::string_view version;

  static void dispatch(
      Handler& handler,
      const std::string_view& message,
      core::json::Buffer& buffer,
      const server::Trace& trace);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq
