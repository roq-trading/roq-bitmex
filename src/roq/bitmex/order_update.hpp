/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/bitmex/shared.hpp"

#include "roq/bitmex/json/order.hpp"

namespace roq {
namespace bitmex {

struct OrderUpdate final {
  explicit OrderUpdate(Shared &shared, uint16_t stream_id, std::string_view const &account) : shared_(shared), stream_id_(stream_id), account_(account) {}

  OrderUpdate(OrderUpdate const &) = delete;

  // drop copy
  void operator()(json::OrderDataItem const &, TraceInfo const &, bool download);
  void operator()(json::Order const &, TraceInfo const &, bool download);

  // order entry
  void operator()(json::OrderDataItem const &, TraceInfo const &, RequestType, uint8_t user_id, uint64_t order_id, uint32_t version);
  void operator()(json::Order const &, TraceInfo const &, RequestType, uint8_t user_id, uint64_t order_id, uint32_t version);

 private:
  Shared &shared_;
  uint16_t const stream_id_;
  std::string_view const account_;
};

}  // namespace bitmex
}  // namespace roq
