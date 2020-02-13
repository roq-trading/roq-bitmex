/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/margin_item.h"

#include "roq/bitmex/json/utils.h"

#include "roq/logging.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  ACCOUNT,
  ACTION,
  AMOUNT,
  AVAILABLE_MARGIN,
  COMMISSION,
  CONFIRMED_DEBIT,
  CURRENCY,
  EXCESS_MARGIN,
  EXCESS_MARGIN_PCNT,
  GROSS_COMM,
  GROSS_EXEC_COST,
  GROSS_LAST_VALUE,
  GROSS_MARK_VALUE,
  GROSS_OPEN_COST,
  GROSS_OPEN_PREMIUM,
  INDICATIVE_TAX,
  INIT_MARGIN,
  MAINT_MARGIN,
  MARGIN_BALANCE,
  MARGIN_BALANCE_PCNT,
  MARGIN_LEVERAGE,
  MARGIN_USED_PCNT,
  PENDING_CREDIT,
  PENDING_DEBIT,
  PREV_REALISED_PNL,
  PREV_STATE,
  PREV_UNREALISED_PNL,
  REALISED_PNL,
  RISK_LIMIT,
  RISK_VALUE,
  SESSION_MARGIN,
  STATE,
  SYNTHETIC_MARGIN,
  TARGET_EXCESS_MARGIN,
  TAXABLE_MARGIN,
  TIMESTAMP,
  UNREALISED_PNL,
  UNREALISED_PROFIT,
  VAR_MARGIN,
  WALLET_BALANCE,
  WITHDRAWABLE_MARGIN,
};

