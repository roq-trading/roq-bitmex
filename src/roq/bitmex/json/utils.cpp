/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/bitmex/json/utils.h"

#include <algorithm>

#include "roq/logging.h"

#include "roq/utils/compare.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
static auto compare(const std::string_view &lhs, const std::string_view &rhs) {
  auto rhs_adj = rhs.substr(0, std::min(std::size(lhs), std::size(rhs)));
  return utils::case_insensitive_compare(lhs, rhs_adj);
}
}  // namespace

Error guess_error(const std::string_view &message) {
  if (message.empty())
    return Error::UNDEFINED;
  // POST /order, PUT /order, POST /order/closePosition
  if (compare(message, "Account is suspended"_sv) == 0)
    return Error::INVALID_ACCOUNT;
  if (compare(message, "Accounts do not match"_sv) == 0)
    return Error::INVALID_ACCOUNT;
  if (compare(message, "Duplicate clOrdID"_sv) == 0)
    return Error::INVALID_ORDER_ID;
  if (compare(message, "Duplicate orderID"_sv) == 0)
    return Error::INVALID_ORDER_ID;
  if (compare(message, "Instrument expired"_sv) == 0)
    return Error::INVALID_SYMBOL;
  if (compare(message, "Instrument not listed for trading yet"_sv) == 0)
    return Error::INVALID_SYMBOL;
  if (compare(message, "Instruments do not match"_sv) == 0)
    return Error::INVALID_SYMBOL;
  if (compare(message, "Invalid account"_sv) == 0)
    return Error::INVALID_ACCOUNT;
  if (compare(message, "Invalid avgPx"_sv) == 0)
    return Error::INVALID_PRICE;
  if (compare(message, "Invalid cumQty"_sv) == 0)
    return Error::INVALID_QUANTITY;
  if (compare(message, "Invalid displayQty for execInst"_sv) == 0)
    return Error::INVALID_EXECUTION_INSTRUCTION;
  if (compare(message, "Invalid displayQty"_sv) == 0)
    return Error::INVALID_QUANTITY;
  if (compare(message, "Invalid execIns"_sv) == 0)
    return Error::INVALID_EXECUTION_INSTRUCTION;
  if (compare(message, "Invalid leavesQty"_sv) == 0)
    return Error::INVALID_QUANTITY;
  if (compare(message, "Invalid ordStatus (trying to amend a canceled or filled order)"_sv) == 0)
    return Error::TOO_LATE_TO_MODIFY_OR_CANCEL;
  if (compare(message, "Invalid ordType for execInst"_sv) == 0)
    return Error::INVALID_EXECUTION_INSTRUCTION;
  if (compare(message, "Invalid ordType or timeInForce for execInst"_sv) == 0)
    return Error::INVALID_EXECUTION_INSTRUCTION;
  if (compare(message, "Invalid orderID"_sv) == 0)
    return Error::INVALID_ORDER_ID;
  if (compare(message, "Invalid orderQty or simpleOrderQty"_sv) == 0)
    return Error::INVALID_QUANTITY;
  if (compare(message, "Invalid orderQty"_sv) == 0)
    return Error::INVALID_QUANTITY;
  if (compare(message, "Invalid origClOrdID"_sv) == 0)
    return Error::TOO_LATE_TO_MODIFY_OR_CANCEL;
  if (compare(message, "Invalid price tickSize"_sv) == 0)
    return Error::INVALID_PRICE;
  if (compare(message, "Invalid price"_sv) == 0)
    return Error::INVALID_PRICE;
  if (compare(message, "Invalid side"_sv) == 0)
    return Error::INVALID_SIDE;
  if (compare(message, "Invalid simpleCumQty"_sv) == 0)
    return Error::INVALID_QUANTITY;
  if (compare(message, "Invalid simpleLeavesQty"_sv) == 0)
    return Error::INVALID_QUANTITY;
  if (compare(message, "Invalid simpleOrderQty"_sv) == 0)
    return Error::INVALID_QUANTITY;
  if (compare(message, "Invalid stopPx for ordType"_sv) == 0)
    return Error::INVALID_STOP_PRICE;
  if (compare(message, "Invalid stopPx tickSize"_sv) == 0)
    return Error::INVALID_STOP_PRICE;
  if (compare(message, "Invalid symbol"_sv) == 0)
    return Error::INVALID_SYMBOL;
  if (compare(message, "Unsupported execInst"_sv) == 0)
    return Error::INVALID_EXECUTION_INSTRUCTION;
  if (compare(message, "Unsupported ordType"_sv) == 0)
    return Error::INVALID_ORDER_TYPE;
  if (compare(message, "Unsupported timeInForce"_sv) == 0)
    return Error::INVALID_TIME_IN_FORCE;
  if (compare(message, "Account has insufficient Available Balance"_sv) == 0)
    return Error::INSUFFICIENT_FUNDS;
  if (compare(message, "Account has no"_sv) == 0)
    return Error::INSUFFICIENT_FUNDS;
  if (compare(message, "Account is in margin call"_sv) == 0)
    return Error::INSUFFICIENT_FUNDS;
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
  if (compare(message, "Order price is above the liquidation price of current short position"_sv) ==
      0)
    return Error::INVALID_PRICE;
  if (compare(message, "Order price is below the liquidation price of current long position"_sv) ==
      0)
    return Error::INVALID_PRICE;
  /*
  Position is in liquidation
  */
  if (compare(message, "Price greater than limitUpPrice"_sv) == 0)
    return Error::INVALID_PRICE;
  if (compare(message, "Price less than limitDownPrice"_sv) == 0)
    return Error::INVALID_PRICE;
  /*
  Underlying leg instrument has no mark price
  Underlying leg market is not open
  Unsupported contingencyType
  Unsupported pegPriceType
  Value of position and orders exceeds position Risk Limit
  Value of positions and orders exceeds account Risk Limit
  */

  // undoc (fixed length)
  if (compare(message, "Invalid leavesQty for lotSize"_sv) == 0)
    return Error::INVALID_QUANTITY;
  if (compare(message, "orderID or origClOrdID must be sent."_sv) == 0)
    return Error::INVALID_ORDER_ID;

  // undoc (variable length)
  // Rate-limit has been reached: usage=60, limit=6
  if (compare(message, "Rate-limit has been reached"_sv) == 0)
    return Error::REQUEST_RATE_LIMIT_REACHED;

  log::warn<1>(R"(*** PLEASE REPORT (error="{}") ***)"_sv, message);
  return Error::UNKNOWN;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
