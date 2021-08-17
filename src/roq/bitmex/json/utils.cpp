/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/bitmex/json/utils.h"

#include <algorithm>

#include "roq/utils/compare.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
static auto compare(const std::string_view &lhs, const std::string_view &rhs) {
  auto lhs_adj = rhs.substr(0, std::min(std::size(lhs), std::size(rhs)));
  return utils::case_insensitive_compare(lhs_adj, rhs);
}
}  // namespace

Error guess_error(const std::string_view &message) {
  if (message.empty())
    return Error::UNDEFINED;
  // POST /order, PUT /order, POST /order/closePosition
  if (compare(message, "Duplicate clOrdID"_sv) == 0)
    return Error::UNKNOWN;
  /*
Duplicate clOrdID
Invalid orderID
Duplicate orderID
Invalid symbol
Instruments do not match
Instrument not listed for trading yet
Instrument expired
Instrument has no mark price
Accounts do not match
Invalid account
Account is suspended
Account has no [XBt]
Invalid ordStatus (trying to amend a canceled or filled order)
Invalid triggered
Invalid workingIndicator
Invalid side
Invalid orderQty or simpleOrderQty
Invalid simpleOrderQty
Invalid orderQty
Invalid simpleLeavesQty
Invalid simpleCumQty
Invalid leavesQty
Invalid cumQty
Invalid avgPx
Invalid price
Invalid price tickSize
Invalid displayQty
Unsupported ordType
Unsupported pegPriceType
Invalid pegPriceType for ordType
Invalid pegOffsetValue for pegPriceType
Invalid pegOffsetValue tickSize
Invalid stopPx for ordType
Invalid stopPx tickSize
Unsupported timeInForce
Unsupported execInst
Invalid execInst
Invalid ordType or timeInForce for execInst
Invalid displayQty for execInst
Invalid ordType for execInst
Unsupported contingencyType
Invalid clOrdLinkID for contingencyType
Invalid multiLegReportingType
Invalid currency
Invalid settlCurrency
  */
  // Reject reasons for orders created by above
  if (compare(message, "Invalid origClOrdID"_sv) == 0)
    return Error::TOO_LATE_TO_MODIFY_OR_CANCEL;
  /*
Invalid ordStatus
Market is not open
Underlying leg market is not open
Underlying leg instrument has no mark price
Invalid price
Invalid price tickSize
Price greater than limitUpPrice
Price less than limitDownPrice
Invalid side
Invalid orderQty or simpleOrderQty
Invalid simpleOrderQty
Invalid orderQty
Invalid leavesQty
Invalid simpleLeavesQty
Invalid pegOffsetValue for pegPriceType
Invalid pegOffsetValue tickSize
Invalid stopPx for ordType
Invalid stopPx tickSize
Account has insufficient Available Balance, [100000 XBt] required
Value of positions and orders exceeds account Risk Limit
Value of position and orders exceeds position Risk Limit
Position is in liquidation
Order price is below the liquidation price of current long position
Order price is above the liquidation price of current short position
Executing at order price would lead to immediate liquidation
Executing at order price would push account deeper into margin call
Account is in margin call
Executing at order price would put account into margin call
Order had execInst of [Close|ReduceOnly] and side of [Buy|Sell] but current position is [100]
  */
  return Error::UNKNOWN;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