constexpr Field parse_acc(const std::string_view& name) {
  if (name.length() == 7 &&
      name[3] == 'o' &&
      name[4] == 'u' &&
      name[5] == 'n' &&
      name[6] == 't') {
    return Field::ACCOUNT;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_act(const std::string_view& name) {
  if (name.length() == 6 &&
      name[3] == 'i' &&
      name[4] == 'o' &&
      name[5] == 'n') {
    return Field::ACTION;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ac(const std::string_view& name) {
  if (name.length() > 2) {
    switch (name[2]) {
      case 'c':
        return parse_acc(name);
      case 't':
        return parse_act(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_am(const std::string_view& name) {
  if (name.length() == 6 &&
      name[2] == 'o' &&
      name[3] == 'u' &&
      name[4] == 'n' &&
      name[5] == 't') {
    return Field::AMOUNT;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_av(const std::string_view& name) {
  if (name.length() == 15 &&
      name[2] == 'a' &&
      name[3] == 'i' &&
      name[4] == 'l' &&
      name[5] == 'a' &&
      name[6] == 'b' &&
      name[7] == 'l' &&
      name[8] == 'e' &&
      name[9] == 'M' &&
      name[10] == 'a' &&
      name[11] == 'r' &&
      name[12] == 'g' &&
      name[13] == 'i' &&
      name[14] == 'n') {
    return Field::AVAILABLE_MARGIN;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_a(const std::string_view& name) {
  if (name.length() > 1) {
    switch (name[1]) {
      case 'c':
        return parse_ac(name);
      case 'm':
        return parse_am(name);
      case 'v':
        return parse_av(name);
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
  if (name.length() == 14 &&
      name[3] == 'f' &&
      name[4] == 'i' &&
      name[5] == 'r' &&
      name[6] == 'm' &&
      name[7] == 'e' &&
      name[8] == 'd' &&
      name[9] == 'D' &&
      name[10] == 'e' &&
      name[11] == 'b' &&
      name[12] == 'i' &&
      name[13] == 't') {
    return Field::CONFIRMED_DEBIT;
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

constexpr Field parse_cu(const std::string_view& name) {
  if (name.length() == 8 &&
      name[2] == 'r' &&
      name[3] == 'r' &&
      name[4] == 'e' &&
      name[5] == 'n' &&
      name[6] == 'c' &&
      name[7] == 'y') {
    return Field::CURRENCY;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_c(const std::string_view& name) {
  if (name.length() > 1) {
    switch (name[1]) {
      case 'o':
        return parse_co(name);
      case 'u':
        return parse_cu(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_excessMargin(const std::string_view& name) {
  if (name.length() == 16 &&
      name[12] == 'P' &&
      name[13] == 'c' &&
      name[14] == 'n' &&
      name[15] == 't') {
    return Field::EXCESS_MARGIN_PCNT;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_e(const std::string_view& name) {
  if (name.length() >= 12 &&
      name[1] == 'x' &&
      name[2] == 'c' &&
      name[3] == 'e' &&
      name[4] == 's' &&
      name[5] == 's' &&
      name[6] == 'M' &&
      name[7] == 'a' &&
      name[8] == 'r' &&
      name[9] == 'g' &&
      name[10] == 'i' &&
      name[11] == 'n') {
    if (name.length() == 12)
      return Field::EXCESS_MARGIN;
    return parse_excessMargin(name);
  }
  return Field::UNKNOWN;
}

constexpr Field parse_grossC(const std::string_view& name) {
  if (name.length() == 9 &&
      name[6] == 'o' &&
      name[7] == 'm' &&
      name[8] == 'm') {
    return Field::GROSS_COMM;
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

constexpr Field parse_grossL(const std::string_view& name) {
  if (name.length() == 14 &&
      name[6] == 'a' &&
      name[7] == 's' &&
      name[8] == 't' &&
      name[9] == 'V' &&
      name[10] == 'a' &&
      name[11] == 'l' &&
      name[12] == 'u' &&
      name[13] == 'e') {
    return Field::GROSS_LAST_VALUE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_grossM(const std::string_view& name) {
  if (name.length() == 14 &&
      name[6] == 'a' &&
      name[7] == 'r' &&
      name[8] == 'k' &&
      name[9] == 'V' &&
      name[10] == 'a' &&
      name[11] == 'l' &&
      name[12] == 'u' &&
      name[13] == 'e') {
    return Field::GROSS_MARK_VALUE;
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
      case 'C':
        return parse_grossC(name);
      case 'E':
        return parse_grossE(name);
      case 'L':
        return parse_grossL(name);
      case 'M':
        return parse_grossM(name);
      case 'O':
        return parse_grossO(name);
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_ind(const std::string_view& name) {
  if (name.length() == 13 &&
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
    return Field::INDICATIVE_TAX;
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

constexpr Field parse_i(const std::string_view& name) {
  if (name.length() >= 3 &&
      name[1] == 'n') {
    switch (name[2]) {
      case 'd':
        return parse_ind(name);
      case 'i':
        return parse_ini(name);
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

constexpr Field parse_marginBalance(const std::string_view& name) {
  if (name.length() == 17 &&
      name[13] == 'P' &&
      name[14] == 'c' &&
      name[15] == 'n' &&
      name[16] == 't') {
    return Field::MARGIN_BALANCE_PCNT;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_marginB(const std::string_view& name) {
  if (name.length() >= 13 &&
      name[7] == 'a' &&
      name[8] == 'l' &&
      name[9] == 'a' &&
      name[10] == 'n' &&
      name[11] == 'c' &&
      name[12] == 'e') {
    if (name.length() == 13)
      return Field::MARGIN_BALANCE;
    return parse_marginBalance(name);
  }
  return Field::UNKNOWN;
}

constexpr Field parse_marginL(const std::string_view& name) {
  if (name.length() == 14 &&
      name[7] == 'e' &&
      name[8] == 'v' &&
      name[9] == 'e' &&
      name[10] == 'r' &&
      name[11] == 'a' &&
      name[12] == 'g' &&
      name[13] == 'e') {
    return Field::MARGIN_LEVERAGE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_marginU(const std::string_view& name) {
  if (name.length() == 14 &&
      name[7] == 's' &&
      name[8] == 'e' &&
      name[9] == 'd' &&
      name[10] == 'P' &&
      name[11] == 'c' &&
      name[12] == 'n' &&
      name[13] == 't') {
    return Field::MARGIN_USED_PCNT;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_mar(const std::string_view& name) {
  if (name.length() >= 7 &&
      name[3] == 'g' &&
      name[4] == 'i' &&
      name[5] == 'n') {
    switch (name[6]) {
      case 'B':
        return parse_marginB(name);
      case 'L':
        return parse_marginL(name);
      case 'U':
        return parse_marginU(name);
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

constexpr Field parse_pendingC(const std::string_view& name) {
  if (name.length() == 13 &&
      name[8] == 'r' &&
      name[9] == 'e' &&
      name[10] == 'd' &&
      name[11] == 'i' &&
      name[12] == 't') {
    return Field::PENDING_CREDIT;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_pendingD(const std::string_view& name) {
  if (name.length() == 12 &&
      name[8] == 'e' &&
      name[9] == 'b' &&
      name[10] == 'i' &&
      name[11] == 't') {
    return Field::PENDING_DEBIT;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_pe(const std::string_view& name) {
  if (name.length() >= 8 &&
      name[2] == 'n' &&
      name[3] == 'd' &&
      name[4] == 'i' &&
      name[5] == 'n' &&
      name[6] == 'g') {
    switch (name[7]) {
      case 'C':
        return parse_pendingC(name);
      case 'D':
        return parse_pendingD(name);
    }
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

constexpr Field parse_prevS(const std::string_view& name) {
  if (name.length() == 9 &&
      name[5] == 't' &&
      name[6] == 'a' &&
      name[7] == 't' &&
      name[8] == 'e') {
    return Field::PREV_STATE;
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
      case 'R':
        return parse_prevR(name);
      case 'S':
        return parse_prevS(name);
      case 'U':
        return parse_prevU(name);
    }
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

constexpr Field parse_re(const std::string_view& name) {
  if (name.length() == 11 &&
      name[2] == 'a' &&
      name[3] == 'l' &&
      name[4] == 'i' &&
      name[5] == 's' &&
      name[6] == 'e' &&
      name[7] == 'd' &&
      name[8] == 'P' &&
      name[9] == 'n' &&
      name[10] == 'l') {
    return Field::REALISED_PNL;
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
  if (name.length() == 15 &&
      name[2] == 'n' &&
      name[3] == 't' &&
      name[4] == 'h' &&
      name[5] == 'e' &&
      name[6] == 't' &&
      name[7] == 'i' &&
      name[8] == 'c' &&
      name[9] == 'M' &&
      name[10] == 'a' &&
      name[11] == 'r' &&
      name[12] == 'g' &&
      name[13] == 'i' &&
      name[14] == 'n') {
    return Field::SYNTHETIC_MARGIN;
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

constexpr Field parse_tax(const std::string_view& name) {
  if (name.length() == 13 &&
      name[3] == 'a' &&
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

constexpr Field parse_unrealisedPn(const std::string_view& name) {
  if (name.length() == 13 &&
      name[12] == 'l') {
    return Field::UNREALISED_PNL;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_unrealisedPr(const std::string_view& name) {
  if (name.length() == 16 &&
      name[12] == 'o' &&
      name[13] == 'f' &&
      name[14] == 'i' &&
      name[15] == 't') {
    return Field::UNREALISED_PROFIT;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_u(const std::string_view& name) {
  if (name.length() >= 12 &&
      name[1] == 'n' &&
      name[2] == 'r' &&
      name[3] == 'e' &&
      name[4] == 'a' &&
      name[5] == 'l' &&
      name[6] == 'i' &&
      name[7] == 's' &&
      name[8] == 'e' &&
      name[9] == 'd' &&
      name[10] == 'P') {
    switch (name[11]) {
      case 'n':
        return parse_unrealisedPn(name);
      case 'r':
        return parse_unrealisedPr(name);
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

constexpr Field parse_wa(const std::string_view& name) {
  if (name.length() == 13 &&
      name[2] == 'l' &&
      name[3] == 'l' &&
      name[4] == 'e' &&
      name[5] == 't' &&
      name[6] == 'B' &&
      name[7] == 'a' &&
      name[8] == 'l' &&
      name[9] == 'a' &&
      name[10] == 'n' &&
      name[11] == 'c' &&
      name[12] == 'e') {
    return Field::WALLET_BALANCE;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_wi(const std::string_view& name) {
  if (name.length() == 18 &&
      name[2] == 't' &&
      name[3] == 'h' &&
      name[4] == 'd' &&
      name[5] == 'r' &&
      name[6] == 'a' &&
      name[7] == 'w' &&
      name[8] == 'a' &&
      name[9] == 'b' &&
      name[10] == 'l' &&
      name[11] == 'e' &&
      name[12] == 'M' &&
      name[13] == 'a' &&
      name[14] == 'r' &&
      name[15] == 'g' &&
      name[16] == 'i' &&
      name[17] == 'n') {
    return Field::WITHDRAWABLE_MARGIN;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_w(const std::string_view& name) {
  if (name.length() > 1) {
    switch (name[1]) {
      case 'a':
        return parse_wa(name);
      case 'i':
        return parse_wi(name);
    }
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
      case 'e':
        return parse_e(name);
      case 'g':
        return parse_g(name);
      case 'i':
        return parse_i(name);
      case 'm':
        return parse_m(name);
      case 'p':
        return parse_p(name);
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
      case 'w':
        return parse_w(name);
    }
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("account") == Field::ACCOUNT);
static_assert(parse_name("action") == Field::ACTION);
static_assert(parse_name("amount") == Field::AMOUNT);
static_assert(parse_name("availableMargin") == Field::AVAILABLE_MARGIN);
static_assert(parse_name("commission") == Field::COMMISSION);
static_assert(parse_name("confirmedDebit") == Field::CONFIRMED_DEBIT);
static_assert(parse_name("currency") == Field::CURRENCY);
static_assert(parse_name("excessMargin") == Field::EXCESS_MARGIN);
static_assert(parse_name("excessMarginPcnt") == Field::EXCESS_MARGIN_PCNT);
static_assert(parse_name("grossComm") == Field::GROSS_COMM);
static_assert(parse_name("grossExecCost") == Field::GROSS_EXEC_COST);
static_assert(parse_name("grossLastValue") == Field::GROSS_LAST_VALUE);
static_assert(parse_name("grossMarkValue") == Field::GROSS_MARK_VALUE);
static_assert(parse_name("grossOpenCost") == Field::GROSS_OPEN_COST);
static_assert(parse_name("grossOpenPremium") == Field::GROSS_OPEN_PREMIUM);
static_assert(parse_name("indicativeTax") == Field::INDICATIVE_TAX);
static_assert(parse_name("initMargin") == Field::INIT_MARGIN);
static_assert(parse_name("maintMargin") == Field::MAINT_MARGIN);
static_assert(parse_name("marginBalance") == Field::MARGIN_BALANCE);
static_assert(parse_name("marginBalancePcnt") == Field::MARGIN_BALANCE_PCNT);
static_assert(parse_name("marginLeverage") == Field::MARGIN_LEVERAGE);
static_assert(parse_name("marginUsedPcnt") == Field::MARGIN_USED_PCNT);
static_assert(parse_name("pendingCredit") == Field::PENDING_CREDIT);
static_assert(parse_name("pendingDebit") == Field::PENDING_DEBIT);
static_assert(parse_name("prevRealisedPnl") == Field::PREV_REALISED_PNL);
static_assert(parse_name("prevState") == Field::PREV_STATE);
static_assert(parse_name("prevUnrealisedPnl") == Field::PREV_UNREALISED_PNL);
static_assert(parse_name("realisedPnl") == Field::REALISED_PNL);
static_assert(parse_name("riskLimit") == Field::RISK_LIMIT);
static_assert(parse_name("riskValue") == Field::RISK_VALUE);
static_assert(parse_name("sessionMargin") == Field::SESSION_MARGIN);
static_assert(parse_name("state") == Field::STATE);
static_assert(parse_name("syntheticMargin") == Field::SYNTHETIC_MARGIN);
static_assert(parse_name("targetExcessMargin") == Field::TARGET_EXCESS_MARGIN);
static_assert(parse_name("taxableMargin") == Field::TAXABLE_MARGIN);
static_assert(parse_name("timestamp") == Field::TIMESTAMP);
static_assert(parse_name("unrealisedPnl") == Field::UNREALISED_PNL);
static_assert(parse_name("unrealisedProfit") == Field::UNREALISED_PROFIT);
static_assert(parse_name("varMargin") == Field::VAR_MARGIN);
static_assert(parse_name("walletBalance") == Field::WALLET_BALANCE);
static_assert(parse_name("withdrawableMargin") == Field::WITHDRAWABLE_MARGIN);

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
    case Field::ACTION:
      update(result.action, value);
      break;
    case Field::AMOUNT:
      update(result.amount, value);
      break;
    case Field::AVAILABLE_MARGIN:
      update(result.available_margin, value);
      break;
    case Field::COMMISSION:
      update(result.commission, value);
      break;
    case Field::CONFIRMED_DEBIT:
      update(result.confirmed_debit, value);
      break;
    case Field::CURRENCY:
      update(result.currency, value);
      break;
    case Field::EXCESS_MARGIN:
      update(result.excess_margin, value);
      break;
    case Field::EXCESS_MARGIN_PCNT:
      update(result.excess_margin_pcnt, value);
      break;
    case Field::GROSS_COMM:
      update(result.gross_comm, value);
      break;
    case Field::GROSS_EXEC_COST:
      update(result.gross_exec_cost, value);
      break;
    case Field::GROSS_LAST_VALUE:
      update(result.gross_last_value, value);
      break;
    case Field::GROSS_MARK_VALUE:
      update(result.gross_mark_value, value);
      break;
    case Field::GROSS_OPEN_COST:
      update(result.gross_open_cost, value);
      break;
    case Field::GROSS_OPEN_PREMIUM:
      update(result.gross_open_premium, value);
      break;
    case Field::INDICATIVE_TAX:
      update(result.indicative_tax, value);
      break;
    case Field::INIT_MARGIN:
      update(result.init_margin, value);
      break;
    case Field::MAINT_MARGIN:
      update(result.maint_margin, value);
      break;
    case Field::MARGIN_BALANCE:
      update(result.margin_balance, value);
      break;
    case Field::MARGIN_BALANCE_PCNT:
      update(result.margin_balance_pcnt, value);
      break;
    case Field::MARGIN_LEVERAGE:
      update(result.margin_leverage, value);
      break;
    case Field::MARGIN_USED_PCNT:
      update(result.margin_used_pcnt, value);
      break;
    case Field::PENDING_CREDIT:
      update(result.pending_credit, value);
      break;
    case Field::PENDING_DEBIT:
      update(result.pending_debit, value);
      break;
    case Field::PREV_REALISED_PNL:
      update(result.prev_realised_pnl, value);
      break;
    case Field::PREV_STATE:
      update(result.prev_state, value);
      break;
    case Field::PREV_UNREALISED_PNL:
      update(result.prev_unrealised_pnl, value);
      break;
    case Field::REALISED_PNL:
      update(result.realised_pnl, value);
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
    case Field::STATE:
      update(result.state, value);
      break;
    case Field::SYNTHETIC_MARGIN:
      update(result.synthetic_margin, value);
      break;
    case Field::TARGET_EXCESS_MARGIN:
      update(result.target_excess_margin, value);
      break;
    case Field::TAXABLE_MARGIN:
      update(result.taxable_margin, value);
      break;
    case Field::TIMESTAMP:
      update(result.timestamp, value);
      break;
    case Field::UNREALISED_PNL:
      update(result.unrealised_pnl, value);
      break;
    case Field::UNREALISED_PROFIT:
      update(result.unrealised_profit, value);
      break;
    case Field::VAR_MARGIN:
      update(result.var_margin, value);
      break;
    case Field::WALLET_BALANCE:
      update(result.wallet_balance, value);
      break;
    case Field::WITHDRAWABLE_MARGIN:
      update(result.withdrawable_margin, value);
      break;
  }
}
}  // namespace

MarginItem::MarginItem(core::json::value_t& value) {
  for (auto [key, value] : std::get<core::json::object_t>(value))
    update_field(*this, key, value);
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
