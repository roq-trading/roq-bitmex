/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/bitmex/shared.hpp"

#include "roq/bitmex/json/order.hpp"
#include "roq/bitmex/json/order_item.hpp"

namespace roq {
namespace bitmex {

class OrderUpdate final {
 public:
  explicit OrderUpdate(Shared &shared, uint16_t stream_id, const std::string_view &account)
      : shared_(shared), stream_id_(stream_id), account_(account) {}

  OrderUpdate(OrderUpdate &&) = delete;
  OrderUpdate(const OrderUpdate &) = delete;

  // drop copy
  void operator()(const json::OrderItem &, const TraceInfo &, bool download);
  void operator()(const json::Order &, const TraceInfo &, bool download);

  // order entry
  void operator()(
      const json::OrderItem &,
      const TraceInfo &,
      RequestType,
      uint8_t user_id,
      uint32_t order_id,
      uint32_t version);
  void operator()(
      const json::Order &,
      const TraceInfo &,
      RequestType,
      uint8_t user_id,
      uint32_t order_id,
      uint32_t version);

 private:
  Shared &shared_;
  const uint16_t stream_id_;
  const std::string_view account_;
};

}  // namespace bitmex
}  // namespace roq
