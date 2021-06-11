/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/bitmex/order_update.h"

#include "roq/logging.h"

#include "roq/bitmex/flags.h"
#include "roq/bitmex/utils.h"

#include "roq/bitmex/json/utils.h"

using namespace roq::literals;

namespace roq {
namespace bitmex {

namespace {
OrderStatus compute_order_status(json::OrdStatus ord_status, bool working_status) {
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
      log::warn("Unexpected: ord_status={}, working_status={}"_fmt, ord_status, working_status);
      break;
  }
  return {};
}

RequestStatus compute_request_status(json::OrdStatus ord_status) {
  if (ord_status == json::OrdStatus::REJECTED)
    return RequestStatus::REJECTED;
  return {};
}
}  // namespace

void OrderUpdate::operator()(
    const json::OrderItem &order_item, const server::TraceInfo &trace_info) {
  if (!Flags::rest_allow_order_updates())
    return;
  auto status = compute_order_status(order_item.ord_status, order_item.working_indicator);
  log::debug("DEBUG: status={}"_fmt, status);
  auto side = json::map(order_item.side);
  auto external_account = roq::format("{}"_fmt, order_item.account);  // XXX alloc
  auto order_type = json::map(order_item.ord_type);
  auto time_in_force = json::map(order_item.time_in_force);
  // XXX TODO(thraneh): execution_instruction
  roq::OrderUpdate order_update{
      .stream_id = stream_id_,
      .account = account_,
      .order_id = {},
      .exchange = Flags::exchange(),
      .symbol = order_item.symbol,
      .side = side,
      .position_effect = {},
      .quantity = order_item.order_qty,
      .max_show_quantity = NaN,
      .order_type = order_type,
      .time_in_force = time_in_force,
      .execution_instruction = {},  // XXX TODO(thraneh)
      .order_template = {},
      .create_time_utc = {},
      .update_time_utc = order_item.timestamp,  // XXX transact_time?
      .external_account = external_account,
      .external_order_id = order_item.order_id,
      .status = status,
      .price = order_item.price,
      .stop_price = order_item.stop_px,
      .remaining_quantity = order_item.leaves_qty,
      .traded_quantity = order_item.cum_qty,
      .average_traded_price = order_item.avg_px,
      .last_traded_price = NaN,
      .last_traded_quantity = NaN,
      .last_liquidity = {},
      .routing_id = {},  // XXX TODO(thraneh): decode clOrdID ?
      .max_request_version = {},
      .max_response_version = {},
      .max_accepted_version = {},
  };
  auto request_id = order_item.cl_ord_id;
  auto found = shared_.find_order(
      stream_id_, trace_info, order_update, request_id, [&](const auto &order, auto callback) {
        auto status = compute_request_status(order_item.ord_status);
        server::Ack ack{
            .stream_id = stream_id_,
            .account = account_,
            .order_id = order.order_id,
            .type = {},
            .origin = Origin::EXCHANGE,
            .status = status,
            .error = order_item.ord_rej_reason.empty() ? Error::UNDEFINED : Error::UNKNOWN,
            .text = order_item.text,
            .version = {},
            .request_id = request_id,
        };
        server::Trace event(trace_info, ack);
        callback(event, true, order.user_id);
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
