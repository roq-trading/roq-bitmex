/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/bitmex/protocol/json/liquidity_ind.hpp"
#include "roq/bitmex/protocol/json/ord_status.hpp"
#include "roq/bitmex/protocol/json/ord_type.hpp"
#include "roq/bitmex/protocol/json/side.hpp"
#include "roq/bitmex/protocol/json/state.hpp"
#include "roq/bitmex/protocol/json/time_in_force.hpp"

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
std::optional<Liquidity> Map<bitmex::protocol::json::LiquidityInd>::helper() const;

template <>
template <>
std::optional<OrderStatus> Map<bitmex::protocol::json::OrdStatus>::helper() const;

template <>
template <>
std::optional<OrderType> Map<bitmex::protocol::json::OrdType>::helper() const;

template <>
template <>
std::optional<Side> Map<bitmex::protocol::json::Side>::helper() const;

template <>
template <>
std::optional<TradingStatus> Map<bitmex::protocol::json::State>::helper() const;

template <>
template <>
std::optional<TimeInForce> Map<bitmex::protocol::json::TimeInForce>::helper() const;

// ===

template <>
template <>
std::optional<bitmex::protocol::json::OrdType> Map<OrderType>::helper() const;

template <>
template <>
std::optional<bitmex::protocol::json::Side> Map<Side>::helper() const;

template <>
template <>
std::optional<bitmex::protocol::json::TimeInForce> Map<TimeInForce>::helper() const;

}  // namespace roq
