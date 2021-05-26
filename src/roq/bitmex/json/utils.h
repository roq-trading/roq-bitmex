/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <chrono>

#include "roq/core/utility.h"

#include "roq/core/json/parser.h"

#include "roq/core/charconv/datetime.h"

#include "roq/bitmex/json/exec_inst.h"
#include "roq/bitmex/json/exec_type.h"
#include "roq/bitmex/json/liquidity_ind.h"
#include "roq/bitmex/json/multi_leg_reporting_type.h"
#include "roq/bitmex/json/ord_status.h"
#include "roq/bitmex/json/ord_type.h"
#include "roq/bitmex/json/settlement_type.h"
#include "roq/bitmex/json/side.h"
#include "roq/bitmex/json/state.h"
#include "roq/bitmex/json/time_in_force.h"
#include "roq/bitmex/json/typ.h"

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

// json

template <>
inline void update(ExecInst &result, const core::json::value_t &value) {
  using result_type = std::remove_reference<decltype(result)>::type;
  result = result_type(core::json::get<std::string_view>(value));
}

template <>
inline void update(ExecType &result, const core::json::value_t &value) {
  using result_type = std::remove_reference<decltype(result)>::type;
  result = result_type(core::json::get<std::string_view>(value));
}

template <>
inline void update(LiquidityInd &result, const core::json::value_t &value) {
  using result_type = std::remove_reference<decltype(result)>::type;
  result = result_type(core::json::get<std::string_view>(value));
}

template <>
inline void update(MultiLegReportingType &result, const core::json::value_t &value) {
  using result_type = std::remove_reference<decltype(result)>::type;
  result = result_type(core::json::get<std::string_view>(value));
}

template <>
inline void update(OrdStatus &result, const core::json::value_t &value) {
  using result_type = std::remove_reference<decltype(result)>::type;
  result = result_type(core::json::get<std::string_view>(value));
}

template <>
inline void update(OrdType &result, const core::json::value_t &value) {
  using result_type = std::remove_reference<decltype(result)>::type;
  result = result_type(core::json::get<std::string_view>(value));
}

template <>
inline void update(SettlementType &result, const core::json::value_t &value) {
  using result_type = std::remove_reference<decltype(result)>::type;
  result = result_type(core::json::get<std::string_view>(value));
}

template <>
inline void update(Side &result, const core::json::value_t &value) {
  using result_type = std::remove_reference<decltype(result)>::type;
  result = result_type(core::json::get<std::string_view>(value));
}

template <>
inline void update(State &result, const core::json::value_t &value) {
  using result_type = std::remove_reference<decltype(result)>::type;
  result = result_type(core::json::get<std::string_view>(value));
}

template <>
inline void update(TimeInForce &result, const core::json::value_t &value) {
  using result_type = std::remove_reference<decltype(result)>::type;
  result = result_type(core::json::get<std::string_view>(value));
}

template <>
inline void update(Typ &result, const core::json::value_t &value) {
  using result_type = std::remove_reference<decltype(result)>::type;
  result = result_type(core::json::get<std::string_view>(value));
}

// utils

// bitmex => roq

inline roq::TradingStatus map(json::State state) {
  switch (state) {
    case json::State::UNDEFINED:
      break;
    case json::State::UNKNOWN:
      break;
    case json::State::CLOSED:
      return roq::TradingStatus::CLOSED;
    case json::State::OPEN:
      return roq::TradingStatus::OPEN;
    case json::State::SETTLED:
      break;
    case json::State::UNLISTED:
      break;
  }
  return roq::TradingStatus::UNDEFINED;
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
    case OrdStatus::NEW:
      return roq::OrderStatus::WORKING;
    case OrdStatus::FILLED:
      return roq::OrderStatus::COMPLETED;
    case OrdStatus::CANCELED:
      return roq::OrderStatus::CANCELED;
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
    case TimeInForce::GOOD_TILL_CANCEL:
      return roq::TimeInForce::GTC;
    case TimeInForce::AT_THE_CLOSE:
      return roq::TimeInForce::GFD;  // HANS not correct
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
    case roq::TimeInForce::FOK:
      break;
    case roq::TimeInForce::IOC:
      break;
    case roq::TimeInForce::GFD:
      break;
    case roq::TimeInForce::GTC:
      return json::TimeInForce::GOOD_TILL_CANCEL;
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

}  // namespace json
}  // namespace bitmex
}  // namespace roq
