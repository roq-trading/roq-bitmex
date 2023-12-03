/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <limits>
#include <string>
#include <vector>

#include "roq/api.hpp"

#include "roq/bitmex/shared.hpp"

#include "roq/bitmex/json/funding_item.hpp"
#include "roq/bitmex/json/instrument_item.hpp"

namespace roq {
namespace bitmex {

struct Product final {
  Product(Shared &, json::InstrumentItem const &);
  Product(Shared &, json::FundingItem const &);

  Product(Product &&) = default;
  Product(Product const &) = delete;

  bool update(json::InstrumentItem const &);
  bool update(json::FundingItem const &);

  ReferenceData reference_data(json::InstrumentItem const &, uint16_t stream_id, bool discard) const;
  MarketStatus market_status(json::InstrumentItem const &, uint16_t stream_id) const;
  StatisticsUpdate statistics_update(json::InstrumentItem const &, uint16_t stream_id) const;

  StatisticsUpdate statistics_update(json::FundingItem const &, uint16_t stream_id) const;

  bool is_market_status_dirty() const { return market_status_dirty_; }
  bool is_statistics_dirty() const { return !std::empty(statistics_); }

  void clear() {
    market_status_dirty_ = false;
    statistics_.clear();
  }

 private:
  Shared &shared_;
  // reference data
  std::string description_;
  std::string quote_currency_;
  std::string settl_currency_;
  double tick_size_ = NaN;
  double multiplier_ = NaN;
  double lot_size_ = NaN;
  double option_strike_price_ = NaN;
  std::string underlying_symbol_;
  std::chrono::milliseconds expiry_ = {};
  std::chrono::milliseconds settle_ = {};
  // market status
  bool market_status_dirty_ = false;
  json::State state_ = {};
  // statistics
  double settlement_price_ = NaN;
  double open_interest_ = NaN;
  double indicative_settle_price_ = NaN;
  double limit_up_price_ = NaN;
  double limit_down_price_ = NaN;
  double fair_price_ = NaN;
  struct statistics_t final {
    double value = NaN;
    std::chrono::seconds begin_time_utc = {};
    std::chrono::seconds end_time_utc = {};
  };
  statistics_t funding_rate_ = {};
  statistics_t indicative_funding_rate_ = {};
  mutable std::vector<Statistics> statistics_;  // XXX mutable due to span
};

}  // namespace bitmex
}  // namespace roq
