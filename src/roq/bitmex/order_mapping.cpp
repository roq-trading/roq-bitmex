/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/order_mapping.h"

#include <fmt/format.h>

#include <cassert>

#include "roq/logging.h"

namespace roq {
namespace bitmex {

namespace {
template <typename T>
inline void copy_to(const std::string_view& value, T& result) {
  result[value.copy(result, sizeof(result) - 1)] = '\0';
}
constexpr auto TOLERANCE = double{1.0e-10};
}  // namespace

OrderMapping::OrderMapping(
    const MessageInfo& message_info,
    const CreateOrder& create_order,
    uint32_t gateway_order_id)
    : _user_custom(
        message_info.source,
        create_order.order_id,
        gateway_order_id),
      _order_type(create_order.order_type),
      _side(create_order.side),
      _quantity(create_order.quantity) {
  copy_to(create_order.symbol, _symbol);
}


// ...

void OrderMapping::update_request(
    const std::string_view& request_id,
    RequestType request) {
  assert(_request_id[0] == '\0');
  assert(_request == RequestType::UNDEFINED);
  copy_to(request_id, _request_id);
  _request = request;
}

void OrderMapping::reset_request() {
  assert(_request_id[0] != '\0');
  assert(_request != RequestType::UNDEFINED);
  _request_id[0] = '\0';
  _request = RequestType::UNDEFINED;
}

}  // namespace bitmex
}  // namespace roq
