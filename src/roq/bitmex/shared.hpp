/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <utility>
#include <vector>

#include "roq/api.hpp"
#include "roq/server.hpp"

#include "roq/bitmex/price_cache.hpp"
#include "roq/bitmex/settings.hpp"

namespace roq {
namespace bitmex {

struct Shared final {
  Shared(server::Dispatcher &, Settings const &);

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
  std::vector<MBPUpdate> bids, asks;
  std::vector<Trade> trades;

  PriceCache price_cache;

 private:
  server::Dispatcher &dispatcher_;

 public:
  Settings const &settings;

 private:
  uint32_t request_id_ = 0;
  std::string request_id_encode_buffer_;
};

}  // namespace bitmex
}  // namespace roq
