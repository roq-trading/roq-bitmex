/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/product.h"

#include "roq/bitmex/options.h"

#include "roq/bitmex/json/utils.h"

namespace roq {
namespace bitmex {

Product::Product(const json::InstrumentItem &item)
    : quote_currency_(item.quote_currency),
      settl_currency_(item.settl_currency), tick_size_(item.tick_size),
      limit_up_price_(item.limit_up_price),
      limit_down_price_(item.limit_down_price), multiplier_(item.multiplier),
      lot_size_(item.lot_size), option_strike_price_(item.option_strike_price),
      state_(item.state) {
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
      .limit_up = limit_up_price_,
      .limit_down = limit_down_price_,
      .multiplier = multiplier_,
      .min_trade_vol = lot_size_,            // XXX correct?
      .option_type = OptionType::UNDEFINED,  // XXX typ?
      .strike_currency = std::string_view(),
      .strike_price = option_strike_price_,
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

}  // namespace bitmex
}  // namespace roq
