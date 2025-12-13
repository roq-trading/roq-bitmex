/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/bitmex/json/liquidity_ind.hpp"
#include "roq/bitmex/json/ord_status.hpp"
#include "roq/bitmex/json/ord_type.hpp"
#include "roq/bitmex/json/side.hpp"
#include "roq/bitmex/json/state.hpp"
#include "roq/bitmex/json/time_in_force.hpp"

#include "roq/liquidity.hpp"
#include "roq/order_status.hpp"
#include "roq/order_type.hpp"
#include "roq/side.hpp"
#include "roq/time_in_force.hpp"
#include "roq/trading_status.hpp"

#include "roq/map.hpp"

namespace roq {

template <>
template <>
std::optional<Liquidity> Map<bitmex::json::LiquidityInd>::helper() const;

template <>
template <>
std::optional<OrderStatus> Map<bitmex::json::OrdStatus>::helper() const;

template <>
template <>
std::optional<OrderType> Map<bitmex::json::OrdType>::helper() const;

template <>
template <>
std::optional<Side> Map<bitmex::json::Side>::helper() const;

template <>
template <>
std::optional<TradingStatus> Map<bitmex::json::State>::helper() const;

template <>
template <>
std::optional<TimeInForce> Map<bitmex::json::TimeInForce>::helper() const;

// ===

template <>
template <>
std::optional<bitmex::json::OrdType> Map<OrderType>::helper() const;

template <>
template <>
std::optional<bitmex::json::Side> Map<Side>::helper() const;

template <>
template <>
std::optional<bitmex::json::TimeInForce> Map<TimeInForce>::helper() const;

}  // namespace roq
