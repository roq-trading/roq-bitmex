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
  FOREIGN_NOTIONAL_24H,
  FRONT,
  FUNDING_BASE_SYMBOL,
  FUNDING_INTERVAL,
  FUNDING_PREMIUM_SYMBOL,
  FUNDING_QUOTE_SYMBOL,
  FUNDING_RATE,
  FUNDING_TIMESTAMP,
  HAS_LIQUIDITY,
  HIGH_PRICE,
  HOME_NOTIONAL_24H,
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
  OPENING_TIMESTAMP,
  OPEN_INTEREST,
  OPEN_VALUE,
  OPTION_MULTIPLIER,
  OPTION_STRIKE_PCNT,
  OPTION_STRIKE_PRICE,
  OPTION_STRIKE_ROUND,
  OPTION_UNDERLYING_PRICE,
  POSITION_CURRENCY,
  PREV_CLOSE_PRICE,
  PREV_PRICE_24H,
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
  TURNOVER_24H,
  TYP,
  UNDERLYING,
  UNDERLYING_SYMBOL,
  UNDERLYING_TO_POSITION_MULTIPLIER,
  UNDERLYING_TO_SETTLE_MULTIPLIER,
  VOLUME,
  VOLUME_24H,
  VWAP,
};

constexpr Field parse_a(auto& name) {
  if (name.compare("askPrice") == 0)
    return Field::ASK_PRICE;
  return Field::UNKNOWN;
}

