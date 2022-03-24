/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/bitmex/gateway.hpp"

#include "roq/logging.hpp"

#include "roq/bitmex/flags.hpp"

using namespace std::literals;

namespace roq {
namespace bitmex {

namespace {
template <typename R>
auto create_security(const Config &config) {
  R result;
  for (auto &[_, iter] : config.accounts)
    result.try_emplace(iter.name, std::make_unique<Security>(config, iter.name));
  return result;
}

template <typename R, typename T>
auto create_order_entry(
    Gateway &gateway,
    core::io::Context &context,
    uint16_t &stream_id,
    T &security,
    Shared &shared) {
  R result;
  for (auto &iter : security)
    result.try_emplace(
        iter.first,
        std::make_unique<OrderEntry>(gateway, context, ++stream_id, *iter.second, shared));
  return result;
}

template <typename R, typename T>
auto create_web_socket(
    Gateway &gateway,
    core::io::Context &context,
    uint16_t &stream_id,
    T &security,
    Shared &shared) {
  R result;
  for (auto &iter : security)
    result.try_emplace(
        iter.first,
        std::make_unique<WebSocket>(gateway, context, ++stream_id, *iter.second, shared));
  return result;
}

template <typename R, typename T>
auto create_drop_copy(
    Gateway &gateway,
    core::io::Context &context,
    uint16_t &stream_id,
    T &security,
    Shared &shared) {
  R result;
  for (auto &iter : security)
    result.try_emplace(
        iter.first,
        std::make_unique<DropCopy>(gateway, context, ++stream_id, *iter.second, shared));
  return result;
}
}  // namespace

Gateway::Gateway(server::Dispatcher &dispatcher, const Config &config)
    : dispatcher_(dispatcher), security_(create_security<decltype(security_)>(config)),
      shared_(dispatcher), order_entry_(create_order_entry<decltype(order_entry_)>(
                               *this, context_, stream_id_, security_, shared_)),
      drop_copy_(
          create_drop_copy<decltype(drop_copy_)>(*this, context_, stream_id_, security_, shared_)),
      market_data_(*this, context_, ++stream_id_, shared_) {
  if (!Flags::ws_cancel_on_disconnect()) [[unlikely]]
    log::warn("Orders will *NOT* be cancelled on disconnect"sv);
}

void Gateway::operator()(const Event<Start> &event) {
  log::info("Starting the gateway..."sv);
  for (auto &[_, order_entry] : order_entry_)
    (*order_entry)(event);
  for (auto &[_, web_socket] : web_socket_)
    (*web_socket)(event);
  for (auto &[_, drop_copy] : drop_copy_)
    (*drop_copy)(event);
  market_data_(event);
}

void Gateway::operator()(const Event<Stop> &event) {
  log::info("Stopping the gateway..."sv);
  market_data_(event);
  for (auto &[_, drop_copy] : drop_copy_)
    (*drop_copy)(event);
  for (auto &[_, web_socket] : web_socket_)
    (*web_socket)(event);
  for (auto &[_, order_entry] : order_entry_)
    (*order_entry)(event);
}

void Gateway::operator()(const Event<Timer> &event) {
  for (auto &[_, order_entry] : order_entry_)
    (*order_entry)(event);
  for (auto &[_, web_socket] : web_socket_)
    (*web_socket)(event);
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
      R"(Disconnected: source="{}", order_cancel_policy={})"sv,
      message_info.source_name,
      disconnected.order_cancel_policy);
  switch (disconnected.order_cancel_policy) {
    case OrderCancelPolicy::UNDEFINED:
      break;
    case OrderCancelPolicy::MANAGED_ORDERS:
      log::warn("*** CANCEL MANAGED ORDERS NOT IMPLEMENTED ***"sv);
      break;
    case OrderCancelPolicy::BY_ACCOUNT:
      log::warn("*** CANCEL ALL ACCOUNT ORDERS ***"sv);
      if (Flags::oms_using_web_socket()) {
        for (auto &[account, web_socket] : web_socket_) {
          if (dispatcher_.can_user_trade_account(account, message_info.source)) {
            log::warn(R"(- account="{}")"sv, account);
            CancelAllOrders cancel_all_orders{
                .account = account,
            };
            Event event(message_info, cancel_all_orders);
            (*web_socket)(event, {});
          }
        }
      } else {
        for (auto &[account, order_entry] : order_entry_) {
          if (dispatcher_.can_user_trade_account(account, message_info.source)) {
            log::warn(R"(- account="{}")"sv, account);
            CancelAllOrders cancel_all_orders{
                .account = account,
            };
            Event event(message_info, cancel_all_orders);
            (*order_entry)(event, {});
          }
        }
      }
  }
}

uint16_t Gateway::operator()(
    const Event<CreateOrder> &event, const oms::Order &order, const std::string_view &request_id) {
  assert(!std::empty(event.value.account));
  if (Flags::oms_using_web_socket())
    return get_web_socket(event.value.account)(event, order, request_id);
  return get_order_entry(event.value.account)(event, order, request_id);
}

uint16_t Gateway::operator()(
    const Event<ModifyOrder> &event,
    const oms::Order &order,
    const std::string_view &request_id,
    const std::string_view &previous_request_id) {
  assert(!std::empty(event.value.account));
  assert(utils::compare(event.value.account, order.account) == 0);
  if (Flags::oms_using_web_socket())
    return get_web_socket(event.value.account)(event, order, request_id, previous_request_id);
  return get_order_entry(event.value.account)(event, order, request_id, previous_request_id);
}

uint16_t Gateway::operator()(
    const Event<CancelOrder> &event,
    const oms::Order &order,
    const std::string_view &request_id,
    const std::string_view &previous_request_id) {
  assert(!std::empty(event.value.account));
  assert(utils::compare(event.value.account, order.account) == 0);
  if (Flags::oms_using_web_socket())
    return get_web_socket(event.value.account)(event, order, request_id, previous_request_id);
  return get_order_entry(event.value.account)(event, order, request_id, previous_request_id);
}

uint16_t Gateway::operator()(
    const Event<CancelAllOrders> &event, const std::string_view &request_id) {
  assert(!std::empty(event.value.account));
  if (Flags::oms_using_web_socket())
    return get_web_socket(event.value.account)(event, request_id);
  return get_order_entry(event.value.account)(event, request_id);
}

void Gateway::operator()(metrics::Writer &writer) {
  for (auto &[_, order_entry] : order_entry_)
    (*order_entry)(writer);
  for (auto &[_, web_socket] : web_socket_)
    (*web_socket)(writer);
  for (auto &[_, drop_copy] : drop_copy_)
    (*drop_copy)(writer);
  market_data_(writer);
}

void Gateway::operator()(const Trace<StreamStatus> &event) {
  dispatcher_(event);
}

void Gateway::operator()(const Trace<ExternalLatency> &event) {
  dispatcher_(event);
}

void Gateway::operator()(const Trace<ReferenceData> &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(const Trace<MarketStatus> &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(const Trace<TopOfBook> &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(const Trace<MarketByPriceUpdate> &event, bool is_last, bool refresh) {
  dispatcher_(
      event,
      is_last,
      refresh,
      shared_.final_bids,
      shared_.final_asks,
      []([[maybe_unused]] auto &market_by_price) {});
}

void Gateway::operator()(const Trace<TradeSummary> &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(const Trace<StatisticsUpdate> &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(const Trace<TradeUpdate> &event, bool is_last, uint8_t user_id) {
  dispatcher_(event, is_last, user_id);
}

void Gateway::operator()(const Trace<PositionUpdate> &event, bool is_last) {
  dispatcher_(event, is_last);
}

OrderEntry &Gateway::get_order_entry(const std::string_view &account) {
  auto iter = order_entry_.find(account);
  if (iter != std::end(order_entry_))
    return *(*iter).second;
  throw RuntimeError(R"(Unknown account="{}")"sv, account);
}

WebSocket &Gateway::get_web_socket(const std::string_view &account) {
  auto iter = web_socket_.find(account);
  if (iter != std::end(web_socket_))
    return *(*iter).second;
  throw RuntimeError(R"(Unknown account="{}")"sv, account);
}

}  // namespace bitmex
}  // namespace roq
