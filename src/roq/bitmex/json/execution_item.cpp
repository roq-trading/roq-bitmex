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
  ORD_REJ_REASON,
  ORD_STATUS,
  ORD_TYPE,
  ORDER_ID,
  ORDER_QTY,
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

constexpr Field parse_av(const std::string_view& name) {
  if (name.length() == 5 &&
      name[2] == 'g' &&
      name[3] == 'P' &&
      name[4] == 'x') {
    return Field::AVG_PX;
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

constexpr Field parse_clOrdI(const std::string_view& name) {
  if (name.length() == 7 &&
      name[6] == 'D') {
    return Field::CL_ORD_ID;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_clOrdL(const std::string_view& name) {
  if (name.length() == 11 &&
      name[6] == 'i' &&
      name[7] == 'n' &&
      name[8] == 'k' &&
      name[9] == 'I' &&
      name[10] == 'D') {
    return Field::CL_ORD_LINK_ID;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_cl(const std::string_view& name) {
  if (name.length() >= 6 &&
      name[2] == 'O' &&
      name[3] == 'r' &&
      name[4] == 'd') {
    switch (name[5]) {
      case 'I':
        return parse_clOrdI(name);
      case 'L':
        return parse_clOrdL(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_com(const std::string_view& name) {
  if (name.length() == 10 &&
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

constexpr Field parse_con(const std::string_view& name) {
  if (name.length() == 15 &&
      name[3] == 't' &&
      name[4] == 'i' &&
      name[5] == 'n' &&
      name[6] == 'g' &&
      name[7] == 'e' &&
      name[8] == 'n' &&
      name[9] == 'c' &&
      name[10] == 'y' &&
      name[11] == 'T' &&
      name[12] == 'y' &&
      name[13] == 'p' &&
      name[14] == 'e') {
    return Field::CONTINGENCY_TYPE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_co(const std::string_view& name) {
  if (name.length() > 2) {
    switch (name[2]) {
      case 'm':
        return parse_com(name);
      case 'n':
        return parse_con(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_cum(const std::string_view& name) {
  if (name.length() == 6 &&
      name[3] == 'Q' &&
      name[4] == 't' &&
      name[5] == 'y') {
    return Field::CUM_QTY;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_cur(const std::string_view& name) {
  if (name.length() == 8 &&
      name[3] == 'r' &&
      name[4] == 'e' &&
      name[5] == 'n' &&
      name[6] == 'c' &&
      name[7] == 'y') {
    return Field::CURRENCY;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_cu(const std::string_view& name) {
  if (name.length() > 2) {
    switch (name[2]) {
      case 'm':
        return parse_cum(name);
      case 'r':
        return parse_cur(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_c(const std::string_view& name) {
  if (name.length() > 1) {
    switch (name[1]) {
      case 'l':
        return parse_cl(name);
      case 'o':
        return parse_co(name);
      case 'u':
        return parse_cu(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_d(const std::string_view& name) {
  if (name.length() == 10 &&
      name[1] == 'i' &&
      name[2] == 's' &&
      name[3] == 'p' &&
      name[4] == 'l' &&
      name[5] == 'a' &&
      name[6] == 'y' &&
      name[7] == 'Q' &&
      name[8] == 't' &&
      name[9] == 'y') {
    return Field::DISPLAY_QTY;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_exD(const std::string_view& name) {
  if (name.length() == 13 &&
      name[3] == 'e' &&
      name[4] == 's' &&
      name[5] == 't' &&
      name[6] == 'i' &&
      name[7] == 'n' &&
      name[8] == 'a' &&
      name[9] == 't' &&
      name[10] == 'i' &&
      name[11] == 'o' &&
      name[12] == 'n') {
    return Field::EX_DESTINATION;
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

constexpr Field parse_execIn(const std::string_view& name) {
  if (name.length() == 8 &&
      name[6] == 's' &&
      name[7] == 't') {
    return Field::EXEC_INST;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_execI(const std::string_view& name) {
  if (name.length() > 5) {
    switch (name[5]) {
      case 'D':
        return Field::EXEC_ID;
      case 'n':
        return parse_execIn(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_execT(const std::string_view& name) {
  if (name.length() == 8 &&
      name[5] == 'y' &&
      name[6] == 'p' &&
      name[7] == 'e') {
    return Field::EXEC_TYPE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_exe(const std::string_view& name) {
  if (name.length() >= 5 &&
      name[3] == 'c') {
    switch (name[4]) {
      case 'C':
        return parse_execC(name);
      case 'I':
        return parse_execI(name);
      case 'T':
        return parse_execT(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_e(const std::string_view& name) {
  if (name.length() >= 3 &&
      name[1] == 'x') {
    switch (name[2]) {
      case 'D':
        return parse_exD(name);
      case 'e':
        return parse_exe(name);
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

constexpr Field parse_lastL(const std::string_view& name) {
  if (name.length() == 16 &&
      name[5] == 'i' &&
      name[6] == 'q' &&
      name[7] == 'u' &&
      name[8] == 'i' &&
      name[9] == 'd' &&
      name[10] == 'i' &&
      name[11] == 't' &&
      name[12] == 'y' &&
      name[13] == 'I' &&
      name[14] == 'n' &&
      name[15] == 'd') {
    return Field::LAST_LIQUIDITY_IND;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_lastM(const std::string_view& name) {
  if (name.length() == 7 &&
      name[5] == 'k' &&
      name[6] == 't') {
    return Field::LAST_MKT;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_lastP(const std::string_view& name) {
  if (name.length() == 6 &&
      name[5] == 'x') {
    return Field::LAST_PX;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_lastQ(const std::string_view& name) {
  if (name.length() == 7 &&
      name[5] == 't' &&
      name[6] == 'y') {
    return Field::LAST_QTY;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_la(const std::string_view& name) {
  if (name.length() >= 5 &&
      name[2] == 's' &&
      name[3] == 't') {
    switch (name[4]) {
      case 'L':
        return parse_lastL(name);
      case 'M':
        return parse_lastM(name);
      case 'P':
        return parse_lastP(name);
      case 'Q':
        return parse_lastQ(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_le(const std::string_view& name) {
  if (name.length() == 9 &&
      name[2] == 'a' &&
      name[3] == 'v' &&
      name[4] == 'e' &&
      name[5] == 's' &&
      name[6] == 'Q' &&
      name[7] == 't' &&
      name[8] == 'y') {
    return Field::LEAVES_QTY;
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
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_m(const std::string_view& name) {
  if (name.length() == 21 &&
      name[1] == 'u' &&
      name[2] == 'l' &&
      name[3] == 't' &&
      name[4] == 'i' &&
      name[5] == 'L' &&
      name[6] == 'e' &&
      name[7] == 'g' &&
      name[8] == 'R' &&
      name[9] == 'e' &&
      name[10] == 'p' &&
      name[11] == 'o' &&
      name[12] == 'r' &&
      name[13] == 't' &&
      name[14] == 'i' &&
      name[15] == 'n' &&
      name[16] == 'g' &&
      name[17] == 'T' &&
      name[18] == 'y' &&
      name[19] == 'p' &&
      name[20] == 'e') {
    return Field::MULTI_LEG_REPORTING_TYPE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ordR(const std::string_view& name) {
  if (name.length() == 12 &&
      name[4] == 'e' &&
      name[5] == 'j' &&
      name[6] == 'R' &&
      name[7] == 'e' &&
      name[8] == 'a' &&
      name[9] == 's' &&
      name[10] == 'o' &&
      name[11] == 'n') {
    return Field::ORD_REJ_REASON;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ordS(const std::string_view& name) {
  if (name.length() == 9 &&
      name[4] == 't' &&
      name[5] == 'a' &&
      name[6] == 't' &&
      name[7] == 'u' &&
      name[8] == 's') {
    return Field::ORD_STATUS;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ordT(const std::string_view& name) {
  if (name.length() == 7 &&
      name[4] == 'y' &&
      name[5] == 'p' &&
      name[6] == 'e') {
    return Field::ORD_TYPE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_orderI(const std::string_view& name) {
  if (name.length() == 7 &&
      name[6] == 'D') {
    return Field::ORDER_ID;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_orderQ(const std::string_view& name) {
  if (name.length() == 8 &&
      name[6] == 't' &&
      name[7] == 'y') {
    return Field::ORDER_QTY;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_orde(const std::string_view& name) {
  if (name.length() >= 6 &&
      name[4] == 'r') {
    switch (name[5]) {
      case 'I':
        return parse_orderI(name);
      case 'Q':
        return parse_orderQ(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_o(const std::string_view& name) {
  if (name.length() >= 4 &&
      name[1] == 'r' &&
      name[2] == 'd') {
    switch (name[3]) {
      case 'R':
        return parse_ordR(name);
      case 'S':
        return parse_ordS(name);
      case 'T':
        return parse_ordT(name);
      case 'e':
        return parse_orde(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_pegO(const std::string_view& name) {
  if (name.length() == 14 &&
      name[4] == 'f' &&
      name[5] == 'f' &&
      name[6] == 's' &&
      name[7] == 'e' &&
      name[8] == 't' &&
      name[9] == 'V' &&
      name[10] == 'a' &&
      name[11] == 'l' &&
      name[12] == 'u' &&
      name[13] == 'e') {
    return Field::PEG_OFFSET_VALUE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_pegP(const std::string_view& name) {
  if (name.length() == 12 &&
      name[4] == 'r' &&
      name[5] == 'i' &&
      name[6] == 'c' &&
      name[7] == 'e' &&
      name[8] == 'T' &&
      name[9] == 'y' &&
      name[10] == 'p' &&
      name[11] == 'e') {
    return Field::PEG_PRICE_TYPE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_pe(const std::string_view& name) {
  if (name.length() >= 4 &&
      name[2] == 'g') {
    switch (name[3]) {
      case 'O':
        return parse_pegO(name);
      case 'P':
        return parse_pegP(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_pr(const std::string_view& name) {
  if (name.length() == 5 &&
      name[2] == 'i' &&
      name[3] == 'c' &&
      name[4] == 'e') {
    return Field::PRICE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_p(const std::string_view& name) {
  if (name.length() > 1) {
    switch (name[1]) {
      case 'e':
        return parse_pe(name);
      case 'r':
        return parse_pr(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_se(const std::string_view& name) {
  if (name.length() == 13 &&
      name[2] == 't' &&
      name[3] == 't' &&
      name[4] == 'l' &&
      name[5] == 'C' &&
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

constexpr Field parse_sid(const std::string_view& name) {
  if (name.length() == 4 &&
      name[3] == 'e') {
    return Field::SIDE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_simpleC(const std::string_view& name) {
  if (name.length() == 12 &&
      name[7] == 'u' &&
      name[8] == 'm' &&
      name[9] == 'Q' &&
      name[10] == 't' &&
      name[11] == 'y') {
    return Field::SIMPLE_CUM_QTY;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_simpleL(const std::string_view& name) {
  if (name.length() == 15 &&
      name[7] == 'e' &&
      name[8] == 'a' &&
      name[9] == 'v' &&
      name[10] == 'e' &&
      name[11] == 's' &&
      name[12] == 'Q' &&
      name[13] == 't' &&
      name[14] == 'y') {
    return Field::SIMPLE_LEAVES_QTY;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_simpleO(const std::string_view& name) {
  if (name.length() == 14 &&
      name[7] == 'r' &&
      name[8] == 'd' &&
      name[9] == 'e' &&
      name[10] == 'r' &&
      name[11] == 'Q' &&
      name[12] == 't' &&
      name[13] == 'y') {
    return Field::SIMPLE_ORDER_QTY;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_sim(const std::string_view& name) {
  if (name.length() >= 7 &&
      name[3] == 'p' &&
      name[4] == 'l' &&
      name[5] == 'e') {
    switch (name[6]) {
      case 'C':
        return parse_simpleC(name);
      case 'L':
        return parse_simpleL(name);
      case 'O':
        return parse_simpleO(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_si(const std::string_view& name) {
  if (name.length() > 2) {
    switch (name[2]) {
      case 'd':
        return parse_sid(name);
      case 'm':
        return parse_sim(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_st(const std::string_view& name) {
  if (name.length() == 6 &&
      name[2] == 'o' &&
      name[3] == 'p' &&
      name[4] == 'P' &&
      name[5] == 'x') {
    return Field::STOP_PX;
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
      case 'i':
        return parse_si(name);
      case 't':
        return parse_st(name);
      case 'y':
        return parse_sy(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_te(const std::string_view& name) {
  if (name.length() == 4 &&
      name[2] == 'x' &&
      name[3] == 't') {
    return Field::TEXT;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_timeI(const std::string_view& name) {
  if (name.length() == 11 &&
      name[5] == 'n' &&
      name[6] == 'F' &&
      name[7] == 'o' &&
      name[8] == 'r' &&
      name[9] == 'c' &&
      name[10] == 'e') {
    return Field::TIME_IN_FORCE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_times(const std::string_view& name) {
  if (name.length() == 9 &&
      name[5] == 't' &&
      name[6] == 'a' &&
      name[7] == 'm' &&
      name[8] == 'p') {
    return Field::TIMESTAMP;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ti(const std::string_view& name) {
  if (name.length() >= 5 &&
      name[2] == 'm' &&
      name[3] == 'e') {
    switch (name[4]) {
      case 'I':
        return parse_timeI(name);
      case 's':
        return parse_times(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_trad(const std::string_view& name) {
  if (name.length() == 21 &&
      name[4] == 'e' &&
      name[5] == 'P' &&
      name[6] == 'u' &&
      name[7] == 'b' &&
      name[8] == 'l' &&
      name[9] == 'i' &&
      name[10] == 's' &&
      name[11] == 'h' &&
      name[12] == 'I' &&
      name[13] == 'n' &&
      name[14] == 'd' &&
      name[15] == 'i' &&
      name[16] == 'c' &&
      name[17] == 'a' &&
      name[18] == 't' &&
      name[19] == 'o' &&
      name[20] == 'r') {
    return Field::TRADE_PUBLISH_INDICATOR;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_tran(const std::string_view& name) {
  if (name.length() == 12 &&
      name[4] == 's' &&
      name[5] == 'a' &&
      name[6] == 'c' &&
      name[7] == 't' &&
      name[8] == 'T' &&
      name[9] == 'i' &&
      name[10] == 'm' &&
      name[11] == 'e') {
    return Field::TRANSACT_TIME;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_tra(const std::string_view& name) {
  if (name.length() > 3) {
    switch (name[3]) {
      case 'd':
        return parse_trad(name);
      case 'n':
        return parse_tran(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_trd(const std::string_view& name) {
  if (name.length() == 10 &&
      name[3] == 'M' &&
      name[4] == 'a' &&
      name[5] == 't' &&
      name[6] == 'c' &&
      name[7] == 'h' &&
      name[8] == 'I' &&
      name[9] == 'D') {
    return Field::TRD_MATCH_ID;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_tri(const std::string_view& name) {
  if (name.length() == 9 &&
      name[3] == 'g' &&
      name[4] == 'g' &&
      name[5] == 'e' &&
      name[6] == 'r' &&
      name[7] == 'e' &&
      name[8] == 'd') {
    return Field::TRIGGERED;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_tr(const std::string_view& name) {
  if (name.length() > 2) {
    switch (name[2]) {
      case 'a':
        return parse_tra(name);
      case 'd':
        return parse_trd(name);
      case 'i':
        return parse_tri(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_t(const std::string_view& name) {
  if (name.length() > 1) {
    switch (name[1]) {
      case 'e':
        return parse_te(name);
      case 'i':
        return parse_ti(name);
      case 'r':
        return parse_tr(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_u(const std::string_view& name) {
  if (name.length() == 16 &&
      name[1] == 'n' &&
      name[2] == 'd' &&
      name[3] == 'e' &&
      name[4] == 'r' &&
      name[5] == 'l' &&
      name[6] == 'y' &&
      name[7] == 'i' &&
      name[8] == 'n' &&
      name[9] == 'g' &&
      name[10] == 'L' &&
      name[11] == 'a' &&
      name[12] == 's' &&
      name[13] == 't' &&
      name[14] == 'P' &&
      name[15] == 'x') {
    return Field::UNDERLYING_LAST_PX;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_w(const std::string_view& name) {
  if (name.length() == 16 &&
      name[1] == 'o' &&
      name[2] == 'r' &&
      name[3] == 'k' &&
      name[4] == 'i' &&
      name[5] == 'n' &&
      name[6] == 'g' &&
      name[7] == 'I' &&
      name[8] == 'n' &&
      name[9] == 'd' &&
      name[10] == 'i' &&
      name[11] == 'c' &&
      name[12] == 'a' &&
      name[13] == 't' &&
      name[14] == 'o' &&
      name[15] == 'r') {
    return Field::WORKING_INDICATOR;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  if (name.length() > 0) {
    switch (name[0]) {
      case 'a':
        return parse_a(name);
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
      case 'l':
        return parse_l(name);
      case 'm':
        return parse_m(name);
      case 'o':
        return parse_o(name);
      case 'p':
        return parse_p(name);
      case 's':
        return parse_s(name);
      case 't':
        return parse_t(name);
      case 'u':
        return parse_u(name);
      case 'w':
        return parse_w(name);
    }
  }
  return Field::UNKNOWN;
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
static_assert(parse_name("ordRejReason") == Field::ORD_REJ_REASON);
static_assert(parse_name("ordStatus") == Field::ORD_STATUS);
static_assert(parse_name("ordType") == Field::ORD_TYPE);
static_assert(parse_name("orderID") == Field::ORDER_ID);
static_assert(parse_name("orderQty") == Field::ORDER_QTY);
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
    case Field::ORD_REJ_REASON:
      update(result.ord_rej_reason, value);
      break;
    case Field::ORD_STATUS:
      update(result.ord_status, value);
      break;
    case Field::ORD_TYPE:
      update(result.ord_type, value);
      break;
    case Field::ORDER_ID:
      update(result.order_id, value);
      break;
    case Field::ORDER_QTY:
      update(result.order_qty, value);
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
