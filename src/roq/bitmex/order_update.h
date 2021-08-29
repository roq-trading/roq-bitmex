/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/bitmex/shared.h"

#include "roq/bitmex/json/order.h"
#include "roq/bitmex/json/order_item.h"

namespace roq {
namespace bitmex {

class OrderUpdate final {
 public:
  explicit OrderUpdate(Shared &shared, uint16_t stream_id, const std::string_view &account)
      : shared_(shared), stream_id_(stream_id), account_(account) {}

  OrderUpdate(OrderUpdate &&) = delete;
  OrderUpdate(const OrderUpdate &) = delete;

  void operator()(const json::OrderItem &, const server::TraceInfo &, bool download);
  void operator()(const json::Order &, const server::TraceInfo &, bool download);

 private:
  Shared &shared_;
  const uint16_t stream_id_;
  const std::string_view account_;
};

}  // namespace bitmex
}  // namespace roq
