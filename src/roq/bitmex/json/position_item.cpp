/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/position_item.h"

#include "roq/bitmex/json/utils.h"

#include "roq/logging.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  ACCOUNT,
  AVG_COST_PRICE,
  AVG_ENTRY_PRICE,
  BANKRUPT_PRICE,
  BREAK_EVEN_PRICE,
  COMMISSION,
  CROSS_MARGIN,
  CURRENCY,
  CURRENT_COMM,
  CURRENT_COST,
  CURRENT_QTY,
  CURRENT_TIMESTAMP,
  DELEVERAGE_PERCENTILE,
  EXEC_BUY_COST,
  EXEC_BUY_QTY,
  EXEC_COMM,
  EXEC_COST,
  EXEC_QTY,
  EXEC_SELL_COST,
  EXEC_SELL_QTY,
  FOREIGN_NOTIONAL,
  GROSS_EXEC_COST,
  GROSS_OPEN_COST,
  GROSS_OPEN_PREMIUM,
  HOME_NOTIONAL,
  INDICATIVE_TAX,
  INDICATIVE_TAX_RATE,
  INIT_MARGIN,
  INIT_MARGIN_REQ,
  IS_OPEN,
  LAST_PRICE,
  LAST_VALUE,
  LEVERAGE,
  LIQUIDATION_PRICE,
  LONG_BANKRUPT,
  MAINT_MARGIN,
  MAINT_MARGIN_REQ,
  MARGIN_CALL_PRICE,
  MARK_PRICE,
  MARK_VALUE,
  OPEN_ORDER_BUY_COST,
  OPEN_ORDER_BUY_PREMIUM,
  OPEN_ORDER_BUY_QTY,
  OPEN_ORDER_SELL_COST,
  OPEN_ORDER_SELL_PREMIUM,
  OPEN_ORDER_SELL_QTY,
  OPENING_COMM,
  OPENING_COST,
  OPENING_QTY,
  OPENING_TIMESTAMP,
  POS_ALLOWANCE,
  POS_COMM,
  POS_COST,
  POS_COST2,
  POS_CROSS,
  POS_INIT,
  POS_LOSS,
  POS_MAINT,
  POS_MARGIN,
  POS_STATE,
  PREV_CLOSE_PRICE,
  PREV_REALISED_PNL,
  PREV_UNREALISED_PNL,
  QUOTE_CURRENCY,
  REALISED_COST,
  REALISED_GROSS_PNL,
  REALISED_PNL,
  REALISED_TAX,
  REBALANCED_PNL,
  RISK_LIMIT,
  RISK_VALUE,
  SESSION_MARGIN,
  SHORT_BANKRUPT,
  SIMPLE_COST,
  SIMPLE_PNL,
  SIMPLE_PNL_PCNT,
  SIMPLE_QTY,
  SIMPLE_VALUE,
  SYMBOL,
  TARGET_EXCESS_MARGIN,
  TAX_BASE,
  TAXABLE_MARGIN,
  TIMESTAMP,
  UNDERLYING,
  UNREALISED_COST,
  UNREALISED_GROSS_PNL,
  UNREALISED_PNL,
  UNREALISED_PNL_PCNT,
  UNREALISED_ROE_PCNT,
  UNREALISED_TAX,
  VAR_MARGIN,
};

