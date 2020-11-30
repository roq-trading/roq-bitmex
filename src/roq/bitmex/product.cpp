/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/product.h"

#include "roq/bitmex/options.h"

#include "roq/bitmex/json/utils.h"

namespace roq {
namespace bitmex {

// XXX markPrice ?
// XXX openInterest ?

Product::Product(const json::InstrumentItem &item)
    : quote_currency_(item.quote_currency),
      settl_currency_(item.settl_currency), tick_size_(item.tick_size),
      multiplier_(item.multiplier), lot_size_(item.lot_size),
      option_strike_price_(item.option_strike_price),
      underlying_symbol_(item.underlying_symbol), expiry_(item.expiry),
      settle_(item.settle),
      state_(item.state), statistics_{
                              Statistics{StatisticsType::UPPER_LIMIT_PRICE,
                                         item.limit_up_price},

                              Statistics{StatisticsType::LOWER_LIMIT_PRICE,
                                         item.limit_down_price}} {
}

bool Product::update(const json::InstrumentItem &item) {
  bool updated = false;
  if (item.state != json::State::UNDEFINED && item.state != state_) {
    state_ = item.state;
    updated = true;
  }
  return updated;
}

ReferenceData Product::create_reference_data(
    const json::InstrumentItem &item) const {
  assert(item.symbol.empty() == false);
  return ReferenceData{
      .exchange = FLAGS_exchange,
      .symbol = item.symbol,
      .security_type = SecurityType::UNDEFINED,  // XXX typ?
      .currency = quote_currency_,               // XXX or position_currency?
      .settlement_currency = settl_currency_,
      .commission_currency = std::string_view(),
      .tick_size = tick_size_,
      .multiplier = multiplier_,
      .min_trade_vol = lot_size_,            // XXX correct?
      .option_type = OptionType::UNDEFINED,  // XXX typ?
      .strike_currency = std::string_view(),
      .strike_price = option_strike_price_,
      .underlying = underlying_symbol_,
      .issue_date_utc = {},
      .expiry_time_utc = expiry_,
      .settlement_date_utc = settle_,
  };
}

MarketStatus Product::create_market_status(
    const json::InstrumentItem &item) const {
  assert(item.symbol.empty() == false);
  return MarketStatus{
      .exchange = FLAGS_exchange,
      .symbol = item.symbol,
      .trading_status = json::map(state_),
  };
}

StatisticsUpdate Product::create_statistics_update(
    const json::InstrumentItem &item) const {
  assert(item.symbol.empty() == false);
  return StatisticsUpdate{
      .exchange = FLAGS_exchange,
      .symbol = item.symbol,
      .statistics = {statistics_.data(), statistics_.size()},
      .snapshot = false,
      .exchange_time_utc = {},
  };
}

}  // namespace bitmex
}  // namespace roq
