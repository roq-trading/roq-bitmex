/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/bitmex/gateway/api.hpp"

#include "roq/exceptions.hpp"

using namespace std::literals;

namespace roq {
namespace bitmex {
namespace gateway {

// === IMPLEMENTATION ===

API API::create(Settings const &) {
  return {
      .market_data{
          .exchange_info = "/fapi/v1/exchangeInfo"sv,
          .depth = "/fapi/v1/depth"sv,
      },
      .order_management{
          .order = "/api/v1/order"sv,
          .order_all = "/api/v1/order/all"sv,
      },
      .ws{
          .realtime = "/realtime"sv,
      },
  };
}

}  // namespace gateway
}  // namespace bitmex
}  // namespace roq
