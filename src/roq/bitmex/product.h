/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

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

 private:
  // reference data
  std::string _quote_currency;
  std::string _settl_currency;
  double _tick_size = std::numeric_limits<double>::quiet_NaN();
  double _limit_up_price = std::numeric_limits<double>::quiet_NaN();
  double _limit_down_price = std::numeric_limits<double>::quiet_NaN();
  double _multiplier = std::numeric_limits<double>::quiet_NaN();
  double _lot_size = std::numeric_limits<double>::quiet_NaN();
  double _option_strike_price = std::numeric_limits<double>::quiet_NaN();
  // market status
  json::State _state = json::State::UNDEFINED;
};

}  // namespace bitmex
}  // namespace roq
