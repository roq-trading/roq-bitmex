/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <limits>

#include "roq/bitmex/json/product.h"
#include "roq/bitmex/json/ticker.h"

namespace roq {
namespace bitmex {

struct Product final {
  // reference data
  bool update_reference_data(const json::Product&);
  double tick_size = std::numeric_limits<double>::quiet_NaN();

  // market status
  bool update_market_status(const json::Product&);
  TradingStatus trading_status = TradingStatus::UNDEFINED;

  // daily statistics
  bool update_daily_statistics(const json::Ticker&);
  double open_price = std::numeric_limits<double>::quiet_NaN();

  // session statistics
  bool update_session_statistics(const json::Ticker&);
  double highest_traded_price = std::numeric_limits<double>::quiet_NaN();
  double lowest_traded_price = std::numeric_limits<double>::quiet_NaN();
};

}  // namespace bitmex
}  // namespace roq
