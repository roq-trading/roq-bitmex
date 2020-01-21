/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <cstddef>

namespace roq {
namespace bitmex {
namespace api {

template <typename T>
T to_enum(const std::string_view&);

// --> Side

enum class Side {
  UNKNOWN,
  BUY,
  SELL,
};

inline const char * const *EnumNamesSide() {
  static const char * const names[] = {
    "UNKNOWN",
    "BUY",
    "SELL",
  };
  return names;
}

inline const char *EnumNameSide(const Side& e) {
  const size_t index = static_cast<int>(e);
  return EnumNamesSide()[index];
}

template <>
api::Side to_enum(const std::string_view& value);

// --> OrderType

enum class OrderType {
  UNKNOWN,
  MARKET,
  LIMIT,
};

inline const char * const *EnumNamesOrderType() {
  static const char * const names[] = {
    "UNKNOWN",
    "MARKET",
    "LIMIT",
  };
  return names;
}

inline const char *EnumNameOrderType(const OrderType& e) {
  const size_t index = static_cast<int>(e);
  return EnumNamesOrderType()[index];
}

template <>
api::OrderType to_enum(const std::string_view& value);

// --> OrderStatus

enum class OrderStatus {
  UNKNOWN,
  OPEN,
};

inline const char * const *EnumNamesOrderStatus() {
  static const char * const names[] = {
    "UNKNOWN",
    "OPEN",
  };
  return names;
}

inline const char *EnumNameOrderStatus(const OrderStatus& e) {
  const size_t index = static_cast<int>(e);
  return EnumNamesOrderStatus()[index];
}

template <>
api::OrderStatus to_enum(const std::string_view& value);

// --> TimeInForce

enum class TimeInForce {
  UNKNOWN,
  GTC,
};

inline const char * const *EnumNamesTimeInForce() {
  static const char * const names[] = {
    "UNKNOWN",
    "GTC",
  };
  return names;
}

inline const char *EnumNameTimeInForce(const TimeInForce& e) {
  const size_t index = static_cast<int>(e);
  return EnumNamesTimeInForce()[index];
}

template <>
api::TimeInForce to_enum(const std::string_view& value);

// --> Reason

enum class Reason {
  UNKNOWN,
  FILLED,
  CANCELED,
};

inline const char * const *EnumNamesReason() {
  static const char * const names[] = {
    "UNKNOWN",
    "FILLED",
    "CANCELED",
  };
  return names;
}

inline const char *EnumNameReason(const Reason& e) {
  const size_t index = static_cast<int>(e);
  return EnumNamesReason()[index];
}

template <>
api::Reason to_enum(const std::string_view& value);

// --> Status

enum class XStatus {
  UNKNOWN,
  ONLINE,
};

inline const char * const *EnumNamesStatus() {
  static const char * const names[] = {
    "UNKNOWN",
    "ONLINE",
  };
  return names;
}

inline const char *EnumNameStatus(const XStatus& e) {
  const size_t index = static_cast<int>(e);
  return EnumNamesStatus()[index];
}

template <>
api::XStatus to_enum(const std::string_view& value);

// --> StopType

enum class StopType {
  UNKNOWN,
  ENTRY,
};

inline const char * const *EnumNamesStopType() {
  static const char * const names[] = {
    "UNKNOWN",
    "ENTRY",
  };
  return names;
}

inline const char *EnumNameStopType(const StopType& e) {
  const size_t index = static_cast<int>(e);
  return EnumNamesStopType()[index];
}

template <>
api::StopType to_enum(const std::string_view& value);

}  // namespace api
}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::api::Side> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::api::Side& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{}",
        roq::bitmex::api::EnumNameSide(value));
  }
};

template <>
struct fmt::formatter<roq::bitmex::api::OrderType> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::api::OrderType& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{}",
        roq::bitmex::api::EnumNameOrderType(value));
  }
};

template <>
struct fmt::formatter<roq::bitmex::api::OrderStatus> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::api::OrderStatus& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{}",
        roq::bitmex::api::EnumNameOrderStatus(value));
  }
};

template <>
struct fmt::formatter<roq::bitmex::api::TimeInForce> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::api::TimeInForce& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{}",
        roq::bitmex::api::EnumNameTimeInForce(value));
  }
};

template <>
struct fmt::formatter<roq::bitmex::api::Reason> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::api::Reason& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{}",
        roq::bitmex::api::EnumNameReason(value));
  }
};

template <>
struct fmt::formatter<roq::bitmex::api::XStatus> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::api::XStatus& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{}",
        roq::bitmex::api::EnumNameStatus(value));
  }
};

template <>
struct fmt::formatter<roq::bitmex::api::StopType> {
  template <typename C>
  constexpr auto parse(C& ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::api::StopType& value, C& ctx) {
    return format_to(
        ctx.out(),
        "{}",
        roq::bitmex::api::EnumNameStopType(value));
  }
};
