/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include "roq/api.hpp"

#include "roq/bitmex/json/exec_type.hpp"
#include "roq/bitmex/json/ord_status.hpp"

namespace roq {
namespace bitmex {

inline RequestType compute_request_type(json::ExecType exec_type) {
  switch (exec_type) {
    using enum json::ExecType::type_t;
    case _UNDEFINED:
    case _UNKNOWN:
      break;
    case NEW:
      return RequestType::CREATE_ORDER;
    case REPLACED:
      return RequestType::MODIFY_ORDER;
    case CANCELED:
      return RequestType::CANCEL_ORDER;
    case REJECTED:
      return {};  // or unknown?
    case TRADE:
      break;
    case FUNDING:
      break;
  }
  return {};
}

inline RequestStatus compute_request_status(json::ExecType exec_type) {
  switch (exec_type) {
    using enum json::ExecType::type_t;
    case _UNDEFINED:
    case _UNKNOWN:
      break;
    case NEW:
    case REPLACED:
    case CANCELED:
      return RequestStatus::ACCEPTED;
    case REJECTED:
      return RequestStatus::REJECTED;
    case TRADE:
      break;
    case FUNDING:
      break;
  }
  return {};
}

inline OrderStatus compute_order_status(json::OrdStatus ord_status, bool working_status) {
  switch (ord_status) {
    using enum json::OrdStatus::type_t;
    case _UNDEFINED:
    case _UNKNOWN:
      // note! back-stop in case we didn't parse OrdStatus
      if (working_status) {
        return OrderStatus::WORKING;
      }
      break;
    case PENDING_NEW:
      return OrderStatus::SENT;
    case NEW:
    case TRIGGERED:
      if (!working_status) {
        return OrderStatus::ACCEPTED;
      } else {
        return OrderStatus::WORKING;
      }
    case DONE_FOR_DAY:
      return OrderStatus::SUSPENDED;
    case PARTIALLY_FILLED:
      return OrderStatus::WORKING;
    case STOPPED:
      return OrderStatus::STOPPED;
    case FILLED:
      return OrderStatus::COMPLETED;
    case EXPIRED:
      return OrderStatus::EXPIRED;
    case CANCELED:
      return OrderStatus::CANCELED;
    case REJECTED:
      return OrderStatus::REJECTED;
    case PENDING_CANCEL:  // XXX HANS how to deal with?
      break;
    case UNTRIGGERED:  // XXX HANS have no idea what this means...
      break;
  }
  return {};
}

inline RequestStatus compute_request_status(json::OrdStatus ord_status) {
  switch (ord_status) {
    using enum json::OrdStatus::type_t;
    case _UNDEFINED:
    case _UNKNOWN:
      break;
    case PENDING_NEW:
      return RequestStatus::ACCEPTED;
    case NEW:
      return RequestStatus::ACCEPTED;
    case TRIGGERED:
      return RequestStatus::ACCEPTED;
    case DONE_FOR_DAY:
      return RequestStatus::ACCEPTED;
    case PARTIALLY_FILLED:
      return RequestStatus::ACCEPTED;
    case STOPPED:
      return RequestStatus::ACCEPTED;
    case FILLED:
      return RequestStatus::ACCEPTED;
    case EXPIRED:
      return RequestStatus::ACCEPTED;
    case CANCELED:
      return RequestStatus::ACCEPTED;
    case REJECTED:
      return RequestStatus::REJECTED;  // note!
    case PENDING_CANCEL:
      return RequestStatus::ACCEPTED;
    case UNTRIGGERED:  // XXX HANS have no idea what this means...
      return RequestStatus::ACCEPTED;
  }
  return {};
}

}  // namespace bitmex
}  // namespace roq
