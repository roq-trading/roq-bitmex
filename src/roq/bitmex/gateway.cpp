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
    : dispatcher_(dispatcher), master_account_(config.get_master_account()),
      security_(create_security(config)), shared_(dispatcher),
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

void Gateway::operator()(const Event<Disconnected> &) {
}

void Gateway::operator()(
    const Event<CreateOrder> &event,
    const std::string_view &request_id,
    uint32_t gateway_order_id) {
  assert(!event.value.account.empty());
  get_order_entry(event.value.account)(event, request_id, gateway_order_id);
}

void Gateway::operator()(
    const Event<ModifyOrder> &event,
    const std::string_view &request_id,
    const server::OMS_Order &order) {
  assert(!event.value.account.empty());
  assert(event.value.account == order.account);
  get_order_entry(event.value.account)(event, request_id, order);
}

void Gateway::operator()(
    const Event<CancelOrder> &event,
    const std::string_view &request_id,
    const server::OMS_Order &order) {
  assert(!event.value.account.empty());
  assert(event.value.account == order.account);
  get_order_entry(event.value.account)(event, request_id, order);
}

void Gateway::operator()(const Event<CancelAllOrders> &event, const std::string_view &request_id) {
  assert(!event.value.account.empty());
  get_order_entry(event.value.account)(event, request_id);
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

void Gateway::operator()(const server::Trace<MarketByPriceUpdate> &event, bool is_last) {
  dispatcher_(event, is_last);
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

/*
void Gateway::operator()(const MarketData &) {
  if (market_data_.connection.ready()) {
    market_data_.download.begin();
  } else {
    market_data_.download.reset();
    symbols_.clear();
    snapshot_ = {};
  }
}
*/

/*
int32_t Gateway::download(MarketDataDownload::State state) {
  if (!order_entry_.connection.ready())
    return -1;
  switch (state) {
    case MarketDataDownload::State::UNDEFINED:
      break;
    case MarketDataDownload::State::ACCOUNTS: {
      return 0;
    }
    case MarketDataDownload::State::INSTRUMENT: {
      subscribe_instrument();
      return 1;
    }
    case MarketDataDownload::State::ORDER_BOOK_L2: {
      subscribe_order_book_l2();
      return 1;
    }
    case MarketDataDownload::State::DONE:
      update_order_manager(ConnectionStatus::READY);
      update_market_data(ConnectionStatus::READY);
      return 0;
  }
  assert(false);
  return 0;
}
*/

/*
void Gateway::operator()(const OrderEntry &) {
  if (order_entry_.connection.ready())
    market_data_.download.bump();
}
*/

OrderEntry &Gateway::get_order_entry(const std::string_view &account) {
  auto iter = order_entry_.find(account);
  if (iter != order_entry_.end())
    return *(*iter).second;
  throw RuntimeErrorException(R"(Unknown account="{}")"_fmt, account);
}

}  // namespace bitmex
}  // namespace roq
