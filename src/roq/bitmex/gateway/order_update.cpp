/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/bitmex/gateway/order_update.hpp"

#include <string>

#include "roq/logging.hpp"

#include "roq/bitmex/gateway/utils.hpp"

#include "roq/bitmex/protocol/json/map.hpp"
#include "roq/bitmex/protocol/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace bitmex {
namespace gateway {

// === IMPLEMENTATION ===

// drop copy

void OrderUpdate::operator()(protocol::json::OrderDataItem const &order_item, TraceInfo const &trace_info, bool download) {
  if (!download) {  // note! the execution table has the information we need
    return;
  }
  auto status = compute_order_status(order_item.ord_status, order_item.working_indicator);
  auto external_account = order_item.account ? fmt::format("{}"sv, order_item.account) : std::string{};
  auto external_order_id = order_item.order_id;
  auto request_type = order_item.ord_status == protocol::json::OrdStatus::CANCELED ? RequestType::CANCEL_ORDER : RequestType::UNDEFINED;
  auto request_status = order_item.ord_status == protocol::json::OrdStatus::REJECTED ? RequestStatus::REJECTED : RequestStatus::ACCEPTED;
  auto update_type = download ? UpdateType::SNAPSHOT : UpdateType::INCREMENTAL;
  auto response = server::oms::Response{
      .request_type = request_type,
      .origin = Origin::EXCHANGE,
      .request_status = request_status,
      .error = protocol::json::guess_error(order_item.ord_rej_reason),
      .text = order_item.text,
      .version = {},
      .request_id = {},  // cancel does not rewrite
      .external_order_id = {},
      .client_order_id = {},
      .quantity = order_item.order_qty,
      .price = order_item.price,
  };
  auto order_update = server::oms::OrderUpdate{
      .account = account_,
      .exchange = shared_.settings.exchange,
      .symbol = order_item.symbol,
      .side = map(order_item.side),
      .position_effect = {},
      .margin_mode = {},
      .max_show_quantity = NaN,
      .order_type = map(order_item.ord_type),
      .time_in_force = map(order_item.time_in_force),
      .execution_instructions = {},
      .create_time_utc = {},
      .update_time_utc = order_item.timestamp,  // XXX transact_time?
      .external_account = external_account,
      .external_order_id = external_order_id,
      .client_order_id = order_item.cl_ord_id,
      .order_status = status,
      .error = {},
      .text = {},
      .quantity = order_item.order_qty,
      .price = order_item.price,
      .stop_price = order_item.stop_px,
      .leverage = NaN,
      .remaining_quantity = order_item.leaves_qty,
      .traded_quantity = order_item.cum_qty,
      .average_traded_price = order_item.avg_px,
      .last_traded_quantity = NaN,
      .last_traded_price = NaN,
      .last_liquidity = {},
      .routing_id = {},
      .max_request_version = {},
      .max_response_version = {},
      .max_accepted_version = {},
      .update_type = update_type,
      .sending_time_utc = {},
  };
  auto request_or_exchange_id = [&]() {
    if (std::empty(order_update.client_order_id)) {
      return order_update.external_order_id;
    }
    return order_update.client_order_id;
  }();
  if (shared_.update_order(request_or_exchange_id, stream_id_, trace_info, response, order_update, []([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn("*** EXTERNAL ORDER ***"sv);
    log::warn("order_item={}"sv, order_item);
  }
}

void OrderUpdate::operator()(protocol::json::Order const &order, TraceInfo const &trace_info, bool download) {
  for (auto &iter : order.data) {
    (*this)(iter, trace_info, download);
  }
}

// order entry

void OrderUpdate::operator()(
    protocol::json::OrderDataItem const &order_item,
    TraceInfo const &trace_info,
    RequestType request_type,
    [[maybe_unused]] uint8_t user_id,
    [[maybe_unused]] uint64_t order_id,
    uint32_t version) {
  auto status = compute_order_status(order_item.ord_status, order_item.working_indicator);
  auto external_account = order_item.account ? fmt::format("{}"sv, order_item.account) : std::string{};
  auto external_order_id = order_item.order_id;
  auto request_status = compute_request_status(order_item.ord_status);
  auto error = protocol::json::guess_error(order_item.ord_rej_reason);
  auto request_id = request_type != RequestType::CANCEL_ORDER ? order_item.cl_ord_id : std::string_view{};
  auto response = server::oms::Response{
      .request_type = request_type,
      .origin = Origin::EXCHANGE,
      .request_status = request_status,
      .error = error,
      .text = order_item.text,
      .version = version,
      .request_id = request_id,
      .external_order_id = {},
      .client_order_id = {},
      .quantity = order_item.order_qty,
      .price = order_item.price,
  };
  auto order_update = server::oms::OrderUpdate{
      .account = account_,
      .exchange = shared_.settings.exchange,
      .symbol = order_item.symbol,
      .side = map(order_item.side),
      .position_effect = {},
      .margin_mode = {},
      .max_show_quantity = NaN,
      .order_type = map(order_item.ord_type),
      .time_in_force = map(order_item.time_in_force),
      .execution_instructions = {},
      .create_time_utc = {},
      .update_time_utc = order_item.timestamp,  // XXX transact_time?
      .external_account = external_account,
      .external_order_id = external_order_id,
      .client_order_id = order_item.cl_ord_id,
      .order_status = status,
      .error = {},
      .text = {},
      .quantity = order_item.order_qty,
      .price = order_item.price,
      .stop_price = order_item.stop_px,
      .leverage = NaN,
      .remaining_quantity = order_item.leaves_qty,
      .traded_quantity = order_item.cum_qty,
      .average_traded_price = order_item.avg_px,
      .last_traded_quantity = NaN,
      .last_traded_price = NaN,
      .last_liquidity = {},
      .routing_id = {},
      .max_request_version = {},
      .max_response_version = {},
      .max_accepted_version = {},
      .update_type = UpdateType::INCREMENTAL,
      .sending_time_utc = {},
  };
  auto request_or_exchange_id = [&]() {
    if (std::empty(order_update.client_order_id)) {
      return order_update.external_order_id;
    }
    return order_update.client_order_id;
  }();
  if (shared_.update_order(request_or_exchange_id, stream_id_, trace_info, response, order_update, []([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn("*** EXTERNAL ORDER ***"sv);
    log::warn("order_item={}"sv, order_item);
  }
}

void OrderUpdate::operator()(
    protocol::json::Order const &order, TraceInfo const &trace_info, RequestType request_type, uint8_t user_id, uint64_t order_id, uint32_t version) {
  for (auto &iter : order.data) {
    (*this)(iter, trace_info, request_type, user_id, order_id, version);
  }
}

}  // namespace gateway
}  // namespace bitmex
}  // namespace roq
