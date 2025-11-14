/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/bitmex/json/parser.hpp"

#include "roq/compat.hpp"

#include "roq/bitmex/json/field.hpp"
#include "roq/bitmex/json/message.hpp"
#include "roq/bitmex/json/table.hpp"
#include "roq/bitmex/json/utils.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace bitmex {
namespace json {

// === HELPERS ===

namespace {
template <typename T>
void dispatch_helper(auto &handler, auto &message, auto &buffer_stack, auto &trace_info) {
  T obj{message, buffer_stack};
  create_trace_and_dispatch(handler, trace_info, obj);
}
}  // namespace

// === IMPLEMENTATION ===

bool Parser::dispatch(
    Parser::Handler &handler,
    std::string_view const &message,
    core::json::BufferStack &buffer_stack,
    TraceInfo const &trace_info,
    bool allow_unknown_event_types) {
  Message message_2{message, buffer_stack};
  if (!std::empty(message_2.info)) [[unlikely]] {
    dispatch_helper<Welcome>(handler, message, buffer_stack, trace_info);
    return true;
  }
  if (!(std::empty(message_2.subscribe) && std::empty(message_2.error))) [[unlikely]] {
    dispatch_helper<Subscribe>(handler, message, buffer_stack, trace_info);
    return true;
  }
  if (message_2.now.count()) {
    dispatch_helper<CancelAllAfter>(handler, message, buffer_stack, trace_info);
    return true;
  }
  switch (message_2.table) {
    using enum Table::type_t;
    case UNDEFINED_INTERNAL:
      break;
    case UNKNOWN_INTERNAL:
      if (allow_unknown_event_types) {
        return false;
      }
      break;
    // public
    case INSTRUMENT:
      dispatch_helper<Instrument>(handler, message, buffer_stack, trace_info);
      return true;
    case QUOTE:
      dispatch_helper<Quote>(handler, message, buffer_stack, trace_info);
      return true;
    case ORDER_BOOK_L2:
      dispatch_helper<OrderBookL2>(handler, message, buffer_stack, trace_info);
      return true;
    case TRADE:
      dispatch_helper<Trade>(handler, message, buffer_stack, trace_info);
      return true;
    case FUNDING:
      dispatch_helper<Funding>(handler, message, buffer_stack, trace_info);
      return true;
    case LIQUIDATION:
      dispatch_helper<Liquidation>(handler, message, buffer_stack, trace_info);
      return true;
    case SETTLEMENT:
      dispatch_helper<Settlement>(handler, message, buffer_stack, trace_info);
      return true;
    // private
    case MARGIN:
      dispatch_helper<Margin>(handler, message, buffer_stack, trace_info);
      return true;
    case POSITION:
      dispatch_helper<Position>(handler, message, buffer_stack, trace_info);
      return true;
    case ORDER:
      dispatch_helper<Order>(handler, message, buffer_stack, trace_info);
      return true;
    case EXECUTION:
      dispatch_helper<Execution>(handler, message, buffer_stack, trace_info);
      return true;
  }
  log::fatal(R"(Unexpected: message="{}")"sv, message);
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