constexpr Field parse_ac(const std::string_view& name) {
  if (name.length() == 7 &&
      name[2] == 'c' &&
      name[3] == 'o' &&
      name[4] == 'u' &&
      name[5] == 'n' &&
      name[6] == 't') {
    return Field::ACCOUNT;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_avgC(const std::string_view& name) {
  if (name.length() == 12 &&
      name[4] == 'o' &&
      name[5] == 's' &&
      name[6] == 't' &&
      name[7] == 'P' &&
      name[8] == 'r' &&
      name[9] == 'i' &&
      name[10] == 'c' &&
      name[11] == 'e') {
    return Field::AVG_COST_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_avgE(const std::string_view& name) {
  if (name.length() == 13 &&
      name[4] == 'n' &&
      name[5] == 't' &&
      name[6] == 'r' &&
      name[7] == 'y' &&
      name[8] == 'P' &&
      name[9] == 'r' &&
      name[10] == 'i' &&
      name[11] == 'c' &&
      name[12] == 'e') {
    return Field::AVG_ENTRY_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_av(const std::string_view& name) {
  if (name.length() >= 4 &&
      name[2] == 'g') {
    switch (name[3]) {
      case 'C':
        return parse_avgC(name);
      case 'E':
        return parse_avgE(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_a(const std::string_view& name) {
  if (name.length() > 1) {
    switch (name[1]) {
      case 'c':
        return parse_ac(name);
      case 'v':
        return parse_av(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ba(const std::string_view& name) {
  if (name.length() == 13 &&
      name[2] == 'n' &&
      name[3] == 'k' &&
      name[4] == 'r' &&
      name[5] == 'u' &&
      name[6] == 'p' &&
      name[7] == 't' &&
      name[8] == 'P' &&
      name[9] == 'r' &&
      name[10] == 'i' &&
      name[11] == 'c' &&
      name[12] == 'e') {
    return Field::BANKRUPT_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_br(const std::string_view& name) {
  if (name.length() == 14 &&
      name[2] == 'e' &&
      name[3] == 'a' &&
      name[4] == 'k' &&
      name[5] == 'E' &&
      name[6] == 'v' &&
      name[7] == 'e' &&
      name[8] == 'n' &&
      name[9] == 'P' &&
      name[10] == 'r' &&
      name[11] == 'i' &&
      name[12] == 'c' &&
      name[13] == 'e') {
    return Field::BREAK_EVEN_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_b(const std::string_view& name) {
  if (name.length() > 1) {
    switch (name[1]) {
      case 'a':
        return parse_ba(name);
      case 'r':
        return parse_br(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_co(const std::string_view& name) {
  if (name.length() == 10 &&
      name[2] == 'm' &&
      name[3] == 'm' &&
      name[4] == 'i' &&
      name[5] == 's' &&
      name[6] == 's' &&
      name[7] == 'i' &&
      name[8] == 'o' &&
      name[9] == 'n') {
    return Field::COMMISSION;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_cr(const std::string_view& name) {
  if (name.length() == 11 &&
      name[2] == 'o' &&
      name[3] == 's' &&
      name[4] == 's' &&
      name[5] == 'M' &&
      name[6] == 'a' &&
      name[7] == 'r' &&
      name[8] == 'g' &&
      name[9] == 'i' &&
      name[10] == 'n') {
    return Field::CROSS_MARGIN;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_currenc(const std::string_view& name) {
  if (name.length() == 8 &&
      name[7] == 'y') {
    return Field::CURRENCY;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_currentCom(const std::string_view& name) {
  if (name.length() == 11 &&
      name[10] == 'm') {
    return Field::CURRENT_COMM;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_currentCos(const std::string_view& name) {
  if (name.length() == 11 &&
      name[10] == 't') {
    return Field::CURRENT_COST;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_currentC(const std::string_view& name) {
  if (name.length() >= 10 &&
      name[8] == 'o') {
    switch (name[9]) {
      case 'm':
        return parse_currentCom(name);
      case 's':
        return parse_currentCos(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_currentQ(const std::string_view& name) {
  if (name.length() == 10 &&
      name[8] == 't' &&
      name[9] == 'y') {
    return Field::CURRENT_QTY;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_currentT(const std::string_view& name) {
  if (name.length() == 16 &&
      name[8] == 'i' &&
      name[9] == 'm' &&
      name[10] == 'e' &&
      name[11] == 's' &&
      name[12] == 't' &&
      name[13] == 'a' &&
      name[14] == 'm' &&
      name[15] == 'p') {
    return Field::CURRENT_TIMESTAMP;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_current(const std::string_view& name) {
  if (name.length() > 7) {
    switch (name[7]) {
      case 'C':
        return parse_currentC(name);
      case 'Q':
        return parse_currentQ(name);
      case 'T':
        return parse_currentT(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_cu(const std::string_view& name) {
  if (name.length() >= 7 &&
      name[2] == 'r' &&
      name[3] == 'r' &&
      name[4] == 'e' &&
      name[5] == 'n') {
    switch (name[6]) {
      case 'c':
        return parse_currenc(name);
      case 't':
        return parse_current(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_c(const std::string_view& name) {
  if (name.length() > 1) {
    switch (name[1]) {
      case 'o':
        return parse_co(name);
      case 'r':
        return parse_cr(name);
      case 'u':
        return parse_cu(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_d(const std::string_view& name) {
  if (name.length() == 20 &&
      name[1] == 'e' &&
      name[2] == 'l' &&
      name[3] == 'e' &&
      name[4] == 'v' &&
      name[5] == 'e' &&
      name[6] == 'r' &&
      name[7] == 'a' &&
      name[8] == 'g' &&
      name[9] == 'e' &&
      name[10] == 'P' &&
      name[11] == 'e' &&
      name[12] == 'r' &&
      name[13] == 'c' &&
      name[14] == 'e' &&
      name[15] == 'n' &&
      name[16] == 't' &&
      name[17] == 'i' &&
      name[18] == 'l' &&
      name[19] == 'e') {
    return Field::DELEVERAGE_PERCENTILE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_execBuyC(const std::string_view& name) {
  if (name.length() == 11 &&
      name[8] == 'o' &&
      name[9] == 's' &&
      name[10] == 't') {
    return Field::EXEC_BUY_COST;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_execBuyQ(const std::string_view& name) {
  if (name.length() == 10 &&
      name[8] == 't' &&
      name[9] == 'y') {
    return Field::EXEC_BUY_QTY;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_execB(const std::string_view& name) {
  if (name.length() >= 8 &&
      name[5] == 'u' &&
      name[6] == 'y') {
    switch (name[7]) {
      case 'C':
        return parse_execBuyC(name);
      case 'Q':
        return parse_execBuyQ(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_execCom(const std::string_view& name) {
  if (name.length() == 8 &&
      name[7] == 'm') {
    return Field::EXEC_COMM;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_execCos(const std::string_view& name) {
  if (name.length() == 8 &&
      name[7] == 't') {
    return Field::EXEC_COST;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_execC(const std::string_view& name) {
  if (name.length() >= 7 &&
      name[5] == 'o') {
    switch (name[6]) {
      case 'm':
        return parse_execCom(name);
      case 's':
        return parse_execCos(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_execQ(const std::string_view& name) {
  if (name.length() == 7 &&
      name[5] == 't' &&
      name[6] == 'y') {
    return Field::EXEC_QTY;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_execSellC(const std::string_view& name) {
  if (name.length() == 12 &&
      name[9] == 'o' &&
      name[10] == 's' &&
      name[11] == 't') {
    return Field::EXEC_SELL_COST;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_execSellQ(const std::string_view& name) {
  if (name.length() == 11 &&
      name[9] == 't' &&
      name[10] == 'y') {
    return Field::EXEC_SELL_QTY;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_execS(const std::string_view& name) {
  if (name.length() >= 9 &&
      name[5] == 'e' &&
      name[6] == 'l' &&
      name[7] == 'l') {
    switch (name[8]) {
      case 'C':
        return parse_execSellC(name);
      case 'Q':
        return parse_execSellQ(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_e(const std::string_view& name) {
  if (name.length() >= 5 &&
      name[1] == 'x' &&
      name[2] == 'e' &&
      name[3] == 'c') {
    switch (name[4]) {
      case 'B':
        return parse_execB(name);
      case 'C':
        return parse_execC(name);
      case 'Q':
        return parse_execQ(name);
      case 'S':
        return parse_execS(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_f(const std::string_view& name) {
  if (name.length() == 15 &&
      name[1] == 'o' &&
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
      name[14] == 'l') {
    return Field::FOREIGN_NOTIONAL;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_grossE(const std::string_view& name) {
  if (name.length() == 13 &&
      name[6] == 'x' &&
      name[7] == 'e' &&
      name[8] == 'c' &&
      name[9] == 'C' &&
      name[10] == 'o' &&
      name[11] == 's' &&
      name[12] == 't') {
    return Field::GROSS_EXEC_COST;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_grossOpenC(const std::string_view& name) {
  if (name.length() == 13 &&
      name[10] == 'o' &&
      name[11] == 's' &&
      name[12] == 't') {
    return Field::GROSS_OPEN_COST;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_grossOpenP(const std::string_view& name) {
  if (name.length() == 16 &&
      name[10] == 'r' &&
      name[11] == 'e' &&
      name[12] == 'm' &&
      name[13] == 'i' &&
      name[14] == 'u' &&
      name[15] == 'm') {
    return Field::GROSS_OPEN_PREMIUM;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_grossO(const std::string_view& name) {
  if (name.length() >= 10 &&
      name[6] == 'p' &&
      name[7] == 'e' &&
      name[8] == 'n') {
    switch (name[9]) {
      case 'C':
        return parse_grossOpenC(name);
      case 'P':
        return parse_grossOpenP(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_g(const std::string_view& name) {
  if (name.length() >= 6 &&
      name[1] == 'r' &&
      name[2] == 'o' &&
      name[3] == 's' &&
      name[4] == 's') {
    switch (name[5]) {
      case 'E':
        return parse_grossE(name);
      case 'O':
        return parse_grossO(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_h(const std::string_view& name) {
  if (name.length() == 12 &&
      name[1] == 'o' &&
      name[2] == 'm' &&
      name[3] == 'e' &&
      name[4] == 'N' &&
      name[5] == 'o' &&
      name[6] == 't' &&
      name[7] == 'i' &&
      name[8] == 'o' &&
      name[9] == 'n' &&
      name[10] == 'a' &&
      name[11] == 'l') {
    return Field::HOME_NOTIONAL;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_indicativeTax(const std::string_view& name) {
  if (name.length() == 17 &&
      name[13] == 'R' &&
      name[14] == 'a' &&
      name[15] == 't' &&
      name[16] == 'e') {
    return Field::INDICATIVE_TAX_RATE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ind(const std::string_view& name) {
  if (name.length() >= 13 &&
      name[3] == 'i' &&
      name[4] == 'c' &&
      name[5] == 'a' &&
      name[6] == 't' &&
      name[7] == 'i' &&
      name[8] == 'v' &&
      name[9] == 'e' &&
      name[10] == 'T' &&
      name[11] == 'a' &&
      name[12] == 'x') {
    if (name.length() == 13)
      return Field::INDICATIVE_TAX;
    return parse_indicativeTax(name);
  }
  return Field::UNKNOWN;
}

constexpr Field parse_initMargin(const std::string_view& name) {
  if (name.length() == 13 &&
      name[10] == 'R' &&
      name[11] == 'e' &&
      name[12] == 'q') {
    return Field::INIT_MARGIN_REQ;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ini(const std::string_view& name) {
  if (name.length() >= 10 &&
      name[3] == 't' &&
      name[4] == 'M' &&
      name[5] == 'a' &&
      name[6] == 'r' &&
      name[7] == 'g' &&
      name[8] == 'i' &&
      name[9] == 'n') {
    if (name.length() == 10)
      return Field::INIT_MARGIN;
    return parse_initMargin(name);
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
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_is(const std::string_view& name) {
  if (name.length() == 6 &&
      name[2] == 'O' &&
      name[3] == 'p' &&
      name[4] == 'e' &&
      name[5] == 'n') {
    return Field::IS_OPEN;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_i(const std::string_view& name) {
  if (name.length() > 1) {
    switch (name[1]) {
      case 'n':
        return parse_in(name);
      case 's':
        return parse_is(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_lastP(const std::string_view& name) {
  if (name.length() == 9 &&
      name[5] == 'r' &&
      name[6] == 'i' &&
      name[7] == 'c' &&
      name[8] == 'e') {
    return Field::LAST_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_lastV(const std::string_view& name) {
  if (name.length() == 9 &&
      name[5] == 'a' &&
      name[6] == 'l' &&
      name[7] == 'u' &&
      name[8] == 'e') {
    return Field::LAST_VALUE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_la(const std::string_view& name) {
  if (name.length() >= 5 &&
      name[2] == 's' &&
      name[3] == 't') {
    switch (name[4]) {
      case 'P':
        return parse_lastP(name);
      case 'V':
        return parse_lastV(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_le(const std::string_view& name) {
  if (name.length() == 8 &&
      name[2] == 'v' &&
      name[3] == 'e' &&
      name[4] == 'r' &&
      name[5] == 'a' &&
      name[6] == 'g' &&
      name[7] == 'e') {
    return Field::LEVERAGE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_li(const std::string_view& name) {
  if (name.length() == 16 &&
      name[2] == 'q' &&
      name[3] == 'u' &&
      name[4] == 'i' &&
      name[5] == 'd' &&
      name[6] == 'a' &&
      name[7] == 't' &&
      name[8] == 'i' &&
      name[9] == 'o' &&
      name[10] == 'n' &&
      name[11] == 'P' &&
      name[12] == 'r' &&
      name[13] == 'i' &&
      name[14] == 'c' &&
      name[15] == 'e') {
    return Field::LIQUIDATION_PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_lo(const std::string_view& name) {
  if (name.length() == 12 &&
      name[2] == 'n' &&
      name[3] == 'g' &&
      name[4] == 'B' &&
      name[5] == 'a' &&
      name[6] == 'n' &&
      name[7] == 'k' &&
      name[8] == 'r' &&
      name[9] == 'u' &&
      name[10] == 'p' &&
      name[11] == 't') {
    return Field::LONG_BANKRUPT;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_l(const std::string_view& name) {
  if (name.length() > 1) {
    switch (name[1]) {
      case 'a':
        return parse_la(name);
      case 'e':
        return parse_le(name);
      case 'i':
        return parse_li(name);
      case 'o':
        return parse_lo(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_maintMargin(const std::string_view& name) {
  if (name.length() == 14 &&
      name[11] == 'R' &&
      name[12] == 'e' &&
      name[13] == 'q') {
    return Field::MAINT_MARGIN_REQ;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_mai(const std::string_view& name) {
  if (name.length() >= 11 &&
      name[3] == 'n' &&
      name[4] == 't' &&
      name[5] == 'M' &&
      name[6] == 'a' &&
      name[7] == 'r' &&
      name[8] == 'g' &&
      name[9] == 'i' &&
      name[10] == 'n') {
    if (name.length() == 11)
      return Field::MAINT_MARGIN;
    return parse_maintMargin(name);
  }
  return Field::UNKNOWN;
}

constexpr Field parse_marg(const std::string_view& name) {
  if (name.length() == 15 &&
      name[4] == 'i' &&
      name[5] == 'n' &&
      name[6] == 'C' &&
      name[7] == 'a' &&
      name[8] == 'l' &&
      name[9] == 'l' &&
      name[10] == 'P' &&
      name[11] == 'r' &&
      name[12] == 'i' &&
      name[13] == 'c' &&
      name[14] == 'e') {
    return Field::MARGIN_CALL_PRICE;
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

constexpr Field parse_markV(const std::string_view& name) {
  if (name.length() == 9 &&
      name[5] == 'a' &&
      name[6] == 'l' &&
      name[7] == 'u' &&
      name[8] == 'e') {
    return Field::MARK_VALUE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_mark(const std::string_view& name) {
  if (name.length() > 4) {
    switch (name[4]) {
      case 'P':
        return parse_markP(name);
      case 'V':
        return parse_markV(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_mar(const std::string_view& name) {
  if (name.length() > 3) {
    switch (name[3]) {
      case 'g':
        return parse_marg(name);
      case 'k':
        return parse_mark(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_m(const std::string_view& name) {
  if (name.length() >= 3 &&
      name[1] == 'a') {
    switch (name[2]) {
      case 'i':
        return parse_mai(name);
      case 'r':
        return parse_mar(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_openOrderBuyC(const std::string_view& name) {
  if (name.length() == 16 &&
      name[13] == 'o' &&
      name[14] == 's' &&
      name[15] == 't') {
    return Field::OPEN_ORDER_BUY_COST;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_openOrderBuyP(const std::string_view& name) {
  if (name.length() == 19 &&
      name[13] == 'r' &&
      name[14] == 'e' &&
      name[15] == 'm' &&
      name[16] == 'i' &&
      name[17] == 'u' &&
      name[18] == 'm') {
    return Field::OPEN_ORDER_BUY_PREMIUM;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_openOrderBuyQ(const std::string_view& name) {
  if (name.length() == 15 &&
      name[13] == 't' &&
      name[14] == 'y') {
    return Field::OPEN_ORDER_BUY_QTY;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_openOrderB(const std::string_view& name) {
  if (name.length() >= 13 &&
      name[10] == 'u' &&
      name[11] == 'y') {
    switch (name[12]) {
      case 'C':
        return parse_openOrderBuyC(name);
      case 'P':
        return parse_openOrderBuyP(name);
      case 'Q':
        return parse_openOrderBuyQ(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_openOrderSellC(const std::string_view& name) {
  if (name.length() == 17 &&
      name[14] == 'o' &&
      name[15] == 's' &&
      name[16] == 't') {
    return Field::OPEN_ORDER_SELL_COST;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_openOrderSellP(const std::string_view& name) {
  if (name.length() == 20 &&
      name[14] == 'r' &&
      name[15] == 'e' &&
      name[16] == 'm' &&
      name[17] == 'i' &&
      name[18] == 'u' &&
      name[19] == 'm') {
    return Field::OPEN_ORDER_SELL_PREMIUM;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_openOrderSellQ(const std::string_view& name) {
  if (name.length() == 16 &&
      name[14] == 't' &&
      name[15] == 'y') {
    return Field::OPEN_ORDER_SELL_QTY;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_openOrderS(const std::string_view& name) {
  if (name.length() >= 14 &&
      name[10] == 'e' &&
      name[11] == 'l' &&
      name[12] == 'l') {
    switch (name[13]) {
      case 'C':
        return parse_openOrderSellC(name);
      case 'P':
        return parse_openOrderSellP(name);
      case 'Q':
        return parse_openOrderSellQ(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_openO(const std::string_view& name) {
  if (name.length() >= 10 &&
      name[5] == 'r' &&
      name[6] == 'd' &&
      name[7] == 'e' &&
      name[8] == 'r') {
    switch (name[9]) {
      case 'B':
        return parse_openOrderB(name);
      case 'S':
        return parse_openOrderS(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_openingCom(const std::string_view& name) {
  if (name.length() == 11 &&
      name[10] == 'm') {
    return Field::OPENING_COMM;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_openingCos(const std::string_view& name) {
  if (name.length() == 11 &&
      name[10] == 't') {
    return Field::OPENING_COST;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_openingC(const std::string_view& name) {
  if (name.length() >= 10 &&
      name[8] == 'o') {
    switch (name[9]) {
      case 'm':
        return parse_openingCom(name);
      case 's':
        return parse_openingCos(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_openingQ(const std::string_view& name) {
  if (name.length() == 10 &&
      name[8] == 't' &&
      name[9] == 'y') {
    return Field::OPENING_QTY;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_openingT(const std::string_view& name) {
  if (name.length() == 16 &&
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

constexpr Field parse_openi(const std::string_view& name) {
  if (name.length() >= 8 &&
      name[5] == 'n' &&
      name[6] == 'g') {
    switch (name[7]) {
      case 'C':
        return parse_openingC(name);
      case 'Q':
        return parse_openingQ(name);
      case 'T':
        return parse_openingT(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_o(const std::string_view& name) {
  if (name.length() >= 5 &&
      name[1] == 'p' &&
      name[2] == 'e' &&
      name[3] == 'n') {
    switch (name[4]) {
      case 'O':
        return parse_openO(name);
      case 'i':
        return parse_openi(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_posA(const std::string_view& name) {
  if (name.length() == 12 &&
      name[4] == 'l' &&
      name[5] == 'l' &&
      name[6] == 'o' &&
      name[7] == 'w' &&
      name[8] == 'a' &&
      name[9] == 'n' &&
      name[10] == 'c' &&
      name[11] == 'e') {
    return Field::POS_ALLOWANCE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_posCom(const std::string_view& name) {
  if (name.length() == 7 &&
      name[6] == 'm') {
    return Field::POS_COMM;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_posCost(const std::string_view& name) {
  if (name.length() == 8 &&
      name[7] == '2') {
    return Field::POS_COST2;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_posCos(const std::string_view& name) {
  if (name.length() >= 7 &&
      name[6] == 't') {
    if (name.length() == 7)
      return Field::POS_COST;
    return parse_posCost(name);
  }
  return Field::UNKNOWN;
}

constexpr Field parse_posCo(const std::string_view& name) {
  if (name.length() > 5) {
    switch (name[5]) {
      case 'm':
        return parse_posCom(name);
      case 's':
        return parse_posCos(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_posCr(const std::string_view& name) {
  if (name.length() == 8 &&
      name[5] == 'o' &&
      name[6] == 's' &&
      name[7] == 's') {
    return Field::POS_CROSS;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_posC(const std::string_view& name) {
  if (name.length() > 4) {
    switch (name[4]) {
      case 'o':
        return parse_posCo(name);
      case 'r':
        return parse_posCr(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_posI(const std::string_view& name) {
  if (name.length() == 7 &&
      name[4] == 'n' &&
      name[5] == 'i' &&
      name[6] == 't') {
    return Field::POS_INIT;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_posL(const std::string_view& name) {
  if (name.length() == 7 &&
      name[4] == 'o' &&
      name[5] == 's' &&
      name[6] == 's') {
    return Field::POS_LOSS;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_posMai(const std::string_view& name) {
  if (name.length() == 8 &&
      name[6] == 'n' &&
      name[7] == 't') {
    return Field::POS_MAINT;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_posMar(const std::string_view& name) {
  if (name.length() == 9 &&
      name[6] == 'g' &&
      name[7] == 'i' &&
      name[8] == 'n') {
    return Field::POS_MARGIN;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_posM(const std::string_view& name) {
  if (name.length() >= 6 &&
      name[4] == 'a') {
    switch (name[5]) {
      case 'i':
        return parse_posMai(name);
      case 'r':
        return parse_posMar(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_posS(const std::string_view& name) {
  if (name.length() == 8 &&
      name[4] == 't' &&
      name[5] == 'a' &&
      name[6] == 't' &&
      name[7] == 'e') {
    return Field::POS_STATE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_po(const std::string_view& name) {
  if (name.length() >= 4 &&
      name[2] == 's') {
    switch (name[3]) {
      case 'A':
        return parse_posA(name);
      case 'C':
        return parse_posC(name);
      case 'I':
        return parse_posI(name);
      case 'L':
        return parse_posL(name);
      case 'M':
        return parse_posM(name);
      case 'S':
        return parse_posS(name);
    }
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

constexpr Field parse_prevR(const std::string_view& name) {
  if (name.length() == 15 &&
      name[5] == 'e' &&
      name[6] == 'a' &&
      name[7] == 'l' &&
      name[8] == 'i' &&
      name[9] == 's' &&
      name[10] == 'e' &&
      name[11] == 'd' &&
      name[12] == 'P' &&
      name[13] == 'n' &&
      name[14] == 'l') {
    return Field::PREV_REALISED_PNL;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_prevU(const std::string_view& name) {
  if (name.length() == 17 &&
      name[5] == 'n' &&
      name[6] == 'r' &&
      name[7] == 'e' &&
      name[8] == 'a' &&
      name[9] == 'l' &&
      name[10] == 'i' &&
      name[11] == 's' &&
      name[12] == 'e' &&
      name[13] == 'd' &&
      name[14] == 'P' &&
      name[15] == 'n' &&
      name[16] == 'l') {
    return Field::PREV_UNREALISED_PNL;
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
      case 'R':
        return parse_prevR(name);
      case 'U':
        return parse_prevU(name);
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
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_q(const std::string_view& name) {
  if (name.length() == 13 &&
      name[1] == 'u' &&
      name[2] == 'o' &&
      name[3] == 't' &&
      name[4] == 'e' &&
      name[5] == 'C' &&
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

constexpr Field parse_realisedC(const std::string_view& name) {
  if (name.length() == 12 &&
      name[9] == 'o' &&
      name[10] == 's' &&
      name[11] == 't') {
    return Field::REALISED_COST;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_realisedG(const std::string_view& name) {
  if (name.length() == 16 &&
      name[9] == 'r' &&
      name[10] == 'o' &&
      name[11] == 's' &&
      name[12] == 's' &&
      name[13] == 'P' &&
      name[14] == 'n' &&
      name[15] == 'l') {
    return Field::REALISED_GROSS_PNL;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_realisedP(const std::string_view& name) {
  if (name.length() == 11 &&
      name[9] == 'n' &&
      name[10] == 'l') {
    return Field::REALISED_PNL;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_realisedT(const std::string_view& name) {
  if (name.length() == 11 &&
      name[9] == 'a' &&
      name[10] == 'x') {
    return Field::REALISED_TAX;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_rea(const std::string_view& name) {
  if (name.length() >= 9 &&
      name[3] == 'l' &&
      name[4] == 'i' &&
      name[5] == 's' &&
      name[6] == 'e' &&
      name[7] == 'd') {
    switch (name[8]) {
      case 'C':
        return parse_realisedC(name);
      case 'G':
        return parse_realisedG(name);
      case 'P':
        return parse_realisedP(name);
      case 'T':
        return parse_realisedT(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_reb(const std::string_view& name) {
  if (name.length() == 13 &&
      name[3] == 'a' &&
      name[4] == 'l' &&
      name[5] == 'a' &&
      name[6] == 'n' &&
      name[7] == 'c' &&
      name[8] == 'e' &&
      name[9] == 'd' &&
      name[10] == 'P' &&
      name[11] == 'n' &&
      name[12] == 'l') {
    return Field::REBALANCED_PNL;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_re(const std::string_view& name) {
  if (name.length() > 2) {
    switch (name[2]) {
      case 'a':
        return parse_rea(name);
      case 'b':
        return parse_reb(name);
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

constexpr Field parse_riskV(const std::string_view& name) {
  if (name.length() == 9 &&
      name[5] == 'a' &&
      name[6] == 'l' &&
      name[7] == 'u' &&
      name[8] == 'e') {
    return Field::RISK_VALUE;
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
      case 'V':
        return parse_riskV(name);
    }
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
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_se(const std::string_view& name) {
  if (name.length() == 13 &&
      name[2] == 's' &&
      name[3] == 's' &&
      name[4] == 'i' &&
      name[5] == 'o' &&
      name[6] == 'n' &&
      name[7] == 'M' &&
      name[8] == 'a' &&
      name[9] == 'r' &&
      name[10] == 'g' &&
      name[11] == 'i' &&
      name[12] == 'n') {
    return Field::SESSION_MARGIN;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_sh(const std::string_view& name) {
  if (name.length() == 13 &&
      name[2] == 'o' &&
      name[3] == 'r' &&
      name[4] == 't' &&
      name[5] == 'B' &&
      name[6] == 'a' &&
      name[7] == 'n' &&
      name[8] == 'k' &&
      name[9] == 'r' &&
      name[10] == 'u' &&
      name[11] == 'p' &&
      name[12] == 't') {
    return Field::SHORT_BANKRUPT;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_simpleC(const std::string_view& name) {
  if (name.length() == 10 &&
      name[7] == 'o' &&
      name[8] == 's' &&
      name[9] == 't') {
    return Field::SIMPLE_COST;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_simplePnl(const std::string_view& name) {
  if (name.length() == 13 &&
      name[9] == 'P' &&
      name[10] == 'c' &&
      name[11] == 'n' &&
      name[12] == 't') {
    return Field::SIMPLE_PNL_PCNT;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_simpleP(const std::string_view& name) {
  if (name.length() >= 9 &&
      name[7] == 'n' &&
      name[8] == 'l') {
    if (name.length() == 9)
      return Field::SIMPLE_PNL;
    return parse_simplePnl(name);
  }
  return Field::UNKNOWN;
}

constexpr Field parse_simpleQ(const std::string_view& name) {
  if (name.length() == 9 &&
      name[7] == 't' &&
      name[8] == 'y') {
    return Field::SIMPLE_QTY;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_simpleV(const std::string_view& name) {
  if (name.length() == 11 &&
      name[7] == 'a' &&
      name[8] == 'l' &&
      name[9] == 'u' &&
      name[10] == 'e') {
    return Field::SIMPLE_VALUE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_si(const std::string_view& name) {
  if (name.length() >= 7 &&
      name[2] == 'm' &&
      name[3] == 'p' &&
      name[4] == 'l' &&
      name[5] == 'e') {
    switch (name[6]) {
      case 'C':
        return parse_simpleC(name);
      case 'P':
        return parse_simpleP(name);
      case 'Q':
        return parse_simpleQ(name);
      case 'V':
        return parse_simpleV(name);
    }
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
      case 'h':
        return parse_sh(name);
      case 'i':
        return parse_si(name);
      case 'y':
        return parse_sy(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_tar(const std::string_view& name) {
  if (name.length() == 18 &&
      name[3] == 'g' &&
      name[4] == 'e' &&
      name[5] == 't' &&
      name[6] == 'E' &&
      name[7] == 'x' &&
      name[8] == 'c' &&
      name[9] == 'e' &&
      name[10] == 's' &&
      name[11] == 's' &&
      name[12] == 'M' &&
      name[13] == 'a' &&
      name[14] == 'r' &&
      name[15] == 'g' &&
      name[16] == 'i' &&
      name[17] == 'n') {
    return Field::TARGET_EXCESS_MARGIN;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_taxB(const std::string_view& name) {
  if (name.length() == 7 &&
      name[4] == 'a' &&
      name[5] == 's' &&
      name[6] == 'e') {
    return Field::TAX_BASE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_taxa(const std::string_view& name) {
  if (name.length() == 13 &&
      name[4] == 'b' &&
      name[5] == 'l' &&
      name[6] == 'e' &&
      name[7] == 'M' &&
      name[8] == 'a' &&
      name[9] == 'r' &&
      name[10] == 'g' &&
      name[11] == 'i' &&
      name[12] == 'n') {
    return Field::TAXABLE_MARGIN;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_tax(const std::string_view& name) {
  if (name.length() > 3) {
    switch (name[3]) {
      case 'B':
        return parse_taxB(name);
      case 'a':
        return parse_taxa(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ta(const std::string_view& name) {
  if (name.length() > 2) {
    switch (name[2]) {
      case 'r':
        return parse_tar(name);
      case 'x':
        return parse_tax(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ti(const std::string_view& name) {
  if (name.length() == 9 &&
      name[2] == 'm' &&
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

constexpr Field parse_t(const std::string_view& name) {
  if (name.length() > 1) {
    switch (name[1]) {
      case 'a':
        return parse_ta(name);
      case 'i':
        return parse_ti(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_und(const std::string_view& name) {
  if (name.length() == 10 &&
      name[3] == 'e' &&
      name[4] == 'r' &&
      name[5] == 'l' &&
      name[6] == 'y' &&
      name[7] == 'i' &&
      name[8] == 'n' &&
      name[9] == 'g') {
    return Field::UNDERLYING;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_unrealisedC(const std::string_view& name) {
  if (name.length() == 14 &&
      name[11] == 'o' &&
      name[12] == 's' &&
      name[13] == 't') {
    return Field::UNREALISED_COST;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_unrealisedG(const std::string_view& name) {
  if (name.length() == 18 &&
      name[11] == 'r' &&
      name[12] == 'o' &&
      name[13] == 's' &&
      name[14] == 's' &&
      name[15] == 'P' &&
      name[16] == 'n' &&
      name[17] == 'l') {
    return Field::UNREALISED_GROSS_PNL;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_unrealisedPnl(const std::string_view& name) {
  if (name.length() == 17 &&
      name[13] == 'P' &&
      name[14] == 'c' &&
      name[15] == 'n' &&
      name[16] == 't') {
    return Field::UNREALISED_PNL_PCNT;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_unrealisedP(const std::string_view& name) {
  if (name.length() >= 13 &&
      name[11] == 'n' &&
      name[12] == 'l') {
    if (name.length() == 13)
      return Field::UNREALISED_PNL;
    return parse_unrealisedPnl(name);
  }
  return Field::UNKNOWN;
}

constexpr Field parse_unrealisedR(const std::string_view& name) {
  if (name.length() == 17 &&
      name[11] == 'o' &&
      name[12] == 'e' &&
      name[13] == 'P' &&
      name[14] == 'c' &&
      name[15] == 'n' &&
      name[16] == 't') {
    return Field::UNREALISED_ROE_PCNT;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_unrealisedT(const std::string_view& name) {
  if (name.length() == 13 &&
      name[11] == 'a' &&
      name[12] == 'x') {
    return Field::UNREALISED_TAX;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_unr(const std::string_view& name) {
  if (name.length() >= 11 &&
      name[3] == 'e' &&
      name[4] == 'a' &&
      name[5] == 'l' &&
      name[6] == 'i' &&
      name[7] == 's' &&
      name[8] == 'e' &&
      name[9] == 'd') {
    switch (name[10]) {
      case 'C':
        return parse_unrealisedC(name);
      case 'G':
        return parse_unrealisedG(name);
      case 'P':
        return parse_unrealisedP(name);
      case 'R':
        return parse_unrealisedR(name);
      case 'T':
        return parse_unrealisedT(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_u(const std::string_view& name) {
  if (name.length() >= 3 &&
      name[1] == 'n') {
    switch (name[2]) {
      case 'd':
        return parse_und(name);
      case 'r':
        return parse_unr(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_v(const std::string_view& name) {
  if (name.length() == 9 &&
      name[1] == 'a' &&
      name[2] == 'r' &&
      name[3] == 'M' &&
      name[4] == 'a' &&
      name[5] == 'r' &&
      name[6] == 'g' &&
      name[7] == 'i' &&
      name[8] == 'n') {
    return Field::VAR_MARGIN;
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
      case 'g':
        return parse_g(name);
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

static_assert(parse_name("account") == Field::ACCOUNT);
static_assert(parse_name("avgCostPrice") == Field::AVG_COST_PRICE);
static_assert(parse_name("avgEntryPrice") == Field::AVG_ENTRY_PRICE);
static_assert(parse_name("bankruptPrice") == Field::BANKRUPT_PRICE);
static_assert(parse_name("breakEvenPrice") == Field::BREAK_EVEN_PRICE);
static_assert(parse_name("commission") == Field::COMMISSION);
static_assert(parse_name("crossMargin") == Field::CROSS_MARGIN);
static_assert(parse_name("currency") == Field::CURRENCY);
static_assert(parse_name("currentComm") == Field::CURRENT_COMM);
static_assert(parse_name("currentCost") == Field::CURRENT_COST);
static_assert(parse_name("currentQty") == Field::CURRENT_QTY);
static_assert(parse_name("currentTimestamp") == Field::CURRENT_TIMESTAMP);
static_assert(parse_name("deleveragePercentile") == Field::DELEVERAGE_PERCENTILE);
static_assert(parse_name("execBuyCost") == Field::EXEC_BUY_COST);
static_assert(parse_name("execBuyQty") == Field::EXEC_BUY_QTY);
static_assert(parse_name("execComm") == Field::EXEC_COMM);
static_assert(parse_name("execCost") == Field::EXEC_COST);
static_assert(parse_name("execQty") == Field::EXEC_QTY);
static_assert(parse_name("execSellCost") == Field::EXEC_SELL_COST);
static_assert(parse_name("execSellQty") == Field::EXEC_SELL_QTY);
static_assert(parse_name("foreignNotional") == Field::FOREIGN_NOTIONAL);
static_assert(parse_name("grossExecCost") == Field::GROSS_EXEC_COST);
static_assert(parse_name("grossOpenCost") == Field::GROSS_OPEN_COST);
static_assert(parse_name("grossOpenPremium") == Field::GROSS_OPEN_PREMIUM);
static_assert(parse_name("homeNotional") == Field::HOME_NOTIONAL);
static_assert(parse_name("indicativeTax") == Field::INDICATIVE_TAX);
static_assert(parse_name("indicativeTaxRate") == Field::INDICATIVE_TAX_RATE);
static_assert(parse_name("initMargin") == Field::INIT_MARGIN);
static_assert(parse_name("initMarginReq") == Field::INIT_MARGIN_REQ);
static_assert(parse_name("isOpen") == Field::IS_OPEN);
static_assert(parse_name("lastPrice") == Field::LAST_PRICE);
static_assert(parse_name("lastValue") == Field::LAST_VALUE);
static_assert(parse_name("leverage") == Field::LEVERAGE);
static_assert(parse_name("liquidationPrice") == Field::LIQUIDATION_PRICE);
static_assert(parse_name("longBankrupt") == Field::LONG_BANKRUPT);
static_assert(parse_name("maintMargin") == Field::MAINT_MARGIN);
static_assert(parse_name("maintMarginReq") == Field::MAINT_MARGIN_REQ);
static_assert(parse_name("marginCallPrice") == Field::MARGIN_CALL_PRICE);
static_assert(parse_name("markPrice") == Field::MARK_PRICE);
static_assert(parse_name("markValue") == Field::MARK_VALUE);
static_assert(parse_name("openOrderBuyCost") == Field::OPEN_ORDER_BUY_COST);
static_assert(parse_name("openOrderBuyPremium") == Field::OPEN_ORDER_BUY_PREMIUM);
static_assert(parse_name("openOrderBuyQty") == Field::OPEN_ORDER_BUY_QTY);
static_assert(parse_name("openOrderSellCost") == Field::OPEN_ORDER_SELL_COST);
static_assert(parse_name("openOrderSellPremium") == Field::OPEN_ORDER_SELL_PREMIUM);
static_assert(parse_name("openOrderSellQty") == Field::OPEN_ORDER_SELL_QTY);
static_assert(parse_name("openingComm") == Field::OPENING_COMM);
static_assert(parse_name("openingCost") == Field::OPENING_COST);
static_assert(parse_name("openingQty") == Field::OPENING_QTY);
static_assert(parse_name("openingTimestamp") == Field::OPENING_TIMESTAMP);
static_assert(parse_name("posAllowance") == Field::POS_ALLOWANCE);
static_assert(parse_name("posComm") == Field::POS_COMM);
static_assert(parse_name("posCost") == Field::POS_COST);
static_assert(parse_name("posCost2") == Field::POS_COST2);
static_assert(parse_name("posCross") == Field::POS_CROSS);
static_assert(parse_name("posInit") == Field::POS_INIT);
static_assert(parse_name("posLoss") == Field::POS_LOSS);
static_assert(parse_name("posMaint") == Field::POS_MAINT);
static_assert(parse_name("posMargin") == Field::POS_MARGIN);
static_assert(parse_name("posState") == Field::POS_STATE);
static_assert(parse_name("prevClosePrice") == Field::PREV_CLOSE_PRICE);
static_assert(parse_name("prevRealisedPnl") == Field::PREV_REALISED_PNL);
static_assert(parse_name("prevUnrealisedPnl") == Field::PREV_UNREALISED_PNL);
static_assert(parse_name("quoteCurrency") == Field::QUOTE_CURRENCY);
static_assert(parse_name("realisedCost") == Field::REALISED_COST);
static_assert(parse_name("realisedGrossPnl") == Field::REALISED_GROSS_PNL);
static_assert(parse_name("realisedPnl") == Field::REALISED_PNL);
static_assert(parse_name("realisedTax") == Field::REALISED_TAX);
static_assert(parse_name("rebalancedPnl") == Field::REBALANCED_PNL);
static_assert(parse_name("riskLimit") == Field::RISK_LIMIT);
static_assert(parse_name("riskValue") == Field::RISK_VALUE);
static_assert(parse_name("sessionMargin") == Field::SESSION_MARGIN);
static_assert(parse_name("shortBankrupt") == Field::SHORT_BANKRUPT);
static_assert(parse_name("simpleCost") == Field::SIMPLE_COST);
static_assert(parse_name("simplePnl") == Field::SIMPLE_PNL);
static_assert(parse_name("simplePnlPcnt") == Field::SIMPLE_PNL_PCNT);
static_assert(parse_name("simpleQty") == Field::SIMPLE_QTY);
static_assert(parse_name("simpleValue") == Field::SIMPLE_VALUE);
static_assert(parse_name("symbol") == Field::SYMBOL);
static_assert(parse_name("targetExcessMargin") == Field::TARGET_EXCESS_MARGIN);
static_assert(parse_name("taxBase") == Field::TAX_BASE);
static_assert(parse_name("taxableMargin") == Field::TAXABLE_MARGIN);
static_assert(parse_name("timestamp") == Field::TIMESTAMP);
static_assert(parse_name("underlying") == Field::UNDERLYING);
static_assert(parse_name("unrealisedCost") == Field::UNREALISED_COST);
static_assert(parse_name("unrealisedGrossPnl") == Field::UNREALISED_GROSS_PNL);
static_assert(parse_name("unrealisedPnl") == Field::UNREALISED_PNL);
static_assert(parse_name("unrealisedPnlPcnt") == Field::UNREALISED_PNL_PCNT);
static_assert(parse_name("unrealisedRoePcnt") == Field::UNREALISED_ROE_PCNT);
static_assert(parse_name("unrealisedTax") == Field::UNREALISED_TAX);
static_assert(parse_name("varMargin") == Field::VAR_MARGIN);

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
    case Field::ACCOUNT:
      update(result.account, value);
      break;
    case Field::AVG_COST_PRICE:
      update(result.avg_cost_price, value);
      break;
    case Field::AVG_ENTRY_PRICE:
      update(result.avg_entry_price, value);
      break;
    case Field::BANKRUPT_PRICE:
      update(result.bankrupt_price, value);
      break;
    case Field::BREAK_EVEN_PRICE:
      update(result.break_even_price, value);
      break;
    case Field::COMMISSION:
      update(result.commission, value);
      break;
    case Field::CROSS_MARGIN:
      update(result.cross_margin, value);
      break;
    case Field::CURRENCY:
      update(result.currency, value);
      break;
    case Field::CURRENT_COMM:
      update(result.current_comm, value);
      break;
    case Field::CURRENT_COST:
      update(result.current_cost, value);
      break;
    case Field::CURRENT_QTY:
      update(result.current_qty, value);
      break;
    case Field::CURRENT_TIMESTAMP:
      update(result.current_timestamp, value);
      break;
    case Field::DELEVERAGE_PERCENTILE:
      update(result.deleverage_percentile, value);
      break;
    case Field::EXEC_BUY_COST:
      update(result.exec_buy_cost, value);
      break;
    case Field::EXEC_BUY_QTY:
      update(result.exec_buy_qty, value);
      break;
    case Field::EXEC_COMM:
      update(result.exec_comm, value);
      break;
    case Field::EXEC_COST:
      update(result.exec_cost, value);
      break;
    case Field::EXEC_QTY:
      update(result.exec_qty, value);
      break;
    case Field::EXEC_SELL_COST:
      update(result.exec_sell_cost, value);
      break;
    case Field::EXEC_SELL_QTY:
      update(result.exec_sell_qty, value);
      break;
    case Field::FOREIGN_NOTIONAL:
      update(result.foreign_notional, value);
      break;
    case Field::GROSS_EXEC_COST:
      update(result.gross_exec_cost, value);
      break;
    case Field::GROSS_OPEN_COST:
      update(result.gross_open_cost, value);
      break;
    case Field::GROSS_OPEN_PREMIUM:
      update(result.gross_open_premium, value);
      break;
    case Field::HOME_NOTIONAL:
      update(result.home_notional, value);
      break;
    case Field::INDICATIVE_TAX:
      update(result.indicative_tax, value);
      break;
    case Field::INDICATIVE_TAX_RATE:
      update(result.indicative_tax_rate, value);
      break;
    case Field::INIT_MARGIN:
      update(result.init_margin, value);
      break;
    case Field::INIT_MARGIN_REQ:
      update(result.init_margin_req, value);
      break;
    case Field::IS_OPEN:
      update(result.is_open, value);
      break;
    case Field::LAST_PRICE:
      update(result.last_price, value);
      break;
    case Field::LAST_VALUE:
      update(result.last_value, value);
      break;
    case Field::LEVERAGE:
      update(result.leverage, value);
      break;
    case Field::LIQUIDATION_PRICE:
      update(result.liquidation_price, value);
      break;
    case Field::LONG_BANKRUPT:
      update(result.long_bankrupt, value);
      break;
    case Field::MAINT_MARGIN:
      update(result.maint_margin, value);
      break;
    case Field::MAINT_MARGIN_REQ:
      update(result.maint_margin_req, value);
      break;
    case Field::MARGIN_CALL_PRICE:
      update(result.margin_call_price, value);
      break;
    case Field::MARK_PRICE:
      update(result.mark_price, value);
      break;
    case Field::MARK_VALUE:
      update(result.mark_value, value);
      break;
    case Field::OPEN_ORDER_BUY_COST:
      update(result.open_order_buy_cost, value);
      break;
    case Field::OPEN_ORDER_BUY_PREMIUM:
      update(result.open_order_buy_premium, value);
      break;
    case Field::OPEN_ORDER_BUY_QTY:
      update(result.open_order_buy_qty, value);
      break;
    case Field::OPEN_ORDER_SELL_COST:
      update(result.open_order_sell_cost, value);
      break;
    case Field::OPEN_ORDER_SELL_PREMIUM:
      update(result.open_order_sell_premium, value);
      break;
    case Field::OPEN_ORDER_SELL_QTY:
      update(result.open_order_sell_qty, value);
      break;
    case Field::OPENING_COMM:
      update(result.opening_comm, value);
      break;
    case Field::OPENING_COST:
      update(result.opening_cost, value);
      break;
    case Field::OPENING_QTY:
      update(result.opening_qty, value);
      break;
    case Field::OPENING_TIMESTAMP:
      update(result.opening_timestamp, value);
      break;
    case Field::POS_ALLOWANCE:
      update(result.pos_allowance, value);
      break;
    case Field::POS_COMM:
      update(result.pos_comm, value);
      break;
    case Field::POS_COST:
      update(result.pos_cost, value);
      break;
    case Field::POS_COST2:
      update(result.pos_cost2, value);
      break;
    case Field::POS_CROSS:
      update(result.pos_cross, value);
      break;
    case Field::POS_INIT:
      update(result.pos_init, value);
      break;
    case Field::POS_LOSS:
      update(result.pos_loss, value);
      break;
    case Field::POS_MAINT:
      update(result.pos_maint, value);
      break;
    case Field::POS_MARGIN:
      update(result.pos_margin, value);
      break;
    case Field::POS_STATE:
      update(result.pos_state, value);
      break;
    case Field::PREV_CLOSE_PRICE:
      update(result.prev_close_price, value);
      break;
    case Field::PREV_REALISED_PNL:
      update(result.prev_realised_pnl, value);
      break;
    case Field::PREV_UNREALISED_PNL:
      update(result.prev_unrealised_pnl, value);
      break;
    case Field::QUOTE_CURRENCY:
      update(result.quote_currency, value);
      break;
    case Field::REALISED_COST:
      update(result.realised_cost, value);
      break;
    case Field::REALISED_GROSS_PNL:
      update(result.realised_gross_pnl, value);
      break;
    case Field::REALISED_PNL:
      update(result.realised_pnl, value);
      break;
    case Field::REALISED_TAX:
      update(result.realised_tax, value);
      break;
    case Field::REBALANCED_PNL:
      update(result.rebalanced_pnl, value);
      break;
    case Field::RISK_LIMIT:
      update(result.risk_limit, value);
      break;
    case Field::RISK_VALUE:
      update(result.risk_value, value);
      break;
    case Field::SESSION_MARGIN:
      update(result.session_margin, value);
      break;
    case Field::SHORT_BANKRUPT:
      update(result.short_bankrupt, value);
      break;
    case Field::SIMPLE_COST:
      update(result.simple_cost, value);
      break;
    case Field::SIMPLE_PNL:
      update(result.simple_pnl, value);
      break;
    case Field::SIMPLE_PNL_PCNT:
      update(result.simple_pnl_pcnt, value);
      break;
    case Field::SIMPLE_QTY:
      update(result.simple_qty, value);
      break;
    case Field::SIMPLE_VALUE:
      update(result.simple_value, value);
      break;
    case Field::SYMBOL:
      update(result.symbol, value);
      break;
    case Field::TARGET_EXCESS_MARGIN:
      update(result.target_excess_margin, value);
      break;
    case Field::TAX_BASE:
      update(result.tax_base, value);
      break;
    case Field::TAXABLE_MARGIN:
      update(result.taxable_margin, value);
      break;
    case Field::TIMESTAMP:
      update(result.timestamp, value);
      break;
    case Field::UNDERLYING:
      update(result.underlying, value);
      break;
    case Field::UNREALISED_COST:
      update(result.unrealised_cost, value);
      break;
    case Field::UNREALISED_GROSS_PNL:
      update(result.unrealised_gross_pnl, value);
      break;
    case Field::UNREALISED_PNL:
      update(result.unrealised_pnl, value);
      break;
    case Field::UNREALISED_PNL_PCNT:
      update(result.unrealised_pnl_pcnt, value);
      break;
    case Field::UNREALISED_ROE_PCNT:
      update(result.unrealised_roe_pcnt, value);
      break;
    case Field::UNREALISED_TAX:
      update(result.unrealised_tax, value);
      break;
    case Field::VAR_MARGIN:
      update(result.var_margin, value);
      break;
  }
}
}  // namespace

PositionItem::PositionItem(core::json::value_t& value) {
  for (auto [key, value] : std::get<core::json::object_t>(value))
    update_field(*this, key, value);
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
