/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string_view>

#include "roq/trace_info.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitmex/json/welcome.hpp"

#include "roq/bitmex/json/cancel_all_after.hpp"
#include "roq/bitmex/json/error.hpp"
#include "roq/bitmex/json/subscribe.hpp"
#include "roq/bitmex/json/unsubscribe.hpp"

#include "roq/bitmex/json/funding.hpp"
#include "roq/bitmex/json/instrument.hpp"
#include "roq/bitmex/json/liquidation.hpp"
#include "roq/bitmex/json/order_book_l2.hpp"
#include "roq/bitmex/json/quote.hpp"
#include "roq/bitmex/json/settlement.hpp"
#include "roq/bitmex/json/trade.hpp"

#include "roq/bitmex/json/action.hpp"

#include "roq/bitmex/json/execution.hpp"
#include "roq/bitmex/json/margin.hpp"
#include "roq/bitmex/json/order.hpp"
#include "roq/bitmex/json/position.hpp"

#undef VERSION

namespace roq {
namespace bitmex {
namespace json {

struct Parser final {
  struct Handler {
    virtual void operator()(Trace<Welcome> const &) = 0;
    //
    virtual void operator()(Trace<CancelAllAfter> const &) = 0;
    virtual void operator()(Trace<Error> const &) = 0;
    virtual void operator()(Trace<Subscribe> const &) = 0;
    virtual void operator()(Trace<Unsubscribe> const &) = 0;
    // public
    virtual void operator()(Trace<Instrument> const &) = 0;
    virtual void operator()(Trace<Quote> const &) = 0;
    virtual void operator()(Trace<OrderBookL2> const &) = 0;
    virtual void operator()(Trace<Trade> const &) = 0;
    virtual void operator()(Trace<Funding> const &) = 0;
    virtual void operator()(Trace<Liquidation> const &) = 0;
    virtual void operator()(Trace<Settlement> const &) = 0;
    // private
    virtual void operator()(Trace<Execution> const &) = 0;
    virtual void operator()(Trace<Margin> const &) = 0;
    virtual void operator()(Trace<Order> const &) = 0;
    virtual void operator()(Trace<Position> const &) = 0;
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
  bool heartbeat_enabled = {};

  static bool dispatch(Handler &, std::string_view const &message, core::json::BufferStack &, TraceInfo const &, bool allow_unknown_event_types);
};

}  // namespace json
}  // namespace bitmex
}  // namespace roq
