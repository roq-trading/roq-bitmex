/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/bitmex/order_update.h"

#include "roq/logging.h"

#include "roq/bitmex/flags.h"
#include "roq/bitmex/utils.h"

#include "roq/bitmex/json/utils.h"

using namespace std::literals;

namespace roq {
namespace bitmex {

// drop copy

void OrderUpdate::operator()(
    const json::OrderItem &order_item, const server::TraceInfo &trace_info, bool download) {
  auto status = compute_order_status(order_item.ord_status, order_item.working_indicator);
  log::debug("status={}"sv, status);
  auto side = json::map(order_item.side);
  auto external_account = fmt::format("{}"sv, order_item.account);  // XXX alloc
  auto external_order_id = order_item.order_id;
  auto order_type = json::map(order_item.ord_type);
  auto time_in_force = json::map(order_item.time_in_force);
  auto request_type = order_item.ord_status == json::OrdStatus::CANCELED ? RequestType::CANCEL_ORDER
                                                                         : RequestType::UNDEFINED;
  auto request_status = order_item.ord_status == json::OrdStatus::REJECTED
                            ? RequestStatus::REJECTED
                            : RequestStatus::ACCEPTED;
  oms::Response response{
      .type = request_type,
      .origin = Origin::EXCHANGE,
      .status = request_status,
      .error = json::guess_error(order_item.ord_rej_reason),
      .text = order_item.text,
      .version = {},
      .request_id = {},  // cancel does not rewrite
      .quantity = order_item.order_qty,
      .price = order_item.price,
  };
  oms::OrderUpdate order_update{
      .account = account_,
      .exchange = Flags::exchange(),
      .symbol = order_item.symbol,
      .side = side,
      .position_effect = {},
      .max_show_quantity = NaN,
      .order_type = order_type,
      .time_in_force = time_in_force,
      .execution_instruction = {},
      .order_template = {},
      .create_time_utc = {},
      .update_time_utc = order_item.timestamp,  // XXX transact_time?
      .external_account = external_account,
      .external_order_id = external_order_id,
      .status = status,
      .quantity = order_item.order_qty,
      .price = order_item.price,
      .stop_price = order_item.stop_px,
      .remaining_quantity = order_item.leaves_qty,
      .traded_quantity = order_item.cum_qty,
      .average_traded_price = order_item.avg_px,
      .last_traded_quantity = NaN,
      .last_traded_price = NaN,
      .last_liquidity = {},
  };
  if (download) {
    if (shared_.create_order(order_item.cl_ord_id, stream_id_, trace_info, order_update)) {
    } else {
      log::warn("*** EXTERNAL ORDER ***"sv);
      log::warn("order_item={}"sv, order_item);
    }
  } else {
    if (shared_.update_order(
            order_item.cl_ord_id,
            stream_id_,
            trace_info,
            response,
            order_update,
            []([[maybe_unused]] auto &order) {})) {
    } else {
      log::warn("*** EXTERNAL ORDER ***"sv);
      log::warn("order_item={}"sv, order_item);
    }
  }
}

void OrderUpdate::operator()(
    const json::Order &order, const server::TraceInfo &trace_info, bool download) {
  log::debug("order={}"sv, order);
  for (auto &iter : order.data)
    (*this)(iter, trace_info, download);
}

// order entry

void OrderUpdate::operator()(
    const json::OrderItem &order_item,
    const server::TraceInfo &trace_info,
    RequestType request_type,
    [[maybe_unused]] uint8_t user_id,
    [[maybe_unused]] uint32_t order_id,
    uint32_t version) {
  auto status = compute_order_status(order_item.ord_status, order_item.working_indicator);
  log::debug("status={}"sv, status);
  auto side = json::map(order_item.side);
  auto external_account = fmt::format("{}"sv, order_item.account);  // XXX alloc
  auto external_order_id = order_item.order_id;
  auto order_type = json::map(order_item.ord_type);
  auto time_in_force = json::map(order_item.time_in_force);
  auto request_status = compute_request_status(order_item.ord_status);
  auto error = json::guess_error(order_item.ord_rej_reason);
  auto request_id =
      request_type != RequestType::CANCEL_ORDER ? order_item.cl_ord_id : std::string_view{};
  oms::Response response{
      .type = request_type,
      .origin = Origin::EXCHANGE,
      .status = request_status,
      .error = error,
      .text = order_item.text,
      .version = version,
      .request_id = request_id,
      .quantity = order_item.order_qty,
      .price = order_item.price,
  };
  oms::OrderUpdate order_update{
      .account = account_,
      .exchange = Flags::exchange(),
      .symbol = order_item.symbol,
      .side = side,
      .position_effect = {},
      .max_show_quantity = NaN,
      .order_type = order_type,
      .time_in_force = time_in_force,
      .execution_instruction = {},
      .order_template = {},
      .create_time_utc = {},
      .update_time_utc = order_item.timestamp,  // XXX transact_time?
      .external_account = external_account,
      .external_order_id = external_order_id,
      .status = status,
      .quantity = order_item.order_qty,
      .price = order_item.price,
      .stop_price = order_item.stop_px,
      .remaining_quantity = order_item.leaves_qty,
      .traded_quantity = order_item.cum_qty,
      .average_traded_price = order_item.avg_px,
      .last_traded_quantity = NaN,
      .last_traded_price = NaN,
      .last_liquidity = {},
  };
  if (shared_.update_order(
          order_item.cl_ord_id,
          stream_id_,
          trace_info,
          response,
          order_update,
          []([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn("*** EXTERNAL ORDER ***"sv);
    log::warn("order_item={}"sv, order_item);
  }
}

void OrderUpdate::operator()(
    const json::Order &order,
    const server::TraceInfo &trace_info,
    RequestType request_type,
    uint8_t user_id,
    uint32_t order_id,
    uint32_t version) {
  log::debug("order={}"sv, order);
  for (auto &iter : order.data)
    (*this)(iter, trace_info, request_type, user_id, order_id, version);
}

}  // namespace bitmex
}  // namespace roq
