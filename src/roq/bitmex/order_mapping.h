/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include "roq/api.h"

#include "roq/core/oms/user_custom.h"

namespace roq {
namespace bitmex {

struct OrderMapping final {
  OrderMapping(
      const MessageInfo& message_info,
      const CreateOrder& create_order,
      uint32_t gateway_order_id);

  inline auto user_id() const {
    return _user_custom.user_id();
  }
  inline auto user_order_id() const {
    return _user_custom.user_order_id();
  }
  inline auto gateway_order_id() const {
    return _user_custom.gateway_order_id();
  }

  inline auto key() const {
    return _user_custom.key();
  }

  inline auto order_type() const {
    return _order_type;
  }
  inline auto side() const {
    return _side;
  }

  inline std::string_view symbol() const {
    return _symbol;
  }

  inline auto quantity() const {
    return _quantity;
  }

  inline std::string_view exchange_order_id() const {
    return _exchange_order_id;
  }

  inline auto traded_quantity() const {
    return _traded_quantity;
  }

  inline auto remaining_quantity() const {
    return _quantity - _traded_quantity;
  }

  inline auto create_time() const {
    return _create_time;
  }
  inline auto update_time() const {
    return _update_time;
  }

  bool ready() const {
    return _create_time.count() > 0;
  }

  // static

  static inline auto key(uint8_t user_id, uint32_t user_order_id) {
    return core::oms::UserCustom::key(user_id, user_order_id);
  }

  // ...

  auto request() const {
    return _request;
  }

  std::string_view request_id() const {
    return _request_id;
  }

  void update_request(
      const std::string_view& request_id,
      RequestType request);

  void reset_request();

 public:
  core::oms::UserCustom _user_custom;
  OrderType _order_type;
  Side _side;
  char _symbol[32] = {};
  double _quantity;
  double _traded_quantity = 0.0;
  char _exchange_order_id[64] = {};
  std::chrono::nanoseconds _create_time = {};
  std::chrono::nanoseconds _update_time = {};
  // request
  char _request_id[40] = {};
  RequestType _request = RequestType::UNDEFINED;
};

}  // namespace bitmex
}  // namespace roq
