/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include <utility>

#include "roq/bitmex/json/action.h"

namespace roq {
namespace bitmex {

class PriceCache final {
 public:
  PriceCache() = default;

  PriceCache(PriceCache &&) = default;
  PriceCache(const PriceCache &) = delete;

  std::pair<double, double> operator()(json::Action action, uint64_t id, double price, double size);

 private:
  absl::flat_hash_map<uint64_t, double> price_lookup_;
};

}  // namespace bitmex
}  // namespace roq
