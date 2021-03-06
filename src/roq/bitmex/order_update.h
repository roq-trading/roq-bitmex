/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include "roq/bitmex/shared.h"

#include "roq/bitmex/json/order.h"
#include "roq/bitmex/json/order_item.h"

namespace roq {
namespace bitmex {

class OrderUpdate final {
 public:
  explicit OrderUpdate(Shared &shared) : shared_(shared) {}

  OrderUpdate(OrderUpdate &&) = delete;
  OrderUpdate(const OrderUpdate &) = delete;

  void operator()(const json::OrderItem &, const server::TraceInfo &);
  void operator()(const json::Order &, const server::TraceInfo &);

 private:
  Shared &shared_;
};

}  // namespace bitmex
}  // namespace roq
