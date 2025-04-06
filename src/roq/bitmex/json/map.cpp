/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/bitmex/json/map.hpp"

using namespace std::literals;

namespace roq {

namespace {
template <typename... Args>
using Helper = detail::MapHelper<Args...>;
}

// bitmex::json => roq

// bitmex::json::LiquidityInd => roq::Liquidity

template <>
template <>
constexpr Helper<bitmex::json::LiquidityInd>::operator std::optional<roq::Liquidity>() const {
  switch (std::get<0>(args_)) {
    using enum bitmex::json::LiquidityInd::type_t;
    case UNDEFINED__:
      return roq::Liquidity::UNDEFINED;
    case UNKNOWN__:
      return roq::Liquidity::UNDEFINED;
    case ADDED_LIQUIDITY:
      return roq::Liquidity::MAKER;
    case REMOVED_LIQUIDITY:
      return roq::Liquidity::TAKER;
  }
  return {};
}

static_assert(Helper{bitmex::json::LiquidityInd{bitmex::json::LiquidityInd::UNDEFINED__}} == roq::Liquidity::UNDEFINED);
static_assert(Helper{bitmex::json::LiquidityInd{bitmex::json::LiquidityInd::ADDED_LIQUIDITY}} == roq::Liquidity::MAKER);
static_assert(Helper{bitmex::json::LiquidityInd{bitmex::json::LiquidityInd::REMOVED_LIQUIDITY}} == roq::Liquidity::TAKER);

template <>
template <>
std::optional<roq::Liquidity> Map<bitmex::json::LiquidityInd>::helper() const {
  return Helper{args_};
}

// bitmex::json::OrdStatus ==> roq::OrderStatus

template <>
template <>
constexpr Helper<bitmex::json::OrdStatus>::operator std::optional<roq::OrderStatus>() const {
  switch (std::get<0>(args_)) {
    using enum bitmex::json::OrdStatus::type_t;
    case UNDEFINED__:
      return roq::OrderStatus::UNDEFINED;
    case UNKNOWN__:
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

static_assert(Helper{bitmex::json::OrdStatus{bitmex::json::OrdStatus::UNDEFINED__}} == roq::OrderStatus::UNDEFINED);
static_assert(Helper{bitmex::json::OrdStatus{bitmex::json::OrdStatus::CANCELED}} == roq::OrderStatus::CANCELED);
static_assert(Helper{bitmex::json::OrdStatus{bitmex::json::OrdStatus::DONE_FOR_DAY}} == roq::OrderStatus::SUSPENDED);
static_assert(Helper{bitmex::json::OrdStatus{bitmex::json::OrdStatus::EXPIRED}} == roq::OrderStatus::EXPIRED);
static_assert(Helper{bitmex::json::OrdStatus{bitmex::json::OrdStatus::FILLED}} == roq::OrderStatus::COMPLETED);
static_assert(Helper{bitmex::json::OrdStatus{bitmex::json::OrdStatus::NEW}} == roq::OrderStatus::WORKING);
static_assert(Helper{bitmex::json::OrdStatus{bitmex::json::OrdStatus::PARTIALLY_FILLED}} == roq::OrderStatus::WORKING);
static_assert(Helper{bitmex::json::OrdStatus{bitmex::json::OrdStatus::PENDING_CANCEL}} == roq::OrderStatus::UNDEFINED);
static_assert(Helper{bitmex::json::OrdStatus{bitmex::json::OrdStatus::PENDING_NEW}} == roq::OrderStatus::SENT);
static_assert(Helper{bitmex::json::OrdStatus{bitmex::json::OrdStatus::REJECTED}} == roq::OrderStatus::REJECTED);
static_assert(Helper{bitmex::json::OrdStatus{bitmex::json::OrdStatus::STOPPED}} == roq::OrderStatus::STOPPED);
static_assert(Helper{bitmex::json::OrdStatus{bitmex::json::OrdStatus::UNTRIGGERED}} == roq::OrderStatus::ACCEPTED);
static_assert(Helper{bitmex::json::OrdStatus{bitmex::json::OrdStatus::TRIGGERED}} == roq::OrderStatus::WORKING);

template <>
template <>
std::optional<roq::OrderStatus> Map<bitmex::json::OrdStatus>::helper() const {
  return Helper{args_};
}

// bitmex::json::OrdType ==> roq::OrderType

template <>
template <>
constexpr Helper<bitmex::json::OrdType>::operator std::optional<roq::OrderType>() const {
  switch (std::get<0>(args_)) {
    using enum bitmex::json::OrdType::type_t;
    case UNDEFINED__:
      return roq::OrderType::UNDEFINED;
    case UNKNOWN__:
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

static_assert(Helper{bitmex::json::OrdType{bitmex::json::OrdType::UNDEFINED__}} == roq::OrderType::UNDEFINED);
static_assert(Helper{bitmex::json::OrdType{bitmex::json::OrdType::LIMIT}} == roq::OrderType::LIMIT);
static_assert(Helper{bitmex::json::OrdType{bitmex::json::OrdType::MARKET}} == roq::OrderType::MARKET);
static_assert(Helper{bitmex::json::OrdType{bitmex::json::OrdType::STOP}} == roq::OrderType::MARKET);
static_assert(Helper{bitmex::json::OrdType{bitmex::json::OrdType::STOP_LIMIT}} == roq::OrderType::LIMIT);
static_assert(Helper{bitmex::json::OrdType{bitmex::json::OrdType::MARKET_IF_TOUCHED}} == roq::OrderType::MARKET);
static_assert(Helper{bitmex::json::OrdType{bitmex::json::OrdType::LIMIT_IF_TOUCHED}} == roq::OrderType::LIMIT);

template <>
template <>
std::optional<roq::OrderType> Map<bitmex::json::OrdType>::helper() const {
  return Helper{args_};
}

// bitmex::json::Side ==> roq::Side

template <>
template <>
constexpr Helper<bitmex::json::Side>::operator std::optional<roq::Side>() const {
  switch (std::get<0>(args_)) {
    using enum bitmex::json::Side::type_t;
    case UNDEFINED__:
      return roq::Side::UNDEFINED;
    case UNKNOWN__:
      return roq::Side::UNDEFINED;
    case BUY:
      return roq::Side::BUY;
    case SELL:
      return roq::Side::SELL;
  }
  return {};
}

static_assert(Helper{bitmex::json::Side{bitmex::json::Side::UNDEFINED__}} == roq::Side::UNDEFINED);
static_assert(Helper{bitmex::json::Side{bitmex::json::Side::BUY}} == roq::Side::BUY);
static_assert(Helper{bitmex::json::Side{bitmex::json::Side::SELL}} == roq::Side::SELL);

template <>
template <>
std::optional<roq::Side> Map<bitmex::json::Side>::helper() const {
  return Helper{args_};
}

// bitmex::json::State ==> roq::TradingStatus

template <>
template <>
constexpr Helper<bitmex::json::State>::operator std::optional<roq::TradingStatus>() const {
  switch (std::get<0>(args_)) {
    using enum bitmex::json::State::type_t;
    case UNDEFINED__:
      return roq::TradingStatus::UNDEFINED;
    case UNKNOWN__:
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
  }
  return {};
}

static_assert(Helper{bitmex::json::State{bitmex::json::State::UNDEFINED__}} == roq::TradingStatus::UNDEFINED);
static_assert(Helper{bitmex::json::State{bitmex::json::State::OPEN}} == roq::TradingStatus::OPEN);
static_assert(Helper{bitmex::json::State{bitmex::json::State::CLOSED}} == roq::TradingStatus::CLOSE);
static_assert(Helper{bitmex::json::State{bitmex::json::State::SETTLED}} == roq::TradingStatus::UNDEFINED);
static_assert(Helper{bitmex::json::State{bitmex::json::State::UNLISTED}} == roq::TradingStatus::UNDEFINED);
static_assert(Helper{bitmex::json::State{bitmex::json::State::EXPIRED}} == roq::TradingStatus::UNDEFINED);

template <>
template <>
std::optional<roq::TradingStatus> Map<bitmex::json::State>::helper() const {
  return Helper{args_};
}

// bitmex::json::TimeInForce ==> roq::TimeInForce

template <>
template <>
constexpr Helper<bitmex::json::TimeInForce>::operator std::optional<roq::TimeInForce>() const {
  switch (std::get<0>(args_)) {
    using enum bitmex::json::TimeInForce::type_t;
    case UNDEFINED__:
      return roq::TimeInForce::UNDEFINED;
    case UNKNOWN__:
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

static_assert(Helper{bitmex::json::TimeInForce{bitmex::json::TimeInForce::UNDEFINED__}} == roq::TimeInForce::UNDEFINED);
static_assert(Helper{bitmex::json::TimeInForce{bitmex::json::TimeInForce::AT_THE_CLOSE}} == roq::TimeInForce::AT_THE_CLOSE);
static_assert(Helper{bitmex::json::TimeInForce{bitmex::json::TimeInForce::DAY}} == roq::TimeInForce::GFD);
static_assert(Helper{bitmex::json::TimeInForce{bitmex::json::TimeInForce::FILL_OR_KILL}} == roq::TimeInForce::FOK);
static_assert(Helper{bitmex::json::TimeInForce{bitmex::json::TimeInForce::GOOD_TILL_CANCEL}} == roq::TimeInForce::GTC);
static_assert(Helper{bitmex::json::TimeInForce{bitmex::json::TimeInForce::IMMEDIATE_OR_CANCEL}} == roq::TimeInForce::IOC);

template <>
template <>
std::optional<roq::TimeInForce> Map<bitmex::json::TimeInForce>::helper() const {
  return Helper{args_};
}

// roq => bitmex::json

// roq::OrderType ==> bitmex::json::OrdType

template <>
template <>
constexpr Helper<roq::OrderType>::operator std::optional<bitmex::json::OrdType>() const {
  switch (std::get<0>(args_)) {
    using enum roq::OrderType;
    case UNDEFINED:
      return bitmex::json::OrdType::UNDEFINED__;
    case MARKET:
      return bitmex::json::OrdType::MARKET;
    case LIMIT:
      return bitmex::json::OrdType::LIMIT;
  }
  return {};
}

static_assert(Helper{roq::OrderType::UNDEFINED} == bitmex::json::OrdType{bitmex::json::OrdType::UNDEFINED__});
static_assert(Helper{roq::OrderType::MARKET} == bitmex::json::OrdType{bitmex::json::OrdType::MARKET});
static_assert(Helper{roq::OrderType::LIMIT} == bitmex::json::OrdType{bitmex::json::OrdType::LIMIT});

template <>
template <>
std::optional<bitmex::json::OrdType> Map<OrderType>::helper() const {
  return Helper{args_};
}

// roq::Side ==> bitmex::json::Side

template <>
template <>
constexpr Helper<roq::Side>::operator std::optional<bitmex::json::Side>() const {
  switch (std::get<0>(args_)) {
    using enum roq::Side;
    case UNDEFINED:
      return bitmex::json::Side::UNDEFINED__;
    case BUY:
      return bitmex::json::Side::BUY;
    case SELL:
      return bitmex::json::Side::SELL;
  }
  return {};
}

static_assert(Helper{roq::Side::UNDEFINED} == bitmex::json::Side{bitmex::json::Side::UNDEFINED__});
static_assert(Helper{roq::Side::BUY} == bitmex::json::Side{bitmex::json::Side::BUY});
static_assert(Helper{roq::Side::SELL} == bitmex::json::Side{bitmex::json::Side::SELL});

template <>
template <>
std::optional<bitmex::json::Side> Map<Side>::helper() const {
  return Helper{args_};
}

// roq::TimeInForce ==> bitmex::json::TimeInForce

template <>
template <>
constexpr Helper<roq::TimeInForce>::operator std::optional<bitmex::json::TimeInForce>() const {
  switch (std::get<0>(args_)) {
    using enum roq::TimeInForce;
    case UNDEFINED:
      return bitmex::json::TimeInForce::UNDEFINED__;
    case GFD:
      return bitmex::json::TimeInForce::DAY;
    case GTC:
      return bitmex::json::TimeInForce::GOOD_TILL_CANCEL;
    case OPG:
      return bitmex::json::TimeInForce::UNDEFINED__;
    case IOC:
      return bitmex::json::TimeInForce::IMMEDIATE_OR_CANCEL;
    case FOK:
      return bitmex::json::TimeInForce::FILL_OR_KILL;
    case GTD:
      return bitmex::json::TimeInForce::UNDEFINED__;
    case GTX:
      return bitmex::json::TimeInForce::UNDEFINED__;
    case AT_THE_CLOSE:
      return bitmex::json::TimeInForce::AT_THE_CLOSE;
    case GOOD_THROUGH_CROSSING:
      return bitmex::json::TimeInForce::UNDEFINED__;
    case AT_CROSSING:
      return bitmex::json::TimeInForce::UNDEFINED__;
    case GOOD_FOR_TIME:
      return bitmex::json::TimeInForce::UNDEFINED__;
    case GFA:
      return bitmex::json::TimeInForce::UNDEFINED__;
    case GFM:
      return bitmex::json::TimeInForce::UNDEFINED__;
  }
  return {};
}

static_assert(Helper{roq::TimeInForce::UNDEFINED} == bitmex::json::TimeInForce{bitmex::json::TimeInForce::UNDEFINED__});
static_assert(Helper{roq::TimeInForce::GFD} == bitmex::json::TimeInForce{bitmex::json::TimeInForce::DAY});
static_assert(Helper{roq::TimeInForce::GTC} == bitmex::json::TimeInForce{bitmex::json::TimeInForce::GOOD_TILL_CANCEL});
static_assert(Helper{roq::TimeInForce::OPG} == bitmex::json::TimeInForce{bitmex::json::TimeInForce::UNDEFINED__});
static_assert(Helper{roq::TimeInForce::IOC} == bitmex::json::TimeInForce{bitmex::json::TimeInForce::IMMEDIATE_OR_CANCEL});
static_assert(Helper{roq::TimeInForce::FOK} == bitmex::json::TimeInForce{bitmex::json::TimeInForce::FILL_OR_KILL});
static_assert(Helper{roq::TimeInForce::GTD} == bitmex::json::TimeInForce{bitmex::json::TimeInForce::UNDEFINED__});
static_assert(Helper{roq::TimeInForce::GTX} == bitmex::json::TimeInForce{bitmex::json::TimeInForce::UNDEFINED__});
static_assert(Helper{roq::TimeInForce::AT_THE_CLOSE} == bitmex::json::TimeInForce{bitmex::json::TimeInForce::AT_THE_CLOSE});
static_assert(Helper{roq::TimeInForce::GOOD_THROUGH_CROSSING} == bitmex::json::TimeInForce{bitmex::json::TimeInForce::UNDEFINED__});
static_assert(Helper{roq::TimeInForce::AT_CROSSING} == bitmex::json::TimeInForce{bitmex::json::TimeInForce::UNDEFINED__});
static_assert(Helper{roq::TimeInForce::GOOD_FOR_TIME} == bitmex::json::TimeInForce{bitmex::json::TimeInForce::UNDEFINED__});
static_assert(Helper{roq::TimeInForce::GFA} == bitmex::json::TimeInForce{bitmex::json::TimeInForce::UNDEFINED__});
static_assert(Helper{roq::TimeInForce::GFM} == bitmex::json::TimeInForce{bitmex::json::TimeInForce::UNDEFINED__});

template <>
template <>
std::optional<bitmex::json::TimeInForce> Map<TimeInForce>::helper() const {
  return Helper{args_};
}

}  // namespace roq
