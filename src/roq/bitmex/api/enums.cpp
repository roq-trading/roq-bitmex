/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/api/enums.h"

#include "roq/api.h"
#include "roq/builtins.h"

namespace roq {
namespace bitmex {
namespace api {

template <>
api::Side to_enum(const std::string_view& value) {
  if (unlikely(value.empty()))
    return api::Side::UNKNOWN;
  switch (value.data()[0]) {
    case 'b': {
      if (value.compare("buy") == 0) {
        return api::Side::BUY;
      }
      break;
    }
    case 's': {
      if (value.compare("sell") == 0) {
        return api::Side::SELL;
      }
      break;
    }
  }
  return api::Side::UNKNOWN;
}

template <>
api::OrderType to_enum(const std::string_view& value) {
  if (unlikely(value.empty()))
    return api::OrderType::UNKNOWN;
  switch (value.data()[0]) {
    case 'l': {
      if (value.compare("limit") == 0) {
        return api::OrderType::LIMIT;
      }
      break;
    }
    case 'm': {
      if (value.compare("market") == 0) {
        return api::OrderType::MARKET;
      }
      break;
    }
  }
  return api::OrderType::UNKNOWN;
}

template <>
api::OrderStatus to_enum(const std::string_view& value) {
  if (unlikely(value.empty()))
    return api::OrderStatus::UNKNOWN;
  switch (value.data()[0]) {
    case 'o': {
      if (value.compare("open") == 0) {
        return api::OrderStatus::OPEN;
      }
      break;
    }
  }
  return api::OrderStatus::UNKNOWN;
}

template <>
api::TimeInForce to_enum(const std::string_view& value) {
  if (unlikely(value.empty()))
    return api::TimeInForce::UNKNOWN;
  switch (value.data()[0]) {
    case 'G': {
      if (value.compare("GTC") == 0) {
        return api::TimeInForce::GTC;
      }
      break;
    }
  }
  return api::TimeInForce::UNKNOWN;
}

template <>
api::Reason to_enum(const std::string_view& value) {
  if (unlikely(value.empty()))
    return api::Reason::UNKNOWN;
  switch (value.data()[0]) {
    case 'c': {
      if (value.compare("canceled") == 0) {
        return api::Reason::CANCELED;
      }
      break;
    }
    case 'f': {
      if (value.compare("filled") == 0) {
        return api::Reason::FILLED;
      }
      break;
    }
  }
  return api::Reason::UNKNOWN;
}

template <>
api::XStatus to_enum(const std::string_view& value) {
  if (unlikely(value.empty()))
    return api::XStatus::UNKNOWN;
  switch (value.data()[0]) {
    case 'o': {
      if (value.compare("online") == 0) {
        return api::XStatus::ONLINE;
      }
      break;
    }
  }
  return api::XStatus::UNKNOWN;
}

template <>
api::StopType to_enum(const std::string_view& value) {
  if (unlikely(value.empty()))
    return api::StopType::UNKNOWN;
  switch (value.data()[0]) {
    case 'e': {
      if (value.compare("entry") == 0) {
        return api::StopType::ENTRY;
      }
      break;
    }
  }
  return api::StopType::UNKNOWN;
}

}  // namespace api
}  // namespace bitmex
}  // namespace roq
