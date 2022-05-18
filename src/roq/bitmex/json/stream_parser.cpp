/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/bitmex/json/stream_parser.hpp"

#include "roq/compat.hpp"

#include "roq/bitmex/json/field.hpp"
#include "roq/bitmex/json/table.hpp"
#include "roq/bitmex/json/utils.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Type {
  UNKNOWN,
  CANCEL_ALL_AFTER,
  ERROR,
  INFO,
  SUBSCRIBE,
  UNSUBSCRIBE,
  TABLE,
};

void update(Type &result, const Type type) {
  assert(type != Type::UNKNOWN);
  if (result == Type::UNKNOWN) {
    result = type;
  } else if (result != type) {
    throw RuntimeError("wrong type"sv);
  }
}
}  // namespace

void StreamParser::dispatch(
    StreamParser::Handler &handler,
    std::string_view const &message,
    core::json::Buffer &buffer,
    TraceInfo const &trace_info) {
  StreamParser result;
  auto type = Type::UNKNOWN;
  auto table = Table::UNKNOWN;
  auto action = Action::UNKNOWN;
  bool dispatched = false;
  for (int i = 0; i < 2; ++i) {
    core::json::Parser parser(message);
    auto root = parser.root();
    for (auto [key, value] : std::get<core::json::Object>(root)) {
      auto field = Field(key);
      switch (field) {
        using enum Field::type_t;
        case UNDEFINED:
          log::fatal("Unexpected"sv);
          break;
        case UNKNOWN:
          log::fatal(R"(Unknown key="{}")"sv, key);
          break;
        case ACTION:
          update(result.action, value);
          update(type, Type::TABLE);
          action = Action(result.action);
          break;
        case ATTRIBUTES:
          // not used
          update(type, Type::TABLE);
          break;
        case CANCEL_TIME:
          update(result.cancel_time, value);
          update(type, Type::CANCEL_ALL_AFTER);
          break;
        case DATA:
          if (action == Action::UNKNOWN) {
            // not ready -- finish and try again
          } else {
            switch (table) {
              using enum Table::type_t;
              case UNDEFINED:
              case UNKNOWN:
                break;
              case EXECUTION: {
                const Execution execution(value, buffer);
                dispatched = true;
                Trace event(trace_info, execution);
                handler(event, action);
                break;
              }
              case FUNDING: {
                const Funding funding(value, buffer);
                dispatched = true;
                Trace event(trace_info, funding);
                handler(event, action);
                break;
              }
              case INSTRUMENT: {
                const Instrument instrument(value, buffer);
                dispatched = true;
                Trace event(trace_info, instrument);
                handler(event, action);
                break;
              }
              case LIQUIDATION: {
                const Liquidation liquidation(value, buffer);
                dispatched = true;
                Trace event(trace_info, liquidation);
                handler(event, action);
                break;
              }
              case MARGIN: {
                const Margin margin(value, buffer);
                dispatched = true;
                Trace event(trace_info, margin);
                handler(event, action);
                break;
              }
              case ORDER: {
                const Order order(value, buffer);
                dispatched = true;
                Trace event(trace_info, order);
                handler(event, action);
                break;
              }
              case ORDER_BOOK_L2: {
                const OrderBookL2 order_book_l2(value, buffer);
                dispatched = true;
                Trace event(trace_info, order_book_l2);
                handler(event, action);
                break;
              }
              case POSITION: {
                const Position position(value, buffer);
                dispatched = true;
                Trace event(trace_info, position);
                handler(event, action);
                break;
              }
              case QUOTE: {
                const Quote quote(value, buffer);
                dispatched = true;
                Trace event(trace_info, quote);
                handler(event, action);
                break;
              }
              case SETTLEMENT: {
                const Settlement settlement(value, buffer);
                dispatched = true;
                Trace event(trace_info, settlement);
                handler(event, action);
                break;
              }
              case TRADE: {
                const Trade trade(value, buffer);
                dispatched = true;
                Trace event(trace_info, trade);
                handler(event, action);
                break;
              }
            }
          }
          break;
        case DOCS:
          // not used
          update(type, Type::INFO);
          break;
        case ERROR:
          update(result.error, value);
          update(type, Type::ERROR);
          break;
        case FAILURE:
          update(result.failure, value);
          update(type, Type::SUBSCRIBE);
          break;
        case FILTER:
          // not used
          update(type, Type::TABLE);
          break;
        case FOREIGN_KEYS:
          // not used
          update(type, Type::TABLE);
          break;
        case INFO:
          // not used
          update(type, Type::INFO);
          break;
        case KEYS:
          // not used
          update(type, Type::TABLE);
          break;
        case LIMIT:
          // XXX should parse! "limit":{"remaining":37}
          update(type, Type::INFO);
          break;
        case META:
          // not used
          update(type, Type::ERROR);
          break;
        case NOW:
          update(result.now, value);
          update(type, Type::CANCEL_ALL_AFTER);
          break;
        case REQUEST:
          // not used
          // => subscribe + error
          break;
        case STATUS:
          update(result.status, value);
          update(type, Type::ERROR);
          break;
        case SUBSCRIBE:
          update(result.subscribe, value);
          update(type, Type::SUBSCRIBE);
          break;
        case UNSUBSCRIBE:
          update(result.unsubscribe, value);
          update(type, Type::UNSUBSCRIBE);
          break;
        case SUCCESS:
          update(result.success, value);
          break;
        case TABLE:
          update(result.table, value);
          update(type, Type::TABLE);
          assert(table == Table::UNKNOWN);
          table = Table(result.table);
          break;
        case TIMESTAMP:
          update(result.timestamp, value);
          update(type, Type::INFO);
          break;
        case TYPES:
          // not used
          update(type, Type::TABLE);
          break;
        case VERSION:
          update(result.version, value);
          update(type, Type::INFO);
          break;
      }
    }
    switch (type) {
      using enum Type;
      case UNKNOWN:
        throw RuntimeError("Can't detect message type"sv);
      case CANCEL_ALL_AFTER: {
        const CancelAllAfter cancel_all_after{
            .cancel_time = result.cancel_time,
            .now = result.now,
        };
        Trace event(trace_info, cancel_all_after);
        handler(event);
        return;
      }
      case ERROR: {
        const Error error{
            .error = result.error,
            .status = result.status,
        };
        Trace event(trace_info, error);
        handler(event);
        return;
      }
      case INFO: {
        const Handshake handshake{
            .docs = {},
            .info = {},
            .timestamp = result.timestamp,
            .version = result.version,
        };
        Trace event(trace_info, handshake);
        handler(event);
        return;
      }
      case SUBSCRIBE: {
        const Subscribe subscribe{
            .failure = result.failure,
            .subscribe = result.subscribe,
            .success = result.success,
        };
        Trace event(trace_info, subscribe);
        handler(event);
        return;
      }
      case UNSUBSCRIBE: {
        const Unsubscribe unsubscribe{
            .failure = result.failure,
            .unsubscribe = result.unsubscribe,
            .success = result.success,
        };
        Trace event(trace_info, unsubscribe);
        handler(event);
        return;
      }
      case TABLE:
        if (dispatched)
          return;
        // perhaps we were just unlucky with the ordering of keys
        // XXX increment warning counter
        break;
    }
  }
  log::warn(R"(message="{}")"sv, message);
  log::fatal("Unexpected"sv);
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
