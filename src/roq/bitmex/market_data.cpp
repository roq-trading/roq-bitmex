/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/bitmex/market_data.h"

#include "roq/utils/mask.h"
#include "roq/utils/update.h"

#include "roq/core/back_emplacer.h"

#include "roq/core/metrics/factory.h"

#include "roq/bitmex/flags.h"

#include "roq/bitmex/json/utils.h"

using namespace roq::literals;

namespace roq {
namespace bitmex {

namespace {
static const auto NAME = "md"_sv;
static const auto SUPPORTS = utils::Mask{
    SupportType::REFERENCE_DATA,
    SupportType::MARKET_STATUS,
    SupportType::TOP_OF_BOOK,
    SupportType::MARKET_BY_PRICE,
    SupportType::TRADE_SUMMARY,
    SupportType::STATISTICS,
};

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(const std::string_view &group, const std::string_view &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

void emplace(MBPUpdate &result, double price, double size) {
  new (&result) MBPUpdate{
      .price = price,
      .quantity = size,
      .implied_quantity = NaN,
      .price_level = {},
      .number_of_orders = {},
  };
}

template <typename T>
void emplace(Trade &result, const T &value) {
  new (&result) Trade{
      .side = json::map(value.side),  // XXX check
      .price = value.price,
      .quantity = value.size,
      .trade_id = value.trd_match_id,
  };
}
}  // namespace

MarketData::MarketData(
    Handler &handler, core::io::Context &context, uint16_t stream_id, Shared &shared)
    : handler_(handler), stream_id_(stream_id), name_(fmt::format("{}:{}"_sv, stream_id_, NAME)),
      connection_(
          *this,
          context,
          core::URI(Flags::ws_uri()),
          {},  // query
          Flags::ws_ping_freq(),
          Flags::decode_buffer_size(),  // XXX need read buffer size
          Flags::encode_buffer_size(),
          []() { return std::string(); }),
      decode_buffer_(Flags::decode_buffer_size()),
      counter_{
          .disconnect = create_metrics(name_, "disconnect"_sv),
      },
      profile_{
          .parse = create_metrics(name_, "parse"_sv),
          .cancel_all_after = create_metrics(name_, "cancel_all_after"_sv),
          .error = create_metrics(name_, "error"_sv),
          .funding = create_metrics(name_, "funding"_sv),
          .handshake = create_metrics(name_, "handshake"_sv),
          .instrument = create_metrics(name_, "instrument"_sv),
          .liquidation = create_metrics(name_, "liquidation"_sv),
          .order_book_l2 = create_metrics(name_, "order_book_l2"_sv),
          .quote = create_metrics(name_, "quote"_sv),
          .settlement = create_metrics(name_, "settlement"_sv),
          .subscribe = create_metrics(name_, "subscribe"_sv),
          .unsubscribe = create_metrics(name_, "unsubscribe"_sv),
          .trade = create_metrics(name_, "trade"_sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"_sv),
          .heartbeat = create_metrics(name_, "heartbeat"_sv),
      },
      shared_(shared),
      download_(Flags::ws_request_timeout(), [this](auto state) { return download(state); }) {
}

void MarketData::operator()(const Event<Start> &) {
  connection_.start();
}

void MarketData::operator()(const Event<Stop> &) {
  connection_.stop();
}

void MarketData::operator()(const Event<Timer> &event) {
  connection_.refresh(event.value.now);
}

void MarketData::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::COUNTER)
      // profile
      .write(profile_.parse, metrics::PROFILE)
      .write(profile_.cancel_all_after, metrics::PROFILE)
      .write(profile_.error, metrics::PROFILE)
      .write(profile_.funding, metrics::PROFILE)
      .write(profile_.handshake, metrics::PROFILE)
      .write(profile_.instrument, metrics::PROFILE)
      .write(profile_.liquidation, metrics::PROFILE)
      .write(profile_.order_book_l2, metrics::PROFILE)
      .write(profile_.quote, metrics::PROFILE)
      .write(profile_.settlement, metrics::PROFILE)
      .write(profile_.subscribe, metrics::PROFILE)
      .write(profile_.unsubscribe, metrics::PROFILE)
      .write(profile_.trade, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY)
      .write(latency_.heartbeat, metrics::LATENCY);
}

void MarketData::operator()(const core::web::Socket::Connected &) {
  // note! don't notify gateway: wait for ready
}

void MarketData::operator()(const core::web::Socket::Disconnected &) {
  ready_ = false;
  partial_received_ = {};
  download_.reset();
  (*this)(ConnectionStatus::DISCONNECTED);
}

void MarketData::operator()(const core::web::Socket::Ready &) {
  // note! don't notify gateway: wait for handshake
  (*this)(ConnectionStatus::LOGIN_SENT);
}

void MarketData::operator()(const core::web::Socket::Close &) {
}

void MarketData::operator()(const core::web::Socket::Latency &latency) {
  server::TraceInfo trace_info;
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .latency = latency.sample,
  };
  server::create_trace_and_dispatch(trace_info, external_latency, handler_);
  latency_.ping.update(latency.sample);
}

void MarketData::operator()(const core::web::Socket::Text &text) {
  parse(text.payload);
}

void MarketData::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    server::TraceInfo trace_info;
    StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = {},
        .supports = SUPPORTS.get(),
        .status = status_,
        .type = StreamType::WEB_SOCKET,
        .priority = Priority::PRIMARY,
    };
    log::info("stream_status={}"_sv, stream_status);
    server::create_trace_and_dispatch(trace_info, stream_status, handler_);
  }
}

