/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/bitmex/json/map.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace bitmex {
namespace json {

// === HELPERS ===

namespace {
// note! constexpr helper for static testing
template <typename... Args>
struct Helper final {
  explicit constexpr Helper(std::tuple<Args...> const &args) : args_{args} {}
  explicit constexpr Helper(Args &&...args_) : args_{std::forward<Args>(args_)...} {}

  template <typename R>
  constexpr operator R();

 private:
  std::tuple<Args...> const args_;
};

// ==> roq

// LiquidityInd ==> roq::Liquidity

template <>
template <>
constexpr Helper<LiquidityInd>::operator roq::Liquidity() {
  switch (std::get<0>(args_)) {
    using enum LiquidityInd::type_t;
    case UNDEFINED__:
      return {};
    case UNKNOWN__:
      break;
    case ADDED_LIQUIDITY:
      return roq::Liquidity::MAKER;
    case REMOVED_LIQUIDITY:
      return roq::Liquidity::TAKER;
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<roq::Liquidity>(Helper{LiquidityInd{LiquidityInd::UNDEFINED__}}) == roq::Liquidity::UNDEFINED);
static_assert(static_cast<roq::Liquidity>(Helper{LiquidityInd{LiquidityInd::ADDED_LIQUIDITY}}) == roq::Liquidity::MAKER);
static_assert(static_cast<roq::Liquidity>(Helper{LiquidityInd{LiquidityInd::REMOVED_LIQUIDITY}}) == roq::Liquidity::TAKER);

// OrdStatus ==> roq::OrderStatus

template <>
template <>
constexpr Helper<OrdStatus>::operator roq::OrderStatus() {
  switch (std::get<0>(args_)) {
    using enum OrdStatus::type_t;
    case UNDEFINED__:
      return {};
    case UNKNOWN__:
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
    case PENDING_CANCEL:
      return {};  // note!
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
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<roq::OrderStatus>(Helper{OrdStatus{OrdStatus::UNDEFINED__}}) == roq::OrderStatus::UNDEFINED);
static_assert(static_cast<roq::OrderStatus>(Helper{OrdStatus{OrdStatus::CANCELED}}) == roq::OrderStatus::CANCELED);
static_assert(static_cast<roq::OrderStatus>(Helper{OrdStatus{OrdStatus::DONE_FOR_DAY}}) == roq::OrderStatus::SUSPENDED);
static_assert(static_cast<roq::OrderStatus>(Helper{OrdStatus{OrdStatus::EXPIRED}}) == roq::OrderStatus::EXPIRED);
static_assert(static_cast<roq::OrderStatus>(Helper{OrdStatus{OrdStatus::FILLED}}) == roq::OrderStatus::COMPLETED);
static_assert(static_cast<roq::OrderStatus>(Helper{OrdStatus{OrdStatus::NEW}}) == roq::OrderStatus::WORKING);
static_assert(static_cast<roq::OrderStatus>(Helper{OrdStatus{OrdStatus::PARTIALLY_FILLED}}) == roq::OrderStatus::WORKING);
static_assert(static_cast<roq::OrderStatus>(Helper{OrdStatus{OrdStatus::PENDING_CANCEL}}) == roq::OrderStatus::UNDEFINED);
static_assert(static_cast<roq::OrderStatus>(Helper{OrdStatus{OrdStatus::PENDING_NEW}}) == roq::OrderStatus::SENT);
static_assert(static_cast<roq::OrderStatus>(Helper{OrdStatus{OrdStatus::REJECTED}}) == roq::OrderStatus::REJECTED);
static_assert(static_cast<roq::OrderStatus>(Helper{OrdStatus{OrdStatus::STOPPED}}) == roq::OrderStatus::STOPPED);
static_assert(static_cast<roq::OrderStatus>(Helper{OrdStatus{OrdStatus::UNTRIGGERED}}) == roq::OrderStatus::ACCEPTED);
static_assert(static_cast<roq::OrderStatus>(Helper{OrdStatus{OrdStatus::TRIGGERED}}) == roq::OrderStatus::WORKING);

// OrdType ==> roq::OrderType

template <>
template <>
constexpr Helper<OrdType>::operator roq::OrderType() {
  switch (std::get<0>(args_)) {
    using enum OrdType::type_t;
    case UNDEFINED__:
      return {};
    case UNKNOWN__:
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
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<roq::OrderType>(Helper{OrdType{OrdType::UNDEFINED__}}) == roq::OrderType::UNDEFINED);
static_assert(static_cast<roq::OrderType>(Helper{OrdType{OrdType::LIMIT}}) == roq::OrderType::LIMIT);
static_assert(static_cast<roq::OrderType>(Helper{OrdType{OrdType::MARKET}}) == roq::OrderType::MARKET);
static_assert(static_cast<roq::OrderType>(Helper{OrdType{OrdType::STOP}}) == roq::OrderType::MARKET);
static_assert(static_cast<roq::OrderType>(Helper{OrdType{OrdType::STOP_LIMIT}}) == roq::OrderType::LIMIT);
static_assert(static_cast<roq::OrderType>(Helper{OrdType{OrdType::MARKET_IF_TOUCHED}}) == roq::OrderType::MARKET);
static_assert(static_cast<roq::OrderType>(Helper{OrdType{OrdType::LIMIT_IF_TOUCHED}}) == roq::OrderType::LIMIT);

// Side ==> roq::Side

template <>
template <>
constexpr Helper<Side>::operator roq::Side() {
  switch (std::get<0>(args_)) {
    using enum Side::type_t;
    case UNDEFINED__:
      return {};
    case UNKNOWN__:
      break;
    case BUY:
      return roq::Side::BUY;
    case SELL:
      return roq::Side::SELL;
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<roq::Side>(Helper{Side{Side::UNDEFINED__}}) == roq::Side::UNDEFINED);
static_assert(static_cast<roq::Side>(Helper{Side{Side::BUY}}) == roq::Side::BUY);
static_assert(static_cast<roq::Side>(Helper{Side{Side::SELL}}) == roq::Side::SELL);

// State ==> roq::TradingStatus

template <>
template <>
constexpr Helper<State>::operator roq::TradingStatus() {
  switch (std::get<0>(args_)) {
    using enum json::State::type_t;
    case UNDEFINED__:
      return {};
    case UNKNOWN__:
      break;
    case OPEN:
      return roq::TradingStatus::OPEN;
    case CLOSED:
      return roq::TradingStatus::CLOSE;
    case SETTLED:
      return {};  // note!
    case UNLISTED:
      return {};  // note!
    case EXPIRED:
      return {};  // note!
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<roq::TradingStatus>(Helper{State{State::UNDEFINED__}}) == roq::TradingStatus::UNDEFINED);
static_assert(static_cast<roq::TradingStatus>(Helper{State{State::OPEN}}) == roq::TradingStatus::OPEN);
static_assert(static_cast<roq::TradingStatus>(Helper{State{State::CLOSED}}) == roq::TradingStatus::CLOSE);
static_assert(static_cast<roq::TradingStatus>(Helper{State{State::SETTLED}}) == roq::TradingStatus::UNDEFINED);
static_assert(static_cast<roq::TradingStatus>(Helper{State{State::UNLISTED}}) == roq::TradingStatus::UNDEFINED);
static_assert(static_cast<roq::TradingStatus>(Helper{State{State::EXPIRED}}) == roq::TradingStatus::UNDEFINED);

// TimeInForce ==> roq::TimeInForce

template <>
template <>
constexpr Helper<TimeInForce>::operator roq::TimeInForce() {
  switch (std::get<0>(args_)) {
    using enum TimeInForce::type_t;
    case UNDEFINED__:
      return {};
    case UNKNOWN__:
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
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<roq::TimeInForce>(Helper{TimeInForce{TimeInForce::UNDEFINED__}}) == roq::TimeInForce::UNDEFINED);
static_assert(static_cast<roq::TimeInForce>(Helper{TimeInForce{TimeInForce::AT_THE_CLOSE}}) == roq::TimeInForce::AT_THE_CLOSE);
static_assert(static_cast<roq::TimeInForce>(Helper{TimeInForce{TimeInForce::DAY}}) == roq::TimeInForce::GFD);
static_assert(static_cast<roq::TimeInForce>(Helper{TimeInForce{TimeInForce::FILL_OR_KILL}}) == roq::TimeInForce::FOK);
static_assert(static_cast<roq::TimeInForce>(Helper{TimeInForce{TimeInForce::GOOD_TILL_CANCEL}}) == roq::TimeInForce::GTC);
static_assert(static_cast<roq::TimeInForce>(Helper{TimeInForce{TimeInForce::IMMEDIATE_OR_CANCEL}}) == roq::TimeInForce::IOC);

// roq ==>

// roq::OrderType ==> OrdType

template <>
template <>
constexpr Helper<roq::OrderType>::operator OrdType() {
  switch (std::get<0>(args_)) {
    using enum roq::OrderType;
    case UNDEFINED:
      return {};
    case MARKET:
      return json::OrdType::MARKET;
    case LIMIT:
      return json::OrdType::LIMIT;
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<OrdType>(Helper{roq::OrderType::UNDEFINED}) == OrdType{OrdType::UNDEFINED__});
static_assert(static_cast<OrdType>(Helper{roq::OrderType::MARKET}) == OrdType{OrdType::MARKET});
static_assert(static_cast<OrdType>(Helper{roq::OrderType::LIMIT}) == OrdType{OrdType::LIMIT});

// roq::Side ==> Side

template <>
template <>
constexpr Helper<roq::Side>::operator Side() {
  switch (std::get<0>(args_)) {
    using enum roq::Side;
    case UNDEFINED:
      return {};
    case BUY:
      return json::Side::BUY;
    case SELL:
      return json::Side::SELL;
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<Side>(Helper{roq::Side::UNDEFINED}) == Side{Side::UNDEFINED__});
static_assert(static_cast<Side>(Helper{roq::Side::BUY}) == Side{Side::BUY});
static_assert(static_cast<Side>(Helper{roq::Side::SELL}) == Side{Side::SELL});

// roq::TimeInForce ==> TimeInForce

template <>
template <>
constexpr Helper<roq::TimeInForce>::operator TimeInForce() {
  switch (std::get<0>(args_)) {
    using enum roq::TimeInForce;
    case UNDEFINED:
      return {};
    case GFD:
      return json::TimeInForce::DAY;
    case GTC:
      return json::TimeInForce::GOOD_TILL_CANCEL;
    case OPG:
      return {};  // note!
    case IOC:
      return json::TimeInForce::IMMEDIATE_OR_CANCEL;
    case FOK:
      return json::TimeInForce::FILL_OR_KILL;
    case GTD:
      return {};  // note!
    case GTX:
      return {};  // note!
    case AT_THE_CLOSE:
      return json::TimeInForce::AT_THE_CLOSE;
    case GOOD_THROUGH_CROSSING:
      return {};  // note!
    case AT_CROSSING:
      return {};  // note!
    case GOOD_FOR_TIME:
      return {};  // note!
    case GFA:
      return {};  // note!
    case GFM:
      return {};  // note!
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<TimeInForce>(Helper{roq::TimeInForce::UNDEFINED}) == TimeInForce{TimeInForce::UNDEFINED__});
static_assert(static_cast<TimeInForce>(Helper{roq::TimeInForce::GFD}) == TimeInForce{TimeInForce::DAY});
static_assert(static_cast<TimeInForce>(Helper{roq::TimeInForce::GTC}) == TimeInForce{TimeInForce::GOOD_TILL_CANCEL});
static_assert(static_cast<TimeInForce>(Helper{roq::TimeInForce::OPG}) == TimeInForce{TimeInForce::UNDEFINED__});
static_assert(static_cast<TimeInForce>(Helper{roq::TimeInForce::IOC}) == TimeInForce{TimeInForce::IMMEDIATE_OR_CANCEL});
static_assert(static_cast<TimeInForce>(Helper{roq::TimeInForce::FOK}) == TimeInForce{TimeInForce::FILL_OR_KILL});
static_assert(static_cast<TimeInForce>(Helper{roq::TimeInForce::GTD}) == TimeInForce{TimeInForce::UNDEFINED__});
static_assert(static_cast<TimeInForce>(Helper{roq::TimeInForce::GTX}) == TimeInForce{TimeInForce::UNDEFINED__});
static_assert(static_cast<TimeInForce>(Helper{roq::TimeInForce::AT_THE_CLOSE}) == TimeInForce{TimeInForce::AT_THE_CLOSE});
static_assert(static_cast<TimeInForce>(Helper{roq::TimeInForce::GOOD_THROUGH_CROSSING}) == TimeInForce{TimeInForce::UNDEFINED__});
static_assert(static_cast<TimeInForce>(Helper{roq::TimeInForce::AT_CROSSING}) == TimeInForce{TimeInForce::UNDEFINED__});
static_assert(static_cast<TimeInForce>(Helper{roq::TimeInForce::GOOD_FOR_TIME}) == TimeInForce{TimeInForce::UNDEFINED__});
static_assert(static_cast<TimeInForce>(Helper{roq::TimeInForce::GFA}) == TimeInForce{TimeInForce::UNDEFINED__});
static_assert(static_cast<TimeInForce>(Helper{roq::TimeInForce::GFM}) == TimeInForce{TimeInForce::UNDEFINED__});
}  // namespace

// === IMPLEMENTATION ===

// ==> roq

template <>
template <>
Map<LiquidityInd>::operator roq::Liquidity() {
  return Helper{args_};
}

template <>
template <>
Map<OrdStatus>::operator roq::OrderStatus() {
  return Helper{args_};
}

template <>
template <>
Map<OrdType>::operator roq::OrderType() {
  return Helper{args_};
}

template <>
template <>
Map<Side>::operator roq::Side() {
  return Helper{args_};
}

template <>
template <>
Map<State>::operator roq::TradingStatus() {
  return Helper{args_};
}

template <>
template <>
Map<TimeInForce>::operator roq::TimeInForce() {
  return Helper{args_};
}

template <>
template <>
Map<roq::OrderType>::operator OrdType() {
  return Helper{args_};
}

template <>
template <>
Map<roq::Side>::operator Side() {
  return Helper{args_};
}

template <>
template <>
Map<roq::TimeInForce>::operator TimeInForce() {
  return Helper{args_};
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
