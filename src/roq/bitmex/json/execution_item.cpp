/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/execution_item.h"

#include "roq/bitmex/json/utils.h"

#include "roq/logging.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  ACCOUNT,
  AVG_PX,
  CL_ORD_ID,
  CL_ORD_LINK_ID,
  COMMISSION,
  CONTINGENCY_TYPE,
  CUM_QTY,
  CURRENCY,
  DISPLAY_QTY,
  EX_DESTINATION,
  EXEC_COMM,
  EXEC_COST,
  EXEC_ID,
  EXEC_INST,
  EXEC_TYPE,
  FOREIGN_NOTIONAL,
  HOME_NOTIONAL,
  LAST_LIQUIDITY_IND,
  LAST_MKT,
  LAST_PX,
  LAST_QTY,
  LEAVES_QTY,
  MULTI_LEG_REPORTING_TYPE,
  ORDER_ID,
  ORDER_QTY,
  ORD_REJ_REASON,
  ORD_STATUS,
  ORD_TYPE,
  PEG_OFFSET_VALUE,
  PEG_PRICE_TYPE,
  PRICE,
  SETTL_CURRENCY,
  SIDE,
  SIMPLE_CUM_QTY,
  SIMPLE_LEAVES_QTY,
  SIMPLE_ORDER_QTY,
  STOP_PX,
  SYMBOL,
  TEXT,
  TIME_IN_FORCE,
  TIMESTAMP,
  TRADE_PUBLISH_INDICATOR,
  TRANSACT_TIME,
  TRD_MATCH_ID,
  TRIGGERED,
  UNDERLYING_LAST_PX,
  WORKING_INDICATOR,
};

