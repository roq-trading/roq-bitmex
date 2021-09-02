/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <chrono>

#include "roq/core/utility.h"

#include "roq/core/json/parser.h"

#include "roq/core/charconv/datetime.h"

#include "roq/bitmex/json/exec_inst.h"
#include "roq/bitmex/json/liquidity_ind.h"
#include "roq/bitmex/json/ord_status.h"
#include "roq/bitmex/json/ord_type.h"
#include "roq/bitmex/json/side.h"
#include "roq/bitmex/json/state.h"
#include "roq/bitmex/json/time_in_force.h"

namespace roq {
namespace bitmex {
namespace json {

template <typename T>
inline void update(T &result, const core::json::value_t &value) {
  result = core::json::get<T>(value);
}

template <>
inline void update(std::chrono::milliseconds &result, const core::json::value_t &value) {
  return std::visit(
      overloaded{
          [&](const core::json::null_t &) { result = std::chrono::milliseconds{}; },
          [](bool) { throw std::bad_cast(); },
          [&](int64_t value) {
            result = std::chrono::milliseconds{static_cast<uint64_t>(value * int64_t{1000})};
          },
          [&](double value) {
            result = std::chrono::milliseconds{static_cast<uint64_t>(value * 1.0e3)};
          },
          [&](const std::string_view &value) {
            result =
                core::charconv::datetime_from_string<std::remove_reference<decltype(result)>::type>(
                    value);
          },
          [](const core::json::object_t &) { throw std::bad_cast(); },
          [](const core::json::array_t &) { throw std::bad_cast(); },
      },
      value);
}

// bitmex => roq

inline roq::TradingStatus map(json::State state) {
  switch (state) {
    case json::State::UNDEFINED:
      break;
    case json::State::UNKNOWN:
      break;
    case json::State::OPEN:
      return roq::TradingStatus::OPEN;
    case json::State::CLOSED:
      return roq::TradingStatus::CLOSE;
    case json::State::SETTLED:
      break;
    case json::State::UNLISTED:
      break;
  }
  return {};
}

inline roq::Side map(json::Side side) {
  switch (side) {
    case json::Side::UNDEFINED:
      break;
    case json::Side::UNKNOWN:
      break;
    case json::Side::BUY:
      return roq::Side::BUY;
    case json::Side::SELL:
      return roq::Side::SELL;
  }
  return roq::Side::UNDEFINED;
}

inline roq::OrderStatus map(json::OrdStatus state) {
  switch (state) {
    case OrdStatus::UNDEFINED:
      break;
    case OrdStatus::UNKNOWN:
      break;
    case OrdStatus::CANCELED:
      return roq::OrderStatus::CANCELED;
    case OrdStatus::DONE_FOR_DAY:
      return roq::OrderStatus::SUSPENDED;
    case OrdStatus::EXPIRED:
      return roq::OrderStatus::EXPIRED;
    case OrdStatus::FILLED:
      return roq::OrderStatus::COMPLETED;
    case OrdStatus::NEW:
      return roq::OrderStatus::WORKING;
    case OrdStatus::PARTIALLY_FILLED:
      return roq::OrderStatus::WORKING;
    case OrdStatus::PENDING_CANCEL:  // XXX HANS what to do?
      break;
    case OrdStatus::PENDING_NEW:
      return roq::OrderStatus::SENT;
    case OrdStatus::REJECTED:
      return roq::OrderStatus::REJECTED;
    case OrdStatus::STOPPED:
      return roq::OrderStatus::STOPPED;
    case OrdStatus::UNTRIGGERED:
      return roq::OrderStatus::ACCEPTED;
    case OrdStatus::TRIGGERED:
      return roq::OrderStatus::WORKING;
  }
  return roq::OrderStatus::UNDEFINED;
}

inline roq::OrderType map(json::OrdType state) {
  switch (state) {
    case OrdType::UNDEFINED:
      break;
    case OrdType::UNKNOWN:
      break;
    case OrdType::LIMIT:
      return roq::OrderType::LIMIT;
    case OrdType::MARKET:
      return roq::OrderType::MARKET;
    case OrdType::STOP:
      return roq::OrderType::MARKET;
    case OrdType::STOP_LIMIT:
      return roq::OrderType::LIMIT;
    case OrdType::MARKET_IF_TOUCHED:
      return roq::OrderType::MARKET;
    case OrdType::LIMIT_IF_TOUCHED:
      return roq::OrderType::LIMIT;
  }
  return roq::OrderType{};
}

inline roq::TimeInForce map(json::TimeInForce state) {
  switch (state) {
    case TimeInForce::UNDEFINED:
      break;
    case TimeInForce::UNKNOWN:
      break;
    case TimeInForce::AT_THE_CLOSE:
      return roq::TimeInForce::AT_THE_CLOSE;
    case TimeInForce::DAY:
      return roq::TimeInForce::GFD;
    case TimeInForce::FILL_OR_KILL:
      return roq::TimeInForce::FOK;
    case TimeInForce::GOOD_TILL_CANCEL:
      return roq::TimeInForce::GTC;
    case TimeInForce::IMMEDIATE_OR_CANCEL:
      return roq::TimeInForce::IOC;
  }
  return roq::TimeInForce{};
}

inline roq::Liquidity map(json::LiquidityInd state) {
  switch (state) {
    case LiquidityInd::UNDEFINED:
      break;
    case LiquidityInd::UNKNOWN:
      break;
    case LiquidityInd::ADDED_LIQUIDITY:
      return roq::Liquidity::MAKER;
    case LiquidityInd::REMOVED_LIQUIDITY:
      return roq::Liquidity::TAKER;
  }
  return roq::Liquidity{};
}

// roq => bitmex

inline json::Side map(roq::Side side) {
  switch (side) {
    case roq::Side::UNDEFINED:
      break;
    case roq::Side::BUY:
      return json::Side::BUY;
    case roq::Side::SELL:
      return json::Side::SELL;
  }
  return json::Side::UNDEFINED;
}

inline json::OrdType map(roq::OrderType order_type) {
  switch (order_type) {
    case roq::OrderType::UNDEFINED:
      break;
    case roq::OrderType::MARKET:
      return json::OrdType::MARKET;
    case roq::OrderType::LIMIT:
      return json::OrdType::LIMIT;
  }
  return json::OrdType::UNDEFINED;
}

inline json::TimeInForce map(roq::TimeInForce time_in_force) {
  switch (time_in_force) {
    case roq::TimeInForce::UNDEFINED:
      break;
    case roq::TimeInForce::GFD:
      return json::TimeInForce::DAY;
    case roq::TimeInForce::GTC:
      return json::TimeInForce::GOOD_TILL_CANCEL;
    case roq::TimeInForce::OPG:
      break;
    case roq::TimeInForce::IOC:
      return json::TimeInForce::IMMEDIATE_OR_CANCEL;
    case roq::TimeInForce::FOK:
      return json::TimeInForce::FILL_OR_KILL;
    case roq::TimeInForce::GTD:
      break;
    case roq::TimeInForce::GTX:
      break;
    case roq::TimeInForce::AT_THE_CLOSE:
      return json::TimeInForce::AT_THE_CLOSE;
    case roq::TimeInForce::GOOD_THROUGH_CROSSING:
      break;
    case roq::TimeInForce::AT_CROSSING:
      break;
    case roq::TimeInForce::GOOD_FOR_TIME:
      break;
    case roq::TimeInForce::GFA:
      break;
    case roq::TimeInForce::GFM:
      break;
  }
  return json::TimeInForce::UNDEFINED;
}

inline json::ExecInst map(roq::ExecutionInstruction execution_instruction) {
  switch (execution_instruction) {
    case roq::ExecutionInstruction::UNDEFINED:
      break;
    case roq::ExecutionInstruction::PARTICIPATE_DO_NOT_INITIATE:
      return json::ExecInst::PARTICIPATE_DO_NOT_INITIATE;
    case roq::ExecutionInstruction::CANCEL_IF_NOT_BEST:
      break;
    case roq::ExecutionInstruction::DO_NOT_INCREASE:
      return json::ExecInst::REDUCE_ONLY;
    case roq::ExecutionInstruction::DO_NOT_REDUCE:
      break;
  }
  return json::ExecInst::UNDEFINED;
}

extern roq::Error guess_error(const std::string_view &message);

}  // namespace json
}  // namespace bitmex
}  // namespace roq
