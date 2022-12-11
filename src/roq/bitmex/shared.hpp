/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <utility>
#include <vector>

#include "roq/api.hpp"
#include "roq/server.hpp"

#include "roq/core/stack/buffer.hpp"

#include "roq/bitmex/price_cache.hpp"

namespace roq {
namespace bitmex {

struct Shared final {
  explicit Shared(server::Dispatcher &);

  Shared(Shared &&) = default;
  Shared(Shared const &) = delete;

  std::string_view next_request_id();

  auto discard_symbol(std::string_view const &name) const { return dispatcher_.discard_symbol(name); }

  template <typename... Args>
  auto update_order(Args &&...args) {
    return dispatcher_.update_order(std::forward<Args>(args)...);
  }

  template <typename... Args>
  auto operator()(Args &&...args) {
    return dispatcher_(std::forward<Args>(args)...);
  }

 public:
  std::vector<Fill> fills;
  std::vector<MBPUpdate> bids, asks, final_bids, final_asks;
  std::vector<Trade> trades;

  PriceCache price_cache;

 private:
  server::Dispatcher &dispatcher_;
  uint32_t request_id_ = 0;
  core::stack::Buffer<char, 32> stack_buffer_;
};

}  // namespace bitmex
}  // namespace roq
