/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/bitmex/product.h"

#include "roq/utils/safe_cast.h"
#include "roq/utils/update.h"

#include "roq/bitmex/flags.h"

#include "roq/bitmex/json/utils.h"

namespace roq {
namespace bitmex {

// XXX markPrice ?
// XXX openInterest ?

Product::Product(const json::InstrumentItem &item)
    : quote_currency_(item.quote_currency), settl_currency_(item.settl_currency),
      tick_size_(item.tick_size), multiplier_(item.multiplier), lot_size_(item.lot_size),
      option_strike_price_(item.option_strike_price), underlying_symbol_(item.underlying_symbol),
      expiry_(item.expiry), settle_(item.settle),
      state_(item.state), statistics_{
                              Statistics{StatisticsType::UPPER_LIMIT_PRICE, item.limit_up_price},
                              Statistics{StatisticsType::LOWER_LIMIT_PRICE, item.limit_down_price},
                          } {
}

bool Product::update(const json::InstrumentItem &item) {
  return utils::update(state_, item.state) != 0;
}

ReferenceData Product::reference_data(const json::InstrumentItem &item, uint16_t stream_id) const {
  assert(!item.symbol.empty());
  return ReferenceData{
      .stream_id = stream_id,
      .exchange = Flags::exchange(),
      .symbol = item.symbol,
      .description = {},
      .security_type = {},          // XXX typ?
      .currency = quote_currency_,  // XXX or position_currency?
      .settlement_currency = settl_currency_,
      .commission_currency = {},
      .tick_size = tick_size_,
      .multiplier = multiplier_,
      .min_trade_vol = lot_size_,  // XXX correct?
      .option_type = {},           // XXX typ?
      .strike_currency = {},
      .strike_price = option_strike_price_,
      .underlying = underlying_symbol_,
      .time_zone = {},
      .issue_date = {},
      .settlement_date = utils::safe_cast(settle_),
      .expiry_datetime = utils::safe_cast(expiry_),
      .expiry_datetime_utc = utils::safe_cast(expiry_),
  };
}

MarketStatus Product::market_status(const json::InstrumentItem &item, uint16_t stream_id) const {
  assert(!item.symbol.empty());
  return MarketStatus{
      .stream_id = stream_id,
      .exchange = Flags::exchange(),
      .symbol = item.symbol,
      .trading_status = json::map(state_),
  };
}

StatisticsUpdate Product::statistics_update(
    const json::InstrumentItem &item, uint16_t stream_id) const {
  assert(!item.symbol.empty());
  return StatisticsUpdate{
      .stream_id = stream_id,
      .exchange = Flags::exchange(),
      .symbol = item.symbol,
      .statistics = statistics_,
      .snapshot = false,
      .exchange_time_utc = {},
  };
}

}  // namespace bitmex
}  // namespace roq
