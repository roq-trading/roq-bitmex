/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/bitmex/price_cache.h"

#include "roq/logging.h"

#include "roq/utils/compare.h"

using namespace roq::literals;

namespace roq {
namespace bitmex {

std::pair<double, double> PriceCache::operator()(
    json::Action action, uint64_t id, double price, double size) {
  auto result = NaN;
  auto iter = price_lookup_.find(id);
  switch (action) {
    case json::Action::UNDEFINED:
    case json::Action::UNKNOWN:
      log::fatal("Unexpected"_sv);
      break;
    case json::Action::PARTIAL:
    case json::Action::INSERT:
      if (!std::isnan(price) && !std::isnan(size)) {
        if (iter == price_lookup_.end()) {
          iter = price_lookup_.emplace(id, price).first;
          result = (*iter).second;
        } else {
          auto previous = (*iter).second;
          if (utils::compare(price, previous) == 0) {
            result = (*iter).second;
          } else {
            // exists as a different price ==> fail
          }
        }
      } else {
        // unexpected price or size ==> fail
        log::fatal("action={} id={} price={} size={}"_fmt, action, id, price, size);
      }
      break;
    case json::Action::UPDATE:
      if (std::isnan(price) && !std::isnan(size) && utils::compare(size, 0.0) > 0) {
        if (iter != price_lookup_.end()) {
          result = (*iter).second;
        } else {
          // unable to find the cached price ==> fail
        }
      } else {
        // unexpected price or size ==> fail
      }
      break;
    case json::Action::DELETE:
      if (std::isnan(price) && (std::isnan(size) || utils::compare(size, 0.0) == 0)) {
        if (iter != price_lookup_.end()) {
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