constexpr Field parse_b(auto& name) {
  if (name.length() >= 2) {
    switch (name.data()[1]) {
      case 'a': {
        if (name.length() >= 14) {
          switch (name.data()[13]) {
            case 'D': {
              if (name.compare("bankruptLimitDownPrice") == 0)
                return Field::BANKRUPT_LIMIT_DOWN_PRICE;
              break;
            }
            case 'U': {
              if (name.compare("bankruptLimitUpPrice") == 0)
                return Field::BANKRUPT_LIMIT_UP_PRICE;
              break;
            }
          }
        }
        break;
      }
      case 'i': {
        if (name.compare("bidPrice") == 0)
          return Field::BID_PRICE;
        break;
      }
      case 'u': {
        if (name.compare("buyLeg") == 0)
          return Field::BUY_LEG;
        break;
      }
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_c(auto& name) {
  if (name.length() >= 3) {
    switch (name.data()[2]) {
      case 'l': {
        if (name.compare("calcInterval") == 0)
          return Field::CALC_INTERVAL;
        break;
      }
      case 'p': {
        if (name.compare("capped") == 0)
          return Field::CAPPED;
        break;
      }
      case 'o': {
        if (name.compare("closingTimestamp") == 0)
          return Field::CLOSING_TIMESTAMP;
        break;
      }
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_d(auto& name) {
  if (name.compare("deleverage") == 0)
    return Field::DELEVERAGE;
  return Field::UNKNOWN;
}

constexpr Field parse_e(auto& name) {
  if (name.compare("expiry") == 0)
    return Field::EXPIRY;
  return Field::UNKNOWN;
}

constexpr Field parse_f(auto& name) {
  if (name.length() >= 3) {
    switch (name.data()[2]) {
      case 'i': {
        if (name.length() >= 5) {
          switch (name.data()[4]) {
            case 'B': {
              if (name.compare("fairBasis") == 0)
                return Field::FAIR_BASIS;
              if (name.compare("fairBasisRate") == 0)
                return Field::FAIR_BASIS_RATE;
              break;
            }
            case 'M': {
              if (name.compare("fairMethod") == 0)
                return Field::FAIR_METHOD;
              break;
            }
            case 'P': {
              if (name.compare("fairPrice") == 0)
                return Field::FAIR_PRICE;
              break;
            }
          }
        }
        break;
      }
      case 'r': {
        if (name.compare("foreignNotional24h") == 0)
          return Field::FOREIGN_NOTIONAL_24H;
        break;
      }
      case 'o': {
        if (name.compare("front") == 0)
          return Field::FRONT;
        break;
      }
      case 'n': {
        if (name.length() >= 8) {
          switch (name.data()[7]) {
            case 'B': {
              if (name.compare("fundingBaseSymbol") == 0)
                return Field::FUNDING_BASE_SYMBOL;
              break;
            }
            case 'I': {
              if (name.compare("fundingInterval") == 0)
                return Field::FUNDING_INTERVAL;
              break;
            }
            case 'P': {
              if (name.compare("fundingPremiumSymbol") == 0)
                return Field::FUNDING_PREMIUM_SYMBOL;
              break;
            }
            case 'Q': {
              if (name.compare("fundingQuoteSymbol") == 0)
                return Field::FUNDING_QUOTE_SYMBOL;
              break;
            }
            case 'R': {
              if (name.compare("fundingRate") == 0)
                return Field::FUNDING_RATE;
              break;
            }
            case 'T': {
              if (name.compare("fundingTimestamp") == 0)
                return Field::FUNDING_TIMESTAMP;
              break;
            }
          }
        }
        break;
      }
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_h(auto& name) {
  if (name.length() >= 2) {
    switch (name.data()[1]) {
      case 'a': {
        if (name.compare("hasLiquidity") == 0)
          return Field::HAS_LIQUIDITY;
        break;
      }
      case 'i': {
        if (name.compare("highPrice") == 0)
          return Field::HIGH_PRICE;
        break;
      }
      case 'o': {
        if (name.compare("homeNotional24h") == 0)
          return Field::HOME_NOTIONAL_24H;
        break;
      }
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_i(auto& name) {
  if (name.length() >= 7) {
    switch (name.data()[6]) {
      case 'A': {
        if (name.compare("impactAskPrice") == 0)
          return Field::IMPACT_ASK_PRICE;
        break;
      }
      case 'B': {
        if (name.compare("impactBidPrice") == 0)
          return Field::IMPACT_BID_PRICE;
        break;
      }
      case 'M': {
        if (name.compare("impactMidPrice") == 0)
          return Field::IMPACT_MID_PRICE;
        break;
      }
      case 't': {
        if (name.length() >= 11) {
          switch (name.data()[10]) {
            case 'F': {
              if (name.compare("indicativeFundingRate") == 0)
                return Field::INDICATIVE_FUNDING_RATE;
              break;
            }
            case 'S': {
              if (name.compare("indicativeSettlePrice") == 0)
                return Field::INDICATIVE_SETTLE_PRICE;
              break;
            }
            case 'T': {
              if (name.compare("indicativeTaxRate") == 0)
                return Field::INDICATIVE_TAX_RATE;
              break;
            }
          }
        }
        if (name.compare("isQuanto") == 0)
          return Field::IS_QUANTO;
        break;
      }

      case 'r': {
        if (name.compare("initMargin") == 0)
          return Field::INIT_MARGIN;
        if (name.compare("isInverse") == 0)
          return Field::IS_INVERSE;
        break;
      }
      case 'n': {
        if (name.compare("insuranceFee") == 0)
          return Field::INSURANCE_FEE;
        break;
      }
      case 'e': {
        if (name.compare("inverseLeg") == 0)
          return Field::INVERSE_LEG;
        break;
      }
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_l(auto& name) {
  if (name.length() >= 6) {
    switch (name.data()[5]) {
      case 'h': {
        if (name.compare("lastChangePcnt") == 0)
          return Field::LAST_CHANGE_PCNT;
        break;
      }
      case 'r': {
        if (name.compare("lastPrice") == 0)
          return Field::LAST_PRICE;
        if (name.compare("lastPriceProtected") == 0)
          return Field::LAST_PRICE_PROTECTED;
        break;
      }
      case 'i': {
        switch (name.data()[1]) {
          case 'a':
            if (name.compare("lastTickDirection") == 0)
              return Field::LAST_TICK_DIRECTION;
            break;
          case 'o':
            if (name.compare("lowPrice") == 0)
              return Field::LOW_PRICE;
            break;
        }
        break;
      }
      case 'D': {
        if (name.compare("limitDownPrice") == 0)
          return Field::LIMIT_DOWN_PRICE;
        break;
      }
      case 'U': {
        if (name.compare("limitUpPrice") == 0)
          return Field::LIMIT_UP_PRICE;
        break;
      }
      case 'n': {
        if (name.compare("listing") == 0)
          return Field::LISTING;
        break;
      }
      case 'z': {
        if (name.compare("lotSize") == 0)
          return Field::LOT_SIZE;
        break;
      }
    }
  }
  if (name.compare("limit") == 0)
    return Field::LIMIT;
  return Field::UNKNOWN;
}

constexpr Field parse_m(auto& name) {
  if (name.length() >= 6) {
    switch (name.data()[5]) {
      case 'M': {
        if (name.compare("maintMargin") == 0)
          return Field::MAINT_MARGIN;
        break;
      }
      case 'F': {
        if (name.compare("makerFee") == 0)
          return Field::MAKER_FEE;
        break;
      }
      case 'e': {
        if (name.compare("markMethod") == 0)
          return Field::MARK_METHOD;
        break;
      }
      case 'r': {
        if (name.compare("markPrice") == 0)
          return Field::MARK_PRICE;
        break;
      }
      case 'd': {
        if (name.compare("maxOrderQty") == 0)
          return Field::MAX_ORDER_QTY;
        break;
      }
      case 'i': {
        switch (name.data()[1]) {
          case 'a':
            if (name.compare("maxPrice") == 0)
              return Field::MAX_PRICE;
            break;
          case 'i':
            if (name.compare("midPrice") == 0)
              return Field::MID_PRICE;
            break;
        }
        break;
      }
      case 'p': {
        if (name.compare("multiplier") == 0)
          return Field::MULTIPLIER;
        break;
      }
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_o(auto& name) {
  if (name.length() >= 3) {
    switch (name.data()[2]) {
      case 'e': {
        if (name.length() >= 7) {
          switch (name.data()[6]) {
            case 'g':
              if (name.compare("openingTimestamp") == 0)
                return Field::OPENING_TIMESTAMP;
              break;
            case 't':
              if (name.compare("openInterest") == 0)
                return Field::OPEN_INTEREST;
              break;
            case 'l':
              if (name.compare("openValue") == 0)
                return Field::OPEN_VALUE;
              break;
          }
        }
        break;
      }
      case 't': {
        if (name.length() >= 16) {
          switch (name.data()[15]) {
            case 'r':
              if (name.compare("optionMultiplier") == 0)
                return Field::OPTION_MULTIPLIER;
              break;
            case 't':
              if (name.compare("optionStrikePcnt") == 0)
                return Field::OPTION_STRIKE_PCNT;
              break;
            case 'c':
              if (name.compare("optionStrikePrice") == 0)
                return Field::OPTION_STRIKE_PRICE;
              break;
            case 'n':
              if (name.compare("optionStrikeRound") == 0)
                return Field::OPTION_STRIKE_ROUND;
              break;
            case 'g':
              if (name.compare("optionUnderlyingPrice") == 0)
                return Field::OPTION_UNDERLYING_PRICE;
              break;
          }
        }
        break;
      }
      case 'p': {
        if (name.compare("multiplier") == 0)
          return Field::MULTIPLIER;
        break;
      }
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_p(auto& name) {
  if (name.length() >= 8) {
    switch (name.data()[7]) {
      case 'n': {
        if (name.compare("positionCurrency") == 0)
          return Field::POSITION_CURRENCY;
        break;
      }
      case 's': {
        if (name.compare("prevClosePrice") == 0)
          return Field::PREV_CLOSE_PRICE;
        break;
      }
      case 'c': {
        if (name.compare("prevPrice24h") == 0)
          return Field::PREV_PRICE_24H;
        break;
      }
      case 'a': {
        if (name.length() >= 10) {
          switch (name.data()[9]) {
            case 'T':
              if (name.compare("prevTotalTurnover") == 0)
                return Field::PREV_TOTAL_TURNOVER;
              break;
            case 'V':
              if (name.compare("prevTotalVolume") == 0)
                return Field::PREV_TOTAL_VOLUME;
              break;
          }
        }
        break;
      }
      case 'I': {
        if (name.compare("publishInterval") == 0)
          return Field::PUBLISH_INTERVAL;
        break;
      }
      case 'T': {
        if (name.compare("publishTime") == 0)
          return Field::PUBLISH_TIME;
        break;
      }
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_q(auto& name) {
  if (name.length() >= 6) {
    switch (name.data()[5]) {
      case 'C': {
        if (name.compare("quoteCurrency") == 0)
          return Field::QUOTE_CURRENCY;
        break;
      }
      case 'T': {
        if (name.compare("quoteToSettleMultiplier") == 0)
          return Field::QUOTE_TO_SETTLE_MULTIPLIER;
        break;
      }
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_r(auto& name) {
  if (name.length() >= 8) {
    switch (name.data()[7]) {
      case 'c': {
        if (name.length() >= 10) {
          switch (name.data()[9]) {
            case 'I': {
              if (name.compare("rebalanceInterval") == 0)
                return Field::REBALANCE_INTERVAL;
              break;
            }
            case 'T': {
              if (name.compare("rebalanceTimestamp") == 0)
                return Field::REBALANCE_TIMESTAMP;
              break;
            }
            case 'S': {
              if (name.compare("referenceSymbol") == 0)
                return Field::REFERENCE_SYMBOL;
              break;
            }
          }
        }
        if (name.compare("reference") == 0)
          return Field::REFERENCE;
        break;
      }
      case 'n': {
        if (name.compare("relistInterval") == 0)
          return Field::RELIST_INTERVAL;
        break;
      }
      case 'i': {
        if (name.compare("riskLimit") == 0)
          return Field::RISK_LIMIT;
        break;
      }
      case 'p': {
        if (name.compare("riskStep") == 0)
          return Field::RISK_STEP;
        break;
      }
      case 'b': {
        if (name.compare("rootSymbol") == 0)
          return Field::ROOT_SYMBOL;
        break;
      }
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_s(auto& name) {
  if (name.length() >= 2) {
    switch (name.data()[1]) {
      case 'e': {
        if (name.length() >= 6) {
          switch (name.data()[5]) {
            case 'e': {
              if (name.length() >= 7) {
                switch (name.data()[6]) {
                  case 'g': {
                    if (name.compare("sellLeg") == 0)
                      return Field::SELL_LEG;
                    break;
                  }
                  case 'd': {
                    if (name.compare("settledPrice") == 0)
                      return Field::SETTLED_PRICE;
                    break;
                  }
                  case 'm': {
                    if (name.compare("settlementFee") == 0)
                      return Field::SETTLEMENT_FEE;
                    break;
                  }
                }
              }
              if (name.compare("settle") == 0)
                return Field::SETTLE;
              break;
            }
            case 'o': {
              if (name.compare("sessionInterval") == 0)
                return Field::SESSION_INTERVAL;
              break;
            }
            case 'C': {
              if (name.compare("settlCurrency") == 0)
                return Field::SETTL_CURRENCY;
              break;
            }
          }
        }
        break;
      }
      case 't': {
        if (name.compare("state") == 0)
          return Field::STATE;
        break;
      }
      case 'y': {
        if (name.compare("symbol") == 0)
          return Field::SYMBOL;
        break;
      }
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_t(auto& name) {
  if (name.length() >= 3) {
    switch (name.data()[2]) {
      case 'k': {
        if (name.compare("takerFee") == 0)
          return Field::TAKER_FEE;
        break;
      }
      case 'x': {
        if (name.compare("taxed") == 0)
          return Field::TAXED;
        break;
      }
      case 'c': {
        if (name.compare("tickSize") == 0)
          return Field::TICK_SIZE;
        break;
      }
      case 'm': {
        if (name.compare("timestamp") == 0)
          return Field::TIMESTAMP;
        break;
      }
      case 't': {
        if (name.length() >= 6) {
          switch (name.data()[5]) {
            case 'T': {
              if (name.compare("totalTurnover") == 0)
                return Field::TOTAL_TURNOVER;
              break;
            }
            case 'V': {
              if (name.compare("totalVolume") == 0)
                return Field::TOTAL_VOLUME;
              break;
            }
          }
        }
        break;
      }
      case 'r': {
        if (name.compare("turnover") == 0)
          return Field::TURNOVER;
        if (name.compare("turnover24h") == 0)
          return Field::TURNOVER_24H;
        break;
      }
      case 'p': {
        if (name.compare("typ") == 0)
          return Field::TYP;
        break;
      }
    }
  }
  return Field::UNKNOWN;
}

constexpr Field parse_u(auto& name) {
  if (name.length() >= 13) {
    switch (name.data()[12]) {
      case 'm': {
        if (name.compare("underlyingSymbol") == 0)
          return Field::UNDERLYING_SYMBOL;
        break;
      }
      case 'P': {
        if (name.compare("underlyingToPositionMultiplier") == 0)
          return Field::UNDERLYING_TO_POSITION_MULTIPLIER;
        break;
      }
      case 'S': {
        if (name.compare("underlyingToSettleMultiplier") == 0)
          return Field::UNDERLYING_TO_SETTLE_MULTIPLIER;
        break;
      }
    }
  }
  if (name.compare("underlying") == 0)
    return Field::UNDERLYING;
  return Field::UNKNOWN;
}

constexpr Field parse_v(auto& name) {
  if (name.length() >= 2) {
    switch (name.data()[1]) {
      case 'o': {
        if (name.compare("volume") == 0)
          return Field::VOLUME;
        if (name.compare("volume24h") == 0)
          return Field::VOLUME_24H;
        break;
      }
      case 'w': {
        if (name.compare("vwap") == 0)
          return Field::VWAP;
        break;
      }
    }
  }
  if (name.compare("underlying") == 0)
    return Field::UNDERLYING;
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  assert(name.empty() == false);
  switch (name.data()[0]) {
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
static_assert(parse_name("foreignNotional24h") == Field::FOREIGN_NOTIONAL_24H);
static_assert(parse_name("front") == Field::FRONT);
static_assert(parse_name("fundingBaseSymbol") == Field::FUNDING_BASE_SYMBOL);
static_assert(parse_name("fundingInterval") == Field::FUNDING_INTERVAL);
static_assert(parse_name("fundingPremiumSymbol") == Field::FUNDING_PREMIUM_SYMBOL);
static_assert(parse_name("fundingQuoteSymbol") == Field::FUNDING_QUOTE_SYMBOL);
static_assert(parse_name("fundingRate") == Field::FUNDING_RATE);
static_assert(parse_name("fundingTimestamp") == Field::FUNDING_TIMESTAMP);

static_assert(parse_name("hasLiquidity") == Field::HAS_LIQUIDITY);
static_assert(parse_name("highPrice") == Field::HIGH_PRICE);
static_assert(parse_name("homeNotional24h") == Field::HOME_NOTIONAL_24H);

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

static_assert(parse_name("openingTimestamp") == Field::OPENING_TIMESTAMP);
static_assert(parse_name("openInterest") == Field::OPEN_INTEREST);
static_assert(parse_name("openValue") == Field::OPEN_VALUE);
static_assert(parse_name("optionMultiplier") == Field::OPTION_MULTIPLIER);
static_assert(parse_name("optionStrikePcnt") == Field::OPTION_STRIKE_PCNT);
static_assert(parse_name("optionStrikePrice") == Field::OPTION_STRIKE_PRICE);
static_assert(parse_name("optionStrikeRound") == Field::OPTION_STRIKE_ROUND);
static_assert(parse_name("optionUnderlyingPrice") == Field::OPTION_UNDERLYING_PRICE);

static_assert(parse_name("positionCurrency") == Field::POSITION_CURRENCY);
static_assert(parse_name("prevClosePrice") == Field::PREV_CLOSE_PRICE);
static_assert(parse_name("prevPrice24h") == Field::PREV_PRICE_24H);
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
static_assert(parse_name("turnover24h") == Field::TURNOVER_24H);
static_assert(parse_name("typ") == Field::TYP);

static_assert(parse_name("underlying") == Field::UNDERLYING);
static_assert(parse_name("underlyingSymbol") == Field::UNDERLYING_SYMBOL);
static_assert(parse_name("underlyingToPositionMultiplier") == Field::UNDERLYING_TO_POSITION_MULTIPLIER);
static_assert(parse_name("underlyingToSettleMultiplier") == Field::UNDERLYING_TO_SETTLE_MULTIPLIER);

static_assert(parse_name("volume") == Field::VOLUME);
static_assert(parse_name("volume24h") == Field::VOLUME_24H);
static_assert(parse_name("vwap") == Field::VWAP);

inline void update_field(
    auto& result,
    auto& field,
    auto& key,
    auto& value) {
  switch (field) {
    case Field::UNKNOWN: {
      DLOG(FATAL)("Unknown key=\"{}\"", key);
      break;
    }
    case Field::ASK_PRICE: {
      update(result.ask_price, value);
      break;
    }
    case Field::BANKRUPT_LIMIT_DOWN_PRICE: {
      update(result.bankrupt_limit_down_price, value);
      break;
    }
    case Field::BANKRUPT_LIMIT_UP_PRICE: {
      update(result.bankrupt_limit_up_price, value);
      break;
    }
    case Field::BID_PRICE: {
      update(result.bid_price, value);
      break;
    }
    case Field::BUY_LEG: {
      update(result.buy_leg, value);
      break;
    }
    case Field::CALC_INTERVAL: {
      update(result.calc_interval, value);
      break;
    }
    case Field::CAPPED: {
      update(result.capped, value);
      break;
    }
    case Field::CLOSING_TIMESTAMP: {
      update(result.closing_timestamp, value);
      break;
    }
    case Field::DELEVERAGE: {
      update(result.deleverage, value);
      break;
    }
    case Field::EXPIRY: {
      update(result.expiry, value);
      break;
    }
    case Field::FAIR_BASIS: {
      update(result.fair_basis, value);
      break;
    }
    case Field::FAIR_BASIS_RATE: {
      update(result.fair_basis_rate, value);
      break;
    }
    case Field::FAIR_METHOD: {
      update(result.fair_method, value);
      break;
    }
    case Field::FAIR_PRICE: {
      update(result.fair_price, value);
      break;
    }
    case Field::FOREIGN_NOTIONAL_24H: {
      update(result.foreign_notional_24h, value);
      break;
    }
    case Field::FRONT: {
      update(result.front, value);
      break;
    }
    case Field::FUNDING_BASE_SYMBOL: {
      update(result.funding_base_symbol, value);
      break;
    }
    case Field::FUNDING_INTERVAL: {
      update(result.funding_interval, value);
      break;
    }
    case Field::FUNDING_PREMIUM_SYMBOL: {
      update(result.funding_premium_symbol, value);
      break;
    }
    case Field::FUNDING_QUOTE_SYMBOL: {
      update(result.funding_quote_symbol, value);
      break;
    }
    case Field::FUNDING_RATE: {
      update(result.funding_rate, value);
      break;
    }
    case Field::FUNDING_TIMESTAMP: {
      update(result.funding_timestamp, value);
      break;
    }
    case Field::HAS_LIQUIDITY: {
      update(result.has_liquidity, value);
      break;
    }
    case Field::HIGH_PRICE: {
      update(result.high_price, value);
      break;
    }
    case Field::HOME_NOTIONAL_24H: {
      update(result.home_notional_24h, value);
      break;
    }
    case Field::IMPACT_ASK_PRICE: {
      update(result.impact_ask_price, value);
      break;
    }
    case Field::IMPACT_BID_PRICE: {
      update(result.impact_bid_price, value);
      break;
    }
    case Field::IMPACT_MID_PRICE: {
      update(result.impact_mid_price, value);
      break;
    }
    case Field::INDICATIVE_FUNDING_RATE: {
      update(result.indicative_funding_rate, value);
      break;
    }
    case Field::INDICATIVE_SETTLE_PRICE: {
      update(result.indicative_settle_price, value);
      break;
    }
    case Field::INDICATIVE_TAX_RATE: {
      update(result.indicative_tax_rate, value);
      break;
    }
    case Field::INIT_MARGIN: {
      update(result.init_margin, value);
      break;
    }
    case Field::INSURANCE_FEE: {
      update(result.insurance_fee, value);
      break;
    }
    case Field::INVERSE_LEG: {
      update(result.inverse_leg, value);
      break;
    }
    case Field::IS_INVERSE: {
      update(result.is_inverse, value);
      break;
    }
    case Field::IS_QUANTO: {
      update(result.is_quanto, value);
      break;
    }
    case Field::LAST_CHANGE_PCNT: {
      update(result.last_change_pcnt, value);
      break;
    }
    case Field::LAST_PRICE: {
      update(result.last_price, value);
      break;
    }
    case Field::LAST_PRICE_PROTECTED: {
      update(result.last_price_protected, value);
      break;
    }
    case Field::LAST_TICK_DIRECTION: {
      update(result.last_tick_direction, value);
      break;
    }
    case Field::LIMIT: {
      update(result.limit, value);
      break;
    }
    case Field::LIMIT_DOWN_PRICE: {
      update(result.limit_down_price, value);
      break;
    }
    case Field::LIMIT_UP_PRICE: {
      update(result.limit_up_price, value);
      break;
    }
    case Field::LISTING: {
      update(result.listing, value);
      break;
    }
    case Field::LOT_SIZE: {
      update(result.lot_size, value);
      break;
    }
    case Field::LOW_PRICE: {
      update(result.low_price, value);
      break;
    }
    case Field::MAINT_MARGIN: {
      update(result.maint_margin, value);
      break;
    }
    case Field::MAKER_FEE: {
      update(result.maker_fee, value);
      break;
    }
    case Field::MARK_METHOD: {
      update(result.mark_method, value);
      break;
    }
    case Field::MARK_PRICE: {
      update(result.mark_price, value);
      break;
    }
    case Field::MAX_ORDER_QTY: {
      update(result.max_order_qty, value);
      break;
    }
    case Field::MAX_PRICE: {
      update(result.max_price, value);
      break;
    }
    case Field::MID_PRICE: {
      update(result.mid_price, value);
      break;
    }
    case Field::MULTIPLIER: {
      update(result.multiplier, value);
      break;
    }
    case Field::OPENING_TIMESTAMP: {
      update(result.opening_timestamp, value);
      break;
    }
    case Field::OPEN_INTEREST: {
      update(result.open_interest, value);
      break;
    }
    case Field::OPEN_VALUE: {
      update(result.open_value, value);
      break;
    }
    case Field::OPTION_MULTIPLIER: {
      update(result.option_multiplier, value);
      break;
    }
    case Field::OPTION_STRIKE_PCNT: {
      update(result.option_strike_pcnt, value);
      break;
    }
    case Field::OPTION_STRIKE_PRICE: {
      update(result.option_strike_price, value);
      break;
    }
    case Field::OPTION_STRIKE_ROUND: {
      update(result.option_strike_round, value);
      break;
    }
    case Field::OPTION_UNDERLYING_PRICE: {
      update(result.option_underlying_price, value);
      break;
    }
    case Field::POSITION_CURRENCY: {
      update(result.position_currency, value);
      break;
    }
    case Field::PREV_CLOSE_PRICE: {
      update(result.prev_close_price, value);
      break;
    }
    case Field::PREV_PRICE_24H: {
      update(result.prev_price_24h, value);
      break;
    }
    case Field::PREV_TOTAL_TURNOVER: {
      update(result.prev_total_turnover, value);
      break;
    }
    case Field::PREV_TOTAL_VOLUME: {
      update(result.prev_total_volume, value);
      break;
    }
    case Field::PUBLISH_INTERVAL: {
      update(result.publish_interval, value);
      break;
    }
    case Field::PUBLISH_TIME: {
      update(result.publish_time, value);
      break;
    }
    case Field::QUOTE_CURRENCY: {
      update(result.quote_currency, value);
      break;
    }
    case Field::QUOTE_TO_SETTLE_MULTIPLIER: {
      update(result.quote_to_settle_multiplier, value);
      break;
    }
    case Field::REBALANCE_INTERVAL: {
      update(result.rebalance_interval, value);
      break;
    }
    case Field::REBALANCE_TIMESTAMP: {
      update(result.rebalance_timestamp, value);
      break;
    }
    case Field::REFERENCE: {
      update(result.reference, value);
      break;
    }
    case Field::REFERENCE_SYMBOL: {
      update(result.reference_symbol, value);
      break;
    }
    case Field::RELIST_INTERVAL: {
      update(result.relist_interval, value);
      break;
    }
    case Field::RISK_LIMIT: {
      update(result.risk_limit, value);
      break;
    }
    case Field::RISK_STEP: {
      update(result.risk_step, value);
      break;
    }
    case Field::ROOT_SYMBOL: {
      update(result.root_symbol, value);
      break;
    }
    case Field::SELL_LEG: {
      update(result.sell_leg, value);
      break;
    }
    case Field::SESSION_INTERVAL: {
      update(result.session_interval, value);
      break;
    }
    case Field::SETTL_CURRENCY: {
      update(result.settl_currency, value);
      break;
    }
    case Field::SETTLE: {
      update(result.settle, value);
      break;
    }
    case Field::SETTLED_PRICE: {
      update(result.settled_price, value);
      break;
    }
    case Field::SETTLEMENT_FEE: {
      update(result.settlement_fee, value);
      break;
    }
    case Field::STATE: {
      update(result.state, value);
      break;
    }
    case Field::SYMBOL: {
      update(result.symbol, value);
      break;
    }
    case Field::TAKER_FEE: {
      update(result.taker_fee, value);
      break;
    }
    case Field::TAXED: {
      update(result.taxed, value);
      break;
    }
    case Field::TICK_SIZE: {
      update(result.tick_size, value);
      break;
    }
    case Field::TIMESTAMP: {
      update(result.timestamp, value);
      break;
    }
    case Field::TOTAL_TURNOVER: {
      update(result.total_turnover, value);
      break;
    }
    case Field::TOTAL_VOLUME: {
      update(result.total_volume, value);
      break;
    }
    case Field::TURNOVER: {
      update(result.turnover, value);
      break;
    }
    case Field::TURNOVER_24H: {
      update(result.turnover_24h, value);
      break;
    }
    case Field::TYP: {
      update(result.typ, value);
      break;
    }
    case Field::UNDERLYING: {
      update(result.underlying, value);
      break;
    }
    case Field::UNDERLYING_SYMBOL: {
      update(result.underlying_symbol, value);
      break;
    }
    case Field::UNDERLYING_TO_POSITION_MULTIPLIER: {
      update(result.underlying_to_position_multiplier, value);
      break;
    }
    case Field::UNDERLYING_TO_SETTLE_MULTIPLIER: {
      update(result.underlying_to_settle_multiplier, value);
      break;
    }
    case Field::VOLUME: {
      update(result.volume, value);
      break;
    }
    case Field::VOLUME_24H: {
      update(result.volume_24h, value);
      break;
    }
    case Field::VWAP: {
      update(result.vwap, value);
      break;
    }
  }
}
}  // namespace

InstrumentItem::InstrumentItem(core::json::value_t& value) {
  for (auto [key, value] : std::get<core::json::object_t>(value)) {
    auto field = parse_name(key);
    update_field(*this, field, key, value);
  }
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
