/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/instrument_item.h"

#include "roq/bitmex/json/utils.h"

#include "roq/logging.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  ASK_PRICE,
  BANKRUPT_LIMIT_DOWN_PRICE,
  BANKRUPT_LIMIT_UP_PRICE,
  BID_PRICE,
  BUY_LEG,
  CALC_INTERVAL,
  CAPPED,
  CLOSING_TIMESTAMP,
  DELEVERAGE,
  EXPIRY,
  FAIR_BASIS,
  FAIR_BASIS_RATE,
  FAIR_METHOD,
  FAIR_PRICE,
  FOREIGN_NOTIONAL24H,
  FRONT,
  FUNDING_BASE_SYMBOL,
  FUNDING_INTERVAL,
  FUNDING_PREMIUM_SYMBOL,
  FUNDING_QUOTE_SYMBOL,
  FUNDING_RATE,
  FUNDING_TIMESTAMP,
  HAS_LIQUIDITY,
  HIGH_PRICE,
  HOME_NOTIONAL24H,
  IMPACT_ASK_PRICE,
  IMPACT_BID_PRICE,
  IMPACT_MID_PRICE,
  INDICATIVE_FUNDING_RATE,
  INDICATIVE_SETTLE_PRICE,
  INDICATIVE_TAX_RATE,
  INIT_MARGIN,
  INSURANCE_FEE,
  INVERSE_LEG,
  IS_INVERSE,
  IS_QUANTO,
  LAST_CHANGE_PCNT,
  LAST_PRICE,
  LAST_PRICE_PROTECTED,
  LAST_TICK_DIRECTION,
  LIMIT,
  LIMIT_DOWN_PRICE,
  LIMIT_UP_PRICE,
  LISTING,
  LOT_SIZE,
  LOW_PRICE,
  MAINT_MARGIN,
  MAKER_FEE,
  MARK_METHOD,
  MARK_PRICE,
  MAX_ORDER_QTY,
  MAX_PRICE,
  MID_PRICE,
  MULTIPLIER,
  OPEN_INTEREST,
  OPEN_VALUE,
  OPENING_TIMESTAMP,
  OPTION_MULTIPLIER,
  OPTION_STRIKE_PCNT,
  OPTION_STRIKE_PRICE,
  OPTION_STRIKE_ROUND,
  OPTION_UNDERLYING_PRICE,
  POSITION_CURRENCY,
  PREV_CLOSE_PRICE,
  PREV_PRICE24H,
  PREV_TOTAL_TURNOVER,
  PREV_TOTAL_VOLUME,
  PUBLISH_INTERVAL,
  PUBLISH_TIME,
  QUOTE_CURRENCY,
  QUOTE_TO_SETTLE_MULTIPLIER,
  REBALANCE_INTERVAL,
  REBALANCE_TIMESTAMP,
  REFERENCE,
  REFERENCE_SYMBOL,
  RELIST_INTERVAL,
  RISK_LIMIT,
  RISK_STEP,
  ROOT_SYMBOL,
  SELL_LEG,
  SESSION_INTERVAL,
  SETTL_CURRENCY,
  SETTLE,
  SETTLED_PRICE,
  SETTLEMENT_FEE,
  STATE,
  SYMBOL,
  TAKER_FEE,
  TAXED,
  TICK_SIZE,
  TIMESTAMP,
  TOTAL_TURNOVER,
  TOTAL_VOLUME,
  TURNOVER,
  TURNOVER24H,
  TYP,
  UNDERLYING,
  UNDERLYING_SYMBOL,
  UNDERLYING_TO_POSITION_MULTIPLIER,
  UNDERLYING_TO_SETTLE_MULTIPLIER,
  VOLUME,
  VOLUME24H,
  VWAP,
};

