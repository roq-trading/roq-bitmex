/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/bitmex/price_cache.hpp"

#include "roq/logging.hpp"

#include "roq/utils/compare.hpp"

using namespace std::literals;

namespace roq {
namespace bitmex {

std::pair<double, double> PriceCache::operator()(
    json::Action action, uint64_t id, double price, double size) {
  auto result = NaN;
  auto iter = price_lookup_.find(id);
  switch (action) {
    using enum json::Action::type_t;
    case UNDEFINED:
    case UNKNOWN:
      log::fatal("Unexpected"sv);
      break;
    case PARTIAL:
    case INSERT:
      if (!std::isnan(price) && !std::isnan(size)) {
        if (iter == std::end(price_lookup_)) {
          iter = price_lookup_.emplace(id, price).first;
          result = (*iter).second;
        } else {
          auto previous = (*iter).second;
          if (utils::is_equal(price, previous)) {
            result = (*iter).second;
          } else {
            // exists as a different price ==> fail
          }
        }
      } else {
        // unexpected price or size ==> fail
        log::fatal("action={} id={} price={} size={}"sv, action, id, price, size);
      }
      break;
    case UPDATE:
      if (std::isnan(price) && !std::isnan(size) && utils::is_greater(size, 0.0)) {
        if (iter != std::end(price_lookup_)) {
          result = (*iter).second;
        } else {
          // unable to find the cached price ==> fail
        }
      } else {
        // unexpected price or size ==> fail
      }
      break;
    case DELETE:
      if (std::isnan(price) && (std::isnan(size) || utils::is_zero(size))) {
        if (iter != std::end(price_lookup_)) {
          result = (*iter).second;
          price_lookup_.erase(iter);
        } else {
          // unable to find the cached price ==> fail
        }
      } else {
        // unexpected price or size ==> fail
      }
  }
  return std::make_pair(result, std::isnan(size) ? 0.0 : size);
}

}  // namespace bitmex
}  // namespace roq
