/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/bitmex/json/utils.hpp"

#include <algorithm>

#include "roq/logging.hpp"

#include "roq/utils/compare.hpp"

using namespace std::literals;

namespace roq {
namespace bitmex {
namespace json {

namespace {
auto compare(std::string_view const &lhs, std::string_view const &rhs) {
  auto rhs_adj = rhs.substr(0, std::min(std::size(lhs), std::size(rhs)));
  return utils::case_insensitive_compare(lhs, rhs_adj);
}
}  // namespace

Error guess_error(std::string_view const &message) {
  if (std::empty(message)) {
    return Error::UNDEFINED;
  }
  // POST /order, PUT /order, POST /order/closePosition
  if (compare(message, "Account is suspended"sv) == 0) {
    return Error::INVALID_ACCOUNT;
  }
  if (compare(message, "Accounts do not match"sv) == 0) {
    return Error::INVALID_ACCOUNT;
  }
  if (compare(message, "Duplicate clOrdID"sv) == 0) {
    return Error::INVALID_ORDER_ID;
  }
  if (compare(message, "Duplicate orderID"sv) == 0) {
    return Error::INVALID_ORDER_ID;
  }
  if (compare(message, "Instrument expired"sv) == 0) {
    return Error::INVALID_SYMBOL;
  }
  if (compare(message, "Instrument not listed for trading yet"sv) == 0) {
    return Error::INVALID_SYMBOL;
  }
  if (compare(message, "Instruments do not match"sv) == 0) {
    return Error::INVALID_SYMBOL;
  }
  if (compare(message, "Invalid account"sv) == 0) {
    return Error::INVALID_ACCOUNT;
  }
  if (compare(message, "Invalid avgPx"sv) == 0) {
    return Error::INVALID_PRICE;
  }
  if (compare(message, "Invalid cumQty"sv) == 0) {
    return Error::INVALID_QUANTITY;
  }
  if (compare(message, "Invalid displayQty for execInst"sv) == 0) {
    return Error::INVALID_EXECUTION_INSTRUCTION;
  }
  if (compare(message, "Invalid displayQty"sv) == 0) {
    return Error::INVALID_QUANTITY;
  }
  if (compare(message, "Invalid execIns"sv) == 0) {
    return Error::INVALID_EXECUTION_INSTRUCTION;
  }
  if (compare(message, "Invalid leavesQty"sv) == 0) {
    return Error::INVALID_QUANTITY;
  }
  if (compare(message, "Invalid ordStatus (trying to amend a canceled or filled order)"sv) == 0) {
    return Error::TOO_LATE_TO_MODIFY_OR_CANCEL;
  }
  if (compare(message, "Invalid ordType for execInst"sv) == 0) {
    return Error::INVALID_EXECUTION_INSTRUCTION;
  }
  if (compare(message, "Invalid ordType or timeInForce for execInst"sv) == 0) {
    return Error::INVALID_EXECUTION_INSTRUCTION;
  }
  if (compare(message, "Invalid orderID"sv) == 0) {
    return Error::INVALID_ORDER_ID;
  }
  if (compare(message, "Invalid orderQty or simpleOrderQty"sv) == 0) {
    return Error::INVALID_QUANTITY;
  }
  if (compare(message, "Invalid orderQty"sv) == 0) {
    return Error::INVALID_QUANTITY;
  }
  if (compare(message, "Invalid origClOrdID"sv) == 0) {
    return Error::TOO_LATE_TO_MODIFY_OR_CANCEL;
  }
  if (compare(message, "Invalid price tickSize"sv) == 0) {
    return Error::INVALID_PRICE;
  }
  if (compare(message, "Invalid price"sv) == 0) {
    return Error::INVALID_PRICE;
  }
  if (compare(message, "Invalid side"sv) == 0) {
    return Error::INVALID_SIDE;
  }
  if (compare(message, "Invalid simpleCumQty"sv) == 0) {
    return Error::INVALID_QUANTITY;
  }
  if (compare(message, "Invalid simpleLeavesQty"sv) == 0) {
    return Error::INVALID_QUANTITY;
  }
  if (compare(message, "Invalid simpleOrderQty"sv) == 0) {
    return Error::INVALID_QUANTITY;
  }
  if (compare(message, "Invalid stopPx for ordType"sv) == 0) {
    return Error::INVALID_STOP_PRICE;
  }
  if (compare(message, "Invalid stopPx tickSize"sv) == 0) {
    return Error::INVALID_STOP_PRICE;
  }
  if (compare(message, "Invalid symbol"sv) == 0) {
    return Error::INVALID_SYMBOL;
  }
  if (compare(message, "Unsupported execInst"sv) == 0) {
    return Error::INVALID_EXECUTION_INSTRUCTION;
  }
  if (compare(message, "Unsupported ordType"sv) == 0) {
    return Error::INVALID_ORDER_TYPE;
  }
  if (compare(message, "Unsupported timeInForce"sv) == 0) {
    return Error::INVALID_TIME_IN_FORCE;
  }
  if (compare(message, "Account has insufficient Available Balance"sv) == 0) {
    return Error::INSUFFICIENT_FUNDS;
  }
  if (compare(message, "Account has no"sv) == 0) {
    return Error::INSUFFICIENT_FUNDS;
  }
  if (compare(message, "Account is in margin call"sv) == 0) {
    return Error::INSUFFICIENT_FUNDS;
  }
  /*
  Executing at order price would lead to immediate liquidation
  Executing at order price would push account deeper into margin call
  Executing at order price would put account into margin call
  Instrument has no mark price
  Invalid clOrdLinkID for contingencyType
  Invalid currency
  Invalid multiLegReportingType
  Invalid ordStatus
  Invalid pegOffsetValue for pegPriceType
  Invalid pegOffsetValue tickSize
  Invalid pegPriceType for ordType
  Invalid settlCurrency
  Invalid triggered
  Invalid workingIndicator
  Market is not open
  Order had execInst of [Close|ReduceOnly] and side of [Buy|Sell] but current position is [100]
  */
  if (compare(message, "Order price is above the liquidation price of current short position"sv) == 0) {
    return Error::INVALID_PRICE;
  }
  if (compare(message, "Order price is below the liquidation price of current long position"sv) == 0) {
    return Error::INVALID_PRICE;
  }
  /*
  Position is in liquidation
  */
  if (compare(message, "Price greater than limitUpPrice"sv) == 0) {
    return Error::INVALID_PRICE;
  }
  if (compare(message, "Price less than limitDownPrice"sv) == 0) {
    return Error::INVALID_PRICE;
  }
  /*
  Underlying leg instrument has no mark price
  Underlying leg market is not open
  Unsupported contingencyType
  Unsupported pegPriceType
  Value of position and orders exceeds position Risk Limit
  Value of positions and orders exceeds account Risk Limit
  */

  // undoc (fixed length)
  if (compare(message, "Invalid leavesQty for lotSize"sv) == 0) {
    return Error::INVALID_QUANTITY;
  }
  if (compare(message, "orderID or origClOrdID must be sent."sv) == 0) {
    return Error::INVALID_ORDER_ID;
  }

  // undoc (variable length)
  // Rate-limit has been reached: usage=60, limit=6
  if (compare(message, "Rate-limit has been reached"sv) == 0) {
    return Error::REQUEST_RATE_LIMIT_REACHED;
  }
  if (compare(message, "Rate limit exceeded, retry in "sv) == 0) {
    return Error::REQUEST_RATE_LIMIT_REACHED;
  }

  log::warn<1>(R"(*** PLEASE REPORT (error="{}") ***)"sv, message);
  return Error::UNKNOWN;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
