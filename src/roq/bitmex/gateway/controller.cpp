/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/bitmex/gateway/controller.hpp"

#include "roq/logging.hpp"

#include "roq/server/oms/exceptions.hpp"

using namespace std::literals;

namespace roq {
namespace bitmex {
namespace gateway {

// === HELPERS ===

namespace {
template <typename R>
R create_accounts(auto &settings, auto &config) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  for (auto &[_, account] : config.accounts) {
    result.try_emplace(static_cast<std::string_view>(account.name), std::make_unique<Account>(settings, config, account.name));
  }
  return result;
}

template <typename R>
R create_order_entry(auto &gateway, auto &context, auto &stream_id, auto &accounts, auto &shared) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  if (!shared.settings.misc.number_of_order_entry_connections) {
    log::fatal("Unexpected: --number_of_order_entry_connections={}"sv, shared.settings.misc.number_of_order_entry_connections);
  }
  for (auto &[name, account] : accounts) {
    std::vector<std::unique_ptr<OrderEntry>> order_entry;
    for (size_t i = 0; i < shared.settings.misc.number_of_order_entry_connections; ++i) {
      order_entry.emplace_back(std::make_unique<OrderEntry>(gateway, context, ++stream_id, *account, shared));
    }
    result.try_emplace(static_cast<std::string_view>(name), std::move(order_entry));
  }
  return result;
}

template <typename R>
R create_drop_copy(auto &gateway, auto &context, auto &stream_id, auto &accounts, auto &shared) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  for (auto &[name, account] : accounts) {
    result.try_emplace(static_cast<std::string_view>(name), std::make_unique<DropCopy>(gateway, context, ++stream_id, *account, shared));
  }
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

std::unique_ptr<server::Handler> Controller::create(server::Dispatcher &dispatcher, Settings const &settings, Config const &config, io::Context &context) {
  return std::make_unique<Controller>(dispatcher, settings, config, context);
}

uint8_t Controller::parse_api(Settings const &) {
  return {};
}

Controller::Controller(server::Dispatcher &dispatcher, Settings const &settings, Config const &config, io::Context &context)
    : dispatcher_{dispatcher}, accounts_{create_accounts<decltype(accounts_)>(settings, config)}, context_{context}, shared_{dispatcher, settings},
      order_entry_{create_order_entry<decltype(order_entry_)>(*this, context_, stream_id_, accounts_, shared_)},
      drop_copy_{create_drop_copy<decltype(drop_copy_)>(*this, context_, stream_id_, accounts_, shared_)},
      market_data_{*this, context_, ++stream_id_, shared_} {
  if (!settings.ws.cancel_on_disconnect) [[unlikely]] {
    log::warn("Orders will *NOT* be cancelled on disconnect"sv);
  }
}

// server::Handler

void Controller::operator()(Event<Start> const &event) {
  log::info("Starting..."sv);
  dispatch(event);
}

void Controller::operator()(Event<Stop> const &event) {
  log::info("Stopping..."sv);
  dispatch(event);
}

void Controller::operator()(Event<Timer> const &event) {
  dispatch(event);
}

void Controller::operator()(Event<Control> const &event) {
  auto &[message_info, control] = event;
  switch (control.action) {
    using enum Action;
    case UNDEFINED:
      assert(false);
      break;
    case ENABLE:
      dispatcher_(State::ENABLED);
      break;
    case DISABLE:
      dispatcher_(State::DISABLED);
      break;
  }
}

void Controller::operator()(Event<Connected> const &) {
}

void Controller::operator()(Event<Disconnected> const &) {
}

void Controller::operator()(Event<Subscribe> const &) {
  // all symbols already subscribed ???
}

uint16_t Controller::operator()(
    Event<CreateOrder> const &event, server::oms::Order const &order, server::oms::RefData const &ref_data, std::string_view const &request_id) {
  assert(!std::empty(event.value.account));
  return get_order_entry(event.value.account)(event, order, ref_data, request_id);
}

uint16_t Controller::operator()(
    Event<ModifyOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  assert(!std::empty(event.value.account));
  assert(event.value.account == order.account);
  return get_order_entry(event.value.account)(event, order, ref_data, request_id, previous_request_id);
}

uint16_t Controller::operator()(
    Event<CancelOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  assert(!std::empty(event.value.account));
  assert(event.value.account == order.account);
  return get_order_entry(event.value.account)(event, order, ref_data, request_id, previous_request_id);
}

uint16_t Controller::operator()(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  assert(!std::empty(event.value.account));
  return get_order_entry(event.value.account)(event, request_id);
}

uint16_t Controller::operator()(Event<MassQuote> const &) {
  throw server::oms::NotSupported{"not supported"sv};
}

uint16_t Controller::operator()(Event<CancelQuotes> const &) {
  throw server::oms::NotSupported{"not supported"sv};
}

void Controller::operator()(metrics::Writer &writer) const {
  dispatch_helper(*this, writer);
}

// streams

void Controller::operator()(Trace<StreamStatus> const &event) {
  dispatcher_(event);
}

void Controller::operator()(Trace<ExternalLatency> const &event) {
  dispatcher_(event);
}

void Controller::operator()(Trace<ReferenceData> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Controller::operator()(Trace<MarketStatus> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Controller::operator()(Trace<TopOfBook> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Controller::operator()(Trace<MarketByPriceUpdate> const &event, bool is_last) {
  auto callback = []([[maybe_unused]] auto &market_by_price) {};
  dispatcher_(event, is_last, bids_, asks_, callback);
}

void Controller::operator()(Trace<TradeSummary> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Controller::operator()(Trace<StatisticsUpdate> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Controller::operator()(Trace<TradeUpdate> const &event, bool is_last, uint8_t user_id) {
  dispatcher_(event, is_last, user_id);
}

void Controller::operator()(Trace<PositionUpdate> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

// utilities

template <typename... Args>
void Controller::dispatch(Args &&...args) {
  dispatch_helper(*this, std::forward<Args>(args)...);
}

template <typename... Args>
void Controller::dispatch_helper(auto &self, Args &&...args) {
  auto helper = [&](auto &target) { target(std::forward<Args>(args)...); };
  for (auto &[_, item] : self.order_entry_) {
    helper(item);
  }
  for (auto &[_, item] : self.drop_copy_) {
    helper(*item);
  }
  helper(self.market_data_);
}

OrderEntry &Controller::get_order_entry(std::string_view const &account) {
  auto iter = order_entry_.find(account);
  if (iter != std::end(order_entry_)) {
    return (*iter).second.get_next();
  }
  throw RuntimeError(R"(Unknown account="{}")"sv, account);
}

// OrderEntryRR

Controller::OrderEntryRR::OrderEntryRR(std::vector<std::unique_ptr<OrderEntry>> &&order_entry) : order_entry_{std::move(order_entry)} {
  for (auto &item : order_entry_) {
    if (item == nullptr) {
      log::fatal("HERE"sv);
    }
  }
}

template <typename... Args>
void Controller::OrderEntryRR::operator()(Args &&...args) {
  for (auto &item : order_entry_) {
    (*item)(args...);
  }
}

template <typename... Args>
void Controller::OrderEntryRR::operator()(Args &&...args) const {
  for (auto &item : order_entry_) {
    (*item)(args...);
  }
}

OrderEntry &Controller::OrderEntryRR::get_next() {
  auto length = std::size(order_entry_);
  for (size_t offset = 0; offset < length; ++offset) {
    auto index = (index_ + offset) % length;
    auto &order_entry = *(order_entry_[index]);
    if (!order_entry.ready()) {
      continue;
    }
    index_ = (index + 1) % length;
    return order_entry;
  }
  throw server::oms::NotReady{"get_next"sv};
}

}  // namespace gateway
}  // namespace bitmex
}  // namespace roq
