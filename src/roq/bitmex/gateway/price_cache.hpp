/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <utility>

#include "roq/utils/container.hpp"

#include "roq/bitmex/json/action.hpp"

namespace roq {
namespace bitmex {
namespace gateway {

struct PriceCache final {
  PriceCache() = default;

  PriceCache(PriceCache const &) = delete;

  std::pair<double, double> operator()(json::Action action, uint64_t id, double price, double size);

 private:
  utils::unordered_map<uint64_t, double> price_lookup_;
};

}  // namespace gateway
}  // namespace bitmex
}  // namespace roq