void MarketData::send_subscribe(const std::string_view &topic) {
  auto message = fmt::format(
      R"({{)"
      R"("op":"subscribe",)"
      R"("args":"{}")"
      R"(}})"_sv,
      topic);
  log::debug(R"(message="{}")"_sv, message);
  connection_.send_text(message);
}

void MarketData::send_unsubscribe(const std::string_view &topic) {
  auto message = fmt::format(
      R"({{)"
      R"("op":"unsubscribe",)"
      R"("args":"{}")"
      R"(}})"_sv,
      topic);
  log::debug(R"(message="{}")"_sv, message);
  connection_.send_text(message);
}

void MarketData::send_subscribe(const roq::span<std::string_view> &topics) {
  assert(!topics.empty());
  if (std::size(topics) == 1) {
    send_subscribe(topics[0]);
  } else {
    auto message = fmt::format(
        R"({{)"
        R"("op":"subscribe",)"
        R"("args":["{}"])"
        R"(}})"_sv,
        fmt::join(topics, R"(",")"_sv));
    log::debug(R"(message="{}")"_sv, message);
    connection_.send_text(message);
  }
}

void MarketData::send_subscribe(const std::string_view &topic, const std::string_view &symbol) {
  auto message = fmt::format(
      R"({{)"
      R"("op":"subscribe",)"
      R"("args":"{}:{}")"
      R"(}})"_sv,
      topic,
      symbol);
  log::debug(R"(message="{}")"_sv, message);
  connection_.send_text(message);
}

void MarketData::send_unsubscribe(const std::string_view &topic, const std::string_view &symbol) {
  auto message = fmt::format(
      R"({{)"
      R"("op":"unsubscribe",)"
      R"("args":"{}:{}")"
      R"(}})"_sv,
      topic,
      symbol);
  log::debug(R"(message="{}")"_sv, message);
  connection_.send_text(message);
}

uint32_t MarketData::download(MarketDataState state) {
  switch (state) {
    case MarketDataState::UNDEFINED:
      assert(false);
      break;
    case MarketDataState::ACCOUNTS:
      return {};
    case MarketDataState::INSTRUMENT:
      subscribe_instrument();
      return 1;
    case MarketDataState::ORDER_BOOK_L2:
      subscribe_order_book_l2();
      return 1;
    case MarketDataState::DONE:
      (*this)(ConnectionStatus::READY);
      assert(!ready_);
      ready_ = true;
      return {};
  }
  assert(false);
  return {};
}

void MarketData::subscribe_instrument() {
  send_subscribe("instrument"_sv);
}

// note! actually "everything else" (than instrument)
void MarketData::subscribe_order_book_l2() {
  std::string_view topics[] = {
      "orderBookL2"_sv,
      "funding"_sv,
      "liquidation"_sv,
      "quote"_sv,
      "settlement"_sv,
      "trade"_sv,
  };
  send_subscribe(topics);
}

void MarketData::parse(const std::string_view &message) {
  log::info<4>(R"(message="{}")"_sv, message);
  profile_.parse([&]() {
    try {
      parse_helper(message);
    } catch (...) {
      log::warn(R"(message="{}")"_sv, message);
      core::tools::UnhandledException::terminate();
    }
  });
}

void MarketData::parse_helper(const std::string_view &message) {
  server::TraceInfo trace_info;
  core::json::Buffer buffer(decode_buffer_);
  json::StreamParser::dispatch(*this, message, buffer, trace_info);
}

