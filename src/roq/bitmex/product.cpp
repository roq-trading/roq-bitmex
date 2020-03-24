/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/product.h"

#include "roq/bitmex/options.h"

#include "roq/bitmex/json/utils.h"

namespace roq {
namespace bitmex {

Product::Product(const json::InstrumentItem& item)
    : _quote_currency(item.quote_currency),
      _settl_currency(item.settl_currency),
      _tick_size(item.tick_size),
      _limit_up_price(item.limit_up_price),
      _limit_down_price(item.limit_down_price),
      _multiplier(item.multiplier),
      _lot_size(item.lot_size),
      _option_strike_price(item.option_strike_price),
      _state(item.state) {
}

bool Product::update(const json::InstrumentItem& item) {
  bool updated = false;
  if (item.state != json::State::UNDEFINED && item.state != _state) {
    _state = item.state;
    updated = true;
  }
  return updated;
}

ReferenceData Product::create_reference_data(
    const json::InstrumentItem& item) const {
  assert(item.symbol.empty() == false);
  return ReferenceData {
    .exchange = FLAGS_exchange,
    .symbol = item.symbol,
    .security_type = SecurityType::UNDEFINED,  // XXX typ?
    .currency = _quote_currency,  // XXX or position_currency?
    .settlement_currency = _settl_currency,
    .commission_currency = std::string_view(),
    .tick_size = _tick_size,
    .limit_up = _limit_up_price,
    .limit_down = _limit_down_price,
    .multiplier = _multiplier,
    .min_trade_vol = _lot_size,  // XXX correct?
    .option_type = OptionType::UNDEFINED,  // XXX typ?
    .strike_currency = std::string_view(),
    .strike_price = _option_strike_price,
  };
}

MarketStatus Product::create_market_status(
    const json::InstrumentItem& item) const {
  assert(item.symbol.empty() == false);
  return MarketStatus {
    .exchange = FLAGS_exchange,
    .symbol = item.symbol,
    .trading_status = json::map(_state),
  };
}

}  // namespace bitmex
}  // namespace roq
