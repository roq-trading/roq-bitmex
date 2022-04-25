/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string_view>

#include "roq/core/json/buffer.hpp"

#include "roq/server.hpp"

#include "roq/bitmex/json/action.hpp"
#include "roq/bitmex/json/cancel_all_after.hpp"
#include "roq/bitmex/json/error.hpp"
#include "roq/bitmex/json/execution.hpp"
#include "roq/bitmex/json/funding.hpp"
#include "roq/bitmex/json/handshake.hpp"
#include "roq/bitmex/json/instrument.hpp"
#include "roq/bitmex/json/liquidation.hpp"
#include "roq/bitmex/json/margin.hpp"
#include "roq/bitmex/json/order.hpp"
#include "roq/bitmex/json/order_book_l2.hpp"
#include "roq/bitmex/json/position.hpp"
#include "roq/bitmex/json/quote.hpp"
#include "roq/bitmex/json/settlement.hpp"
#include "roq/bitmex/json/subscribe.hpp"
#include "roq/bitmex/json/trade.hpp"
#include "roq/bitmex/json/unsubscribe.hpp"

#undef VERSION

namespace roq {
namespace bitmex {
namespace json {

struct StreamParser final {
  struct Handler {
    virtual void operator()(const Trace<CancelAllAfter const> &) = 0;
    virtual void operator()(const Trace<Error const> &) = 0;
    virtual void operator()(const Trace<Handshake const> &) = 0;
    virtual void operator()(const Trace<Subscribe const> &) = 0;
    virtual void operator()(const Trace<Unsubscribe const> &) = 0;
    // table
    virtual void operator()(const Trace<Execution const> &, Action) = 0;
    virtual void operator()(const Trace<Funding const> &, Action) = 0;
    virtual void operator()(const Trace<Instrument const> &, Action) = 0;
    virtual void operator()(const Trace<Liquidation const> &, Action) = 0;
    virtual void operator()(const Trace<Margin const> &, Action) = 0;
    virtual void operator()(const Trace<Order const> &, Action) = 0;
    virtual void operator()(const Trace<OrderBookL2 const> &, Action) = 0;
    virtual void operator()(const Trace<Position const> &, Action) = 0;
    virtual void operator()(const Trace<Quote const> &, Action) = 0;
    virtual void operator()(const Trace<Settlement const> &, Action) = 0;
    virtual void operator()(const Trace<Trade const> &, Action) = 0;
  };

  std::string_view action;
  std::chrono::milliseconds cancel_time = {};
  std::string_view error;
  bool failure = false;
  std::chrono::milliseconds now = {};
  int32_t status = 0;
  std::string_view subscribe;
  std::string_view unsubscribe;
  bool success = false;
  std::string_view table;
  std::chrono::milliseconds timestamp = {};
  std::string_view version;

  static void dispatch(
      Handler &handler,
      const std::string_view &message,
      core::json::Buffer &buffer,
      const TraceInfo &trace);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq
