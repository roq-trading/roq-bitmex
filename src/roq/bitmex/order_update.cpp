/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/bitmex/order_update.h"

#include "roq/logging.h"

#include "roq/bitmex/flags.h"

#include "roq/bitmex/json/utils.h"

using namespace roq::literals;

namespace roq {
namespace bitmex {

namespace {
auto compute_order_status(json::OrdStatus ord_status, bool working_status) {
  switch (ord_status) {
    case json::OrdStatus::UNDEFINED:
    case json::OrdStatus::UNKNOWN:
      if (working_status)
        return OrderStatus::WORKING;
      break;
    case json::OrdStatus::NEW:
      return working_status ? OrderStatus::WORKING : OrderStatus::ACCEPTED;
    case json::OrdStatus::FILLED:
      return OrderStatus::COMPLETED;
    case json::OrdStatus::CANCELED:
      return OrderStatus::CANCELED;
  }
  return OrderStatus::UNDEFINED;
}

auto compute_request_status(RequestType request_type, json::OrdStatus ord_status) {
  switch (ord_status) {
    case json::OrdStatus::UNDEFINED:
    case json::OrdStatus::UNKNOWN:
      break;
    case json::OrdStatus::NEW: {
      switch (request_type) {
        case RequestType::UNDEFINED:
          log::warn("*** EXTERNAL ACTION ***"_sv);
          break;
        case RequestType::CREATE_ORDER:
        case RequestType::MODIFY_ORDER:
          return RequestStatus::ACCEPTED;
        case RequestType::CANCEL_ORDER:
          log::fatal("DEBUG: UNEXPECTED"_sv);
          break;
      }
      break;
    }
    case json::OrdStatus::FILLED: {
      break;
    }
    case json::OrdStatus::CANCELED: {
      switch (request_type) {
        case RequestType::UNDEFINED:
          log::warn("*** EXTERNAL ACTION ***"_sv);
          break;
        case RequestType::CANCEL_ORDER:
          return RequestStatus::ACCEPTED;
        case RequestType::CREATE_ORDER:
        case RequestType::MODIFY_ORDER:
          log::fatal("DEBUG: UNEXPECTED"_sv);
          break;
      }
      break;
    }
  }
  return RequestStatus::UNDEFINED;
}

}  // namespace

void OrderUpdate::operator()(
    const json::OrderItem &order_item, const server::TraceInfo &trace_info) {
  if (!Flags::rest_allow_order_updates())
    return;
  auto order_status = compute_order_status(order_item.ord_status, order_item.working_indicator);
  log::debug("DEBUG: order_status={}"_fmt, order_status);
  server::OMS_Lookup order_lookup{
      .symbol = order_item.symbol,
      .side = json::map(order_item.side),
      .status = order_status,
      .price = order_item.price,
      .remaining_quantity = order_item.leaves_qty,
      .traded_quantity = order_item.cum_qty,
      .timestamp = order_item.timestamp,  // XXX transact_time?
      .external_account = {},
      .external_order_id = order_item.order_id,
  };
  auto found = shared_.find_order(
      order_item.order_id,
      order_item.cl_ord_id,
      order_lookup,
      trace_info,
      [&](const auto &order, auto &result) {
        result.request_status = compute_request_status(order.request_type, order_item.ord_status);

        if (result.request_status != RequestStatus::UNDEFINED) {
          result.origin = Origin::EXCHANGE;
          result.error = order_item.ord_rej_reason.empty() ? Error::UNDEFINED : Error::UNKNOWN,
          result.text = order_item.ord_rej_reason;  // XXX text ???
        }
      });

  if (!found) {
    log::warn("*** EXTERNAL ORDER ***"_sv);
    log::warn("order_item={}"_fmt, order_item);
  }
}

void OrderUpdate::operator()(const json::Order &order, const server::TraceInfo &trace_info) {
  log::debug("DEBUG: order={}"_fmt, order);
  for (auto &iter : order.data)
    (*this)(iter, trace_info);
}

}  // namespace bitmex
}  // namespace roq
