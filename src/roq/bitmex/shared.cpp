/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/bitmex/shared.h"

#include <magic_enum.hpp>

#include "roq/bitmex/flags.h"

namespace roq {
namespace bitmex {

Shared::Shared(server::Dispatcher &dispatcher)
    : fills(server::Flags::cache_fills_max_depth()), bids(server::Flags::cache_mbp_max_depth()),
      asks(server::Flags::cache_mbp_max_depth()), trades(server::Flags::cache_trades_max_depth()),
      dispatcher_(dispatcher) {
}

}  // namespace bitmex
}  // namespace roq
