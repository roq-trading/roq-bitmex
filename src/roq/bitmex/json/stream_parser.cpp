/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/bitmex/json/stream_parser.h"

#include "roq/compat.h"

#include "roq/bitmex/json/field.h"
#include "roq/bitmex/json/table.h"
#include "roq/bitmex/json/utils.h"

#include "roq/logging.h"

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
    throw RuntimeErrorException("wrong type"sv);
  }
}
}  // namespace

void StreamParser::dispatch(
    StreamParser::Handler &handler,
    const std::string_view &message,
    core::json::Buffer &buffer,
    const server::TraceInfo &trace_info) {
  StreamParser result;
  auto type = Type::UNKNOWN;
  auto table = Table::UNKNOWN;
  auto action = Action::UNKNOWN;
  bool dispatched = false;
  for (int i = 0; i < 2; ++i) {
    core::json::Parser parser(message);
    auto root = parser.root();
    for (auto [key, value] : std::get<core::json::object_t>(root)) {
      auto field = Field(key);
      switch (field) {
        case Field::UNDEFINED:
          log::fatal("Unexpected"sv);
          break;
        case Field::UNKNOWN:
          log::fatal(R"(Unknown key="{}")"sv, key);
          break;
        case Field::ACTION:
          update(result.action, value);
          update(type, Type::TABLE);
          action = Action(result.action);
          break;
        case Field::ATTRIBUTES:
          // not used
          update(type, Type::TABLE);
          break;
        case Field::CANCEL_TIME:
          update(result.cancel_time, value);
          update(type, Type::CANCEL_ALL_AFTER);
          break;
        case Field::DATA:
          if (action == Action::UNKNOWN) {
            // not ready -- finish and try again
          } else {
            switch (table) {
              case Table::UNDEFINED:
              case Table::UNKNOWN:
                break;
              case Table::EXECUTION: {
                Execution execution(value, buffer);
                dispatched = true;
                server::Trace event(trace_info, execution);
                handler(event, action);
                break;
              }
              case Table::FUNDING: {
                Funding funding(value, buffer);
                dispatched = true;
                server::Trace event(trace_info, funding);
                handler(event, action);
                break;
              }
              case Table::INSTRUMENT: {
                Instrument instrument(value, buffer);
                dispatched = true;
                server::Trace event(trace_info, instrument);
                handler(event, action);
                break;
              }
              case Table::LIQUIDATION: {
                Liquidation liquidation(value, buffer);
                dispatched = true;
                server::Trace event(trace_info, liquidation);
                handler(event, action);
                break;
              }
              case Table::MARGIN: {
                Margin margin(value, buffer);
                dispatched = true;
                server::Trace event(trace_info, margin);
                handler(event, action);
                break;
              }
              case Table::ORDER: {
                Order order(value, buffer);
                dispatched = true;
                server::Trace event(trace_info, order);
                handler(event, action);
                break;
              }
              case Table::ORDER_BOOK_L2: {
                OrderBookL2 order_book_l2(value, buffer);
                dispatched = true;
                server::Trace event(trace_info, order_book_l2);
                handler(event, action);
                break;
              }
              case Table::POSITION: {
                Position position(value, buffer);
                dispatched = true;
                server::Trace event(trace_info, position);
                handler(event, action);
                break;
              }
              case Table::QUOTE: {
                Quote quote(value, buffer);
                dispatched = true;
                server::Trace event(trace_info, quote);
                handler(event, action);
                break;
              }
              case Table::SETTLEMENT: {
                Settlement settlement(value, buffer);
                dispatched = true;
                server::Trace event(trace_info, settlement);
                handler(event, action);
                break;
              }
              case Table::TRADE: {
                Trade trade(value, buffer);
                dispatched = true;
                server::Trace event(trace_info, trade);
                handler(event, action);
                break;
              }
            }
          }
          break;
        case Field::DOCS:
          // not used
          update(type, Type::INFO);
          break;
        case Field::ERROR:
          update(result.error, value);
          update(type, Type::ERROR);
          break;
        case Field::FAILURE:
          update(result.failure, value);
          update(type, Type::SUBSCRIBE);
          break;
        case Field::FILTER:
          // not used
          update(type, Type::TABLE);
          break;
        case Field::FOREIGN_KEYS:
          // not used
          update(type, Type::TABLE);
          break;
        case Field::INFO:
          // not used
          update(type, Type::INFO);
          break;
        case Field::KEYS:
          // not used
          update(type, Type::TABLE);
          break;
        case Field::LIMIT:
          // XXX should parse! "limit":{"remaining":37}
          update(type, Type::INFO);
          break;
        case Field::META:
          // not used
          update(type, Type::ERROR);
          break;
        case Field::NOW:
          update(result.now, value);
          update(type, Type::CANCEL_ALL_AFTER);
          break;
        case Field::REQUEST:
          // not used
          // => subscribe + error
          break;
        case Field::STATUS:
          update(result.status, value);
          update(type, Type::ERROR);
          break;
        case Field::SUBSCRIBE:
          update(result.subscribe, value);
          update(type, Type::SUBSCRIBE);
          break;
        case Field::UNSUBSCRIBE:
          update(result.unsubscribe, value);
          update(type, Type::UNSUBSCRIBE);
          break;
        case Field::SUCCESS:
          update(result.success, value);
          break;
        case Field::TABLE:
          update(result.table, value);
          update(type, Type::TABLE);
          assert(table == Table::UNKNOWN);
          table = Table(result.table);
          break;
        case Field::TIMESTAMP:
          update(result.timestamp, value);
          update(type, Type::INFO);
          break;
        case Field::TYPES:
          // not used
          update(type, Type::TABLE);
          break;
        case Field::VERSION:
          update(result.version, value);
          update(type, Type::INFO);
          break;
      }
    }
    switch (type) {
      case Type::UNKNOWN:
        throw RuntimeErrorException("Can't detect message type"sv);
      case Type::CANCEL_ALL_AFTER: {
        CancelAllAfter cancel_all_after{
            .cancel_time = result.cancel_time,
            .now = result.now,
        };
        server::Trace event(trace_info, cancel_all_after);
        handler(event);
        return;
      }
      case Type::ERROR: {
        Error error{
            .error = result.error,
            .status = result.status,
        };
        server::Trace event(trace_info, error);
        handler(event);
        return;
      }
      case Type::INFO: {
        Handshake handshake{
            .docs = {},
            .info = {},
            .timestamp = result.timestamp,
            .version = result.version,
        };
        server::Trace event(trace_info, handshake);
        handler(event);
        return;
      }
      case Type::SUBSCRIBE: {
        Subscribe subscribe{
            .failure = result.failure,
            .subscribe = result.subscribe,
            .success = result.success,
        };
        server::Trace event(trace_info, subscribe);
        handler(event);
        return;
      }
      case Type::UNSUBSCRIBE: {
        Unsubscribe unsubscribe{
            .failure = result.failure,
            .unsubscribe = result.unsubscribe,
            .success = result.success,
        };
        server::Trace event(trace_info, unsubscribe);
        handler(event);
        return;
      }
      case Type::TABLE:
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
