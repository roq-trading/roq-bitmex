/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/product.h"

#include "roq/bitmex/api/utils.h"

namespace roq {
namespace bitmex {

constexpr auto TOLERANCE = double{1.0e-10};

static inline bool update(double& lhs, double rhs) {
  if (std::isnan(rhs) || std::fabs(lhs - rhs) < TOLERANCE)
    return false;
  lhs = rhs;
  return true;
}


bool Product::update_reference_data(
    const json::Product& product) {
  return update(tick_size, product.quote_increment);
}

bool Product::update_market_status(
    const json::Product& product) {
  auto updated = false;
  auto trading_status_tmp = api::map(product.status);
  if (trading_status != trading_status_tmp) {
    trading_status = trading_status_tmp;
    updated = true;
  }
  return updated;
}

bool Product::update_daily_statistics(
    const json::Ticker& ticker) {
  return update(open_price, ticker.open_24h);
}

bool Product::update_session_statistics(
    const json::Ticker& ticker) {
  return update(highest_traded_price, ticker.high_24h) ||
    update(lowest_traded_price, ticker.low_24h);
}

}  // namespace bitmex
}  // namespace roq
