/* Copyright (c) 2017-2021, Hans Erik Thrane */

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
#include "roq/bitmex/json/unsubscribe.h"

#undef VERSION

namespace roq {
namespace bitmex {
namespace json {

struct StreamParser final {
  struct Handler {
    virtual void operator()(const server::Trace<CancelAllAfter> &) = 0;
    virtual void operator()(const server::Trace<Error> &) = 0;
    virtual void operator()(const server::Trace<Handshake> &) = 0;
    virtual void operator()(const server::Trace<Subscribe> &) = 0;
    virtual void operator()(const server::Trace<Unsubscribe> &) = 0;
    // table
    virtual void operator()(const server::Trace<Execution> &, Action) = 0;
    virtual void operator()(const server::Trace<Funding> &, Action) = 0;
    virtual void operator()(const server::Trace<Instrument> &, Action) = 0;
    virtual void operator()(const server::Trace<Liquidation> &, Action) = 0;
    virtual void operator()(const server::Trace<Margin> &, Action) = 0;
    virtual void operator()(const server::Trace<Order> &, Action) = 0;
    virtual void operator()(const server::Trace<OrderBookL2> &, Action) = 0;
    virtual void operator()(const server::Trace<Position> &, Action) = 0;
    virtual void operator()(const server::Trace<Quote> &, Action) = 0;
    virtual void operator()(const server::Trace<Settlement> &, Action) = 0;
    virtual void operator()(const server::Trace<Trade> &, Action) = 0;
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
      const server::TraceInfo &trace);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq
