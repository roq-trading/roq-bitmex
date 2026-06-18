/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <utility>
#include <vector>

#include "roq/api.hpp"
#include "roq/server.hpp"

#include "roq/bitmex/gateway/api.hpp"
#include "roq/bitmex/gateway/price_cache.hpp"
#include "roq/bitmex/gateway/settings.hpp"

namespace roq {
namespace bitmex {
namespace gateway {

struct Shared final {
  Shared(server::Dispatcher &, Settings const &);

  Shared(Shared const &) = delete;

  std::string_view next_request_id();

  auto discard_symbol(std::string_view const &name) const { return dispatcher.discard_symbol(name); }

  template <typename... Args>
  auto operator()(Args &&...args) {
    return dispatcher(std::forward<Args>(args)...);
  }

 public:
  std::vector<Fill> fills;
  std::vector<MBPUpdate> bids, asks;
  std::vector<Trade> trades;

  PriceCache price_cache;

  server::Dispatcher &dispatcher;

 public:
  Settings const &settings;
  API const api;

 private:
  uint32_t request_id_ = 0;
  std::string request_id_encode_buffer_;
};

}  // namespace gateway
}  // namespace bitmex
}  // namespace roq
