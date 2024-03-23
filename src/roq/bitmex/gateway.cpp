/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/bitmex/gateway.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace bitmex {

// === HELPERS ===

namespace {
template <typename R>
R create_accounts(auto &settings, auto &config) {
  using result_type = std::remove_cvref<R>::type;
  result_type result;
  for (auto &[_, account] : config.accounts)
    result.try_emplace(
        static_cast<std::string_view>(account.name), std::make_unique<Account>(settings, config, account.name));
  return result;
}

template <typename R>
R create_order_entry(auto &gateway, auto &context, auto &stream_id, auto &accounts, auto &shared) {
  using result_type = std::remove_cvref<R>::type;
  result_type result;
  for (auto &[name, account] : accounts)
    result.try_emplace(
        static_cast<std::string_view>(name),
        std::make_unique<OrderEntry>(gateway, context, ++stream_id, *account, shared));
  return result;
}

template <typename R>
R create_web_socket(auto &gateway, auto &context, auto &stream_id, auto &accounts, auto &shared) {
  using result_type = std::remove_cvref<R>::type;
  result_type result;
  for (auto &[name, account] : accounts)
    result.try_emplace(
        static_cast<std::string_view>(name),
        std::make_unique<WebSocket>(gateway, context, ++stream_id, *account, shared));
  return result;
}

template <typename R>
R create_drop_copy(auto &gateway, auto &context, auto &stream_id, auto &accounts, auto &shared) {
  using result_type = std::remove_cvref<R>::type;
  result_type result;
  for (auto &[name, account] : accounts)
    result.try_emplace(
        static_cast<std::string_view>(name),
        std::make_unique<DropCopy>(gateway, context, ++stream_id, *account, shared));
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

Gateway::Gateway(server::Dispatcher &dispatcher, Settings const &settings, Config const &config, io::Context &context)
    : dispatcher_{dispatcher}, accounts_{create_accounts<decltype(accounts_)>(settings, config)}, context_{context},
      shared_{dispatcher, settings},
      order_entry_{create_order_entry<decltype(order_entry_)>(*this, context_, stream_id_, accounts_, shared_)},
      drop_copy_{create_drop_copy<decltype(drop_copy_)>(*this, context_, stream_id_, accounts_, shared_)},
      market_data_{*this, context_, ++stream_id_, shared_} {
  if (!settings.ws.cancel_on_disconnect) [[unlikely]]
    log::warn("Orders will *NOT* be cancelled on disconnect"sv);
}

void Gateway::operator()(Event<Start> const &event) {
  log::info("Starting..."sv);
  dispatch(event);
}

void Gateway::operator()(Event<Stop> const &event) {
  log::info("Stopping..."sv);
  dispatch(event);
}

void Gateway::operator()(Event<Timer> const &event) {
  dispatch(event);
}

void Gateway::operator()(Event<Connected> const &) {
}

void Gateway::operator()(Event<Disconnected> const &) {
}

uint16_t Gateway::operator()(
    Event<CreateOrder> const &event, server::oms::Order const &order, std::string_view const &request_id) {
  assert(!std::empty(event.value.account));
  if (shared_.settings.misc.oms_using_web_socket)
    return get_web_socket(event.value.account)(event, order, request_id);
  return get_order_entry(event.value.account)(event, order, request_id);
}

uint16_t Gateway::operator()(
    Event<ModifyOrder> const &event,
    server::oms::Order const &order,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  assert(!std::empty(event.value.account));
  assert(event.value.account == order.account);
  if (shared_.settings.misc.oms_using_web_socket)
    return get_web_socket(event.value.account)(event, order, request_id, previous_request_id);
  return get_order_entry(event.value.account)(event, order, request_id, previous_request_id);
}

uint16_t Gateway::operator()(
    Event<CancelOrder> const &event,
    server::oms::Order const &order,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  assert(!std::empty(event.value.account));
  assert(event.value.account == order.account);
  if (shared_.settings.misc.oms_using_web_socket)
    return get_web_socket(event.value.account)(event, order, request_id, previous_request_id);
  return get_order_entry(event.value.account)(event, order, request_id, previous_request_id);
}

uint16_t Gateway::operator()(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  assert(!std::empty(event.value.account));
  if (shared_.settings.misc.oms_using_web_socket)
    return get_web_socket(event.value.account)(event, request_id);
  return get_order_entry(event.value.account)(event, request_id);
}

void Gateway::operator()(metrics::Writer &writer) {
  dispatch(writer);
}

void Gateway::operator()(Trace<StreamStatus> const &event) {
  dispatcher_(event);
}

void Gateway::operator()(Trace<ExternalLatency> const &event) {
  dispatcher_(event);
}

void Gateway::operator()(Trace<ReferenceData> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<MarketStatus> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<TopOfBook> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<MarketByPriceUpdate> const &event, bool is_last) {
  auto callback = []([[maybe_unused]] auto &market_by_price) {};
  dispatcher_(event, is_last, bids_, asks_, callback);
}

void Gateway::operator()(Trace<TradeSummary> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<StatisticsUpdate> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(
    Trace<TradeUpdate> const &event, bool is_last, uint8_t user_id, std::string_view const &request_id) {
  dispatcher_(event, is_last, user_id, request_id);
}

void Gateway::operator()(Trace<PositionUpdate> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

template <typename... Args>
void Gateway::dispatch(Args &&...args) {
  auto helper = [&](auto &target) { target(std::forward<Args>(args)...); };
  for (auto &[_, item] : order_entry_)
    helper(*item);
  for (auto &[_, item] : web_socket_)
    helper(*item);
  for (auto &[_, item] : drop_copy_)
    helper(*item);
  helper(market_data_);
}

OrderEntry &Gateway::get_order_entry(std::string_view const &account) {
  auto iter = order_entry_.find(account);
  if (iter != std::end(order_entry_))
    return *(*iter).second;
  throw RuntimeError{R"(Unknown account="{}")"sv, account};
}

WebSocket &Gateway::get_web_socket(std::string_view const &account) {
  auto iter = web_socket_.find(account);
  if (iter != std::end(web_socket_))
    return *(*iter).second;
  throw RuntimeError{R"(Unknown account="{}")"sv, account};
}

}  // namespace bitmex
}  // namespace roq
