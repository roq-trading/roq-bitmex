/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/bitmex/product.h"

#include <magic_enum.hpp>

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
      expiry_(item.expiry), settle_(item.settle) {
  statistics_.reserve(magic_enum::enum_count<StatisticsType::type_t>());
  update(item);
}

bool Product::update(const json::InstrumentItem &item) {
  // market status
  market_status_dirty_ |= utils::update(state_, item.state) != 0;
  // statistics update
  if (utils::update(settlement_price_, item.mark_price) != 0)
    statistics_.emplace_back(
        Statistics{.type = StatisticsType::SETTLEMENT_PRICE, .value = settlement_price_});
  if (utils::update(open_interest_, item.open_interest) != 0)
    statistics_.emplace_back(
        Statistics{.type = StatisticsType::OPEN_INTEREST, .value = open_interest_});
  if (utils::update(indicative_settle_price_, item.indicative_settle_price) != 0)
    statistics_.emplace_back(Statistics{
        .type = StatisticsType::PRE_SETTLEMENT_PRICE, .value = indicative_settle_price_});
  if (utils::update(limit_up_price_, item.limit_up_price) != 0)
    statistics_.emplace_back(
        Statistics{.type = StatisticsType::UPPER_LIMIT_PRICE, .value = limit_up_price_});
  if (utils::update(limit_down_price_, item.limit_down_price) != 0)
    statistics_.emplace_back(
        Statistics{.type = StatisticsType::LOWER_LIMIT_PRICE, .value = limit_down_price_});
  if (utils::update(fair_price_, item.fair_price) != 0)
    statistics_.emplace_back(Statistics{.type = StatisticsType::INDEX_VALUE, .value = fair_price_});
  return market_status_dirty_ || !statistics_.empty();
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
      .statistics = {statistics_.data(), statistics_.size()},
      .snapshot = false,
      .exchange_time_utc = {},
  };
}

}  // namespace bitmex
}  // namespace roq