void MarketData::operator()(const json::CancelAllAfter &cancel_all_after) {
  profile_.cancel_all_after([&]() { log::info<2>("cancel_all_after={}"_sv, cancel_all_after); });
}

void MarketData::operator()(const json::Error &error) {
  profile_.error([&]() { log::warn("error={}"_sv, error); });
  connection_.close();
}

void MarketData::operator()(const json::Handshake &handshake) {
  profile_.handshake([&]() {
    log::info<2>("handshake={}"_sv, handshake);
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
  });
}

void MarketData::operator()(const json::Subscribe &subscribe) {
  profile_.subscribe([&]() {
    log::info<2>("subscribe={}"_sv, subscribe);
    if (subscribe.success) {
      assert(!subscribe.failure);
      log::info(R"(Successfully subscribed to topic="{}")"_sv, subscribe.subscribe);
    } else if (subscribe.failure) {
      assert(!subscribe.success);
      log::warn(R"(Failed to subscribe topic="{}")"_sv, subscribe.subscribe);
    } else {
      log::fatal("Expected success or failure"_sv);
    }
    // TODO(thraneh): clear timeout
  });
}

void MarketData::operator()(const json::Unsubscribe &unsubscribe) {
  profile_.unsubscribe([&]() {
    log::info<2>("unsubscribe={}"_sv, unsubscribe);
    if (unsubscribe.success) {
      assert(!unsubscribe.failure);
      log::info(R"(Successfully unsubscribed from topic="{}")"_sv, unsubscribe.unsubscribe);
    } else if (unsubscribe.failure) {
      assert(!unsubscribe.success);
      log::warn(R"(Failed to unsubscribe topic="{}")"_sv, unsubscribe.unsubscribe);
    } else {
      log::fatal("Expected success or failure"_sv);
    }
    // TODO(thraneh): clear timeout
  });
}

void MarketData::operator()(
    const json::Action action, const json::Funding &funding, const server::TraceInfo &trace_info) {
  profile_.funding([&]() {
    log::info<4>("event={{action={}, funding={}}}"_sv, action, funding);
    for (auto &item : funding.data) {
      auto &product = find_product(item);
      if (product.update(item)) {
        if (product.is_statistics_dirty()) {
          auto statistics_update = product.statistics_update(item, stream_id_);
          server::create_trace_and_dispatch(trace_info, statistics_update, handler_, true);
        }
        product.clear();
      }
    }
  });
}

void MarketData::operator()(
    const json::Action action,
    const json::Instrument &instrument,
    const server::TraceInfo &trace_info) {
  profile_.instrument([&]() {
    log::info<4>("event={{action={}, instrument={}}}"_sv, action, instrument);
    // note!
    //   first partial update will include *all* instruments
    //   drop everything received before partial (as per bitmex documentation)
    switch (action) {
      case json::Action::UNDEFINED:
      case json::Action::UNKNOWN:
        log::fatal("Unexpected"_sv);
        break;
      case json::Action::PARTIAL:
        if (partial_received_.instrument) {
          log::debug("event={{action={}, instrument={}}}"_sv, action, instrument);
          assert(false);  // didn't expect this
        } else {
          partial_received_.instrument = true;
          size_t security_count = {};
          for (auto &item : instrument.data) {
            if (shared_.discard_symbol(item.symbol)) {
              log::info<2>(R"(Drop symbol="{}")"_sv, item.symbol);
              continue;
            }
            ++security_count;
            auto &product = find_product(item);
            auto reference_data = product.reference_data(item, stream_id_);
            server::create_trace_and_dispatch(trace_info, reference_data, handler_, true);
            if (product.is_market_status_dirty()) {
              auto market_status = product.market_status(item, stream_id_);
              server::create_trace_and_dispatch(trace_info, market_status, handler_, true);
            }
            if (product.is_statistics_dirty()) {
              auto statistics_update = product.statistics_update(item, stream_id_);
              server::create_trace_and_dispatch(trace_info, statistics_update, handler_, true);
            }
            product.clear();
          }
          log::info<2>("- securities: {} (/{})"_sv, security_count, instrument.data.size());
          // release download state
          download_.check_relaxed(MarketDataState::INSTRUMENT);
        }
        break;
      case json::Action::INSERT:
        log::debug("event={{action={}, instrument={}}}"_sv, action, instrument);
        assert(false);  // XXX should we just drop these updates?
        break;
      case json::Action::UPDATE: {
        // only process after "partial" (as per bitmex documentation)
        if (partial_received_.instrument) {
          for (auto &item : instrument.data) {
            if (shared_.discard_symbol(item.symbol))
              continue;
            auto &product = find_product(item);
            if (product.update(item)) {
              if (product.is_market_status_dirty()) {
                auto market_status = product.market_status(item, stream_id_);
                server::create_trace_and_dispatch(trace_info, market_status, handler_, true);
              }
              if (product.is_statistics_dirty()) {
                auto statistics_update = product.statistics_update(item, stream_id_);
                server::create_trace_and_dispatch(trace_info, statistics_update, handler_, true);
              }
              product.clear();
            }
          }
        }
        break;
      }
      case json::Action::DELETE:
        // don't do anything
        break;
    }
  });
}

