/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <tuple>

#include "roq/api.hpp"

#include "roq/bitmex/json/liquidity_ind.hpp"
#include "roq/bitmex/json/ord_status.hpp"
#include "roq/bitmex/json/ord_type.hpp"
#include "roq/bitmex/json/side.hpp"
#include "roq/bitmex/json/state.hpp"
#include "roq/bitmex/json/time_in_force.hpp"

namespace roq {
namespace bitmex {
namespace json {

template <typename... Args>
struct Map final {
  explicit Map(Args &&...args) : args_{std::forward<Args>(args)...} {}
  explicit Map(Args const &...args) : args_{args...} {}

  Map(Map const &) = delete;

  template <typename R>
  operator R();

 private:
  std::tuple<Args...> const args_;
};

template <typename R, typename... Args>
inline R map(Args &&...args) {
  return static_cast<R>(Map{std::forward<Args>(args)...});
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
