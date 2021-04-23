/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <array>
#include <limits>
#include <string>

#include "roq/api.h"

#include "roq/bitmex/json/instrument_item.h"

namespace roq {
namespace bitmex {

class Product final {
 public:
  explicit Product(const json::InstrumentItem &);

  Product(Product &&) = default;
  Product(const Product &) = delete;

  bool update(const json::InstrumentItem &);

  ReferenceData reference_data(const json::InstrumentItem &, uint16_t stream_id) const;
  MarketStatus market_status(const json::InstrumentItem &, uint16_t stream_id) const;
  StatisticsUpdate statistics_update(const json::InstrumentItem &, uint16_t stream_id) const;

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
  json::State state_ = {};
  // statistics update
  // FIXME because of StatisticsUpdate::span<Statistics>
  mutable std::array<Statistics, 2> statistics_;
};

}  // namespace bitmex
}  // namespace roq
