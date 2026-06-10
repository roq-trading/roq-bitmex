/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/bitmex/protocol/json/map.hpp"

using namespace std::literals;

namespace roq {

namespace {
template <typename... Args>
using Helper = detail::MapHelper<Args...>;
}

// bitmex::json => roq

// bitmex::protocol::json::LiquidityInd => roq::Liquidity

template <>
template <>
constexpr Helper<bitmex::protocol::json::LiquidityInd>::operator std::optional<roq::Liquidity>() const {
  switch (std::get<0>(args_)) {
    using enum bitmex::protocol::json::LiquidityInd::type_t;
    case UNDEFINED_INTERNAL:
      return roq::Liquidity::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::Liquidity::UNDEFINED;
    case ADDED_LIQUIDITY:
      return roq::Liquidity::MAKER;
    case REMOVED_LIQUIDITY:
      return roq::Liquidity::TAKER;
  }
  return {};
}

static_assert(Helper{bitmex::protocol::json::LiquidityInd{bitmex::protocol::json::LiquidityInd::UNDEFINED_INTERNAL}} == roq::Liquidity::UNDEFINED);
static_assert(Helper{bitmex::protocol::json::LiquidityInd{bitmex::protocol::json::LiquidityInd::ADDED_LIQUIDITY}} == roq::Liquidity::MAKER);
static_assert(Helper{bitmex::protocol::json::LiquidityInd{bitmex::protocol::json::LiquidityInd::REMOVED_LIQUIDITY}} == roq::Liquidity::TAKER);

template <>
template <>
std::optional<roq::Liquidity> Map<bitmex::protocol::json::LiquidityInd>::helper() const {
  return Helper{args_};
}

// bitmex::protocol::json::OrdStatus ==> roq::OrderStatus

template <>
template <>
constexpr Helper<bitmex::protocol::json::OrdStatus>::operator std::optional<roq::OrderStatus>() const {
  switch (std::get<0>(args_)) {
    using enum bitmex::protocol::json::OrdStatus::type_t;
    case UNDEFINED_INTERNAL:
      return roq::OrderStatus::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::OrderStatus::UNDEFINED;
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
    case PENDING_CANCEL:
      return roq::OrderStatus::UNDEFINED;
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
  return {};
}

static_assert(Helper{bitmex::protocol::json::OrdStatus{bitmex::protocol::json::OrdStatus::UNDEFINED_INTERNAL}} == roq::OrderStatus::UNDEFINED);
static_assert(Helper{bitmex::protocol::json::OrdStatus{bitmex::protocol::json::OrdStatus::CANCELED}} == roq::OrderStatus::CANCELED);
static_assert(Helper{bitmex::protocol::json::OrdStatus{bitmex::protocol::json::OrdStatus::DONE_FOR_DAY}} == roq::OrderStatus::SUSPENDED);
static_assert(Helper{bitmex::protocol::json::OrdStatus{bitmex::protocol::json::OrdStatus::EXPIRED}} == roq::OrderStatus::EXPIRED);
static_assert(Helper{bitmex::protocol::json::OrdStatus{bitmex::protocol::json::OrdStatus::FILLED}} == roq::OrderStatus::COMPLETED);
static_assert(Helper{bitmex::protocol::json::OrdStatus{bitmex::protocol::json::OrdStatus::NEW}} == roq::OrderStatus::WORKING);
static_assert(Helper{bitmex::protocol::json::OrdStatus{bitmex::protocol::json::OrdStatus::PARTIALLY_FILLED}} == roq::OrderStatus::WORKING);
static_assert(Helper{bitmex::protocol::json::OrdStatus{bitmex::protocol::json::OrdStatus::PENDING_CANCEL}} == roq::OrderStatus::UNDEFINED);
static_assert(Helper{bitmex::protocol::json::OrdStatus{bitmex::protocol::json::OrdStatus::PENDING_NEW}} == roq::OrderStatus::SENT);
static_assert(Helper{bitmex::protocol::json::OrdStatus{bitmex::protocol::json::OrdStatus::REJECTED}} == roq::OrderStatus::REJECTED);
static_assert(Helper{bitmex::protocol::json::OrdStatus{bitmex::protocol::json::OrdStatus::STOPPED}} == roq::OrderStatus::STOPPED);
static_assert(Helper{bitmex::protocol::json::OrdStatus{bitmex::protocol::json::OrdStatus::UNTRIGGERED}} == roq::OrderStatus::ACCEPTED);
static_assert(Helper{bitmex::protocol::json::OrdStatus{bitmex::protocol::json::OrdStatus::TRIGGERED}} == roq::OrderStatus::WORKING);

template <>
template <>
std::optional<roq::OrderStatus> Map<bitmex::protocol::json::OrdStatus>::helper() const {
  return Helper{args_};
}

// bitmex::protocol::json::OrdType ==> roq::OrderType

template <>
template <>
constexpr Helper<bitmex::protocol::json::OrdType>::operator std::optional<roq::OrderType>() const {
  switch (std::get<0>(args_)) {
    using enum bitmex::protocol::json::OrdType::type_t;
    case UNDEFINED_INTERNAL:
      return roq::OrderType::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::OrderType::UNDEFINED;
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
  return {};
}

static_assert(Helper{bitmex::protocol::json::OrdType{bitmex::protocol::json::OrdType::UNDEFINED_INTERNAL}} == roq::OrderType::UNDEFINED);
static_assert(Helper{bitmex::protocol::json::OrdType{bitmex::protocol::json::OrdType::LIMIT}} == roq::OrderType::LIMIT);
static_assert(Helper{bitmex::protocol::json::OrdType{bitmex::protocol::json::OrdType::MARKET}} == roq::OrderType::MARKET);
static_assert(Helper{bitmex::protocol::json::OrdType{bitmex::protocol::json::OrdType::STOP}} == roq::OrderType::MARKET);
static_assert(Helper{bitmex::protocol::json::OrdType{bitmex::protocol::json::OrdType::STOP_LIMIT}} == roq::OrderType::LIMIT);
static_assert(Helper{bitmex::protocol::json::OrdType{bitmex::protocol::json::OrdType::MARKET_IF_TOUCHED}} == roq::OrderType::MARKET);
static_assert(Helper{bitmex::protocol::json::OrdType{bitmex::protocol::json::OrdType::LIMIT_IF_TOUCHED}} == roq::OrderType::LIMIT);

template <>
template <>
std::optional<roq::OrderType> Map<bitmex::protocol::json::OrdType>::helper() const {
  return Helper{args_};
}

// bitmex::protocol::json::Side ==> roq::Side

template <>
template <>
constexpr Helper<bitmex::protocol::json::Side>::operator std::optional<roq::Side>() const {
  switch (std::get<0>(args_)) {
    using enum bitmex::protocol::json::Side::type_t;
    case UNDEFINED_INTERNAL:
      return roq::Side::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::Side::UNDEFINED;
    case BUY:
      return roq::Side::BUY;
    case SELL:
      return roq::Side::SELL;
  }
  return {};
}

static_assert(Helper{bitmex::protocol::json::Side{bitmex::protocol::json::Side::UNDEFINED_INTERNAL}} == roq::Side::UNDEFINED);
static_assert(Helper{bitmex::protocol::json::Side{bitmex::protocol::json::Side::BUY}} == roq::Side::BUY);
static_assert(Helper{bitmex::protocol::json::Side{bitmex::protocol::json::Side::SELL}} == roq::Side::SELL);

template <>
template <>
std::optional<roq::Side> Map<bitmex::protocol::json::Side>::helper() const {
  return Helper{args_};
}

// bitmex::protocol::json::State ==> roq::TradingStatus

template <>
template <>
constexpr Helper<bitmex::protocol::json::State>::operator std::optional<roq::TradingStatus>() const {
  switch (std::get<0>(args_)) {
    using enum bitmex::protocol::json::State::type_t;
    case UNDEFINED_INTERNAL:
      return roq::TradingStatus::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::TradingStatus::UNDEFINED;
    case OPEN:
      return roq::TradingStatus::OPEN;
    case CLOSED:
      return roq::TradingStatus::CLOSE;
    case SETTLED:
      return roq::TradingStatus::UNDEFINED;
    case UNLISTED:
      return roq::TradingStatus::UNDEFINED;
    case EXPIRED:
      return roq::TradingStatus::UNDEFINED;
    case CLEARED:
      return roq::TradingStatus::UNDEFINED;
  }
  return {};
}

static_assert(Helper{bitmex::protocol::json::State{bitmex::protocol::json::State::UNDEFINED_INTERNAL}} == roq::TradingStatus::UNDEFINED);
static_assert(Helper{bitmex::protocol::json::State{bitmex::protocol::json::State::OPEN}} == roq::TradingStatus::OPEN);
static_assert(Helper{bitmex::protocol::json::State{bitmex::protocol::json::State::CLOSED}} == roq::TradingStatus::CLOSE);
static_assert(Helper{bitmex::protocol::json::State{bitmex::protocol::json::State::SETTLED}} == roq::TradingStatus::UNDEFINED);
static_assert(Helper{bitmex::protocol::json::State{bitmex::protocol::json::State::UNLISTED}} == roq::TradingStatus::UNDEFINED);
static_assert(Helper{bitmex::protocol::json::State{bitmex::protocol::json::State::EXPIRED}} == roq::TradingStatus::UNDEFINED);

template <>
template <>
std::optional<roq::TradingStatus> Map<bitmex::protocol::json::State>::helper() const {
  return Helper{args_};
}

// bitmex::protocol::json::TimeInForce ==> roq::TimeInForce

template <>
template <>
constexpr Helper<bitmex::protocol::json::TimeInForce>::operator std::optional<roq::TimeInForce>() const {
  switch (std::get<0>(args_)) {
    using enum bitmex::protocol::json::TimeInForce::type_t;
    case UNDEFINED_INTERNAL:
      return roq::TimeInForce::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::TimeInForce::UNDEFINED;
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
  return {};
}

static_assert(Helper{bitmex::protocol::json::TimeInForce{bitmex::protocol::json::TimeInForce::UNDEFINED_INTERNAL}} == roq::TimeInForce::UNDEFINED);
static_assert(Helper{bitmex::protocol::json::TimeInForce{bitmex::protocol::json::TimeInForce::AT_THE_CLOSE}} == roq::TimeInForce::AT_THE_CLOSE);
static_assert(Helper{bitmex::protocol::json::TimeInForce{bitmex::protocol::json::TimeInForce::DAY}} == roq::TimeInForce::GFD);
static_assert(Helper{bitmex::protocol::json::TimeInForce{bitmex::protocol::json::TimeInForce::FILL_OR_KILL}} == roq::TimeInForce::FOK);
static_assert(Helper{bitmex::protocol::json::TimeInForce{bitmex::protocol::json::TimeInForce::GOOD_TILL_CANCEL}} == roq::TimeInForce::GTC);
static_assert(Helper{bitmex::protocol::json::TimeInForce{bitmex::protocol::json::TimeInForce::IMMEDIATE_OR_CANCEL}} == roq::TimeInForce::IOC);

template <>
template <>
std::optional<roq::TimeInForce> Map<bitmex::protocol::json::TimeInForce>::helper() const {
  return Helper{args_};
}

// roq => bitmex::json

// roq::OrderType ==> bitmex::protocol::json::OrdType

template <>
template <>
constexpr Helper<roq::OrderType>::operator std::optional<bitmex::protocol::json::OrdType>() const {
  switch (std::get<0>(args_)) {
    using enum roq::OrderType;
    case UNDEFINED:
      return bitmex::protocol::json::OrdType::UNDEFINED_INTERNAL;
    case MARKET:
      return bitmex::protocol::json::OrdType::MARKET;
    case LIMIT:
      return bitmex::protocol::json::OrdType::LIMIT;
  }
  return {};
}

static_assert(Helper{roq::OrderType::UNDEFINED} == bitmex::protocol::json::OrdType{bitmex::protocol::json::OrdType::UNDEFINED_INTERNAL});
static_assert(Helper{roq::OrderType::MARKET} == bitmex::protocol::json::OrdType{bitmex::protocol::json::OrdType::MARKET});
static_assert(Helper{roq::OrderType::LIMIT} == bitmex::protocol::json::OrdType{bitmex::protocol::json::OrdType::LIMIT});

template <>
template <>
std::optional<bitmex::protocol::json::OrdType> Map<OrderType>::helper() const {
  return Helper{args_};
}

// roq::Side ==> bitmex::protocol::json::Side

template <>
template <>
constexpr Helper<roq::Side>::operator std::optional<bitmex::protocol::json::Side>() const {
  switch (std::get<0>(args_)) {
    using enum roq::Side;
    case UNDEFINED:
      return bitmex::protocol::json::Side::UNDEFINED_INTERNAL;
    case BUY:
      return bitmex::protocol::json::Side::BUY;
    case SELL:
      return bitmex::protocol::json::Side::SELL;
  }
  return {};
}

static_assert(Helper{roq::Side::UNDEFINED} == bitmex::protocol::json::Side{bitmex::protocol::json::Side::UNDEFINED_INTERNAL});
static_assert(Helper{roq::Side::BUY} == bitmex::protocol::json::Side{bitmex::protocol::json::Side::BUY});
static_assert(Helper{roq::Side::SELL} == bitmex::protocol::json::Side{bitmex::protocol::json::Side::SELL});

template <>
template <>
std::optional<bitmex::protocol::json::Side> Map<Side>::helper() const {
  return Helper{args_};
}

// roq::TimeInForce ==> bitmex::protocol::json::TimeInForce

template <>
template <>
constexpr Helper<roq::TimeInForce>::operator std::optional<bitmex::protocol::json::TimeInForce>() const {
  switch (std::get<0>(args_)) {
    using enum roq::TimeInForce;
    case UNDEFINED:
      return bitmex::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case GFD:
      return bitmex::protocol::json::TimeInForce::DAY;
    case GTC:
      return bitmex::protocol::json::TimeInForce::GOOD_TILL_CANCEL;
    case OPG:
      return bitmex::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case IOC:
      return bitmex::protocol::json::TimeInForce::IMMEDIATE_OR_CANCEL;
    case FOK:
      return bitmex::protocol::json::TimeInForce::FILL_OR_KILL;
    case GTD:
      return bitmex::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case GTX:
      return bitmex::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case AT_THE_CLOSE:
      return bitmex::protocol::json::TimeInForce::AT_THE_CLOSE;
    case GOOD_THROUGH_CROSSING:
      return bitmex::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case AT_CROSSING:
      return bitmex::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case GOOD_FOR_TIME:
      return bitmex::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case GFA:
      return bitmex::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case GFM:
      return bitmex::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
  }
  return {};
}

static_assert(Helper{roq::TimeInForce::UNDEFINED} == bitmex::protocol::json::TimeInForce{bitmex::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GFD} == bitmex::protocol::json::TimeInForce{bitmex::protocol::json::TimeInForce::DAY});
static_assert(Helper{roq::TimeInForce::GTC} == bitmex::protocol::json::TimeInForce{bitmex::protocol::json::TimeInForce::GOOD_TILL_CANCEL});
static_assert(Helper{roq::TimeInForce::OPG} == bitmex::protocol::json::TimeInForce{bitmex::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::IOC} == bitmex::protocol::json::TimeInForce{bitmex::protocol::json::TimeInForce::IMMEDIATE_OR_CANCEL});
static_assert(Helper{roq::TimeInForce::FOK} == bitmex::protocol::json::TimeInForce{bitmex::protocol::json::TimeInForce::FILL_OR_KILL});
static_assert(Helper{roq::TimeInForce::GTD} == bitmex::protocol::json::TimeInForce{bitmex::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GTX} == bitmex::protocol::json::TimeInForce{bitmex::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::AT_THE_CLOSE} == bitmex::protocol::json::TimeInForce{bitmex::protocol::json::TimeInForce::AT_THE_CLOSE});
static_assert(Helper{roq::TimeInForce::GOOD_THROUGH_CROSSING} == bitmex::protocol::json::TimeInForce{bitmex::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::AT_CROSSING} == bitmex::protocol::json::TimeInForce{bitmex::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GOOD_FOR_TIME} == bitmex::protocol::json::TimeInForce{bitmex::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GFA} == bitmex::protocol::json::TimeInForce{bitmex::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GFM} == bitmex::protocol::json::TimeInForce{bitmex::protocol::json::TimeInForce::UNDEFINED_INTERNAL});

template <>
template <>
std::optional<bitmex::protocol::json::TimeInForce> Map<TimeInForce>::helper() const {
  return Helper{args_};
}

}  // namespace roq
