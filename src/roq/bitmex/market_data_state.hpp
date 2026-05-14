/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <cstdint>

namespace roq {
namespace bitmex {

enum class MarketDataState : uint8_t {
  UNDEFINED = 0,
  ACCOUNTS,
  INSTRUMENT,
  ORDER_BOOK_L2,
  DONE,
};

}  // namespace bitmex
}  // namespace roq
