/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <limits>
#include <string>
#include <vector>

#include "roq/api.h"

#include "roq/bitmex/json/funding_item.h"
#include "roq/bitmex/json/instrument_item.h"

namespace roq {
namespace bitmex {

class Product final {
 public:
  explicit Product(const json::InstrumentItem &);
  explicit Product(const json::FundingItem &);

  Product(Product &&) = default;
  Product(const Product &) = delete;

  bool update(const json::InstrumentItem &);
  bool update(const json::FundingItem &);

  ReferenceData reference_data(const json::InstrumentItem &, uint16_t stream_id) const;
  MarketStatus market_status(const json::InstrumentItem &, uint16_t stream_id) const;
  StatisticsUpdate statistics_update(const json::InstrumentItem &, uint16_t stream_id) const;

  StatisticsUpdate statistics_update(const json::FundingItem &, uint16_t stream_id) const;

  bool is_market_status_dirty() const { return market_status_dirty_; }
  bool is_statistics_dirty() const { return !std::empty(statistics_); }

  void clear() {
    market_status_dirty_ = false;
    statistics_.clear();
  }

 private:
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
