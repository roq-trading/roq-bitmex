/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/bitmex/gateway.h"

#include "roq/logging.h"

#include "roq/bitmex/flags.h"

using namespace roq::literals;

namespace roq {
namespace bitmex {

namespace {
static auto create_security(const Config &config) {
  absl::flat_hash_map<std::string, std::unique_ptr<Security>> result;
  for (auto &[_, iter] : config.accounts) {
    result.try_emplace(iter.name, std::make_unique<Security>(config, iter.name));
  }
  return result;
}

template <typename T>
static auto create_order_entry(
    Gateway &gateway,
    core::io::Context &context,
    uint16_t &stream_id,
    T &security,
    Shared &shared) {
  absl::flat_hash_map<std::string, std::unique_ptr<OrderEntry>> result;
  for (auto &iter : security) {
    result.try_emplace(
        iter.first,
        std::make_unique<OrderEntry>(gateway, context, ++stream_id, *iter.second, shared));
  }
  return result;
}

template <typename T>
static auto create_drop_copy(
    Gateway &gateway,
    core::io::Context &context,
    uint16_t &stream_id,
    T &security,
    Shared &shared) {
  absl::flat_hash_map<std::string, std::unique_ptr<DropCopy>> result;
  for (auto &iter : security) {
    result.try_emplace(
        iter.first,
        std::make_unique<DropCopy>(gateway, context, ++stream_id, *iter.second, shared));
  }
  return result;
}
}  // namespace

Gateway::Gateway(server::Dispatcher &dispatcher, const Config &config)
    : dispatcher_(dispatcher), security_(create_security(config)), shared_(dispatcher),
      order_entry_(create_order_entry(*this, context_, stream_id_, security_, shared_)),
      drop_copy_(create_drop_copy(*this, context_, stream_id_, security_, shared_)),
      market_data_(*this, context_, ++stream_id_, shared_) {
  if (ROQ_UNLIKELY(!Flags::ws_cancel_on_disconnect()))
    log::warn("Orders will *NOT* be cancelled on disconnect"_sv);
}

void Gateway::operator()(const Event<Start> &event) {
  log::info("Starting the gateway..."_sv);
  for (auto &[_, order_entry] : order_entry_)
    (*order_entry)(event);
  for (auto &[_, drop_copy] : drop_copy_)
    (*drop_copy)(event);
  market_data_(event);
}

void Gateway::operator()(const Event<Stop> &event) {
  log::info("Stopping the gateway..."_sv);
  market_data_(event);
  for (auto &[_, drop_copy] : drop_copy_)
    (*drop_copy)(event);
  for (auto &[_, order_entry] : order_entry_)
    (*order_entry)(event);
}

void Gateway::operator()(const Event<Timer> &event) {
  for (auto &[_, order_entry] : order_entry_)
    (*order_entry)(event);
  for (auto &[_, drop_copy] : drop_copy_)
    (*drop_copy)(event);
  market_data_(event);
  context_.dispatch(true);
}

void Gateway::operator()(const Event<Connected> &) {
}

void Gateway::operator()(const Event<Disconnected> &event) {
  const auto &[message_info, disconnected] = event;
  log::warn(
      R"(Disconnected: source="{}", order_cancel_policy={})"_sv,
      message_info.source_name,
      disconnected.order_cancel_policy);
  switch (disconnected.order_cancel_policy) {
    case OrderCancelPolicy::UNDEFINED:
      break;
    case OrderCancelPolicy::MANAGED_ORDERS:
      log::warn("*** CANCEL MANAGED ORDERS NOT IMPLEMENTED ***"_sv);
      break;
    case OrderCancelPolicy::BY_ACCOUNT:
      log::warn("*** CANCEL ALL ACCOUNT ORDERS ***"_sv);
      for (auto &[account, order_entry] : order_entry_) {
        if (dispatcher_.can_user_trade_account(account, message_info.source)) {
          log::warn(R"(- account="{}")"_sv, account);
          CancelAllOrders cancel_all_orders{
              .account = account,
          };
          Event event(message_info, cancel_all_orders);
          (*order_entry)(event, {});
        }
      }
  }
}

uint16_t Gateway::operator()(
    const Event<CreateOrder> &event, const oms::Order &order, const std::string_view &request_id) {
  assert(!event.value.account.empty());
  return get_order_entry(event.value.account)(event, order, request_id);
}

uint16_t Gateway::operator()(
    const Event<ModifyOrder> &event,
    const oms::Order &order,
    const std::string_view &request_id,
    const std::string_view &previous_request_id) {
  assert(!event.value.account.empty());
  assert(event.value.account == order.account);
  return get_order_entry(event.value.account)(event, order, request_id, previous_request_id);
}

uint16_t Gateway::operator()(
    const Event<CancelOrder> &event,
    const oms::Order &order,
    const std::string_view &request_id,
    const std::string_view &previous_request_id) {
  assert(!event.value.account.empty());
  assert(event.value.account == order.account);
  return get_order_entry(event.value.account)(event, order, request_id, previous_request_id);
}

uint16_t Gateway::operator()(
    const Event<CancelAllOrders> &event, const std::string_view &request_id) {
  assert(!event.value.account.empty());
  return get_order_entry(event.value.account)(event, request_id);
}

void Gateway::operator()(metrics::Writer &writer) {
  for (auto &[_, order_entry] : order_entry_)
    (*order_entry)(writer);
  for (auto &[_, drop_copy] : drop_copy_)
    (*drop_copy)(writer);
  market_data_(writer);
}

void Gateway::operator()(const server::Trace<StreamStatus> &event) {
  dispatcher_(event);
}

void Gateway::operator()(const server::Trace<ExternalLatency> &event) {
  dispatcher_(event);
}

void Gateway::operator()(const server::Trace<ReferenceData> &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(const server::Trace<MarketStatus> &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(const server::Trace<TopOfBook> &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(
    const server::Trace<MarketByPriceUpdate> &event, bool is_last, bool refresh) {
  dispatcher_(
      event,
      is_last,
      refresh,
      shared_.final_bids,
      shared_.final_asks,
      []([[maybe_unused]] auto &market_by_price) {});
}

void Gateway::operator()(const server::Trace<TradeSummary> &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(const server::Trace<StatisticsUpdate> &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(const server::Trace<TradeUpdate> &event, bool is_last, uint8_t user_id) {
  dispatcher_(event, is_last, user_id);
}

void Gateway::operator()(const server::Trace<PositionUpdate> &event, bool is_last) {
  dispatcher_(event, is_last);
}

OrderEntry &Gateway::get_order_entry(const std::string_view &account) {
  auto iter = order_entry_.find(account);
  if (iter != order_entry_.end())
    return *(*iter).second;
  throw RuntimeErrorException(R"(Unknown account="{}")"_sv, account);
}

}  // namespace bitmex
}  // namespace roq
