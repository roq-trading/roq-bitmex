/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>
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

  server::Dispatcher &dispatcher;

  Settings const &settings;
  API const api;

  PriceCache price_cache;

  std::vector<Fill> fills;
  std::vector<MBPUpdate> bids, asks, final_bids, final_asks;
  std::vector<Trade> trades;

 private:
  uint32_t request_id_ = {};
  std::string request_id_encode_buffer_;
};

}  // namespace gateway
}  // namespace bitmex
}  // namespace roq
