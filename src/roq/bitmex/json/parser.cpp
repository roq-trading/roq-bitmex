/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/parser.h"

#include "roq/compat.h"

#include "roq/bitmex/json/field.h"
#include "roq/bitmex/json/table.h"
#include "roq/bitmex/json/utils.h"

#include "roq/logging.h"

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
  TABLE,
};

void update(Type& result, const Type type) {
  assert(type != Type::UNKNOWN);
  if (result == Type::UNKNOWN) {
    result = type;
  } else if (result != type) {
    throw std::runtime_error("wrong type");
  }
}
}  // namespace

void Parser::dispatch(
    Parser::Handler& handler,
    const std::string_view& message,
    core::json::Buffer& buffer,
    const server::TraceInfo& trace_info) {
  Parser result;
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
          LOG(FATAL)("Unexpected");
          break;
        case Field::UNKNOWN:
          DLOG(FATAL)(
              FMT_STRING(R"(Unknown key="{}")"),
              key);
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
                handler(
                    action,
                    execution,
                    trace_info);
                break;
              }
              case Table::FUNDING: {
                Funding funding(value, buffer);
                dispatched = true;
                handler(
                    action,
                    funding,
                    trace_info);
                break;
              }
              case Table::INSTRUMENT: {
                Instrument instrument(value, buffer);
                dispatched = true;
                handler(
                    action,
                    instrument,
                    trace_info);
                break;
              }
              case Table::LIQUIDATION: {
                Liquidation liquidation(value, buffer);
                dispatched = true;
                handler(
                    action,
                    liquidation,
                    trace_info);
                break;
              }
              case Table::MARGIN: {
                Margin margin(value, buffer);
                dispatched = true;
                handler(
                    action,
                    margin,
                    trace_info);
                break;
              }
              case Table::ORDER: {
                Order order(value, buffer);
                dispatched = true;
                handler(
                    action,
                    order,
                    trace_info);
                break;
              }
              case Table::ORDER_BOOK_L2: {
                OrderBookL2 order_book_l2(value, buffer);
                dispatched = true;
                handler(
                    action,
                    order_book_l2,
                    trace_info);
                break;
              }
              case Table::POSITION: {
                Position position(value, buffer);
                dispatched = true;
                handler(
                    action,
                    position,
                    trace_info);
                break;
              }
              case Table::QUOTE: {
                Quote quote(value, buffer);
                dispatched = true;
                handler(
                    action,
                    quote,
                    trace_info);
                break;
              }
              case Table::SETTLEMENT: {
                Settlement settlement(value, buffer);
                dispatched = true;
                handler(
                    action,
                    settlement,
                    trace_info);
                break;
              }
              case Table::TRADE: {
                Trade trade(value, buffer);
                dispatched = true;
                handler(
                    action,
                    trade,
                    trace_info);
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
        case Field::SUCCESS:
          update(result.success, value);
          update(type, Type::SUBSCRIBE);
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
        throw std::runtime_error("Can't detect message type");
      case Type::CANCEL_ALL_AFTER: {
        CancelAllAfter cancel_all_after = {
          .cancel_time = result.cancel_time,
          .now = result.now,
        };
        handler(cancel_all_after);
        return;
      }
      case Type::ERROR: {
        Error error = {
          .error = result.error,
          .status = result.status,
        };
        handler(error);
        return;
      }
      case Type::INFO: {
        Handshake handshake {
          .docs = nullptr,
          .info = nullptr,
          .timestamp = result.timestamp,
          .version = result.version,
        };
        handler(handshake);
        return;
      }
      case Type::SUBSCRIBE: {
        Subscribe subscribe {
          .failure = result.failure,
          .subscribe = result.subscribe,
          .success = result.success,
        };
        handler(subscribe);
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
  LOG(WARNING)(
      FMT_STRING(R"(message="{}")"),
      message);
  LOG(FATAL)("Unexpected");
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
