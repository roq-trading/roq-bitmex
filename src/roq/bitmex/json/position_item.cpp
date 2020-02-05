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
  OPENING_COMM,
  OPENING_COST,
  OPENING_QTY,
  OPENING_TIMESTAMP,
  OPEN_ORDER_BUY_COST,
  OPEN_ORDER_BUY_PREMIUM,
  OPEN_ORDER_BUY_QTY,
  OPEN_ORDER_SELL_COST,
  OPEN_ORDER_SELL_PREMIUM,
  OPEN_ORDER_SELL_QTY,
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
  TAXABLE_MARGIN,
  TAX_BASE,
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

constexpr Field parse_a(auto& name) {
  if (name.length() >= 4) {
    switch (name[3]) {
      case 'o':
        if (name.compare("account") == 0)
          return Field::ACCOUNT;
        break;
      case 'C':
        if (name.compare("avgCostPrice") == 0)
          return Field::AVG_COST_PRICE;
        break;
      case 'E':
        if (name.compare("avgEntryPrice") == 0)
          return Field::AVG_ENTRY_PRICE;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_b(auto& name) {
  if (name.length() >= 2) {
    switch (name[1]) {
      case 'a':
        if (name.compare("bankruptPrice") == 0)
          return Field::BANKRUPT_PRICE;
        break;
      case 'r':
        if (name.compare("breakEvenPrice") == 0)
          return Field::BREAK_EVEN_PRICE;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_c(auto& name) {
  if (name.length() >= 8) {
    switch (name[7]) {
      case 'i':
        if (name.compare("commission") == 0)
          return Field::COMMISSION;
        break;
      case 'r':
        if (name.compare("crossMargin") == 0)
          return Field::CROSS_MARGIN;
        break;
      case 'y':
        if (name.compare("currency") == 0)
          return Field::CURRENCY;
        break;
      case 'C': {
        if (name.length() >= 10) {
          switch (name[9]) {
            case 'm':
              if (name.compare("currentComm") == 0)
                return Field::CURRENT_COMM;
              break;
            case 's':
              if (name.compare("currentCost") == 0)
                return Field::CURRENT_COST;
              break;
          }
        }
        break;
      }
      case 'Q':
        if (name.compare("currentQty") == 0)
          return Field::CURRENT_QTY;
        break;
      case 'T':
        if (name.compare("currentTimestamp") == 0)
          return Field::CURRENT_TIMESTAMP;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_d(auto& name) {
  if (name.compare("deleveragePercentile") == 0)
    return Field::DELEVERAGE_PERCENTILE;
  return Field::UNKNOWN;
}

constexpr Field parse_e(auto& name) {
  if (name.length() >= 8) {
    switch (name[7]) {
      case 'C':
        if (name.compare("execBuyCost") == 0)
          return Field::EXEC_BUY_COST;
        break;
      case 'Q':
        if (name.compare("execBuyQty") == 0)
          return Field::EXEC_BUY_QTY;
        break;
      case 'm':
        if (name.compare("execComm") == 0)
          return Field::EXEC_COMM;
        break;
      case 't':
        if (name.compare("execCost") == 0)
          return Field::EXEC_COST;
        break;
      case 'l': {
        if (name.length() >= 9) {
          switch (name[8]) {
            case 'C':
              if (name.compare("execSellCost") == 0)
                return Field::EXEC_SELL_COST;
              break;
            case 'Q':
              if (name.compare("execSellQty") == 0)
                return Field::EXEC_SELL_QTY;
              break;
          }
        }
      }
    }
  }
  if (name.compare("execQty") == 0)
    return Field::EXEC_QTY;
  return Field::UNKNOWN;
}

constexpr Field parse_f(auto& name) {
  if (name.compare("foreignNotional") == 0)
    return Field::FOREIGN_NOTIONAL;
  return Field::UNKNOWN;
}

constexpr Field parse_g(auto& name) {
  if (name.length() >= 10) {
    switch (name[9]) {
      case 'C': {
        switch (name[5]) {
          case 'E':
            if (name.compare("grossExecCost") == 0)
              return Field::GROSS_EXEC_COST;
            break;
          case 'O':
            if (name.compare("grossOpenCost") == 0)
              return Field::GROSS_OPEN_COST;
            break;
        }
      }
      case 'P':
        if (name.compare("grossOpenPremium") == 0)
          return Field::GROSS_OPEN_PREMIUM;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_h(auto& name) {
  if (name.compare("homeNotional") == 0)
    return Field::HOME_NOTIONAL;
  return Field::UNKNOWN;
}

constexpr Field parse_i(auto& name) {
  if (name.length() >= 2) {
    switch (name[1]) {
      case 'n': {
        if (name.length() >= 3) {
          switch (name[2]) {
            case 'd':
              if (name.compare("indicativeTax") == 0)
                return Field::INDICATIVE_TAX;
              if (name.compare("indicativeTaxRate") == 0)
                return Field::INDICATIVE_TAX_RATE;
              break;
            case 'i':
              if (name.compare("initMargin") == 0)
                return Field::INIT_MARGIN;
              if (name.compare("initMarginReq") == 0)
                return Field::INIT_MARGIN_REQ;
              break;
          }
        }
        break;
      }
      case 's':
        if (name.compare("isOpen") == 0)
          return Field::IS_OPEN;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_l(auto& name) {
  if (name.length() >= 5) {
    switch (name[4]) {
      case 'P':
        if (name.compare("lastPrice") == 0)
          return Field::LAST_PRICE;
        break;
      case 'V':
        if (name.compare("lastValue") == 0)
          return Field::LAST_VALUE;
        break;
      case 'r':
        if (name.compare("leverage") == 0)
          return Field::LEVERAGE;
        break;
      case 'i':
        if (name.compare("liquidationPrice") == 0)
          return Field::LIQUIDATION_PRICE;
        break;
      case 'B':
        if (name.compare("longBankrupt") == 0)
          return Field::LONG_BANKRUPT;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_m(auto& name) {
  if (name.length() >= 5) {
    switch (name[4]) {
      case 't':
        if (name.compare("maintMargin") == 0)
          return Field::MAINT_MARGIN;
        if (name.compare("maintMarginReq") == 0)
          return Field::MAINT_MARGIN_REQ;
        break;
      case 'i':
        if (name.compare("marginCallPrice") == 0)
          return Field::MARGIN_CALL_PRICE;
        break;
      case 'P':
        if (name.compare("markPrice") == 0)
          return Field::MARK_PRICE;
        break;
      case 'V':
        if (name.compare("markValue") == 0)
          return Field::MARK_VALUE;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_o(auto& name) {
  if (name.length() >= 10) {
    switch (name[9]) {
      case 'm': {
        switch (name[7]) {
          case 'C':
            if (name.compare("openingComm") == 0)
              return Field::OPENING_COMM;
            break;
          case 'T':
            if (name.compare("openingTimestamp") == 0)
              return Field::OPENING_TIMESTAMP;
            break;
        }
        break;
      }
      case 's':
        if (name.compare("openingCost") == 0)
          return Field::OPENING_COST;
        break;
      case 'y':
        if (name.compare("openingQty") == 0)
          return Field::OPENING_QTY;
        break;
      case 'B': {
        if (name.length() >= 13) {
          switch (name[12]) {
            case'C':
              if (name.compare("openOrderBuyCost") == 0)
                return Field::OPEN_ORDER_BUY_COST;
              break;
            case'P':
              if (name.compare("openOrderBuyPremium") == 0)
                return Field::OPEN_ORDER_BUY_PREMIUM;
              break;
            case'Q':
              if (name.compare("openOrderBuyQty") == 0)
                return Field::OPEN_ORDER_BUY_QTY;
              break;
          }
        }
        break;
      }
      case 'S': {
        if (name.length() >= 14) {
          switch (name[13]) {
            case'C':
              if (name.compare("openOrderSellCost") == 0)
                return Field::OPEN_ORDER_SELL_COST;
              break;
            case'P':
              if (name.compare("openOrderSellPremium") == 0)
                return Field::OPEN_ORDER_SELL_PREMIUM;
              break;
            case'Q':
              if (name.compare("openOrderSellQty") == 0)
                return Field::OPEN_ORDER_SELL_QTY;
              break;
          }
        }
        break;
      }
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_p(auto& name) {
  if (name.length() >= 6) {
    switch (name[5]) {
      case 'l': {
        switch (name[1]) {
          case 'o':
            if (name.compare("posAllowance") == 0)
              return Field::POS_ALLOWANCE;
            break;
          case 'r':
            if (name.compare("prevClosePrice") == 0)
              return Field::PREV_CLOSE_PRICE;
            break;
        }
        break;
      }
      case 'm':
        if (name.compare("posComm") == 0)
          return Field::POS_COMM;
        break;
      case 's': {
        switch (name[3]) {
          case 'C':
            if (name.compare("posCost") == 0)
              return Field::POS_COST;
            if (name.compare("posCost2") == 0)
              return Field::POS_COST2;
            break;
          case 'L':
            if (name.compare("posLoss") == 0)
              return Field::POS_LOSS;
            break;
        }
        break;
      }
      case 'o':
        if (name.compare("posCross") == 0)
          return Field::POS_CROSS;
        break;
      case 'i': {
        switch (name[3]) {
          case 'I':
            if (name.compare("posInit") == 0)
              return Field::POS_INIT;
            break;
          case 'M':
            if (name.compare("posMaint") == 0)
              return Field::POS_MAINT;
            break;
        }
        break;
      }
      case 'r':
        if (name.compare("posMargin") == 0)
          return Field::POS_MARGIN;
        break;
      case 'a':
        if (name.compare("posState") == 0)
          return Field::POS_STATE;
        break;
      case 'e':
        if (name.compare("prevRealisedPnl") == 0)
          return Field::PREV_REALISED_PNL;
        break;
      case 'n':
        if (name.compare("prevUnrealisedPnl") == 0)
          return Field::PREV_UNREALISED_PNL;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_q(auto& name) {
  if (name.compare("quoteCurrency") == 0)
    return Field::QUOTE_CURRENCY;
  return Field::UNKNOWN;
}

constexpr Field parse_r(auto& name) {
  if (name.length() >= 7) {
    switch (name[6]) {
      case 'e': {
        if (name.length() >= 9) {
          switch (name[8]) {
            case 'C':
              if (name.compare("realisedCost") == 0)
                return Field::REALISED_COST;
              break;
            case 'G':
              if (name.compare("realisedGrossPnl") == 0)
                return Field::REALISED_GROSS_PNL;
              break;
            case 'P':
              if (name.compare("realisedPnl") == 0)
                return Field::REALISED_PNL;
              break;
            case 'T':
              if (name.compare("realisedTax") == 0)
                return Field::REALISED_TAX;
              break;
          }
        }
        break;
      }
      case 'n':
        if (name.compare("rebalancedPnl") == 0)
          return Field::REBALANCED_PNL;
        break;
      case 'm':
        if (name.compare("riskLimit") == 0)
          return Field::RISK_LIMIT;
        break;
      case 'l':
        if (name.compare("riskValue") == 0)
          return Field::RISK_VALUE;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_s(auto& name) {
  if (name.length() >= 2) {
    switch (name[1]) {
      case 'e':
        if (name.compare("sessionMargin") == 0)
          return Field::SESSION_MARGIN;
        break;
      case 'h':
        if (name.compare("shortBankrupt") == 0)
          return Field::SHORT_BANKRUPT;
        break;
      case 'i': {
        if (name.length() >= 7) {
          switch (name[6]) {
            case 'C':
              if (name.compare("simpleCost") == 0)
                return Field::SIMPLE_COST;
              break;
            case 'P':
              if (name.compare("simplePnl") == 0)
                return Field::SIMPLE_PNL;
              if (name.compare("simplePnlPcnt") == 0)
                return Field::SIMPLE_PNL_PCNT;
              break;
            case 'Q':
              if (name.compare("simpleQty") == 0)
                return Field::SIMPLE_QTY;
              break;
            case 'V':
              if (name.compare("simpleValue") == 0)
                return Field::SIMPLE_VALUE;
              break;
          }
        }
        break;
      }
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
      case 'g':
        if (name.compare("targetExcessMargin") == 0)
          return Field::TARGET_EXCESS_MARGIN;
        break;
      case 'a':
        if (name.compare("taxableMargin") == 0)
          return Field::TAXABLE_MARGIN;
        break;
      case 'B':
        if (name.compare("taxBase") == 0)
          return Field::TAX_BASE;
        break;
      case 'e':
        if (name.compare("timestamp") == 0)
          return Field::TIMESTAMP;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_u(auto& name) {
  if (name.length() >= 11) {
    switch (name[10]) {
      case 'C':
        if (name.compare("unrealisedCost") == 0)
          return Field::UNREALISED_COST;
        break;
      case 'G':
        if (name.compare("unrealisedGrossPnl") == 0)
          return Field::UNREALISED_GROSS_PNL;
        break;
      case 'P':
        if (name.compare("unrealisedPnl") == 0)
          return Field::UNREALISED_PNL;
        if (name.compare("unrealisedPnlPcnt") == 0)
          return Field::UNREALISED_PNL_PCNT;
        break;
      case 'R':
        if (name.compare("unrealisedRoePcnt") == 0)
          return Field::UNREALISED_ROE_PCNT;
        break;
      case 'T':
        if (name.compare("unrealisedTax") == 0)
          return Field::UNREALISED_TAX;
        break;
    }
  }
  if (name.compare("underlying") == 0)
    return Field::UNDERLYING;
  return Field::UNKNOWN;
}

constexpr Field parse_v(auto& name) {
  if (name.compare("varMargin") == 0)
    return Field::VAR_MARGIN;
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  if (name.empty())
    return Field::UNKNOWN;
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
    default:
      return Field::UNKNOWN;
  }
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

static_assert(parse_name("openingComm") == Field::OPENING_COMM);
static_assert(parse_name("openingCost") == Field::OPENING_COST);
static_assert(parse_name("openingQty") == Field::OPENING_QTY);
static_assert(parse_name("openingTimestamp") == Field::OPENING_TIMESTAMP);
static_assert(parse_name("openOrderBuyCost") == Field::OPEN_ORDER_BUY_COST);
static_assert(parse_name("openOrderBuyPremium") == Field::OPEN_ORDER_BUY_PREMIUM);
static_assert(parse_name("openOrderBuyQty") == Field::OPEN_ORDER_BUY_QTY);
static_assert(parse_name("openOrderSellCost") == Field::OPEN_ORDER_SELL_COST);
static_assert(parse_name("openOrderSellPremium") == Field::OPEN_ORDER_SELL_PREMIUM);
static_assert(parse_name("openOrderSellQty") == Field::OPEN_ORDER_SELL_QTY);

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
static_assert(parse_name("taxableMargin") == Field::TAXABLE_MARGIN);
static_assert(parse_name("taxBase") == Field::TAX_BASE);
static_assert(parse_name("timestamp") == Field::TIMESTAMP);

static_assert(parse_name("underlying") == Field::UNDERLYING);
static_assert(parse_name("unrealisedCost") == Field::UNREALISED_COST);
static_assert(parse_name("unrealisedGrossPnl") == Field::UNREALISED_GROSS_PNL);
static_assert(parse_name("unrealisedPnl") == Field::UNREALISED_PNL);
static_assert(parse_name("unrealisedPnlPcnt") == Field::UNREALISED_PNL_PCNT);
static_assert(parse_name("unrealisedRoePcnt") == Field::UNREALISED_ROE_PCNT);
static_assert(parse_name("unrealisedTax") == Field::UNREALISED_TAX);

static_assert(parse_name("varMargin") == Field::VAR_MARGIN);

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
    case Field::TAXABLE_MARGIN:
      update(result.taxable_margin, value);
      break;
    case Field::TAX_BASE:
      update(result.tax_base, value);
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
