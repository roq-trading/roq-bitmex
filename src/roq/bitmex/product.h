/* Copyright (c) 2017-2020, Hans Erik Thrane */

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

  Product(Product &&) = delete;
  Product(const Product &) = delete;

  bool update(const json::InstrumentItem &);

  ReferenceData create_reference_data(const json::InstrumentItem &) const;
  MarketStatus create_market_status(const json::InstrumentItem &) const;
  StatisticsUpdate create_statistics_update(const json::InstrumentItem &) const;

 private:
  // reference data
  std::string quote_currency_;
  std::string settl_currency_;
  double tick_size_ = std::numeric_limits<double>::quiet_NaN();
  double multiplier_ = std::numeric_limits<double>::quiet_NaN();
  double lot_size_ = std::numeric_limits<double>::quiet_NaN();
  double option_strike_price_ = std::numeric_limits<double>::quiet_NaN();
  std::string underlying_symbol_;
  std::chrono::milliseconds expiry_ = {};
  std::chrono::milliseconds settle_ = {};
  // market status
  json::State state_ = json::State::UNDEFINED;
  // statistics update
  // FIXME because of StatisticsUpdate::span<Statistics>
  mutable std::array<Statistics, 2> statistics_;
};

}  // namespace bitmex
}  // namespace roq
