/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include "roq/api.h"

#include "roq/bitmex/json/exec_type.h"
#include "roq/bitmex/json/ord_status.h"

namespace roq {
namespace bitmex {

inline RequestType compute_request_type(json::ExecType exec_type) {
  switch (exec_type) {
    case json::ExecType::UNDEFINED:
    case json::ExecType::UNKNOWN:
      break;
    case json::ExecType::NEW:
      return RequestType::CREATE_ORDER;
    case json::ExecType::REPLACED:
      return RequestType::MODIFY_ORDER;
    case json::ExecType::CANCELED:
      return RequestType::CANCEL_ORDER;
    case json::ExecType::REJECTED:
      return {};  // or unknown?
    case json::ExecType::TRADE:
      break;
    case json::ExecType::FUNDING:
      break;
  }
  return {};
}

inline RequestStatus compute_request_status(json::ExecType exec_type) {
  switch (exec_type) {
    case json::ExecType::UNDEFINED:
    case json::ExecType::UNKNOWN:
      break;
    case json::ExecType::NEW:
    case json::ExecType::REPLACED:
    case json::ExecType::CANCELED:
      return RequestStatus::ACCEPTED;
    case json::ExecType::REJECTED:
      return RequestStatus::REJECTED;
    case json::ExecType::TRADE:
      break;
    case json::ExecType::FUNDING:
      break;
  }
  return {};
}

inline OrderStatus compute_order_status(json::OrdStatus ord_status, bool working_status) {
  switch (ord_status) {
    case json::OrdStatus::UNDEFINED:
    case json::OrdStatus::UNKNOWN:
      // note! back-stop in case we didn't parse OrdStatus
      if (working_status)
        return OrderStatus::WORKING;
      break;
    case json::OrdStatus::PENDING_NEW:
      return OrderStatus::SENT;
    case json::OrdStatus::NEW:
    case json::OrdStatus::TRIGGERED:
      if (!working_status)
        return OrderStatus::ACCEPTED;
      else
        return OrderStatus::WORKING;
    case json::OrdStatus::DONE_FOR_DAY:
      return OrderStatus::SUSPENDED;
    case json::OrdStatus::PARTIALLY_FILLED:
      return OrderStatus::WORKING;
    case json::OrdStatus::STOPPED:
      return OrderStatus::STOPPED;
    case json::OrdStatus::FILLED:
      return OrderStatus::COMPLETED;
    case json::OrdStatus::EXPIRED:
      return OrderStatus::EXPIRED;
    case json::OrdStatus::CANCELED:
      return OrderStatus::CANCELED;
    case json::OrdStatus::REJECTED:
      return OrderStatus::REJECTED;
    case json::OrdStatus::PENDING_CANCEL:  // XXX HANS how to deal with?
      break;
    case json::OrdStatus::UNTRIGGERED:  // XXX HANS have no idea what this means...
      break;
  }
  return {};
}

inline RequestStatus compute_request_status(json::OrdStatus ord_status) {
  switch (ord_status) {
    case json::OrdStatus::UNDEFINED:
    case json::OrdStatus::UNKNOWN:
      break;
    case json::OrdStatus::PENDING_NEW:
      return RequestStatus::ACCEPTED;
    case json::OrdStatus::NEW:
      return RequestStatus::ACCEPTED;
    case json::OrdStatus::TRIGGERED:
      return RequestStatus::ACCEPTED;
    case json::OrdStatus::DONE_FOR_DAY:
      return RequestStatus::ACCEPTED;
    case json::OrdStatus::PARTIALLY_FILLED:
      return RequestStatus::ACCEPTED;
    case json::OrdStatus::STOPPED:
      return RequestStatus::ACCEPTED;
    case json::OrdStatus::FILLED:
      return RequestStatus::ACCEPTED;
    case json::OrdStatus::EXPIRED:
      return RequestStatus::ACCEPTED;
    case json::OrdStatus::CANCELED:
      return RequestStatus::ACCEPTED;
    case json::OrdStatus::REJECTED:
      return RequestStatus::REJECTED;  // note!
    case json::OrdStatus::PENDING_CANCEL:
      return RequestStatus::ACCEPTED;
    case json::OrdStatus::UNTRIGGERED:  // XXX HANS have no idea what this means...
      return RequestStatus::ACCEPTED;
  }
  return {};
}

}  // namespace bitmex
}  // namespace roq
