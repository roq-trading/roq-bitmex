/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <chrono>

#include "roq/core/utility.hpp"

#include "roq/core/json/parser.hpp"

#include "roq/core/charconv/datetime.hpp"

#include "roq/bitmex/json/exec_inst.hpp"
#include "roq/bitmex/json/liquidity_ind.hpp"
#include "roq/bitmex/json/ord_status.hpp"
#include "roq/bitmex/json/ord_type.hpp"
#include "roq/bitmex/json/side.hpp"
#include "roq/bitmex/json/state.hpp"
#include "roq/bitmex/json/time_in_force.hpp"

namespace roq {
namespace bitmex {
namespace json {

template <typename T>
inline void update(T &result, core::json::Value const &value) {
  result = core::json::get<T>(value);
}

template <>
inline void update(std::chrono::milliseconds &result, core::json::Value const &value) {
  std::visit(
      overloaded{
          [&](core::json::Null const &) { result = std::chrono::milliseconds{}; },
          [](bool) { throw std::bad_cast{}; },
          [&](int64_t value) { result = std::chrono::milliseconds{static_cast<uint64_t>(value * int64_t{1000})}; },
          [&](double value) { result = std::chrono::milliseconds{static_cast<uint64_t>(value * 1.0e3)}; },
          [&](std::string_view const &value) {
            result = core::charconv::datetime_from_string<std::remove_reference<decltype(result)>::type>(value);
          },
          [](core::json::Object const &) { throw std::bad_cast{}; },
          [](core::json::Array const &) { throw std::bad_cast{}; },
      },
      value);
}

// bitmex => roq

inline roq::TradingStatus map(json::State state) {
  switch (state) {
    using enum json::State::type_t;
    case UNDEFINED:
      break;
    case UNKNOWN:
      break;
    case OPEN:
      return roq::TradingStatus::OPEN;
    case CLOSED:
      return roq::TradingStatus::CLOSE;
    case SETTLED:
      break;
    case UNLISTED:
      break;
    case EXPIRED:
      break;
  }
  return {};
}

inline roq::Side map(json::Side side) {
  switch (side) {
    using enum json::Side::type_t;
    case UNDEFINED:
      break;
    case UNKNOWN:
      break;
    case BUY:
      return roq::Side::BUY;
    case SELL:
      return roq::Side::SELL;
  }
  return roq::Side::UNDEFINED;
}

inline roq::OrderStatus map(json::OrdStatus state) {
  switch (state) {
    using enum OrdStatus::type_t;
    case UNDEFINED:
      break;
    case UNKNOWN:
      break;
    case CANCELED:
      return roq::OrderStatus::CANCELED;
    case DONE_FOR_DAY:
      return roq::OrderStatus::SUSPENDED;
    case EXPIRED:
      return roq::OrderStatus::EXPIRED;
    case FILLED:
      return roq::OrderStatus::COMPLETED;
    case NEW:
      return roq::OrderStatus::WORKING;
    case PARTIALLY_FILLED:
      return roq::OrderStatus::WORKING;
    case PENDING_CANCEL:  // XXX HANS what to do?
      break;
    case PENDING_NEW:
      return roq::OrderStatus::SENT;
    case REJECTED:
      return roq::OrderStatus::REJECTED;
    case STOPPED:
      return roq::OrderStatus::STOPPED;
    case UNTRIGGERED:
      return roq::OrderStatus::ACCEPTED;
    case TRIGGERED:
      return roq::OrderStatus::WORKING;
  }
  return roq::OrderStatus::UNDEFINED;
}

inline roq::OrderType map(json::OrdType state) {
  switch (state) {
    using enum OrdType::type_t;
    case UNDEFINED:
      break;
    case UNKNOWN:
      break;
    case LIMIT:
      return roq::OrderType::LIMIT;
    case MARKET:
      return roq::OrderType::MARKET;
    case STOP:
      return roq::OrderType::MARKET;
    case STOP_LIMIT:
      return roq::OrderType::LIMIT;
    case MARKET_IF_TOUCHED:
      return roq::OrderType::MARKET;
    case LIMIT_IF_TOUCHED:
      return roq::OrderType::LIMIT;
  }
  return roq::OrderType{};
}

inline roq::TimeInForce map(json::TimeInForce state) {
  switch (state) {
    using enum TimeInForce::type_t;
    case UNDEFINED:
      break;
    case UNKNOWN:
      break;
    case AT_THE_CLOSE:
      return roq::TimeInForce::AT_THE_CLOSE;
    case DAY:
      return roq::TimeInForce::GFD;
    case FILL_OR_KILL:
      return roq::TimeInForce::FOK;
    case GOOD_TILL_CANCEL:
      return roq::TimeInForce::GTC;
    case IMMEDIATE_OR_CANCEL:
      return roq::TimeInForce::IOC;
  }
  return roq::TimeInForce{};
}

inline roq::Liquidity map(json::LiquidityInd state) {
  switch (state) {
    using enum LiquidityInd::type_t;
    case UNDEFINED:
      break;
    case UNKNOWN:
      break;
    case ADDED_LIQUIDITY:
      return roq::Liquidity::MAKER;
    case REMOVED_LIQUIDITY:
      return roq::Liquidity::TAKER;
  }
  return roq::Liquidity{};
}

// roq => bitmex

inline json::Side map(roq::Side side) {
  switch (side) {
    using enum roq::Side;
    case UNDEFINED:
      break;
    case BUY:
      return json::Side::BUY;
    case SELL:
      return json::Side::SELL;
  }
  return json::Side::UNDEFINED;
}

inline json::OrdType map(roq::OrderType order_type) {
  switch (order_type) {
    using enum roq::OrderType;
    case UNDEFINED:
      break;
    case MARKET:
      return json::OrdType::MARKET;
    case LIMIT:
      return json::OrdType::LIMIT;
  }
  return json::OrdType::UNDEFINED;
}

inline json::TimeInForce map(roq::TimeInForce time_in_force) {
  switch (time_in_force) {
    using enum roq::TimeInForce;
    case UNDEFINED:
      break;
    case GFD:
      return json::TimeInForce::DAY;
    case GTC:
      return json::TimeInForce::GOOD_TILL_CANCEL;
    case OPG:
      break;
    case IOC:
      return json::TimeInForce::IMMEDIATE_OR_CANCEL;
    case FOK:
      return json::TimeInForce::FILL_OR_KILL;
    case GTD:
      break;
    case GTX:
      break;
    case AT_THE_CLOSE:
      return json::TimeInForce::AT_THE_CLOSE;
    case GOOD_THROUGH_CROSSING:
      break;
    case AT_CROSSING:
      break;
    case GOOD_FOR_TIME:
      break;
    case GFA:
      break;
    case GFM:
      break;
  }
  return json::TimeInForce::UNDEFINED;
}

inline json::ExecInst map(Mask<roq::ExecutionInstruction> execution_instructions) {
  // XXX support multiple?
  if (execution_instructions.has(roq::ExecutionInstruction::PARTICIPATE_DO_NOT_INITIATE))
    return json::ExecInst::PARTICIPATE_DO_NOT_INITIATE;
  if (execution_instructions.has(roq::ExecutionInstruction::DO_NOT_INCREASE))
    return json::ExecInst::REDUCE_ONLY;
  return json::ExecInst::UNDEFINED;
}

extern roq::Error guess_error(std::string_view const &message);

}  // namespace json
}  // namespace bitmex
}  // namespace roq