constexpr Field parse_a(const std::string_view& name) {
  if (name.length() == 8 &&
      name[1] == 's' &&
      name[2] == 'k' &&
      name[3] == 'P' &&
      name[4] == 'r' &&
      name[5] == 'i' &&
      name[6] == 'c' &&
      name[7] == 'e') {
    return Field::ASK_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_bankruptLimitD(const std::string_view& name) {
  if (name.length() == 22 &&
      name[14] == 'o' &&
      name[15] == 'w' &&
      name[16] == 'n' &&
      name[17] == 'P' &&
      name[18] == 'r' &&
      name[19] == 'i' &&
      name[20] == 'c' &&
      name[21] == 'e') {
    return Field::BANKRUPT_LIMIT_DOWN_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_bankruptLimitU(const std::string_view& name) {
  if (name.length() == 20 &&
      name[14] == 'p' &&
      name[15] == 'P' &&
      name[16] == 'r' &&
      name[17] == 'i' &&
      name[18] == 'c' &&
      name[19] == 'e') {
    return Field::BANKRUPT_LIMIT_UP_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ba(const std::string_view& name) {
  if (name.length() >= 14 &&
      name[2] == 'n' &&
      name[3] == 'k' &&
      name[4] == 'r' &&
      name[5] == 'u' &&
      name[6] == 'p' &&
      name[7] == 't' &&
      name[8] == 'L' &&
      name[9] == 'i' &&
      name[10] == 'm' &&
      name[11] == 'i' &&
      name[12] == 't') {
    switch (name[13]) {
      case 'D':
        return parse_bankruptLimitD(name);
      case 'U':
        return parse_bankruptLimitU(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_bi(const std::string_view& name) {
  if (name.length() == 8 &&
      name[2] == 'd' &&
      name[3] == 'P' &&
      name[4] == 'r' &&
      name[5] == 'i' &&
      name[6] == 'c' &&
      name[7] == 'e') {
    return Field::BID_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_bu(const std::string_view& name) {
  if (name.length() == 6 &&
      name[2] == 'y' &&
      name[3] == 'L' &&
      name[4] == 'e' &&
      name[5] == 'g') {
    return Field::BUY_LEG;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_b(const std::string_view& name) {
  if (name.length() > 1) {
    switch (name[1]) {
      case 'a':
        return parse_ba(name);
      case 'i':
        return parse_bi(name);
      case 'u':
        return parse_bu(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_cal(const std::string_view& name) {
  if (name.length() == 12 &&
      name[3] == 'c' &&
      name[4] == 'I' &&
      name[5] == 'n' &&
      name[6] == 't' &&
      name[7] == 'e' &&
      name[8] == 'r' &&
      name[9] == 'v' &&
      name[10] == 'a' &&
      name[11] == 'l') {
    return Field::CALC_INTERVAL;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_cap(const std::string_view& name) {
  if (name.length() == 6 &&
      name[3] == 'p' &&
      name[4] == 'e' &&
      name[5] == 'd') {
    return Field::CAPPED;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ca(const std::string_view& name) {
  if (name.length() > 2) {
    switch (name[2]) {
      case 'l':
        return parse_cal(name);
      case 'p':
        return parse_cap(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_cl(const std::string_view& name) {
  if (name.length() == 16 &&
      name[2] == 'o' &&
      name[3] == 's' &&
      name[4] == 'i' &&
      name[5] == 'n' &&
      name[6] == 'g' &&
      name[7] == 'T' &&
      name[8] == 'i' &&
      name[9] == 'm' &&
      name[10] == 'e' &&
      name[11] == 's' &&
      name[12] == 't' &&
      name[13] == 'a' &&
      name[14] == 'm' &&
      name[15] == 'p') {
    return Field::CLOSING_TIMESTAMP;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_c(const std::string_view& name) {
  if (name.length() > 1) {
    switch (name[1]) {
      case 'a':
        return parse_ca(name);
      case 'l':
        return parse_cl(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_d(const std::string_view& name) {
  if (name.length() == 10 &&
      name[1] == 'e' &&
      name[2] == 'l' &&
      name[3] == 'e' &&
      name[4] == 'v' &&
      name[5] == 'e' &&
      name[6] == 'r' &&
      name[7] == 'a' &&
      name[8] == 'g' &&
      name[9] == 'e') {
    return Field::DELEVERAGE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_e(const std::string_view& name) {
  if (name.length() == 6 &&
      name[1] == 'x' &&
      name[2] == 'p' &&
      name[3] == 'i' &&
      name[4] == 'r' &&
      name[5] == 'y') {
    return Field::EXPIRY;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_fairBasis(const std::string_view& name) {
  if (name.length() == 13 &&
      name[9] == 'R' &&
      name[10] == 'a' &&
      name[11] == 't' &&
      name[12] == 'e') {
    return Field::FAIR_BASIS_RATE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_fairB(const std::string_view& name) {
  if (name.length() >= 9 &&
      name[5] == 'a' &&
      name[6] == 's' &&
      name[7] == 'i' &&
      name[8] == 's') {
    if (name.length() == 9)
      return Field::FAIR_BASIS;
    return parse_fairBasis(name);
  }
  return Field::UNKNOWN;
}

constexpr Field parse_fairM(const std::string_view& name) {
  if (name.length() == 10 &&
      name[5] == 'e' &&
      name[6] == 't' &&
      name[7] == 'h' &&
      name[8] == 'o' &&
      name[9] == 'd') {
    return Field::FAIR_METHOD;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_fairP(const std::string_view& name) {
  if (name.length() == 9 &&
      name[5] == 'r' &&
      name[6] == 'i' &&
      name[7] == 'c' &&
      name[8] == 'e') {
    return Field::FAIR_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_fa(const std::string_view& name) {
  if (name.length() >= 5 &&
      name[2] == 'i' &&
      name[3] == 'r') {
    switch (name[4]) {
      case 'B':
        return parse_fairB(name);
      case 'M':
        return parse_fairM(name);
      case 'P':
        return parse_fairP(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_fo(const std::string_view& name) {
  if (name.length() == 18 &&
      name[2] == 'r' &&
      name[3] == 'e' &&
      name[4] == 'i' &&
      name[5] == 'g' &&
      name[6] == 'n' &&
      name[7] == 'N' &&
      name[8] == 'o' &&
      name[9] == 't' &&
      name[10] == 'i' &&
      name[11] == 'o' &&
      name[12] == 'n' &&
      name[13] == 'a' &&
      name[14] == 'l' &&
      name[15] == '2' &&
      name[16] == '4' &&
      name[17] == 'h') {
    return Field::FOREIGN_NOTIONAL24H;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_fr(const std::string_view& name) {
  if (name.length() == 5 &&
      name[2] == 'o' &&
      name[3] == 'n' &&
      name[4] == 't') {
    return Field::FRONT;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_fundingB(const std::string_view& name) {
  if (name.length() == 17 &&
      name[8] == 'a' &&
      name[9] == 's' &&
      name[10] == 'e' &&
      name[11] == 'S' &&
      name[12] == 'y' &&
      name[13] == 'm' &&
      name[14] == 'b' &&
      name[15] == 'o' &&
      name[16] == 'l') {
    return Field::FUNDING_BASE_SYMBOL;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_fundingI(const std::string_view& name) {
  if (name.length() == 15 &&
      name[8] == 'n' &&
      name[9] == 't' &&
      name[10] == 'e' &&
      name[11] == 'r' &&
      name[12] == 'v' &&
      name[13] == 'a' &&
      name[14] == 'l') {
    return Field::FUNDING_INTERVAL;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_fundingP(const std::string_view& name) {
  if (name.length() == 20 &&
      name[8] == 'r' &&
      name[9] == 'e' &&
      name[10] == 'm' &&
      name[11] == 'i' &&
      name[12] == 'u' &&
      name[13] == 'm' &&
      name[14] == 'S' &&
      name[15] == 'y' &&
      name[16] == 'm' &&
      name[17] == 'b' &&
      name[18] == 'o' &&
      name[19] == 'l') {
    return Field::FUNDING_PREMIUM_SYMBOL;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_fundingQ(const std::string_view& name) {
  if (name.length() == 18 &&
      name[8] == 'u' &&
      name[9] == 'o' &&
      name[10] == 't' &&
      name[11] == 'e' &&
      name[12] == 'S' &&
      name[13] == 'y' &&
      name[14] == 'm' &&
      name[15] == 'b' &&
      name[16] == 'o' &&
      name[17] == 'l') {
    return Field::FUNDING_QUOTE_SYMBOL;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_fundingR(const std::string_view& name) {
  if (name.length() == 11 &&
      name[8] == 'a' &&
      name[9] == 't' &&
      name[10] == 'e') {
    return Field::FUNDING_RATE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_fundingT(const std::string_view& name) {
  if (name.length() == 16 &&
      name[8] == 'i' &&
      name[9] == 'm' &&
      name[10] == 'e' &&
      name[11] == 's' &&
      name[12] == 't' &&
      name[13] == 'a' &&
      name[14] == 'm' &&
      name[15] == 'p') {
    return Field::FUNDING_TIMESTAMP;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_fu(const std::string_view& name) {
  if (name.length() >= 8 &&
      name[2] == 'n' &&
      name[3] == 'd' &&
      name[4] == 'i' &&
      name[5] == 'n' &&
      name[6] == 'g') {
    switch (name[7]) {
      case 'B':
        return parse_fundingB(name);
      case 'I':
        return parse_fundingI(name);
      case 'P':
        return parse_fundingP(name);
      case 'Q':
        return parse_fundingQ(name);
      case 'R':
        return parse_fundingR(name);
      case 'T':
        return parse_fundingT(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_f(const std::string_view& name) {
  if (name.length() > 1) {
    switch (name[1]) {
      case 'a':
        return parse_fa(name);
      case 'o':
        return parse_fo(name);
      case 'r':
        return parse_fr(name);
      case 'u':
        return parse_fu(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ha(const std::string_view& name) {
  if (name.length() == 12 &&
      name[2] == 's' &&
      name[3] == 'L' &&
      name[4] == 'i' &&
      name[5] == 'q' &&
      name[6] == 'u' &&
      name[7] == 'i' &&
      name[8] == 'd' &&
      name[9] == 'i' &&
      name[10] == 't' &&
      name[11] == 'y') {
    return Field::HAS_LIQUIDITY;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_hi(const std::string_view& name) {
  if (name.length() == 9 &&
      name[2] == 'g' &&
      name[3] == 'h' &&
      name[4] == 'P' &&
      name[5] == 'r' &&
      name[6] == 'i' &&
      name[7] == 'c' &&
      name[8] == 'e') {
    return Field::HIGH_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ho(const std::string_view& name) {
  if (name.length() == 15 &&
      name[2] == 'm' &&
      name[3] == 'e' &&
      name[4] == 'N' &&
      name[5] == 'o' &&
      name[6] == 't' &&
      name[7] == 'i' &&
      name[8] == 'o' &&
      name[9] == 'n' &&
      name[10] == 'a' &&
      name[11] == 'l' &&
      name[12] == '2' &&
      name[13] == '4' &&
      name[14] == 'h') {
    return Field::HOME_NOTIONAL24H;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_h(const std::string_view& name) {
  if (name.length() > 1) {
    switch (name[1]) {
      case 'a':
        return parse_ha(name);
      case 'i':
        return parse_hi(name);
      case 'o':
        return parse_ho(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_impactA(const std::string_view& name) {
  if (name.length() == 14 &&
      name[7] == 's' &&
      name[8] == 'k' &&
      name[9] == 'P' &&
      name[10] == 'r' &&
      name[11] == 'i' &&
      name[12] == 'c' &&
      name[13] == 'e') {
    return Field::IMPACT_ASK_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_impactB(const std::string_view& name) {
  if (name.length() == 14 &&
      name[7] == 'i' &&
      name[8] == 'd' &&
      name[9] == 'P' &&
      name[10] == 'r' &&
      name[11] == 'i' &&
      name[12] == 'c' &&
      name[13] == 'e') {
    return Field::IMPACT_BID_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_impactM(const std::string_view& name) {
  if (name.length() == 14 &&
      name[7] == 'i' &&
      name[8] == 'd' &&
      name[9] == 'P' &&
      name[10] == 'r' &&
      name[11] == 'i' &&
      name[12] == 'c' &&
      name[13] == 'e') {
    return Field::IMPACT_MID_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_im(const std::string_view& name) {
  if (name.length() >= 7 &&
      name[2] == 'p' &&
      name[3] == 'a' &&
      name[4] == 'c' &&
      name[5] == 't') {
    switch (name[6]) {
      case 'A':
        return parse_impactA(name);
      case 'B':
        return parse_impactB(name);
      case 'M':
        return parse_impactM(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_indicativeF(const std::string_view& name) {
  if (name.length() == 21 &&
      name[11] == 'u' &&
      name[12] == 'n' &&
      name[13] == 'd' &&
      name[14] == 'i' &&
      name[15] == 'n' &&
      name[16] == 'g' &&
      name[17] == 'R' &&
      name[18] == 'a' &&
      name[19] == 't' &&
      name[20] == 'e') {
    return Field::INDICATIVE_FUNDING_RATE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_indicativeS(const std::string_view& name) {
  if (name.length() == 21 &&
      name[11] == 'e' &&
      name[12] == 't' &&
      name[13] == 't' &&
      name[14] == 'l' &&
      name[15] == 'e' &&
      name[16] == 'P' &&
      name[17] == 'r' &&
      name[18] == 'i' &&
      name[19] == 'c' &&
      name[20] == 'e') {
    return Field::INDICATIVE_SETTLE_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_indicativeT(const std::string_view& name) {
  if (name.length() == 17 &&
      name[11] == 'a' &&
      name[12] == 'x' &&
      name[13] == 'R' &&
      name[14] == 'a' &&
      name[15] == 't' &&
      name[16] == 'e') {
    return Field::INDICATIVE_TAX_RATE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ind(const std::string_view& name) {
  if (name.length() >= 11 &&
      name[3] == 'i' &&
      name[4] == 'c' &&
      name[5] == 'a' &&
      name[6] == 't' &&
      name[7] == 'i' &&
      name[8] == 'v' &&
      name[9] == 'e') {
    switch (name[10]) {
      case 'F':
        return parse_indicativeF(name);
      case 'S':
        return parse_indicativeS(name);
      case 'T':
        return parse_indicativeT(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ini(const std::string_view& name) {
  if (name.length() == 10 &&
      name[3] == 't' &&
      name[4] == 'M' &&
      name[5] == 'a' &&
      name[6] == 'r' &&
      name[7] == 'g' &&
      name[8] == 'i' &&
      name[9] == 'n') {
    return Field::INIT_MARGIN;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ins(const std::string_view& name) {
  if (name.length() == 12 &&
      name[3] == 'u' &&
      name[4] == 'r' &&
      name[5] == 'a' &&
      name[6] == 'n' &&
      name[7] == 'c' &&
      name[8] == 'e' &&
      name[9] == 'F' &&
      name[10] == 'e' &&
      name[11] == 'e') {
    return Field::INSURANCE_FEE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_inv(const std::string_view& name) {
  if (name.length() == 10 &&
      name[3] == 'e' &&
      name[4] == 'r' &&
      name[5] == 's' &&
      name[6] == 'e' &&
      name[7] == 'L' &&
      name[8] == 'e' &&
      name[9] == 'g') {
    return Field::INVERSE_LEG;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_in(const std::string_view& name) {
  if (name.length() > 2) {
    switch (name[2]) {
      case 'd':
        return parse_ind(name);
      case 'i':
        return parse_ini(name);
      case 's':
        return parse_ins(name);
      case 'v':
        return parse_inv(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_isI(const std::string_view& name) {
  if (name.length() == 9 &&
      name[3] == 'n' &&
      name[4] == 'v' &&
      name[5] == 'e' &&
      name[6] == 'r' &&
      name[7] == 's' &&
      name[8] == 'e') {
    return Field::IS_INVERSE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_isQ(const std::string_view& name) {
  if (name.length() == 8 &&
      name[3] == 'u' &&
      name[4] == 'a' &&
      name[5] == 'n' &&
      name[6] == 't' &&
      name[7] == 'o') {
    return Field::IS_QUANTO;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_is(const std::string_view& name) {
  if (name.length() > 2) {
    switch (name[2]) {
      case 'I':
        return parse_isI(name);
      case 'Q':
        return parse_isQ(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_i(const std::string_view& name) {
  if (name.length() > 1) {
    switch (name[1]) {
      case 'm':
        return parse_im(name);
      case 'n':
        return parse_in(name);
      case 's':
        return parse_is(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_lastC(const std::string_view& name) {
  if (name.length() == 14 &&
      name[5] == 'h' &&
      name[6] == 'a' &&
      name[7] == 'n' &&
      name[8] == 'g' &&
      name[9] == 'e' &&
      name[10] == 'P' &&
      name[11] == 'c' &&
      name[12] == 'n' &&
      name[13] == 't') {
    return Field::LAST_CHANGE_PCNT;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_lastPrice(const std::string_view& name) {
  if (name.length() == 18 &&
      name[9] == 'P' &&
      name[10] == 'r' &&
      name[11] == 'o' &&
      name[12] == 't' &&
      name[13] == 'e' &&
      name[14] == 'c' &&
      name[15] == 't' &&
      name[16] == 'e' &&
      name[17] == 'd') {
    return Field::LAST_PRICE_PROTECTED;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_lastP(const std::string_view& name) {
  if (name.length() >= 9 &&
      name[5] == 'r' &&
      name[6] == 'i' &&
      name[7] == 'c' &&
      name[8] == 'e') {
    if (name.length() == 9)
      return Field::LAST_PRICE;
    return parse_lastPrice(name);
  }
  return Field::UNKNOWN;
}

constexpr Field parse_lastT(const std::string_view& name) {
  if (name.length() == 17 &&
      name[5] == 'i' &&
      name[6] == 'c' &&
      name[7] == 'k' &&
      name[8] == 'D' &&
      name[9] == 'i' &&
      name[10] == 'r' &&
      name[11] == 'e' &&
      name[12] == 'c' &&
      name[13] == 't' &&
      name[14] == 'i' &&
      name[15] == 'o' &&
      name[16] == 'n') {
    return Field::LAST_TICK_DIRECTION;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_la(const std::string_view& name) {
  if (name.length() >= 5 &&
      name[2] == 's' &&
      name[3] == 't') {
    switch (name[4]) {
      case 'C':
        return parse_lastC(name);
      case 'P':
        return parse_lastP(name);
      case 'T':
        return parse_lastT(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_limitD(const std::string_view& name) {
  if (name.length() == 14 &&
      name[6] == 'o' &&
      name[7] == 'w' &&
      name[8] == 'n' &&
      name[9] == 'P' &&
      name[10] == 'r' &&
      name[11] == 'i' &&
      name[12] == 'c' &&
      name[13] == 'e') {
    return Field::LIMIT_DOWN_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_limitU(const std::string_view& name) {
  if (name.length() == 12 &&
      name[6] == 'p' &&
      name[7] == 'P' &&
      name[8] == 'r' &&
      name[9] == 'i' &&
      name[10] == 'c' &&
      name[11] == 'e') {
    return Field::LIMIT_UP_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_limit(const std::string_view& name) {
  if (name.length() > 5) {
    switch (name[5]) {
      case 'D':
        return parse_limitD(name);
      case 'U':
        return parse_limitU(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_lim(const std::string_view& name) {
  if (name.length() >= 5 &&
      name[3] == 'i' &&
      name[4] == 't') {
    if (name.length() == 5)
      return Field::LIMIT;
    return parse_limit(name);
  }
  return Field::UNKNOWN;
}

constexpr Field parse_lis(const std::string_view& name) {
  if (name.length() == 7 &&
      name[3] == 't' &&
      name[4] == 'i' &&
      name[5] == 'n' &&
      name[6] == 'g') {
    return Field::LISTING;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_li(const std::string_view& name) {
  if (name.length() > 2) {
    switch (name[2]) {
      case 'm':
        return parse_lim(name);
      case 's':
        return parse_lis(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_lot(const std::string_view& name) {
  if (name.length() == 7 &&
      name[3] == 'S' &&
      name[4] == 'i' &&
      name[5] == 'z' &&
      name[6] == 'e') {
    return Field::LOT_SIZE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_low(const std::string_view& name) {
  if (name.length() == 8 &&
      name[3] == 'P' &&
      name[4] == 'r' &&
      name[5] == 'i' &&
      name[6] == 'c' &&
      name[7] == 'e') {
    return Field::LOW_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_lo(const std::string_view& name) {
  if (name.length() > 2) {
    switch (name[2]) {
      case 't':
        return parse_lot(name);
      case 'w':
        return parse_low(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_l(const std::string_view& name) {
  if (name.length() > 1) {
    switch (name[1]) {
      case 'a':
        return parse_la(name);
      case 'i':
        return parse_li(name);
      case 'o':
        return parse_lo(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_mai(const std::string_view& name) {
  if (name.length() == 11 &&
      name[3] == 'n' &&
      name[4] == 't' &&
      name[5] == 'M' &&
      name[6] == 'a' &&
      name[7] == 'r' &&
      name[8] == 'g' &&
      name[9] == 'i' &&
      name[10] == 'n') {
    return Field::MAINT_MARGIN;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_mak(const std::string_view& name) {
  if (name.length() == 8 &&
      name[3] == 'e' &&
      name[4] == 'r' &&
      name[5] == 'F' &&
      name[6] == 'e' &&
      name[7] == 'e') {
    return Field::MAKER_FEE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_markM(const std::string_view& name) {
  if (name.length() == 10 &&
      name[5] == 'e' &&
      name[6] == 't' &&
      name[7] == 'h' &&
      name[8] == 'o' &&
      name[9] == 'd') {
    return Field::MARK_METHOD;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_markP(const std::string_view& name) {
  if (name.length() == 9 &&
      name[5] == 'r' &&
      name[6] == 'i' &&
      name[7] == 'c' &&
      name[8] == 'e') {
    return Field::MARK_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_mar(const std::string_view& name) {
  if (name.length() >= 5 &&
      name[3] == 'k') {
    switch (name[4]) {
      case 'M':
        return parse_markM(name);
      case 'P':
        return parse_markP(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_maxO(const std::string_view& name) {
  if (name.length() == 11 &&
      name[4] == 'r' &&
      name[5] == 'd' &&
      name[6] == 'e' &&
      name[7] == 'r' &&
      name[8] == 'Q' &&
      name[9] == 't' &&
      name[10] == 'y') {
    return Field::MAX_ORDER_QTY;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_maxP(const std::string_view& name) {
  if (name.length() == 8 &&
      name[4] == 'r' &&
      name[5] == 'i' &&
      name[6] == 'c' &&
      name[7] == 'e') {
    return Field::MAX_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_max(const std::string_view& name) {
  if (name.length() > 3) {
    switch (name[3]) {
      case 'O':
        return parse_maxO(name);
      case 'P':
        return parse_maxP(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ma(const std::string_view& name) {
  if (name.length() > 2) {
    switch (name[2]) {
      case 'i':
        return parse_mai(name);
      case 'k':
        return parse_mak(name);
      case 'r':
        return parse_mar(name);
      case 'x':
        return parse_max(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_mi(const std::string_view& name) {
  if (name.length() == 8 &&
      name[2] == 'd' &&
      name[3] == 'P' &&
      name[4] == 'r' &&
      name[5] == 'i' &&
      name[6] == 'c' &&
      name[7] == 'e') {
    return Field::MID_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_mu(const std::string_view& name) {
  if (name.length() == 10 &&
      name[2] == 'l' &&
      name[3] == 't' &&
      name[4] == 'i' &&
      name[5] == 'p' &&
      name[6] == 'l' &&
      name[7] == 'i' &&
      name[8] == 'e' &&
      name[9] == 'r') {
    return Field::MULTIPLIER;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_m(const std::string_view& name) {
  if (name.length() > 1) {
    switch (name[1]) {
      case 'a':
        return parse_ma(name);
      case 'i':
        return parse_mi(name);
      case 'u':
        return parse_mu(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_openI(const std::string_view& name) {
  if (name.length() == 12 &&
      name[5] == 'n' &&
      name[6] == 't' &&
      name[7] == 'e' &&
      name[8] == 'r' &&
      name[9] == 'e' &&
      name[10] == 's' &&
      name[11] == 't') {
    return Field::OPEN_INTEREST;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_openV(const std::string_view& name) {
  if (name.length() == 9 &&
      name[5] == 'a' &&
      name[6] == 'l' &&
      name[7] == 'u' &&
      name[8] == 'e') {
    return Field::OPEN_VALUE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_openi(const std::string_view& name) {
  if (name.length() == 16 &&
      name[5] == 'n' &&
      name[6] == 'g' &&
      name[7] == 'T' &&
      name[8] == 'i' &&
      name[9] == 'm' &&
      name[10] == 'e' &&
      name[11] == 's' &&
      name[12] == 't' &&
      name[13] == 'a' &&
      name[14] == 'm' &&
      name[15] == 'p') {
    return Field::OPENING_TIMESTAMP;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ope(const std::string_view& name) {
  if (name.length() >= 5 &&
      name[3] == 'n') {
    switch (name[4]) {
      case 'I':
        return parse_openI(name);
      case 'V':
        return parse_openV(name);
      case 'i':
        return parse_openi(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_optionM(const std::string_view& name) {
  if (name.length() == 16 &&
      name[7] == 'u' &&
      name[8] == 'l' &&
      name[9] == 't' &&
      name[10] == 'i' &&
      name[11] == 'p' &&
      name[12] == 'l' &&
      name[13] == 'i' &&
      name[14] == 'e' &&
      name[15] == 'r') {
    return Field::OPTION_MULTIPLIER;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_optionStrikePc(const std::string_view& name) {
  if (name.length() == 16 &&
      name[14] == 'n' &&
      name[15] == 't') {
    return Field::OPTION_STRIKE_PCNT;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_optionStrikePr(const std::string_view& name) {
  if (name.length() == 17 &&
      name[14] == 'i' &&
      name[15] == 'c' &&
      name[16] == 'e') {
    return Field::OPTION_STRIKE_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_optionStrikeP(const std::string_view& name) {
  if (name.length() > 13) {
    switch (name[13]) {
      case 'c':
        return parse_optionStrikePc(name);
      case 'r':
        return parse_optionStrikePr(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_optionStrikeR(const std::string_view& name) {
  if (name.length() == 17 &&
      name[13] == 'o' &&
      name[14] == 'u' &&
      name[15] == 'n' &&
      name[16] == 'd') {
    return Field::OPTION_STRIKE_ROUND;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_optionS(const std::string_view& name) {
  if (name.length() >= 13 &&
      name[7] == 't' &&
      name[8] == 'r' &&
      name[9] == 'i' &&
      name[10] == 'k' &&
      name[11] == 'e') {
    switch (name[12]) {
      case 'P':
        return parse_optionStrikeP(name);
      case 'R':
        return parse_optionStrikeR(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_optionU(const std::string_view& name) {
  if (name.length() == 21 &&
      name[7] == 'n' &&
      name[8] == 'd' &&
      name[9] == 'e' &&
      name[10] == 'r' &&
      name[11] == 'l' &&
      name[12] == 'y' &&
      name[13] == 'i' &&
      name[14] == 'n' &&
      name[15] == 'g' &&
      name[16] == 'P' &&
      name[17] == 'r' &&
      name[18] == 'i' &&
      name[19] == 'c' &&
      name[20] == 'e') {
    return Field::OPTION_UNDERLYING_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_opt(const std::string_view& name) {
  if (name.length() >= 7 &&
      name[3] == 'i' &&
      name[4] == 'o' &&
      name[5] == 'n') {
    switch (name[6]) {
      case 'M':
        return parse_optionM(name);
      case 'S':
        return parse_optionS(name);
      case 'U':
        return parse_optionU(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_o(const std::string_view& name) {
  if (name.length() >= 3 &&
      name[1] == 'p') {
    switch (name[2]) {
      case 'e':
        return parse_ope(name);
      case 't':
        return parse_opt(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_po(const std::string_view& name) {
  if (name.length() == 16 &&
      name[2] == 's' &&
      name[3] == 'i' &&
      name[4] == 't' &&
      name[5] == 'i' &&
      name[6] == 'o' &&
      name[7] == 'n' &&
      name[8] == 'C' &&
      name[9] == 'u' &&
      name[10] == 'r' &&
      name[11] == 'r' &&
      name[12] == 'e' &&
      name[13] == 'n' &&
      name[14] == 'c' &&
      name[15] == 'y') {
    return Field::POSITION_CURRENCY;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_prevC(const std::string_view& name) {
  if (name.length() == 14 &&
      name[5] == 'l' &&
      name[6] == 'o' &&
      name[7] == 's' &&
      name[8] == 'e' &&
      name[9] == 'P' &&
      name[10] == 'r' &&
      name[11] == 'i' &&
      name[12] == 'c' &&
      name[13] == 'e') {
    return Field::PREV_CLOSE_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_prevP(const std::string_view& name) {
  if (name.length() == 12 &&
      name[5] == 'r' &&
      name[6] == 'i' &&
      name[7] == 'c' &&
      name[8] == 'e' &&
      name[9] == '2' &&
      name[10] == '4' &&
      name[11] == 'h') {
    return Field::PREV_PRICE24H;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_prevTotalT(const std::string_view& name) {
  if (name.length() == 17 &&
      name[10] == 'u' &&
      name[11] == 'r' &&
      name[12] == 'n' &&
      name[13] == 'o' &&
      name[14] == 'v' &&
      name[15] == 'e' &&
      name[16] == 'r') {
    return Field::PREV_TOTAL_TURNOVER;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_prevTotalV(const std::string_view& name) {
  if (name.length() == 15 &&
      name[10] == 'o' &&
      name[11] == 'l' &&
      name[12] == 'u' &&
      name[13] == 'm' &&
      name[14] == 'e') {
    return Field::PREV_TOTAL_VOLUME;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_prevT(const std::string_view& name) {
  if (name.length() >= 10 &&
      name[5] == 'o' &&
      name[6] == 't' &&
      name[7] == 'a' &&
      name[8] == 'l') {
    switch (name[9]) {
      case 'T':
        return parse_prevTotalT(name);
      case 'V':
        return parse_prevTotalV(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_pr(const std::string_view& name) {
  if (name.length() >= 5 &&
      name[2] == 'e' &&
      name[3] == 'v') {
    switch (name[4]) {
      case 'C':
        return parse_prevC(name);
      case 'P':
        return parse_prevP(name);
      case 'T':
        return parse_prevT(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_publishI(const std::string_view& name) {
  if (name.length() == 15 &&
      name[8] == 'n' &&
      name[9] == 't' &&
      name[10] == 'e' &&
      name[11] == 'r' &&
      name[12] == 'v' &&
      name[13] == 'a' &&
      name[14] == 'l') {
    return Field::PUBLISH_INTERVAL;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_publishT(const std::string_view& name) {
  if (name.length() == 11 &&
      name[8] == 'i' &&
      name[9] == 'm' &&
      name[10] == 'e') {
    return Field::PUBLISH_TIME;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_pu(const std::string_view& name) {
  if (name.length() >= 8 &&
      name[2] == 'b' &&
      name[3] == 'l' &&
      name[4] == 'i' &&
      name[5] == 's' &&
      name[6] == 'h') {
    switch (name[7]) {
      case 'I':
        return parse_publishI(name);
      case 'T':
        return parse_publishT(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_p(const std::string_view& name) {
  if (name.length() > 1) {
    switch (name[1]) {
      case 'o':
        return parse_po(name);
      case 'r':
        return parse_pr(name);
      case 'u':
        return parse_pu(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_quoteC(const std::string_view& name) {
  if (name.length() == 13 &&
      name[6] == 'u' &&
      name[7] == 'r' &&
      name[8] == 'r' &&
      name[9] == 'e' &&
      name[10] == 'n' &&
      name[11] == 'c' &&
      name[12] == 'y') {
    return Field::QUOTE_CURRENCY;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_quoteT(const std::string_view& name) {
  if (name.length() == 23 &&
      name[6] == 'o' &&
      name[7] == 'S' &&
      name[8] == 'e' &&
      name[9] == 't' &&
      name[10] == 't' &&
      name[11] == 'l' &&
      name[12] == 'e' &&
      name[13] == 'M' &&
      name[14] == 'u' &&
      name[15] == 'l' &&
      name[16] == 't' &&
      name[17] == 'i' &&
      name[18] == 'p' &&
      name[19] == 'l' &&
      name[20] == 'i' &&
      name[21] == 'e' &&
      name[22] == 'r') {
    return Field::QUOTE_TO_SETTLE_MULTIPLIER;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_q(const std::string_view& name) {
  if (name.length() >= 6 &&
      name[1] == 'u' &&
      name[2] == 'o' &&
      name[3] == 't' &&
      name[4] == 'e') {
    switch (name[5]) {
      case 'C':
        return parse_quoteC(name);
      case 'T':
        return parse_quoteT(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_rebalanceI(const std::string_view& name) {
  if (name.length() == 17 &&
      name[10] == 'n' &&
      name[11] == 't' &&
      name[12] == 'e' &&
      name[13] == 'r' &&
      name[14] == 'v' &&
      name[15] == 'a' &&
      name[16] == 'l') {
    return Field::REBALANCE_INTERVAL;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_rebalanceT(const std::string_view& name) {
  if (name.length() == 18 &&
      name[10] == 'i' &&
      name[11] == 'm' &&
      name[12] == 'e' &&
      name[13] == 's' &&
      name[14] == 't' &&
      name[15] == 'a' &&
      name[16] == 'm' &&
      name[17] == 'p') {
    return Field::REBALANCE_TIMESTAMP;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_reb(const std::string_view& name) {
  if (name.length() >= 10 &&
      name[3] == 'a' &&
      name[4] == 'l' &&
      name[5] == 'a' &&
      name[6] == 'n' &&
      name[7] == 'c' &&
      name[8] == 'e') {
    switch (name[9]) {
      case 'I':
        return parse_rebalanceI(name);
      case 'T':
        return parse_rebalanceT(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_reference(const std::string_view& name) {
  if (name.length() == 15 &&
      name[9] == 'S' &&
      name[10] == 'y' &&
      name[11] == 'm' &&
      name[12] == 'b' &&
      name[13] == 'o' &&
      name[14] == 'l') {
    return Field::REFERENCE_SYMBOL;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ref(const std::string_view& name) {
  if (name.length() >= 9 &&
      name[3] == 'e' &&
      name[4] == 'r' &&
      name[5] == 'e' &&
      name[6] == 'n' &&
      name[7] == 'c' &&
      name[8] == 'e') {
    if (name.length() == 9)
      return Field::REFERENCE;
    return parse_reference(name);
  }
  return Field::UNKNOWN;
}

constexpr Field parse_rel(const std::string_view& name) {
  if (name.length() == 14 &&
      name[3] == 'i' &&
      name[4] == 's' &&
      name[5] == 't' &&
      name[6] == 'I' &&
      name[7] == 'n' &&
      name[8] == 't' &&
      name[9] == 'e' &&
      name[10] == 'r' &&
      name[11] == 'v' &&
      name[12] == 'a' &&
      name[13] == 'l') {
    return Field::RELIST_INTERVAL;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_re(const std::string_view& name) {
  if (name.length() > 2) {
    switch (name[2]) {
      case 'b':
        return parse_reb(name);
      case 'f':
        return parse_ref(name);
      case 'l':
        return parse_rel(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_riskL(const std::string_view& name) {
  if (name.length() == 9 &&
      name[5] == 'i' &&
      name[6] == 'm' &&
      name[7] == 'i' &&
      name[8] == 't') {
    return Field::RISK_LIMIT;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_riskS(const std::string_view& name) {
  if (name.length() == 8 &&
      name[5] == 't' &&
      name[6] == 'e' &&
      name[7] == 'p') {
    return Field::RISK_STEP;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ri(const std::string_view& name) {
  if (name.length() >= 5 &&
      name[2] == 's' &&
      name[3] == 'k') {
    switch (name[4]) {
      case 'L':
        return parse_riskL(name);
      case 'S':
        return parse_riskS(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ro(const std::string_view& name) {
  if (name.length() == 10 &&
      name[2] == 'o' &&
      name[3] == 't' &&
      name[4] == 'S' &&
      name[5] == 'y' &&
      name[6] == 'm' &&
      name[7] == 'b' &&
      name[8] == 'o' &&
      name[9] == 'l') {
    return Field::ROOT_SYMBOL;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_r(const std::string_view& name) {
  if (name.length() > 1) {
    switch (name[1]) {
      case 'e':
        return parse_re(name);
      case 'i':
        return parse_ri(name);
      case 'o':
        return parse_ro(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_sel(const std::string_view& name) {
  if (name.length() == 7 &&
      name[3] == 'l' &&
      name[4] == 'L' &&
      name[5] == 'e' &&
      name[6] == 'g') {
    return Field::SELL_LEG;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ses(const std::string_view& name) {
  if (name.length() == 15 &&
      name[3] == 's' &&
      name[4] == 'i' &&
      name[5] == 'o' &&
      name[6] == 'n' &&
      name[7] == 'I' &&
      name[8] == 'n' &&
      name[9] == 't' &&
      name[10] == 'e' &&
      name[11] == 'r' &&
      name[12] == 'v' &&
      name[13] == 'a' &&
      name[14] == 'l') {
    return Field::SESSION_INTERVAL;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_settlC(const std::string_view& name) {
  if (name.length() == 13 &&
      name[6] == 'u' &&
      name[7] == 'r' &&
      name[8] == 'r' &&
      name[9] == 'e' &&
      name[10] == 'n' &&
      name[11] == 'c' &&
      name[12] == 'y') {
    return Field::SETTL_CURRENCY;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_settled(const std::string_view& name) {
  if (name.length() == 12 &&
      name[7] == 'P' &&
      name[8] == 'r' &&
      name[9] == 'i' &&
      name[10] == 'c' &&
      name[11] == 'e') {
    return Field::SETTLED_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_settlem(const std::string_view& name) {
  if (name.length() == 13 &&
      name[7] == 'e' &&
      name[8] == 'n' &&
      name[9] == 't' &&
      name[10] == 'F' &&
      name[11] == 'e' &&
      name[12] == 'e') {
    return Field::SETTLEMENT_FEE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_settle(const std::string_view& name) {
  if (name.length() >= 6) {
    if (name.length() == 6)
      return Field::SETTLE;
    switch (name[6]) {
      case 'd':
        return parse_settled(name);
      case 'm':
        return parse_settlem(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_set(const std::string_view& name) {
  if (name.length() >= 6 &&
      name[3] == 't' &&
      name[4] == 'l') {
    switch (name[5]) {
      case 'C':
        return parse_settlC(name);
      case 'e':
        return parse_settle(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_se(const std::string_view& name) {
  if (name.length() > 2) {
    switch (name[2]) {
      case 'l':
        return parse_sel(name);
      case 's':
        return parse_ses(name);
      case 't':
        return parse_set(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_st(const std::string_view& name) {
  if (name.length() == 5 &&
      name[2] == 'a' &&
      name[3] == 't' &&
      name[4] == 'e') {
    return Field::STATE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_sy(const std::string_view& name) {
  if (name.length() == 6 &&
      name[2] == 'm' &&
      name[3] == 'b' &&
      name[4] == 'o' &&
      name[5] == 'l') {
    return Field::SYMBOL;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_s(const std::string_view& name) {
  if (name.length() > 1) {
    switch (name[1]) {
      case 'e':
        return parse_se(name);
      case 't':
        return parse_st(name);
      case 'y':
        return parse_sy(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_tak(const std::string_view& name) {
  if (name.length() == 8 &&
      name[3] == 'e' &&
      name[4] == 'r' &&
      name[5] == 'F' &&
      name[6] == 'e' &&
      name[7] == 'e') {
    return Field::TAKER_FEE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_tax(const std::string_view& name) {
  if (name.length() == 5 &&
      name[3] == 'e' &&
      name[4] == 'd') {
    return Field::TAXED;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ta(const std::string_view& name) {
  if (name.length() > 2) {
    switch (name[2]) {
      case 'k':
        return parse_tak(name);
      case 'x':
        return parse_tax(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_tic(const std::string_view& name) {
  if (name.length() == 8 &&
      name[3] == 'k' &&
      name[4] == 'S' &&
      name[5] == 'i' &&
      name[6] == 'z' &&
      name[7] == 'e') {
    return Field::TICK_SIZE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_tim(const std::string_view& name) {
  if (name.length() == 9 &&
      name[3] == 'e' &&
      name[4] == 's' &&
      name[5] == 't' &&
      name[6] == 'a' &&
      name[7] == 'm' &&
      name[8] == 'p') {
    return Field::TIMESTAMP;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ti(const std::string_view& name) {
  if (name.length() > 2) {
    switch (name[2]) {
      case 'c':
        return parse_tic(name);
      case 'm':
        return parse_tim(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_totalT(const std::string_view& name) {
  if (name.length() == 13 &&
      name[6] == 'u' &&
      name[7] == 'r' &&
      name[8] == 'n' &&
      name[9] == 'o' &&
      name[10] == 'v' &&
      name[11] == 'e' &&
      name[12] == 'r') {
    return Field::TOTAL_TURNOVER;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_totalV(const std::string_view& name) {
  if (name.length() == 11 &&
      name[6] == 'o' &&
      name[7] == 'l' &&
      name[8] == 'u' &&
      name[9] == 'm' &&
      name[10] == 'e') {
    return Field::TOTAL_VOLUME;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_to(const std::string_view& name) {
  if (name.length() >= 6 &&
      name[2] == 't' &&
      name[3] == 'a' &&
      name[4] == 'l') {
    switch (name[5]) {
      case 'T':
        return parse_totalT(name);
      case 'V':
        return parse_totalV(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_turnover(const std::string_view& name) {
  if (name.length() == 11 &&
      name[8] == '2' &&
      name[9] == '4' &&
      name[10] == 'h') {
    return Field::TURNOVER24H;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_tu(const std::string_view& name) {
  if (name.length() >= 8 &&
      name[2] == 'r' &&
      name[3] == 'n' &&
      name[4] == 'o' &&
      name[5] == 'v' &&
      name[6] == 'e' &&
      name[7] == 'r') {
    if (name.length() == 8)
      return Field::TURNOVER;
    return parse_turnover(name);
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ty(const std::string_view& name) {
  if (name.length() == 3 &&
      name[2] == 'p') {
    return Field::TYP;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_t(const std::string_view& name) {
  if (name.length() > 1) {
    switch (name[1]) {
      case 'a':
        return parse_ta(name);
      case 'i':
        return parse_ti(name);
      case 'o':
        return parse_to(name);
      case 'u':
        return parse_tu(name);
      case 'y':
        return parse_ty(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_underlyingS(const std::string_view& name) {
  if (name.length() == 16 &&
      name[11] == 'y' &&
      name[12] == 'm' &&
      name[13] == 'b' &&
      name[14] == 'o' &&
      name[15] == 'l') {
    return Field::UNDERLYING_SYMBOL;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_underlyingToP(const std::string_view& name) {
  if (name.length() == 30 &&
      name[13] == 'o' &&
      name[14] == 's' &&
      name[15] == 'i' &&
      name[16] == 't' &&
      name[17] == 'i' &&
      name[18] == 'o' &&
      name[19] == 'n' &&
      name[20] == 'M' &&
      name[21] == 'u' &&
      name[22] == 'l' &&
      name[23] == 't' &&
      name[24] == 'i' &&
      name[25] == 'p' &&
      name[26] == 'l' &&
      name[27] == 'i' &&
      name[28] == 'e' &&
      name[29] == 'r') {
    return Field::UNDERLYING_TO_POSITION_MULTIPLIER;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_underlyingToS(const std::string_view& name) {
  if (name.length() == 28 &&
      name[13] == 'e' &&
      name[14] == 't' &&
      name[15] == 't' &&
      name[16] == 'l' &&
      name[17] == 'e' &&
      name[18] == 'M' &&
      name[19] == 'u' &&
      name[20] == 'l' &&
      name[21] == 't' &&
      name[22] == 'i' &&
      name[23] == 'p' &&
      name[24] == 'l' &&
      name[25] == 'i' &&
      name[26] == 'e' &&
      name[27] == 'r') {
    return Field::UNDERLYING_TO_SETTLE_MULTIPLIER;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_underlyingT(const std::string_view& name) {
  if (name.length() >= 13 &&
      name[11] == 'o') {
    switch (name[12]) {
      case 'P':
        return parse_underlyingToP(name);
      case 'S':
        return parse_underlyingToS(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_underlying(const std::string_view& name) {
  if (name.length() > 10) {
    switch (name[10]) {
      case 'S':
        return parse_underlyingS(name);
      case 'T':
        return parse_underlyingT(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_u(const std::string_view& name) {
  if (name.length() >= 10 &&
      name[1] == 'n' &&
      name[2] == 'd' &&
      name[3] == 'e' &&
      name[4] == 'r' &&
      name[5] == 'l' &&
      name[6] == 'y' &&
      name[7] == 'i' &&
      name[8] == 'n' &&
      name[9] == 'g') {
    if (name.length() == 10)
      return Field::UNDERLYING;
    return parse_underlying(name);
  }
  return Field::UNKNOWN;
}

constexpr Field parse_volume(const std::string_view& name) {
  if (name.length() == 9 &&
      name[6] == '2' &&
      name[7] == '4' &&
      name[8] == 'h') {
    return Field::VOLUME24H;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_vo(const std::string_view& name) {
  if (name.length() >= 6 &&
      name[2] == 'l' &&
      name[3] == 'u' &&
      name[4] == 'm' &&
      name[5] == 'e') {
    if (name.length() == 6)
      return Field::VOLUME;
    return parse_volume(name);
  }
  return Field::UNKNOWN;
}

constexpr Field parse_vw(const std::string_view& name) {
  if (name.length() == 4 &&
      name[2] == 'a' &&
      name[3] == 'p') {
    return Field::VWAP;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_v(const std::string_view& name) {
  if (name.length() > 1) {
    switch (name[1]) {
      case 'o':
        return parse_vo(name);
      case 'w':
        return parse_vw(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  if (name.length() > 0) {
    switch (name[0]) {
      case 'a':
        return parse_a(name);
      case 'b':
        return parse_b(name);
      case 'c':
        return parse_c(name);
      case 'd':
        return parse_d(name);
      case 'e':
        return parse_e(name);
      case 'f':
        return parse_f(name);
      case 'h':
        return parse_h(name);
      case 'i':
        return parse_i(name);
      case 'l':
        return parse_l(name);
      case 'm':
        return parse_m(name);
      case 'o':
        return parse_o(name);
      case 'p':
        return parse_p(name);
      case 'q':
        return parse_q(name);
      case 'r':
        return parse_r(name);
      case 's':
        return parse_s(name);
      case 't':
        return parse_t(name);
      case 'u':
        return parse_u(name);
      case 'v':
        return parse_v(name);
    }
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("askPrice") == Field::ASK_PRICE);
static_assert(parse_name("bankruptLimitDownPrice") == Field::BANKRUPT_LIMIT_DOWN_PRICE);
static_assert(parse_name("bankruptLimitUpPrice") == Field::BANKRUPT_LIMIT_UP_PRICE);
static_assert(parse_name("bidPrice") == Field::BID_PRICE);
static_assert(parse_name("buyLeg") == Field::BUY_LEG);
static_assert(parse_name("calcInterval") == Field::CALC_INTERVAL);
static_assert(parse_name("capped") == Field::CAPPED);
static_assert(parse_name("closingTimestamp") == Field::CLOSING_TIMESTAMP);
static_assert(parse_name("deleverage") == Field::DELEVERAGE);
static_assert(parse_name("expiry") == Field::EXPIRY);
static_assert(parse_name("fairBasis") == Field::FAIR_BASIS);
static_assert(parse_name("fairBasisRate") == Field::FAIR_BASIS_RATE);
static_assert(parse_name("fairMethod") == Field::FAIR_METHOD);
static_assert(parse_name("fairPrice") == Field::FAIR_PRICE);
static_assert(parse_name("foreignNotional24h") == Field::FOREIGN_NOTIONAL24H);
static_assert(parse_name("front") == Field::FRONT);
static_assert(parse_name("fundingBaseSymbol") == Field::FUNDING_BASE_SYMBOL);
static_assert(parse_name("fundingInterval") == Field::FUNDING_INTERVAL);
static_assert(parse_name("fundingPremiumSymbol") == Field::FUNDING_PREMIUM_SYMBOL);
static_assert(parse_name("fundingQuoteSymbol") == Field::FUNDING_QUOTE_SYMBOL);
static_assert(parse_name("fundingRate") == Field::FUNDING_RATE);
static_assert(parse_name("fundingTimestamp") == Field::FUNDING_TIMESTAMP);
static_assert(parse_name("hasLiquidity") == Field::HAS_LIQUIDITY);
static_assert(parse_name("highPrice") == Field::HIGH_PRICE);
static_assert(parse_name("homeNotional24h") == Field::HOME_NOTIONAL24H);
static_assert(parse_name("impactAskPrice") == Field::IMPACT_ASK_PRICE);
static_assert(parse_name("impactBidPrice") == Field::IMPACT_BID_PRICE);
static_assert(parse_name("impactMidPrice") == Field::IMPACT_MID_PRICE);
static_assert(parse_name("indicativeFundingRate") == Field::INDICATIVE_FUNDING_RATE);
static_assert(parse_name("indicativeSettlePrice") == Field::INDICATIVE_SETTLE_PRICE);
static_assert(parse_name("indicativeTaxRate") == Field::INDICATIVE_TAX_RATE);
static_assert(parse_name("initMargin") == Field::INIT_MARGIN);
static_assert(parse_name("insuranceFee") == Field::INSURANCE_FEE);
static_assert(parse_name("inverseLeg") == Field::INVERSE_LEG);
static_assert(parse_name("isInverse") == Field::IS_INVERSE);
static_assert(parse_name("isQuanto") == Field::IS_QUANTO);
static_assert(parse_name("lastChangePcnt") == Field::LAST_CHANGE_PCNT);
static_assert(parse_name("lastPrice") == Field::LAST_PRICE);
static_assert(parse_name("lastPriceProtected") == Field::LAST_PRICE_PROTECTED);
static_assert(parse_name("lastTickDirection") == Field::LAST_TICK_DIRECTION);
static_assert(parse_name("limit") == Field::LIMIT);
static_assert(parse_name("limitDownPrice") == Field::LIMIT_DOWN_PRICE);
static_assert(parse_name("limitUpPrice") == Field::LIMIT_UP_PRICE);
static_assert(parse_name("listing") == Field::LISTING);
static_assert(parse_name("lotSize") == Field::LOT_SIZE);
static_assert(parse_name("lowPrice") == Field::LOW_PRICE);
static_assert(parse_name("maintMargin") == Field::MAINT_MARGIN);
static_assert(parse_name("makerFee") == Field::MAKER_FEE);
static_assert(parse_name("markMethod") == Field::MARK_METHOD);
static_assert(parse_name("markPrice") == Field::MARK_PRICE);
static_assert(parse_name("maxOrderQty") == Field::MAX_ORDER_QTY);
static_assert(parse_name("maxPrice") == Field::MAX_PRICE);
static_assert(parse_name("midPrice") == Field::MID_PRICE);
static_assert(parse_name("multiplier") == Field::MULTIPLIER);
static_assert(parse_name("openInterest") == Field::OPEN_INTEREST);
static_assert(parse_name("openValue") == Field::OPEN_VALUE);
static_assert(parse_name("openingTimestamp") == Field::OPENING_TIMESTAMP);
static_assert(parse_name("optionMultiplier") == Field::OPTION_MULTIPLIER);
static_assert(parse_name("optionStrikePcnt") == Field::OPTION_STRIKE_PCNT);
static_assert(parse_name("optionStrikePrice") == Field::OPTION_STRIKE_PRICE);
static_assert(parse_name("optionStrikeRound") == Field::OPTION_STRIKE_ROUND);
static_assert(parse_name("optionUnderlyingPrice") == Field::OPTION_UNDERLYING_PRICE);
static_assert(parse_name("positionCurrency") == Field::POSITION_CURRENCY);
static_assert(parse_name("prevClosePrice") == Field::PREV_CLOSE_PRICE);
static_assert(parse_name("prevPrice24h") == Field::PREV_PRICE24H);
static_assert(parse_name("prevTotalTurnover") == Field::PREV_TOTAL_TURNOVER);
static_assert(parse_name("prevTotalVolume") == Field::PREV_TOTAL_VOLUME);
static_assert(parse_name("publishInterval") == Field::PUBLISH_INTERVAL);
static_assert(parse_name("publishTime") == Field::PUBLISH_TIME);
static_assert(parse_name("quoteCurrency") == Field::QUOTE_CURRENCY);
static_assert(parse_name("quoteToSettleMultiplier") == Field::QUOTE_TO_SETTLE_MULTIPLIER);
static_assert(parse_name("rebalanceInterval") == Field::REBALANCE_INTERVAL);
static_assert(parse_name("rebalanceTimestamp") == Field::REBALANCE_TIMESTAMP);
static_assert(parse_name("reference") == Field::REFERENCE);
static_assert(parse_name("referenceSymbol") == Field::REFERENCE_SYMBOL);
static_assert(parse_name("relistInterval") == Field::RELIST_INTERVAL);
static_assert(parse_name("riskLimit") == Field::RISK_LIMIT);
static_assert(parse_name("riskStep") == Field::RISK_STEP);
static_assert(parse_name("rootSymbol") == Field::ROOT_SYMBOL);
static_assert(parse_name("sellLeg") == Field::SELL_LEG);
static_assert(parse_name("sessionInterval") == Field::SESSION_INTERVAL);
static_assert(parse_name("settlCurrency") == Field::SETTL_CURRENCY);
static_assert(parse_name("settle") == Field::SETTLE);
static_assert(parse_name("settledPrice") == Field::SETTLED_PRICE);
static_assert(parse_name("settlementFee") == Field::SETTLEMENT_FEE);
static_assert(parse_name("state") == Field::STATE);
static_assert(parse_name("symbol") == Field::SYMBOL);
static_assert(parse_name("takerFee") == Field::TAKER_FEE);
static_assert(parse_name("taxed") == Field::TAXED);
static_assert(parse_name("tickSize") == Field::TICK_SIZE);
static_assert(parse_name("timestamp") == Field::TIMESTAMP);
static_assert(parse_name("totalTurnover") == Field::TOTAL_TURNOVER);
static_assert(parse_name("totalVolume") == Field::TOTAL_VOLUME);
static_assert(parse_name("turnover") == Field::TURNOVER);
static_assert(parse_name("turnover24h") == Field::TURNOVER24H);
static_assert(parse_name("typ") == Field::TYP);
static_assert(parse_name("underlying") == Field::UNDERLYING);
static_assert(parse_name("underlyingSymbol") == Field::UNDERLYING_SYMBOL);
static_assert(parse_name("underlyingToPositionMultiplier") == Field::UNDERLYING_TO_POSITION_MULTIPLIER);
static_assert(parse_name("underlyingToSettleMultiplier") == Field::UNDERLYING_TO_SETTLE_MULTIPLIER);
static_assert(parse_name("volume") == Field::VOLUME);
static_assert(parse_name("volume24h") == Field::VOLUME24H);
static_assert(parse_name("vwap") == Field::VWAP);

void update_field(
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
    case Field::ASK_PRICE:
      update(result.ask_price, value);
      break;
    case Field::BANKRUPT_LIMIT_DOWN_PRICE:
      update(result.bankrupt_limit_down_price, value);
      break;
    case Field::BANKRUPT_LIMIT_UP_PRICE:
      update(result.bankrupt_limit_up_price, value);
      break;
    case Field::BID_PRICE:
      update(result.bid_price, value);
      break;
    case Field::BUY_LEG:
      update(result.buy_leg, value);
      break;
    case Field::CALC_INTERVAL:
      update(result.calc_interval, value);
      break;
    case Field::CAPPED:
      update(result.capped, value);
      break;
    case Field::CLOSING_TIMESTAMP:
      update(result.closing_timestamp, value);
      break;
    case Field::DELEVERAGE:
      update(result.deleverage, value);
      break;
    case Field::EXPIRY:
      update(result.expiry, value);
      break;
    case Field::FAIR_BASIS:
      update(result.fair_basis, value);
      break;
    case Field::FAIR_BASIS_RATE:
      update(result.fair_basis_rate, value);
      break;
    case Field::FAIR_METHOD:
      update(result.fair_method, value);
      break;
    case Field::FAIR_PRICE:
      update(result.fair_price, value);
      break;
    case Field::FOREIGN_NOTIONAL24H:
      update(result.foreign_notional24h, value);
      break;
    case Field::FRONT:
      update(result.front, value);
      break;
    case Field::FUNDING_BASE_SYMBOL:
      update(result.funding_base_symbol, value);
      break;
    case Field::FUNDING_INTERVAL:
      update(result.funding_interval, value);
      break;
    case Field::FUNDING_PREMIUM_SYMBOL:
      update(result.funding_premium_symbol, value);
      break;
    case Field::FUNDING_QUOTE_SYMBOL:
      update(result.funding_quote_symbol, value);
      break;
    case Field::FUNDING_RATE:
      update(result.funding_rate, value);
      break;
    case Field::FUNDING_TIMESTAMP:
      update(result.funding_timestamp, value);
      break;
    case Field::HAS_LIQUIDITY:
      update(result.has_liquidity, value);
      break;
    case Field::HIGH_PRICE:
      update(result.high_price, value);
      break;
    case Field::HOME_NOTIONAL24H:
      update(result.home_notional24h, value);
      break;
    case Field::IMPACT_ASK_PRICE:
      update(result.impact_ask_price, value);
      break;
    case Field::IMPACT_BID_PRICE:
      update(result.impact_bid_price, value);
      break;
    case Field::IMPACT_MID_PRICE:
      update(result.impact_mid_price, value);
      break;
    case Field::INDICATIVE_FUNDING_RATE:
      update(result.indicative_funding_rate, value);
      break;
    case Field::INDICATIVE_SETTLE_PRICE:
      update(result.indicative_settle_price, value);
      break;
    case Field::INDICATIVE_TAX_RATE:
      update(result.indicative_tax_rate, value);
      break;
    case Field::INIT_MARGIN:
      update(result.init_margin, value);
      break;
    case Field::INSURANCE_FEE:
      update(result.insurance_fee, value);
      break;
    case Field::INVERSE_LEG:
      update(result.inverse_leg, value);
      break;
    case Field::IS_INVERSE:
      update(result.is_inverse, value);
      break;
    case Field::IS_QUANTO:
      update(result.is_quanto, value);
      break;
    case Field::LAST_CHANGE_PCNT:
      update(result.last_change_pcnt, value);
      break;
    case Field::LAST_PRICE:
      update(result.last_price, value);
      break;
    case Field::LAST_PRICE_PROTECTED:
      update(result.last_price_protected, value);
      break;
    case Field::LAST_TICK_DIRECTION:
      update(result.last_tick_direction, value);
      break;
    case Field::LIMIT:
      update(result.limit, value);
      break;
    case Field::LIMIT_DOWN_PRICE:
      update(result.limit_down_price, value);
      break;
    case Field::LIMIT_UP_PRICE:
      update(result.limit_up_price, value);
      break;
    case Field::LISTING:
      update(result.listing, value);
      break;
    case Field::LOT_SIZE:
      update(result.lot_size, value);
      break;
    case Field::LOW_PRICE:
      update(result.low_price, value);
      break;
    case Field::MAINT_MARGIN:
      update(result.maint_margin, value);
      break;
    case Field::MAKER_FEE:
      update(result.maker_fee, value);
      break;
    case Field::MARK_METHOD:
      update(result.mark_method, value);
      break;
    case Field::MARK_PRICE:
      update(result.mark_price, value);
      break;
    case Field::MAX_ORDER_QTY:
      update(result.max_order_qty, value);
      break;
    case Field::MAX_PRICE:
      update(result.max_price, value);
      break;
    case Field::MID_PRICE:
      update(result.mid_price, value);
      break;
    case Field::MULTIPLIER:
      update(result.multiplier, value);
      break;
    case Field::OPEN_INTEREST:
      update(result.open_interest, value);
      break;
    case Field::OPEN_VALUE:
      update(result.open_value, value);
      break;
    case Field::OPENING_TIMESTAMP:
      update(result.opening_timestamp, value);
      break;
    case Field::OPTION_MULTIPLIER:
      update(result.option_multiplier, value);
      break;
    case Field::OPTION_STRIKE_PCNT:
      update(result.option_strike_pcnt, value);
      break;
    case Field::OPTION_STRIKE_PRICE:
      update(result.option_strike_price, value);
      break;
    case Field::OPTION_STRIKE_ROUND:
      update(result.option_strike_round, value);
      break;
    case Field::OPTION_UNDERLYING_PRICE:
      update(result.option_underlying_price, value);
      break;
    case Field::POSITION_CURRENCY:
      update(result.position_currency, value);
      break;
    case Field::PREV_CLOSE_PRICE:
      update(result.prev_close_price, value);
      break;
    case Field::PREV_PRICE24H:
      update(result.prev_price24h, value);
      break;
    case Field::PREV_TOTAL_TURNOVER:
      update(result.prev_total_turnover, value);
      break;
    case Field::PREV_TOTAL_VOLUME:
      update(result.prev_total_volume, value);
      break;
    case Field::PUBLISH_INTERVAL:
      update(result.publish_interval, value);
      break;
    case Field::PUBLISH_TIME:
      update(result.publish_time, value);
      break;
    case Field::QUOTE_CURRENCY:
      update(result.quote_currency, value);
      break;
    case Field::QUOTE_TO_SETTLE_MULTIPLIER:
      update(result.quote_to_settle_multiplier, value);
      break;
    case Field::REBALANCE_INTERVAL:
      update(result.rebalance_interval, value);
      break;
    case Field::REBALANCE_TIMESTAMP:
      update(result.rebalance_timestamp, value);
      break;
    case Field::REFERENCE:
      update(result.reference, value);
      break;
    case Field::REFERENCE_SYMBOL:
      update(result.reference_symbol, value);
      break;
    case Field::RELIST_INTERVAL:
      update(result.relist_interval, value);
      break;
    case Field::RISK_LIMIT:
      update(result.risk_limit, value);
      break;
    case Field::RISK_STEP:
      update(result.risk_step, value);
      break;
    case Field::ROOT_SYMBOL:
      update(result.root_symbol, value);
      break;
    case Field::SELL_LEG:
      update(result.sell_leg, value);
      break;
    case Field::SESSION_INTERVAL:
      update(result.session_interval, value);
      break;
    case Field::SETTL_CURRENCY:
      update(result.settl_currency, value);
      break;
    case Field::SETTLE:
      update(result.settle, value);
      break;
    case Field::SETTLED_PRICE:
      update(result.settled_price, value);
      break;
    case Field::SETTLEMENT_FEE:
      update(result.settlement_fee, value);
      break;
    case Field::STATE:
      update(result.state, value);
      break;
    case Field::SYMBOL:
      update(result.symbol, value);
      break;
    case Field::TAKER_FEE:
      update(result.taker_fee, value);
      break;
    case Field::TAXED:
      update(result.taxed, value);
      break;
    case Field::TICK_SIZE:
      update(result.tick_size, value);
      break;
    case Field::TIMESTAMP:
      update(result.timestamp, value);
      break;
    case Field::TOTAL_TURNOVER:
      update(result.total_turnover, value);
      break;
    case Field::TOTAL_VOLUME:
      update(result.total_volume, value);
      break;
    case Field::TURNOVER:
      update(result.turnover, value);
      break;
    case Field::TURNOVER24H:
      update(result.turnover24h, value);
      break;
    case Field::TYP:
      update(result.typ, value);
      break;
    case Field::UNDERLYING:
      update(result.underlying, value);
      break;
    case Field::UNDERLYING_SYMBOL:
      update(result.underlying_symbol, value);
      break;
    case Field::UNDERLYING_TO_POSITION_MULTIPLIER:
      update(result.underlying_to_position_multiplier, value);
      break;
    case Field::UNDERLYING_TO_SETTLE_MULTIPLIER:
      update(result.underlying_to_settle_multiplier, value);
      break;
    case Field::VOLUME:
      update(result.volume, value);
      break;
    case Field::VOLUME24H:
      update(result.volume24h, value);
      break;
    case Field::VWAP:
      update(result.vwap, value);
      break;
  }
}
}  // namespace

InstrumentItem::InstrumentItem(core::json::value_t& value) {
  for (auto [key, value] : std::get<core::json::object_t>(value))
    update_field(*this, key, value);
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
