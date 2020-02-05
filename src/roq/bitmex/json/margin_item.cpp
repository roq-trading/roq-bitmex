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

constexpr Field parse_a(auto& name) {
  if (name.length() >= 3) {
    switch (name[2]) {
      case 'c':
        if (name.compare("account") == 0)
          return Field::ACCOUNT;
        break;
      case 't':
        if (name.compare("action") == 0)
          return Field::ACTION;
        break;
      case 'o':
        if (name.compare("amount") == 0)
          return Field::AMOUNT;
        break;
      case 'a':
        if (name.compare("availableMargin") == 0)
          return Field::AVAILABLE_MARGIN;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_c(auto& name) {
  if (name.length() >= 2) {
    switch (name[2]) {
      case 'm':
        if (name.compare("commission") == 0)
          return Field::COMMISSION;
        break;
      case 'n':
        if (name.compare("confirmedDebit") == 0)
          return Field::CONFIRMED_DEBIT;
        break;
      case 'r':
        if (name.compare("currency") == 0)
          return Field::CURRENCY;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_e(auto& name) {
  if (name.length() > 12) {
    if (name.compare("excessMarginPcnt") == 0)
      return Field::EXCESS_MARGIN_PCNT;
  } else {
    if (name.compare("excessMargin") == 0)
      return Field::EXCESS_MARGIN;
  }
  return Field::UNKNOWN;
}

constexpr Field parse_g(auto& name) {
  if (name.length() >= 6) {
    switch (name[5]) {
      case 'C':
        if (name.compare("grossComm") == 0)
          return Field::GROSS_COMM;
        break;
      case 'E':
        if (name.compare("grossExecCost") == 0)
          return Field::GROSS_EXEC_COST;
        break;
      case 'L':
        if (name.compare("grossLastValue") == 0)
          return Field::GROSS_LAST_VALUE;
        break;
      case 'M':
        if (name.compare("grossMarkValue") == 0)
          return Field::GROSS_MARK_VALUE;
        break;
      case 'O':
        if (name.length() >= 10) {
          switch (name[9]) {
            case 'C':
              if (name.compare("grossOpenCost") == 0)
                return Field::GROSS_OPEN_COST;
              break;
            case 'P':
              if (name.compare("grossOpenPremium") == 0)
                return Field::GROSS_OPEN_PREMIUM;
              break;
          }
        }
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_i(auto& name) {
  if (name.length() >= 3) {
    switch (name[2]) {
      case 'd':
        if (name.compare("indicativeTax") == 0)
          return Field::INDICATIVE_TAX;
        break;
      case 'i':
        if (name.compare("initMargin") == 0)
          return Field::INIT_MARGIN;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_m(auto& name) {
  if (name.length() >= 7) {
    switch (name[6]) {
      case 'a':
        if (name.compare("maintMargin") == 0)
          return Field::MAINT_MARGIN;
        break;
      case 'B':
        if (name.length() >= 14) {
          if (name.compare("marginBalancePcnt") == 0)
            return Field::MARGIN_BALANCE_PCNT;
        } else {
          if (name.compare("marginBalance") == 0)
            return Field::MARGIN_BALANCE;
        }
        break;
      case 'L':
        if (name.compare("marginLeverage") == 0)
          return Field::MARGIN_LEVERAGE;
        break;
      case 'U':
        if (name.compare("marginUsedPcnt") == 0)
          return Field::MARGIN_USED_PCNT;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_p(auto& name) {
  if (name.length() >= 8) {
    switch (name[7]) {
      case 'C':
        if (name.compare("pendingCredit") == 0)
          return Field::PENDING_CREDIT;
        break;
      case 'D':
        if (name.compare("pendingDebit") == 0)
          return Field::PENDING_DEBIT;
        break;
      case 'l':
        if (name.compare("prevRealisedPnl") == 0)
          return Field::PREV_REALISED_PNL;
        break;
      case 't':
        if (name.compare("prevState") == 0)
          return Field::PREV_STATE;
        break;
      case 'e':
        if (name.compare("prevUnrealisedPnl") == 0)
          return Field::PREV_UNREALISED_PNL;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_r(auto& name) {
  if (name.length() >= 5) {
    switch (name[4]) {
      case 'i':
        if (name.compare("realisedPnl") == 0)
          return Field::REALISED_PNL;
        break;
      case 'L':
        if (name.compare("riskLimit") == 0)
          return Field::RISK_LIMIT;
        break;
      case 'V':
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
      case 't':
        if (name.compare("state") == 0)
          return Field::STATE;
        break;
      case 'y':
        if (name.compare("syntheticMargin") == 0)
          return Field::SYNTHETIC_MARGIN;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_t(auto& name) {
  if (name.length() >= 2) {
    switch (name[2]) {
      case 'r':
        if (name.compare("targetExcessMargin") == 0)
          return Field::TARGET_EXCESS_MARGIN;
        break;
      case 'x':
        if (name.compare("taxableMargin") == 0)
          return Field::TAXABLE_MARGIN;
        break;
      case 'm':
        if (name.compare("timestamp") == 0)
          return Field::TIMESTAMP;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_u(auto& name) {
  if (name.length() >= 12) {
    switch (name[11]) {
      case 'n':
        if (name.compare("unrealisedPnl") == 0)
          return Field::UNREALISED_PNL;
        break;
      case 'r':
        if (name.compare("unrealisedProfit") == 0)
          return Field::UNREALISED_PROFIT;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_v(auto& name) {
  if (name.compare("varMargin") == 0)
    return Field::VAR_MARGIN;
  return Field::UNKNOWN;
}

constexpr Field parse_w(auto& name) {
  if (name.length() >= 2) {
    switch (name[1]) {
      case 'a':
        if (name.compare("walletBalance") == 0)
          return Field::WALLET_BALANCE;
        break;
      case 'i':
        if (name.compare("withdrawableMargin") == 0)
          return Field::WITHDRAWABLE_MARGIN;
        break;
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  if (name.empty())
    return Field::UNKNOWN;
  switch (name[0]) {
    case 'a': return parse_a(name);
    case 'c': return parse_c(name);
    case 'e': return parse_e(name);
    case 'g': return parse_g(name);
    case 'i': return parse_i(name);
    case 'm': return parse_m(name);
    case 'p': return parse_p(name);
    case 'r': return parse_r(name);
    case 's': return parse_s(name);
    case 't': return parse_t(name);
    case 'u': return parse_u(name);
    case 'v': return parse_v(name);
    case 'w': return parse_w(name);
    default:
      return Field::UNKNOWN;
  }
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
