/* Copyright (c) 2017-2025, Hans Erik Thrane */

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

// === HELPERS ===

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

void update(Type &result, Type const type) {
  assert(type != Type::UNKNOWN);
  if (result == Type::UNKNOWN) {
    result = type;
  } else if (result != type) {
    throw RuntimeError{"wrong type"sv};
  }
}
}  // namespace

// === IMPLEMENTATION ===

bool StreamParser::dispatch(StreamParser::Handler &handler, std::string_view const &message, std::span<std::byte> const &buffer, TraceInfo const &trace_info) {
  StreamParser result;
  auto type = Type::UNKNOWN;
  auto table = Table::UNKNOWN__;
  auto action = Action::UNKNOWN__;
  bool dispatched = false;
  for (int i = 0; i < 2; ++i) {
    core::json::Parser parser{message};
    auto root = parser.root();
    for (auto [key, value] : std::get<core::json::Object>(root)) {
      Field field{key};
      switch (field) {
        using enum Field::type_t;
        case UNDEFINED__:
          log::warn("Unexpected"sv);
          return false;  // note!
        case UNKNOWN__:
          log::warn(R"(Unknown key="{}")"sv, key);
          return false;  // note!
        case ACTION:
          update(result.action, value);
          update(type, Type::TABLE);
          action = Action{result.action};
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
          if (action == Action::UNKNOWN__) {
            // not ready -- finish and try again
          } else {
            core::json::Buffer buffer_2{buffer};
            switch (table) {
              using enum Table::type_t;
              case UNDEFINED__:
              case UNKNOWN__:
                break;
              case EXECUTION: {
                Execution execution{value, buffer_2};
                dispatched = true;
                Trace event{trace_info, execution};
                handler(event, action);
                break;
              }
              case FUNDING: {
                Funding funding{value, buffer_2};
                dispatched = true;
                Trace event{trace_info, funding};
                handler(event, action);
                break;
              }
              case INSTRUMENT: {
                Instrument instrument{value, buffer_2};
                dispatched = true;
                Trace event{trace_info, instrument};
                handler(event, action);
                break;
              }
              case LIQUIDATION: {
                Liquidation liquidation{value, buffer_2};
                dispatched = true;
                Trace event{trace_info, liquidation};
                handler(event, action);
                break;
              }
              case MARGIN: {
                Margin margin{value, buffer_2};
                dispatched = true;
                Trace event{trace_info, margin};
                handler(event, action);
                break;
              }
              case ORDER: {
                Order order{value, buffer_2};
                dispatched = true;
                Trace event{trace_info, order};
                handler(event, action);
                break;
              }
              case ORDER_BOOK_L2: {
                OrderBookL2 order_book_l2{value, buffer_2};
                dispatched = true;
                Trace event{trace_info, order_book_l2};
                handler(event, action);
                break;
              }
              case POSITION: {
                Position position{value, buffer_2};
                dispatched = true;
                Trace event{trace_info, position};
                handler(event, action);
                break;
              }
              case QUOTE: {
                Quote quote{value, buffer_2};
                dispatched = true;
                Trace event{trace_info, quote};
                handler(event, action);
                break;
              }
              case SETTLEMENT: {
                Settlement settlement{value, buffer_2};
                dispatched = true;
                Trace event{trace_info, settlement};
                handler(event, action);
                break;
              }
              case TRADE: {
                Trade trade{value, buffer_2};
                dispatched = true;
                Trace event{trace_info, trade};
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
          assert(table == Table::UNKNOWN__);
          table = Table{result.table};
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
        case HEARTBEAT_ENABLED:
          update(result.heartbeat_enabled, value);
          break;
      }
    }
    switch (type) {
      using enum Type;
      case UNKNOWN:
        throw RuntimeError{"Can't detect message type"sv};
      case CANCEL_ALL_AFTER: {
        auto cancel_all_after = CancelAllAfter{
            .cancel_time = result.cancel_time,
            .now = result.now,
        };
        Trace event{trace_info, cancel_all_after};
        handler(event);
        return true;
      }
      case ERROR: {
        auto error = Error{
            .error = result.error,
            .status = result.status,
        };
        Trace event{trace_info, error};
        handler(event);
        return true;
      }
      case INFO: {
        auto handshake = Handshake{
            .docs = {},
            .info = {},
            .timestamp = result.timestamp,
            .version = result.version,
            .heartbeat_enabled = result.heartbeat_enabled,
        };
        Trace event{trace_info, handshake};
        handler(event);
        return true;
      }
      case SUBSCRIBE: {
        auto subscribe = Subscribe{
            .failure = result.failure,
            .subscribe = result.subscribe,
            .success = result.success,
        };
        Trace event{trace_info, subscribe};
        handler(event);
        return true;
      }
      case UNSUBSCRIBE: {
        auto unsubscribe = Unsubscribe{
            .failure = result.failure,
            .unsubscribe = result.unsubscribe,
            .success = result.success,
        };
        Trace event{trace_info, unsubscribe};
        handler(event);
        return true;
      }
      case TABLE:
        if (dispatched) {
          return true;
        }
        // perhaps we were just unlucky with the ordering of keys
        // XXX increment warning counter
        break;
    }
  }
  return false;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