constexpr Field parse_a(auto& name) {
  if (name.length() >= 2) {
    switch (name[1]) {
      case 'c':
        if (name.compare("account") == 0)
          return Field::ACCOUNT;
        break;
      case 'v':
        if (name.compare("avgPx") == 0)
          return Field::AVG_PX;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_c(auto& name) {
  if (name.length() >= 6) {
    switch (name[5]) {
      case 'I':
        if (name.compare("clOrdID") == 0)
          return Field::CL_ORD_ID;
        break;
      case 'L':
        if (name.compare("clOrdLinkID") == 0)
          return Field::CL_ORD_LINK_ID;
        break;
      case 's':
        if (name.compare("commission") == 0)
          return Field::COMMISSION;
        break;
      case 'n':
        switch (name[1]) {
          case 'o':
            if (name.compare("contingencyType") == 0)
              return Field::CONTINGENCY_TYPE;
            break;
          case 'u':
            if (name.compare("currency") == 0)
              return Field::CURRENCY;
            break;
        }
        break;
      case 'y':
        if (name.compare("cumQty") == 0)
          return Field::CUM_QTY;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_d(auto& name) {
  if (name.compare("displayQty") == 0)
    return Field::DISPLAY_QTY;
  return Field::UNKNOWN;
}

constexpr Field parse_e(auto& name) {
  if (name.length() >= 6) {
    switch (name[5]) {
      case 't':
        if (name.compare("exDestination") == 0)
          return Field::EX_DESTINATION;
        break;
      case 'o':
        if (name.length() >= 7) {
          switch (name[6]) {
            case 'm':
              if (name.compare("execComm") == 0)
                return Field::EXEC_COMM;
              break;
            case 's':
              if (name.compare("execCost") == 0)
                return Field::EXEC_COST;
              break;
          }
        }
        break;
      case 'D':
        if (name.compare("execID") == 0)
          return Field::EXEC_ID;
        break;
      case 'n':
        if (name.compare("execInst") == 0)
          return Field::EXEC_INST;
        break;
      case 'y':
        if (name.compare("execType") == 0)
          return Field::EXEC_TYPE;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_f(auto& name) {
  if (name.compare("foreignNotional") == 0)
    return Field::FOREIGN_NOTIONAL;
  return Field::UNKNOWN;
}

constexpr Field parse_h(auto& name) {
  if (name.compare("homeNotional") == 0)
    return Field::HOME_NOTIONAL;
  return Field::UNKNOWN;
}

constexpr Field parse_l(auto& name) {
  if (name.length() >= 5) {
    switch (name[4]) {
      case 'L':
        if (name.compare("lastLiquidityInd") == 0)
          return Field::LAST_LIQUIDITY_IND;
        break;
      case 'M':
        if (name.compare("lastMkt") == 0)
          return Field::LAST_MKT;
        break;
      case 'P':
        if (name.compare("lastPx") == 0)
          return Field::LAST_PX;
        break;
      case 'Q':
        if (name.compare("lastQty") == 0)
          return Field::LAST_QTY;
        break;
      case 'e':
        if (name.compare("leavesQty") == 0)
          return Field::LEAVES_QTY;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_m(auto& name) {
  if (name.compare("multiLegReportingType") == 0)
    return Field::MULTI_LEG_REPORTING_TYPE;
  return Field::UNKNOWN;
}

constexpr Field parse_o(auto& name) {
  if (name.length() >= 6) {
    switch (name[5]) {
      case 'I':
        if (name.compare("orderID") == 0)
          return Field::ORDER_ID;
        break;
      case 'Q':
        if (name.compare("orderQty") == 0)
          return Field::ORDER_QTY;
        break;
      case 'j':
        if (name.compare("ordRejReason") == 0)
          return Field::ORD_REJ_REASON;
        break;
      case 'a':
        if (name.compare("ordStatus") == 0)
          return Field::ORD_STATUS;
        break;
      case 'p':
        if (name.compare("ordType") == 0)
          return Field::ORD_TYPE;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_p(auto& name) {
  if (name.length() >= 4) {
    switch (name[3]) {
      case 'O':
        if (name.compare("pegOffsetValue") == 0)
          return Field::PEG_OFFSET_VALUE;
        break;
      case 'P':
        if (name.compare("pegPriceType") == 0)
          return Field::PEG_PRICE_TYPE;
        break;
      case 'c':
        if (name.compare("price") == 0)
          return Field::PRICE;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_s(auto& name) {
  if (name.length() >= 2) {
    switch (name[1]) {
      case 'e':
        if (name.compare("settlCurrency") == 0)
          return Field::SETTL_CURRENCY;
        break;
      case 'i':
        if (name.length() >= 7) {
          switch (name[6]) {
            case 'C':
              if (name.compare("simpleCumQty") == 0)
                return Field::SIMPLE_CUM_QTY;
              break;
            case 'L':
              if (name.compare("simpleLeavesQty") == 0)
                return Field::SIMPLE_LEAVES_QTY;
              break;
            case 'O':
              if (name.compare("simpleOrderQty") == 0)
                return Field::SIMPLE_ORDER_QTY;
              break;
          }
        } else {
            if (name.compare("side") == 0)
              return Field::SIDE;
        }
        break;
      case 't':
        if (name.compare("stopPx") == 0)
          return Field::STOP_PX;
        break;
      case 'y':
        if (name.compare("symbol") == 0)
          return Field::SYMBOL;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_t(auto& name) {
  if (name.length() >= 4) {
    switch (name[3]) {
      case 't':
        if (name.compare("text") == 0)
          return Field::TEXT;
        break;
      case 'e':
        if (name.length() >= 5) {
          switch (name[4]) {
            case 'I':
              if (name.compare("timeInForce") == 0)
                return Field::TIME_IN_FORCE;
              break;
            case 's':
              if (name.compare("timestamp") == 0)
                return Field::TIMESTAMP;
              break;
          }
        }
        break;
      case 'd':
        if (name.compare("tradePublishIndicator") == 0)
          return Field::TRADE_PUBLISH_INDICATOR;
        break;
      case 'n':
        if (name.compare("transactTime") == 0)
          return Field::TRANSACT_TIME;
        break;
      case 'M':
        if (name.compare("trdMatchID") == 0)
          return Field::TRD_MATCH_ID;
        break;
      case 'g':
        if (name.compare("triggered") == 0)
          return Field::TRIGGERED;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_u(auto& name) {
  if (name.compare("underlyingLastPx") == 0)
    return Field::UNDERLYING_LAST_PX;
  return Field::UNKNOWN;
}

constexpr Field parse_w(auto& name) {
  if (name.compare("workingIndicator") == 0)
    return Field::WORKING_INDICATOR;
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  if (name.empty())
    return Field::UNKNOWN;
  switch (name[0]) {
    case 'a': return parse_a(name);
    case 'c': return parse_c(name);
    case 'd': return parse_d(name);
    case 'e': return parse_e(name);
    case 'f': return parse_f(name);
    case 'h': return parse_h(name);
    case 'l': return parse_l(name);
    case 'm': return parse_m(name);
    case 'o': return parse_o(name);
    case 'p': return parse_p(name);
    case 's': return parse_s(name);
    case 't': return parse_t(name);
    case 'u': return parse_u(name);
    case 'w': return parse_w(name);
    default:
      return Field::UNKNOWN;
  }
}

static_assert(parse_name("account") == Field::ACCOUNT);
static_assert(parse_name("avgPx") == Field::AVG_PX);
static_assert(parse_name("clOrdID") == Field::CL_ORD_ID);
static_assert(parse_name("clOrdLinkID") == Field::CL_ORD_LINK_ID);
static_assert(parse_name("commission") == Field::COMMISSION);
static_assert(parse_name("contingencyType") == Field::CONTINGENCY_TYPE);
static_assert(parse_name("cumQty") == Field::CUM_QTY);
static_assert(parse_name("currency") == Field::CURRENCY);
static_assert(parse_name("displayQty") == Field::DISPLAY_QTY);
static_assert(parse_name("exDestination") == Field::EX_DESTINATION);
static_assert(parse_name("execComm") == Field::EXEC_COMM);
static_assert(parse_name("execCost") == Field::EXEC_COST);
static_assert(parse_name("execID") == Field::EXEC_ID);
static_assert(parse_name("execInst") == Field::EXEC_INST);
static_assert(parse_name("execType") == Field::EXEC_TYPE);
static_assert(parse_name("foreignNotional") == Field::FOREIGN_NOTIONAL);
static_assert(parse_name("homeNotional") == Field::HOME_NOTIONAL);
static_assert(parse_name("lastLiquidityInd") == Field::LAST_LIQUIDITY_IND);
static_assert(parse_name("lastMkt") == Field::LAST_MKT);
static_assert(parse_name("lastPx") == Field::LAST_PX);
static_assert(parse_name("lastQty") == Field::LAST_QTY);
static_assert(parse_name("leavesQty") == Field::LEAVES_QTY);
static_assert(parse_name("multiLegReportingType") == Field::MULTI_LEG_REPORTING_TYPE);
static_assert(parse_name("orderID") == Field::ORDER_ID);
static_assert(parse_name("orderQty") == Field::ORDER_QTY);
static_assert(parse_name("ordRejReason") == Field::ORD_REJ_REASON);
static_assert(parse_name("ordStatus") == Field::ORD_STATUS);
static_assert(parse_name("ordType") == Field::ORD_TYPE);
static_assert(parse_name("pegOffsetValue") == Field::PEG_OFFSET_VALUE);
static_assert(parse_name("pegPriceType") == Field::PEG_PRICE_TYPE);
static_assert(parse_name("price") == Field::PRICE);
static_assert(parse_name("settlCurrency") == Field::SETTL_CURRENCY);
static_assert(parse_name("side") == Field::SIDE);
static_assert(parse_name("simpleCumQty") == Field::SIMPLE_CUM_QTY);
static_assert(parse_name("simpleLeavesQty") == Field::SIMPLE_LEAVES_QTY);
static_assert(parse_name("simpleOrderQty") == Field::SIMPLE_ORDER_QTY);
static_assert(parse_name("stopPx") == Field::STOP_PX);
static_assert(parse_name("symbol") == Field::SYMBOL);
static_assert(parse_name("text") == Field::TEXT);
static_assert(parse_name("timeInForce") == Field::TIME_IN_FORCE);
static_assert(parse_name("timestamp") == Field::TIMESTAMP);
static_assert(parse_name("tradePublishIndicator") == Field::TRADE_PUBLISH_INDICATOR);
static_assert(parse_name("transactTime") == Field::TRANSACT_TIME);
static_assert(parse_name("trdMatchID") == Field::TRD_MATCH_ID);
static_assert(parse_name("triggered") == Field::TRIGGERED);
static_assert(parse_name("underlyingLastPx") == Field::UNDERLYING_LAST_PX);
static_assert(parse_name("workingIndicator") == Field::WORKING_INDICATOR);

inline void update_field(
    auto& result,
    auto& key,
    auto& value) {
  auto field = parse_name(key);
  switch (field) {
    case Field::UNKNOWN:
      DLOG(FATAL)(
          FMT_STRING("Unknown key=\"{}\""),
          key);
      break;
    case Field::ACCOUNT:
      update(result.account, value);
      break;
    case Field::AVG_PX:
      update(result.avg_px, value);
      break;
    case Field::CL_ORD_ID:
      update(result.cl_ord_id, value);
      break;
    case Field::CL_ORD_LINK_ID:
      update(result.cl_ord_link_id, value);
      break;
    case Field::COMMISSION:
      update(result.commission, value);
      break;
    case Field::CONTINGENCY_TYPE:
      update(result.contingency_type, value);
      break;
    case Field::CUM_QTY:
      update(result.cum_qty, value);
      break;
    case Field::CURRENCY:
      update(result.currency, value);
      break;
    case Field::DISPLAY_QTY:
      update(result.display_qty, value);
      break;
    case Field::EX_DESTINATION:
      update(result.ex_destination, value);
      break;
    case Field::EXEC_COMM:
      update(result.exec_comm, value);
      break;
    case Field::EXEC_COST:
      update(result.exec_cost, value);
      break;
    case Field::EXEC_ID:
      update(result.exec_id, value);
      break;
    case Field::EXEC_INST:
      update(result.exec_inst, value);
      break;
    case Field::EXEC_TYPE:
      update(result.exec_type, value);
      break;
    case Field::FOREIGN_NOTIONAL:
      update(result.foreign_notional, value);
      break;
    case Field::HOME_NOTIONAL:
      update(result.home_notional, value);
      break;
    case Field::LAST_LIQUIDITY_IND:
      update(result.last_liquidity_ind, value);
      break;
    case Field::LAST_MKT:
      update(result.last_mkt, value);
      break;
    case Field::LAST_PX:
      update(result.last_px, value);
      break;
    case Field::LAST_QTY:
      update(result.last_qty, value);
      break;
    case Field::LEAVES_QTY:
      update(result.leaves_qty, value);
      break;
    case Field::MULTI_LEG_REPORTING_TYPE:
      update(result.multi_leg_reporting_type, value);
      break;
    case Field::ORDER_ID:
      update(result.order_id, value);
      break;
    case Field::ORDER_QTY:
      update(result.order_qty, value);
      break;
    case Field::ORD_REJ_REASON:
      update(result.ord_rej_reason, value);
      break;
    case Field::ORD_STATUS:
      update(result.ord_status, value);
      break;
    case Field::ORD_TYPE:
      update(result.ord_type, value);
      break;
    case Field::PEG_OFFSET_VALUE:
      update(result.peg_offset_value, value);
      break;
    case Field::PEG_PRICE_TYPE:
      update(result.peg_price_type, value);
      break;
    case Field::PRICE:
      update(result.price, value);
      break;
    case Field::SETTL_CURRENCY:
      update(result.settl_currency, value);
      break;
    case Field::SIDE:
      update(result.side, value);
      break;
    case Field::SIMPLE_CUM_QTY:
      update(result.simple_cum_qty, value);
      break;
    case Field::SIMPLE_LEAVES_QTY:
      update(result.simple_leaves_qty, value);
      break;
    case Field::SIMPLE_ORDER_QTY:
      update(result.simple_order_qty, value);
      break;
    case Field::STOP_PX:
      update(result.stop_px, value);
      break;
    case Field::SYMBOL:
      update(result.symbol, value);
      break;
    case Field::TEXT:
      update(result.text, value);
      break;
    case Field::TIME_IN_FORCE:
      update(result.time_in_force, value);
      break;
    case Field::TIMESTAMP:
      update(result.timestamp, value);
      break;
    case Field::TRADE_PUBLISH_INDICATOR:
      update(result.trade_publish_indicator, value);
      break;
    case Field::TRANSACT_TIME:
      update(result.transact_time, value);
      break;
    case Field::TRD_MATCH_ID:
      update(result.trd_match_id, value);
      break;
    case Field::TRIGGERED:
      update(result.triggered, value);
      break;
    case Field::UNDERLYING_LAST_PX:
      update(result.underlying_last_px, value);
      break;
    case Field::WORKING_INDICATOR:
      update(result.working_indicator, value);
      break;
  }
}
}  // namespace

ExecutionItem::ExecutionItem(core::json::value_t& value) {
  for (auto [key, value] : std::get<core::json::object_t>(value))
    update_field(*this, key, value);
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
