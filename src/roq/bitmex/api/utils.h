/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include "roq/api.h"

#include "roq/bitmex/api/enums.h"

namespace roq {
namespace bitmex {
namespace api {

static inline TradingStatus map(const XStatus& status) {
  switch (status) {
    case XStatus::ONLINE:
      return TradingStatus::OPEN;
    default: {
      auto message = fmt::format(
          "Can't map status={} to TradingStatus", status);
      throw std::runtime_error(message);
    }
  }
}

static inline roq::Side map(const api::Side& side) {
  switch (side) {
    case api::Side::BUY:
      return roq::Side::BUY;
    case api::Side::SELL:
      return roq::Side::SELL;
    default: {
      auto message = fmt::format(
          "Can't map side={} to Side", side);
      throw std::runtime_error(message);
    }
  }
}

}  // namespace api
}  // namespace bitmex
}  // namespace roq