void MarketData::operator()(
    const json::Action action, const json::Liquidation &liquidation, const server::TraceInfo &) {
  profile_.liquidation([&]() {
    log::info<4>("event={{action={}, liquidation={}}}"_sv, action, liquidation);
    // don't use
  });
}

void MarketData::operator()(
    const json::Action action,
    const json::OrderBookL2 &order_book_l2,
    const server::TraceInfo &trace_info) {
  profile_.order_book_l2([&]() {
    log::info<4>("event={{action={}, order_book_l2={}}}"_sv, action, order_book_l2);
    assert(action != json::Action::UNKNOWN);
    auto snapshot = action == json::Action::PARTIAL;
    // note!
    //   first partial update will include *all* instruments
    //   drop everything received before partial (as per bitmex documentation)
    if (!snapshot && !partial_received_.order_book_l2)
      return;
    std::string_view previous;
    core::back_emplacer bids(shared_.bids), asks(shared_.asks);
    for (auto &item : order_book_l2.data) {
      if (previous.empty()) {
        previous = item.symbol;
      } else if (previous.compare(item.symbol) != 0) {
        publish_market_by_price(trace_info, false, previous, bids, asks, snapshot);
        previous = item.symbol;
        bids.clear();
        asks.clear();
      }
      auto price_size = shared_.price_cache(action, item.id, item.price, item.size);
      auto price = price_size.first;
      auto size = price_size.second;
      if (std::isfinite(price)) {
        switch (item.side) {
          case json::Side::BUY:
            bids.emplace_back([&](auto &result) { emplace(result, price, size); });
            break;
          case json::Side::SELL:
            asks.emplace_back([&](auto &result) { emplace(result, price, size); });
            break;
          default:
            log::fatal("Unexpected"_sv);
        }
      } else {
        log::warn(
            "Closing web-socket: "
            "unexpected action={} id={} price={} size={}"_sv,
            action,
            item.id,
            item.price,
            item.size);
        connection_.close();
        return;
      }
    }
    assert(!previous.empty());
    publish_market_by_price(trace_info, false, previous, bids, asks, snapshot);
    // state management
    if (snapshot) {
      partial_received_.order_book_l2 = true;
      // release download state
      download_.check_relaxed(MarketDataState::ORDER_BOOK_L2);
    }
  });
}

void MarketData::operator()(
    const json::Action action, const json::Quote &quote, const server::TraceInfo &trace_info) {
  profile_.quote([&]() {
    log::info<4>("event={{action={}, quote={}}}"_sv, action, quote);
    for (auto &item : quote.data) {
      TopOfBook top_of_book{
          .stream_id = stream_id_,
          .exchange = Flags::exchange(),
          .symbol = item.symbol,
          .layer{
              .bid_price = item.bid_price,
              .bid_quantity = item.bid_size,
              .ask_price = item.ask_price,
              .ask_quantity = item.ask_size,
          },
          .update_type = UpdateType::INCREMENTAL,  // XXX ???
          .exchange_time_utc = item.timestamp,
      };
      server::create_trace_and_dispatch(trace_info, top_of_book, handler_, true);
    }
  });
}

void MarketData::operator()(
    const json::Action action, const json::Settlement &settlement, const server::TraceInfo &) {
  profile_.settlement([&]() {
    log::info<4>("event={{action={}, settlement={}}}"_sv, action, settlement);
    // do nothing
  });
}

