/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <utility>

#include "roq/api.h"
#include "roq/server.h"

#include "roq/core/memory.h"

#include "roq/bitmex/price_cache.h"

namespace roq {
namespace bitmex {

struct Shared final {
  explicit Shared(server::Dispatcher &);

  Shared(Shared &&) = default;
  Shared(const Shared &) = delete;

  auto next_request_id() { return dispatcher_.next_request_id(); }

  auto next_trade_id() { return dispatcher_.next_trade_id(); }

  auto discard_symbol(const std::string_view &name) const {
    return dispatcher_.discard_symbol(name);
  }

  template <typename... Args>
  auto find_order(Args &&...args) {
    return dispatcher_.find_order(std::forward<Args>(args)...);
  }

 public:
  core::page_aligned_vector<Fill> fills;
  core::page_aligned_vector<MBPUpdate> bids, asks;
  core::page_aligned_vector<Trade> trades;

  PriceCache price_cache;

 private:
  server::Dispatcher &dispatcher_;
};

}  // namespace bitmex
}  // namespace roq
