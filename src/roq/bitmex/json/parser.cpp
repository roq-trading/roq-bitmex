/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/parser.h"

#include "roq/compat.h"

#include "roq/bitmex/json/table.h"
#include "roq/bitmex/json/utils.h"

#include "roq/logging.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  ACTION,
  ATTRIBUTES,
  CANCEL_TIME,
  DATA,
  DOCS,
  ERROR,
  FAILURE,
  FILTER,
  FOREIGN_KEYS,
  INFO,
  KEYS,
  LIMIT,
  NOW,
  META,
  REQUEST,
  STATUS,
  SUBSCRIBE,
  SUCCESS,
  TABLE,
  TIMESTAMP,
  TYPES,
  VERSION,
};


constexpr auto parse_a(auto& name) {
  if (name.length() >= 2) {
    switch (name.data()[1]) {
      case 'c':
        if (name.compare("action") == 0)
          return Field::ACTION;
        break;
      case 't':
        if (name.compare("attributes") == 0)
          return Field::ATTRIBUTES;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr auto parse_c(auto& name) {
  if (name.compare("cancelTime") == 0)
    return Field::CANCEL_TIME;
  return Field::UNKNOWN;
}

constexpr auto parse_d(auto& name) {
  if (name.length() >= 2) {
    switch (name.data()[1]) {
      case 'a':
        if (name.compare("data") == 0)
          return Field::DATA;
        break;
      case 'o':
        if (name.compare("docs") == 0)
          return Field::DOCS;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr auto parse_e(auto& name) {
  if (name.compare("error") == 0)
    return Field::ERROR;
  return Field::UNKNOWN;
}

constexpr auto parse_f(auto& name) {
  if (name.length() >= 2) {
    switch (name.data()[1]) {
      case 'a':
        if (name.compare("failure") == 0)
          return Field::FAILURE;
        break;
      case 'i':
        if (name.compare("filter") == 0)
          return Field::FILTER;
        break;
      case 'o':
        if (name.compare("foreignKeys") == 0)
          return Field::FOREIGN_KEYS;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr auto parse_i(auto& name) {
  if (name.compare("info") == 0)
    return Field::INFO;
  return Field::UNKNOWN;
}

constexpr auto parse_k(auto& name) {
  if (name.compare("keys") == 0)
    return Field::KEYS;
  return Field::UNKNOWN;
}

constexpr auto parse_l(auto& name) {
  if (name.compare("limit") == 0)
    return Field::LIMIT;
  return Field::UNKNOWN;
}

constexpr auto parse_m(auto& name) {
  if (name.compare("meta") == 0)
    return Field::META;
  return Field::UNKNOWN;
}

constexpr auto parse_n(auto& name) {
  if (name.compare("now") == 0)
    return Field::NOW;
  return Field::UNKNOWN;
}

constexpr auto parse_r(auto& name) {
  if (name.compare("request") == 0)
    return Field::REQUEST;
  return Field::UNKNOWN;
}

constexpr auto parse_s(auto& name) {
  if (name.length() >= 3) {
    switch (name.data()[2]) {
      case 'a':
        if (name.compare("status") == 0)
          return Field::STATUS;
        break;
      case 'b':
        if (name.compare("subscribe") == 0)
          return Field::SUBSCRIBE;
        break;
      case 'c':
        if (name.compare("success") == 0)
          return Field::SUCCESS;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr auto parse_t(auto& name) {
  if (name.length() >= 2) {
    switch (name.data()[1]) {
      case 'a':
        if (name.compare("table") == 0)
          return Field::TABLE;
        break;
      case 'i':
        if (name.compare("timestamp") == 0)
          return Field::TIMESTAMP;
        break;
      case 'y':
        if (name.compare("types") == 0)
          return Field::TYPES;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr auto parse_v(auto& name) {
  if (name.compare("version") == 0)
    return Field::VERSION;
  return Field::UNKNOWN;
}

constexpr auto parse_field(const std::string_view& name) {
  if (name.empty())
    return Field::UNKNOWN;
  switch (name.data()[0]) {
    case 'a': return parse_a(name);
    case 'c': return parse_c(name);
    case 'd': return parse_d(name);
    case 'e': return parse_e(name);
    case 'f': return parse_f(name);
    case 'i': return parse_i(name);
    case 'k': return parse_k(name);
    case 'l': return parse_l(name);
    case 'm': return parse_m(name);
    case 'n': return parse_n(name);
    case 'r': return parse_r(name);
    case 's': return parse_s(name);
    case 't': return parse_t(name);
    case 'v': return parse_v(name);
    default:
      return Field::UNKNOWN;
  }
}

static_assert(parse_field("action") == Field::ACTION);
static_assert(parse_field("attributes") == Field::ATTRIBUTES);

static_assert(parse_field("cancelTime") == Field::CANCEL_TIME);

static_assert(parse_field("data") == Field::DATA);
static_assert(parse_field("docs") == Field::DOCS);

static_assert(parse_field("error") == Field::ERROR);

static_assert(parse_field("failure") == Field::FAILURE);
static_assert(parse_field("filter") == Field::FILTER);
static_assert(parse_field("foreignKeys") == Field::FOREIGN_KEYS);

static_assert(parse_field("info") == Field::INFO);

static_assert(parse_field("keys") == Field::KEYS);

static_assert(parse_field("limit") == Field::LIMIT);

static_assert(parse_field("meta") == Field::META);

static_assert(parse_field("now") == Field::NOW);

static_assert(parse_field("request") == Field::REQUEST);

static_assert(parse_field("status") == Field::STATUS);
static_assert(parse_field("subscribe") == Field::SUBSCRIBE);
static_assert(parse_field("success") == Field::SUCCESS);

static_assert(parse_field("table") == Field::TABLE);
static_assert(parse_field("timestamp") == Field::TIMESTAMP);
static_assert(parse_field("types") == Field::TYPES);

static_assert(parse_field("version") == Field::VERSION);

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
    core::json::Buffer& buffer) {
  Parser result;
  auto type = Type::UNKNOWN;
  auto table = Table::UNKNOWN;
  auto action = Action::UNKNOWN;
  bool dispatched = false;
  for (int i = 0; i < 2; ++i) {
    core::json::Parser parser(message);
    auto root = parser.root();
    for (auto [key, value] : std::get<core::json::object_t>(root)) {
      auto field = parse_field(key);
      switch (field) {
        case Field::UNKNOWN:
          DLOG(FATAL)(
              FMT_STRING("Unknown key=\"{}\""),
              key);
          break;
        case Field::ACTION:
          update(result.action, value);
          update(type, Type::TABLE);
          action = parse_action(result.action);
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
              case Table::UNKNOWN:
                break;
              case Table::EXECUTION: {
                Execution execution(
                    value,
                    buffer,
                    action);
                dispatched = true;
                handler(execution);
                break;
              }
              case Table::FUNDING: {
                Funding funding(
                    value,
                    buffer,
                    action);
                dispatched = true;
                handler(funding);
                break;
              }
              case Table::INSTRUMENT: {
                Instrument instrument(
                    value,
                    buffer,
                    action);
                dispatched = true;
                handler(instrument);
                break;
              }
              case Table::LIQUIDATION: {
                Liquidation liquidation(
                    value,
                    buffer,
                    action);
                dispatched = true;
                handler(liquidation);
                break;
              }
              case Table::MARGIN: {
                Margin margin(
                    value,
                    buffer,
                    action);
                dispatched = true;
                handler(margin);
                break;
              }
              case Table::ORDER: {
                Order order(
                    value,
                    buffer,
                    action);
                dispatched = true;
                handler(order);
                break;
              }
              case Table::ORDER_BOOK_L2: {
                OrderBookL2 order_book_l2(
                    value,
                    buffer,
                    action);
                dispatched = true;
                handler(order_book_l2);
                break;
              }
              case Table::POSITION: {
                Position position(
                    value,
                    buffer,
                    action);
                dispatched = true;
                handler(position);
                break;
              }
              case Table::QUOTE: {
                Quote quote(
                    value,
                    buffer,
                    action);
                dispatched = true;
                handler(quote);
                break;
              }
              case Table::SETTLEMENT: {
                Settlement settlement(
                    value,
                    buffer,
                    action);
                dispatched = true;
                handler(settlement);
                break;
              }
              case Table::TRADE: {
                Trade trade(
                    value,
                    buffer,
                    action);
                dispatched = true;
                handler(trade);
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
          table = parse_table(result.table);
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
  throw std::runtime_error("Can't parse message");
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