void MarketData::operator()(
    const json::Action action, const json::Trade &trade, const server::TraceInfo &trace_info) {
  profile_.trade([&]() {
    log::info<4>("event={{action={}, trade={}}}"_sv, action, trade);
    if (action != json::Action::INSERT)
      return;
    std::string_view previous;
    core::back_emplacer trades(shared_.trades);
    std::chrono::nanoseconds timestamp = {};
    for (auto &item : trade.data) {
      if (timestamp == timestamp.zero()) {
        timestamp = item.timestamp;
      } else {
        assert(timestamp == item.timestamp);
      }
      if (item.symbol.compare(previous) != 0) {
        if (!previous.empty() && !trades.empty()) {
          TradeSummary trade_summary{
              .stream_id = stream_id_,
              .exchange = Flags::exchange(),
              .symbol = previous,
              .trades = trades,
              .exchange_time_utc = timestamp,
          };
          server::create_trace_and_dispatch(trace_info, trade_summary, handler_, false);
        }
        previous = item.symbol;
        trades.clear();
      }
      trades.emplace_back([&item](auto &result) { emplace(result, item); });
    }
    if (!previous.empty() && !trades.empty()) {
      TradeSummary trade_summary{
          .stream_id = stream_id_,
          .exchange = Flags::exchange(),
          .symbol = previous,
          .trades = trades,
          .exchange_time_utc = timestamp,
      };
      server::create_trace_and_dispatch(trace_info, trade_summary, handler_, true);
    }
  });
}

void MarketData::operator()(
    const json::Action action, const json::Execution &execution, const server::TraceInfo &) {
  log::fatal("Unexpected: action={}, execution={}"_sv, action, execution);
}

void MarketData::operator()(
    const json::Action action, const json::Margin &margin, const server::TraceInfo &) {
  log::fatal("Unexpected: action={}, margin={}"_sv, action, margin);
}

void MarketData::operator()(
    const json::Action action, const json::Order &order, const server::TraceInfo &) {
  log::fatal("Unexpected: action={}, order={}"_sv, action, order);
}

void MarketData::operator()(
    const json::Action action, const json::Position &position, const server::TraceInfo &) {
  log::fatal("Unexpected: action={}, position={}"_sv, action, position);
}

Product &MarketData::find_product(const json::InstrumentItem &item) {
  auto iter = product_cache_.find(item.symbol);
  if (iter == product_cache_.end()) {
    iter = product_cache_.emplace(item.symbol, item).first;
  } else {
    (*iter).second.update(item);
  }
  assert(iter != product_cache_.end());
  return (*iter).second;
}

Product &MarketData::find_product(const json::FundingItem &item) {
  auto iter = product_cache_.find(item.symbol);
  if (iter == product_cache_.end()) {
    log::warn<1>(R"(Create product symbol="{}" from funding (no reference data))"_sv, item.symbol);
    iter = product_cache_.emplace(item.symbol, item).first;
  } else {
    (*iter).second.update(item);
  }
  assert(iter != product_cache_.end());
  return (*iter).second;
}

void MarketData::publish_market_by_price(
    const server::TraceInfo &trace_info,
    bool is_last,
    const std::string_view &symbol,
    const roq::span<MBPUpdate> &bids,
    const roq::span<MBPUpdate> &asks,
    bool snapshot) {
  assert(!(bids.empty() && asks.empty()));
  if (ROQ_UNLIKELY(snapshot)) {
    log::info<1>(R"(Received market data snapshot for symbol="{}")"_sv, symbol);
  }
  const MarketByPriceUpdate market_by_price_update{
      .stream_id = stream_id_,
      .exchange = Flags::exchange(),
      .symbol = symbol,
      .bids = bids,
      .asks = asks,
      .update_type = snapshot ? UpdateType::SNAPSHOT : UpdateType::INCREMENTAL,
      .exchange_time_utc = {},
  };
  log::info<3>("market_by_price_update={}"_sv, market_by_price_update);
  try {
    server::create_trace_and_dispatch(trace_info, market_by_price_update, handler_, is_last, false);
  } catch (market::BadState &) {
    resubscribe_order_book_l2(symbol);
  }
}

void MarketData::resubscribe_order_book_l2(const std::string_view &symbol) {
  log::warn<1>(R"(*** RESUBSCRIBE *** (symbol="{}"))"_sv, symbol);
  /* this does not work because we subscribe everything
  send_unsubscribe("orderBookL2"_sv, symbol);
  latch_[symbol] = false;
  send_subscribe("orderBookL2"_sv, symbol);
  */
  send_unsubscribe("orderBookL2"_sv);
  partial_received_.order_book_l2 = false;
  send_subscribe("orderBookL2"_sv);
  // note! we should maybe also reset all MbP books here...
}

}  // namespace bitmex
}  // namespace roq
