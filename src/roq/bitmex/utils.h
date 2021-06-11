/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include "roq/api.h"

#include "roq/bitmex/json/exec_type.h"

namespace roq {
namespace bitmex {

static RequestType compute_request_type(json::ExecType exec_type) {
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

static RequestStatus compute_request_status(json::ExecType exec_type) {
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

}  // namespace bitmex
}  // namespace roq
