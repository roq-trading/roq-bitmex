/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/bitmex/product.hpp"

#include "roq/logging.hpp"

#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/bitmex/flags.hpp"

#include "roq/bitmex/json/utils.hpp"

namespace roq {
namespace bitmex {

// === HELPERS ===

namespace {
auto strip_time_part(auto timestamp) {
  auto seconds = std::chrono::duration_cast<std::chrono::seconds>(timestamp);
  return std::chrono::seconds{seconds.count() % 86400};
}
}  // namespace

// === IMPLEMENTATION ===

// XXX markPrice ?
// XXX openInterest ?

Product::Product(json::InstrumentItem const &item)
    : quote_currency_(item.quote_currency), settl_currency_(item.settl_currency), tick_size_(item.tick_size),
      multiplier_(item.multiplier), lot_size_(item.lot_size), option_strike_price_(item.option_strike_price),
      underlying_symbol_(item.underlying_symbol), expiry_(item.expiry), settle_(item.settle) {
  statistics_.reserve(magic_enum::enum_count<StatisticsType>());
  update(item);
}

Product::Product(json::FundingItem const &) {
  statistics_.reserve(magic_enum::enum_count<StatisticsType>());
}

bool Product::update(json::InstrumentItem const &item) {
  // market status
  market_status_dirty_ |= item.state && utils::update(state_, item.state) != 0;
  // statistics update
  if (utils::update(settlement_price_, item.mark_price) != 0)
    statistics_.emplace_back(Statistics{
        .type = StatisticsType::SETTLEMENT_PRICE,
        .value = settlement_price_,
        .begin_time_utc = {},
        .end_time_utc = {},
    });
  if (utils::update(open_interest_, item.open_interest) != 0)
    statistics_.emplace_back(Statistics{
        .type = StatisticsType::OPEN_INTEREST,
        .value = open_interest_,
        .begin_time_utc = {},
        .end_time_utc = {},
    });
  if (utils::update(indicative_settle_price_, item.indicative_settle_price) != 0)
    statistics_.emplace_back(Statistics{
        .type = StatisticsType::PRE_SETTLEMENT_PRICE,
        .value = indicative_settle_price_,
        .begin_time_utc = {},
        .end_time_utc = {},
    });
  if (utils::update(limit_up_price_, item.limit_up_price) != 0)
    statistics_.emplace_back(Statistics{
        .type = StatisticsType::UPPER_LIMIT_PRICE,
        .value = limit_up_price_,
        .begin_time_utc = {},
        .end_time_utc = {},
    });
  if (utils::update(limit_down_price_, item.limit_down_price) != 0)
    statistics_.emplace_back(Statistics{
        .type = StatisticsType::LOWER_LIMIT_PRICE,
        .value = limit_down_price_,
        .begin_time_utc = {},
        .end_time_utc = {},
    });
  if (utils::update(fair_price_, item.fair_price) != 0)
    statistics_.emplace_back(Statistics{
        .type = StatisticsType::INDEX_VALUE,
        .value = fair_price_,
        .begin_time_utc = {},
        .end_time_utc = {},
    });
  // funding rate
  if (!std::isnan(item.funding_rate)) {
    using begin_time_t = decltype(Statistics::begin_time_utc);
    using end_time_t = decltype(Statistics::end_time_utc);
    auto begin_time_utc = std::chrono::duration_cast<begin_time_t>(item.timestamp);
    auto duration = strip_time_part(item.funding_interval);
    auto end_time_utc =
        begin_time_utc.count() ? std::chrono::duration_cast<end_time_t>(begin_time_utc + duration) : end_time_t{};
    auto update_funding_rate = false;
    update_funding_rate |= utils::update(funding_rate_.value, item.funding_rate) != 0;
    update_funding_rate |= utils::update(funding_rate_.begin_time_utc, begin_time_utc) != 0;
    update_funding_rate |= utils::update(funding_rate_.end_time_utc, end_time_utc) != 0;
    if (update_funding_rate) {
      statistics_.emplace_back(Statistics{
          .type = StatisticsType::FUNDING_RATE,
          .value = funding_rate_.value,
          .begin_time_utc = funding_rate_.begin_time_utc,
          .end_time_utc = funding_rate_.end_time_utc,
      });
    }
  }
  // indicative funding rate
  if (!std::isnan(item.indicative_funding_rate)) {
    if (utils::update(indicative_funding_rate_.value, item.indicative_funding_rate) != 0) {
      statistics_.emplace_back(Statistics{
          .type = StatisticsType::FUNDING_RATE_PREDICTION,
          .value = indicative_funding_rate_.value,
          .begin_time_utc = {},
          .end_time_utc = {},
      });
    }
  }
  return market_status_dirty_ || !std::empty(statistics_);
}

bool Product::update(json::FundingItem const &) {
  /*
  // statistics update
  using begin_time_t = decltype(Statistics::begin_time_utc);
  using end_time_t = decltype(Statistics::end_time_utc);
  auto begin_time_utc = std::chrono::duration_cast<begin_time_t>(item.timestamp);
  auto duration = strip_time_part(item.funding_interval);
  auto end_time_utc = begin_time_utc.count()
                          ? std::chrono::duration_cast<end_time_t>(begin_time_utc + duration)
                          : end_time_t{};
  auto update_funding_rate = false;
  update_funding_rate |= utils::update(funding_rate_.value, item.funding_rate) != 0;
  update_funding_rate |= utils::update(funding_rate_.begin_time_utc, begin_time_utc) != 0;
  update_funding_rate |= utils::update(funding_rate_.end_time_utc, end_time_utc) != 0;
  if (update_funding_rate) {
    statistics_.emplace_back(Statistics{
        .type = StatisticsType::FUNDING_RATE,
        .value = funding_rate_.value,
        .begin_time_utc = funding_rate_.begin_time_utc,
        .end_time_utc = funding_rate_.end_time_utc,
    });
  }
  auto update_funding_rate_daily = false;
  update_funding_rate_daily |=
      utils::update(funding_rate_daily_.value, item.funding_rate_daily) != 0;
  update_funding_rate_daily |=
      utils::update(funding_rate_daily_.begin_time_utc, begin_time_utc) != 0;
  update_funding_rate_daily |= utils::update(funding_rate_daily_.end_time_utc, end_time_utc) != 0;
  if (update_funding_rate_daily) {
    statistics_.emplace_back(Statistics{
        .type = StatisticsType::DAILY_FUNDING_RATE,
        .value = funding_rate_daily_.value,
        .begin_time_utc = funding_rate_daily_.begin_time_utc,
        .end_time_utc = funding_rate_daily_.end_time_utc,
    });
  }
  return !std::empty(statistics_);
  */
  return false;
}

ReferenceData Product::reference_data(json::InstrumentItem const &item, uint16_t stream_id, bool discard) const {
  assert(!std::empty(item.symbol));
  return ReferenceData{
      .stream_id = stream_id,
      .exchange = Flags::exchange(),
      .symbol = item.symbol,
      .description = {},
      .security_type = {},  // XXX typ?
      .base_currency = settl_currency_,
      .quote_currency = quote_currency_,
      .margin_currency = {},
      .commission_currency = {},
      .tick_size = tick_size_,
      .multiplier = multiplier_,
      .min_notional = NaN,
      .min_trade_vol = lot_size_,  // XXX correct?
      .max_trade_vol = NaN,
      .trade_vol_step_size = lot_size_,  // XXX correct?
      .option_type = {},                 // XXX typ?
      .strike_currency = {},
      .strike_price = option_strike_price_,
      .underlying = underlying_symbol_,
      .time_zone = {},
      .issue_date = {},
      .settlement_date = utils::safe_cast(settle_),
      .expiry_datetime = utils::safe_cast(expiry_),
      .expiry_datetime_utc = utils::safe_cast(expiry_),
      .discard = discard,
  };
}

MarketStatus Product::market_status(json::InstrumentItem const &item, uint16_t stream_id) const {
  assert(!std::empty(item.symbol));
  auto trading_status = json::map(state_);
  return MarketStatus{
      .stream_id = stream_id,
      .exchange = Flags::exchange(),
      .symbol = item.symbol,
      .trading_status = trading_status,
  };
}

StatisticsUpdate Product::statistics_update(json::InstrumentItem const &item, uint16_t stream_id) const {
  assert(!std::empty(item.symbol));
  return StatisticsUpdate{
      .stream_id = stream_id,
      .exchange = Flags::exchange(),
      .symbol = item.symbol,
      .statistics = {std::data(statistics_), std::size(statistics_)},
      .update_type = UpdateType::INCREMENTAL,
      .exchange_time_utc = {},
  };
}

StatisticsUpdate Product::statistics_update(json::FundingItem const &item, uint16_t stream_id) const {
  assert(!std::empty(item.symbol));
  return StatisticsUpdate{
      .stream_id = stream_id,
      .exchange = Flags::exchange(),
      .symbol = item.symbol,
      .statistics = {std::data(statistics_), std::size(statistics_)},
      .update_type = UpdateType::INCREMENTAL,
      .exchange_time_utc = {},
  };
}

}  // namespace bitmex
}  // namespace roq
