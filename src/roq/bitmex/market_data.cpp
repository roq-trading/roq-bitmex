/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/bitmex/market_data.hpp"

#include "roq/mask.hpp"
#include "roq/utils/update.hpp"

#include "roq/core/back_emplacer.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/bitmex/flags.hpp"

#include "roq/bitmex/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace bitmex {

namespace {
const auto NAME = "md"sv;
const Mask SUPPORTS{
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

auto create_connection(auto &handler, auto &context) {
  auto uri = Flags::ws_uri();
  core::web::ClientSocket::Config config{
      .validate_certificate = server::Flags::tls_validate_certificate(),
      .uris = {&uri, 1},
      .query = {},
      .ping_frequency = Flags::ws_ping_freq(),
      .read_buffer_size = Flags::decode_buffer_size(),
      .encode_buffer_size = Flags::encode_buffer_size(),
  };
  return core::web::ClientSocket{handler, context, config, []() { return std::string(); }};
}

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
      .side = json::map(value.side),
      .price = value.price,
      .quantity = value.size,
      .trade_id = value.trd_match_id,
  };
}
}  // namespace

MarketData::MarketData(
    Handler &handler, core::io::Context &context, uint16_t stream_id, Shared &shared)
    : handler_(handler), stream_id_(stream_id), name_(fmt::format("{}:{}"sv, stream_id_, NAME)),
      connection_(create_connection(*this, context)), decode_buffer_(Flags::decode_buffer_size()),
      counter_{
          .disconnect = create_metrics(name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(name_, "parse"sv),
          .cancel_all_after = create_metrics(name_, "cancel_all_after"sv),
          .error = create_metrics(name_, "error"sv),
          .funding = create_metrics(name_, "funding"sv),
          .handshake = create_metrics(name_, "handshake"sv),
          .instrument = create_metrics(name_, "instrument"sv),
          .liquidation = create_metrics(name_, "liquidation"sv),
          .order_book_l2 = create_metrics(name_, "order_book_l2"sv),
          .quote = create_metrics(name_, "quote"sv),
          .settlement = create_metrics(name_, "settlement"sv),
          .subscribe = create_metrics(name_, "subscribe"sv),
          .unsubscribe = create_metrics(name_, "unsubscribe"sv),
          .trade = create_metrics(name_, "trade"sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"sv),
          .heartbeat = create_metrics(name_, "heartbeat"sv),
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

void MarketData::operator()(const core::web::ClientSocket::Connected &) {
  // note! don't notify gateway: wait for ready
}

void MarketData::operator()(const core::web::ClientSocket::Disconnected &) {
  ready_ = false;
  partial_received_ = {};
  download_.reset();
  (*this)(ConnectionStatus::DISCONNECTED);
}

void MarketData::operator()(const core::web::ClientSocket::Ready &) {
  // note! don't notify gateway: wait for handshake
  (*this)(ConnectionStatus::LOGIN_SENT);
}

void MarketData::operator()(const core::web::ClientSocket::Close &) {
}

void MarketData::operator()(const core::web::ClientSocket::Latency &latency) {
  auto trace_info = server::create_trace_info();
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .account = {},
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void MarketData::operator()(const core::web::ClientSocket::Text &text) {
  parse(text.payload);
}

void MarketData::operator()(const core::web::ClientSocket::Binary &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    auto trace_info = server::create_trace_info();
    StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = {},
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::WS,
        .encoding = {Encoding::JSON},
        .priority = Priority::PRIMARY,
        .connection_status = status_,
    };
    log::info("stream_status={}"sv, stream_status);
    create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

void MarketData::send_subscribe(const std::string_view &topic) {
  auto message = fmt::format(
      R"({{)"
      R"("op":"subscribe",)"
      R"("args":"{}")"
      R"(}})"sv,
      topic);
  log::debug(R"(message="{}")"sv, message);
  connection_.send_text(message);
}

void MarketData::send_unsubscribe(const std::string_view &topic) {
  auto message = fmt::format(
      R"({{)"
      R"("op":"unsubscribe",)"
      R"("args":"{}")"
      R"(}})"sv,
      topic);
  log::debug(R"(message="{}")"sv, message);
  connection_.send_text(message);
}

void MarketData::send_subscribe(const std::span<std::string_view> &topics) {
  assert(!std::empty(topics));
  if (std::size(topics) == 1) {
    send_subscribe(topics[0]);
  } else {
    auto message = fmt::format(
        R"({{)"
        R"("op":"subscribe",)"
        R"("args":["{}"])"
        R"(}})"sv,
        fmt::join(topics, R"(",")"sv));
    log::debug(R"(message="{}")"sv, message);
    connection_.send_text(message);
  }
}

void MarketData::send_subscribe(const std::string_view &topic, const std::string_view &symbol) {
  auto message = fmt::format(
      R"({{)"
      R"("op":"subscribe",)"
      R"("args":"{}:{}")"
      R"(}})"sv,
      topic,
      symbol);
  log::debug(R"(message="{}")"sv, message);
  connection_.send_text(message);
}

void MarketData::send_unsubscribe(const std::string_view &topic, const std::string_view &symbol) {
  auto message = fmt::format(
      R"({{)"
      R"("op":"unsubscribe",)"
      R"("args":"{}:{}")"
      R"(}})"sv,
      topic,
      symbol);
  log::debug(R"(message="{}")"sv, message);
  connection_.send_text(message);
}

uint32_t MarketData::download(MarketDataState state) {
  switch (state) {
    using enum MarketDataState;
    case UNDEFINED:
      assert(false);
      break;
    case ACCOUNTS:
      return {};
    case INSTRUMENT:
      subscribe_instrument();
      return 1;
    case ORDER_BOOK_L2:
      subscribe_order_book_l2();
      return 1;
    case DONE:
      (*this)(ConnectionStatus::READY);
      assert(!ready_);
      ready_ = true;
      return {};
  }
  assert(false);
  return {};
}

void MarketData::subscribe_instrument() {
  send_subscribe("instrument"sv);
}

// note! actually "everything else" (than instrument)
void MarketData::subscribe_order_book_l2() {
  std::string_view topics[] = {
      "orderBookL2"sv,
      "funding"sv,
      "liquidation"sv,
      "quote"sv,
      "settlement"sv,
      "trade"sv,
  };
  send_subscribe(topics);
}

void MarketData::parse(const std::string_view &message) {
  log::info<4>(R"(message="{}")"sv, message);
  profile_.parse([&]() {
    try {
      parse_helper(message);
    } catch (...) {
      log::warn(R"(message="{}")"sv, message);
      core::tools::UnhandledException::terminate();
    }
  });
}

void MarketData::parse_helper(const std::string_view &message) {
  auto trace_info = server::create_trace_info();
  core::json::Buffer buffer(decode_buffer_);
  json::StreamParser::dispatch(*this, message, buffer, trace_info);
}

void MarketData::operator()(const Trace<json::CancelAllAfter> &event) {
  profile_.cancel_all_after([&]() {
    auto &[trace_info, cancel_all_after] = event;
    log::info<2>("cancel_all_after={}"sv, cancel_all_after);
  });
}

void MarketData::operator()(const Trace<json::Error> &event) {
  profile_.error([&]() {
    auto &[trace_info, error] = event;
    log::warn("error={}"sv, error);
  });
  connection_.close();
}

void MarketData::operator()(const Trace<json::Handshake> &event) {
  profile_.handshake([&]() {
    auto &[trace_info, handshake] = event;
    log::info<2>("handshake={}"sv, handshake);
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
  });
}

void MarketData::operator()(const Trace<json::Subscribe> &event) {
  profile_.subscribe([&]() {
    auto &[trace_info, subscribe] = event;
    log::info<2>("subscribe={}"sv, subscribe);
    if (subscribe.success) {
      assert(!subscribe.failure);
      log::info(R"(Successfully subscribed to topic="{}")"sv, subscribe.subscribe);
    } else if (subscribe.failure) {
      assert(!subscribe.success);
      log::warn(R"(Failed to subscribe topic="{}")"sv, subscribe.subscribe);
    } else {
      log::fatal("Expected success or failure"sv);
    }
    // TODO(thraneh): clear timeout
  });
}

void MarketData::operator()(const Trace<json::Unsubscribe> &event) {
  profile_.unsubscribe([&]() {
    auto &[trace_info, unsubscribe] = event;
    log::info<2>("unsubscribe={}"sv, unsubscribe);
    if (unsubscribe.success) {
      assert(!unsubscribe.failure);
      log::info(R"(Successfully unsubscribed from topic="{}")"sv, unsubscribe.unsubscribe);
    } else if (unsubscribe.failure) {
      assert(!unsubscribe.success);
      log::warn(R"(Failed to unsubscribe topic="{}")"sv, unsubscribe.unsubscribe);
    } else {
      log::fatal("Expected success or failure"sv);
    }
    // TODO(thraneh): clear timeout
  });
}

void MarketData::operator()(const Trace<json::Funding> &event, json::Action action) {
  profile_.funding([&]() {
    auto &[trace_info, funding] = event;
    log::info<4>("event={{action={}, funding={}}}"sv, action, funding);
    for (auto &item : funding.data) {
      auto &product = find_product(item);
      if (product.update(item)) {
        if (product.is_statistics_dirty()) {
          auto statistics_update = product.statistics_update(item, stream_id_);
          create_trace_and_dispatch(handler_, trace_info, statistics_update, true);
        }
        product.clear();
      }
    }
  });
}

void MarketData::operator()(const Trace<json::Instrument> &event, json::Action action) {
  profile_.instrument([&]() {
    auto &[trace_info, instrument] = event;
    log::info<4>("event={{action={}, instrument={}}}"sv, action, instrument);
    // note!
    //   first partial update will include *all* instruments
    //   drop everything received before partial (as per bitmex documentation)
    switch (action) {
      using enum json::Action::type_t;
      case UNDEFINED:
      case UNKNOWN:
        log::fatal("Unexpected"sv);
        break;
      case PARTIAL:
        if (partial_received_.instrument) {
          log::debug("event={{action={}, instrument={}}}"sv, action, instrument);
          assert(false);  // didn't expect this
        } else {
          partial_received_.instrument = true;
          size_t security_count = {};
          for (auto &item : instrument.data) {
            if (shared_.discard_symbol(item.symbol)) {
              log::info<2>(R"(Drop symbol="{}")"sv, item.symbol);
              continue;
            }
            ++security_count;
            auto &product = find_product(item);
            auto reference_data = product.reference_data(item, stream_id_);
            create_trace_and_dispatch(handler_, trace_info, reference_data, true);
            if (product.is_market_status_dirty()) {
              auto market_status = product.market_status(item, stream_id_);
              create_trace_and_dispatch(handler_, trace_info, market_status, true);
            }
            if (product.is_statistics_dirty()) {
              auto statistics_update = product.statistics_update(item, stream_id_);
              create_trace_and_dispatch(handler_, trace_info, statistics_update, true);
            }
            product.clear();
          }
          log::info<2>("- securities: {} (/{})"sv, security_count, std::size(instrument.data));
          // release download state
          download_.check_relaxed(MarketDataState::INSTRUMENT);
        }
        break;
      case INSERT:
        log::debug("event={{action={}, instrument={}}}"sv, action, instrument);
        assert(false);  // XXX should we just drop these updates?
        break;
      case UPDATE: {
        // only process after "partial" (as per bitmex documentation)
        if (partial_received_.instrument) {
          for (auto &item : instrument.data) {
            if (shared_.discard_symbol(item.symbol))
              continue;
            auto &product = find_product(item);
            if (product.update(item)) {
              if (product.is_market_status_dirty()) {
                auto market_status = product.market_status(item, stream_id_);
                create_trace_and_dispatch(handler_, trace_info, market_status, true);
              }
              if (product.is_statistics_dirty()) {
                auto statistics_update = product.statistics_update(item, stream_id_);
                create_trace_and_dispatch(handler_, trace_info, statistics_update, true);
              }
              product.clear();
            }
          }
        }
        break;
      }
      case DELETE:
        // don't do anything
        break;
    }
  });
}

void MarketData::operator()(const Trace<json::Liquidation> &event, json::Action action) {
  profile_.liquidation([&]() {
    auto &[trace_info, liquidation] = event;
    log::info<4>("event={{action={}, liquidation={}}}"sv, action, liquidation);
    // don't use
  });
}

void MarketData::operator()(const Trace<json::OrderBookL2> &event, json::Action action) {
  profile_.order_book_l2([&]() {
    auto &[trace_info, order_book_l2] = event;
    log::info<4>("event={{action={}, order_book_l2={}}}"sv, action, order_book_l2);
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
      if (std::empty(previous)) {
        previous = item.symbol;
      } else if (previous.compare(item.symbol) != 0) {
        publish_market_by_price(trace_info, false, previous, bids, asks, snapshot);
        previous = item.symbol;
        bids.clear();
        asks.clear();
      }
      auto price_size = shared_.price_cache(action, item.id, item.price, item.size);  // XXX clang13
      auto price = price_size.first;
      auto size = price_size.second;
      if (std::isfinite(price)) {
        switch (item.side) {
          using enum json::Side::type_t;
          case BUY:
            bids.emplace_back([&](auto &result) { emplace(result, price, size); });
            break;
          case SELL:
            asks.emplace_back([&](auto &result) { emplace(result, price, size); });
            break;
          default:
            log::fatal("Unexpected"sv);
        }
      } else {
        log::warn(
            "Closing web-socket: "
            "unexpected action={} id={} price={} size={}"sv,
            action,
            item.id,
            item.price,
            item.size);
        connection_.close();
        return;
      }
    }
    assert(!std::empty(previous));
    publish_market_by_price(trace_info, false, previous, bids, asks, snapshot);
    // state management
    if (snapshot) {
      partial_received_.order_book_l2 = true;
      // release download state
      download_.check_relaxed(MarketDataState::ORDER_BOOK_L2);
    }
  });
}

void MarketData::operator()(const Trace<json::Quote> &event, json::Action action) {
  profile_.quote([&]() {
    auto &[trace_info, quote] = event;
    log::info<4>("event={{action={}, quote={}}}"sv, action, quote);
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
          .update_type = UpdateType::INCREMENTAL,
          .exchange_time_utc = item.timestamp,
          .exchange_sequence = {},
      };
      create_trace_and_dispatch(handler_, trace_info, top_of_book, true);
    }
  });
}

void MarketData::operator()(const Trace<json::Settlement> &event, json::Action action) {
  profile_.settlement([&]() {
    auto &[trace_info, settlement] = event;
    log::info<4>("event={{action={}, settlement={}}}"sv, action, settlement);
    // do nothing
  });
}

void MarketData::operator()(const Trace<json::Trade> &event, json::Action action) {
  profile_.trade([&]() {
    auto &[trace_info, trade] = event;
    log::info<4>("event={{action={}, trade={}}}"sv, action, trade);
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
        if (!std::empty(previous) && !std::empty(trades)) {
          TradeSummary trade_summary{
              .stream_id = stream_id_,
              .exchange = Flags::exchange(),
              .symbol = previous,
              .trades = trades,
              .exchange_time_utc = timestamp,
          };
          create_trace_and_dispatch(handler_, trace_info, trade_summary, false);
        }
        previous = item.symbol;
        trades.clear();
      }
      trades.emplace_back([&item](auto &result) { emplace(result, item); });
    }
    if (!std::empty(previous) && !std::empty(trades)) {
      TradeSummary trade_summary{
          .stream_id = stream_id_,
          .exchange = Flags::exchange(),
          .symbol = previous,
          .trades = trades,
          .exchange_time_utc = timestamp,
      };
      create_trace_and_dispatch(handler_, trace_info, trade_summary, true);
    }
  });
}

void MarketData::operator()(const Trace<json::Execution> &event, json::Action action) {
  auto &[trace_info, execution] = event;
  log::fatal("Unexpected: action={}, execution={}"sv, action, execution);
}

void MarketData::operator()(const Trace<json::Margin> &event, json::Action action) {
  auto &[trace_info, margin] = event;
  log::fatal("Unexpected: action={}, margin={}"sv, action, margin);
}

void MarketData::operator()(const Trace<json::Order> &event, json::Action action) {
  auto &[trace_info, order] = event;
  log::fatal("Unexpected: action={}, order={}"sv, action, order);
}

void MarketData::operator()(const Trace<json::Position> &event, json::Action action) {
  auto &[trace_info, position] = event;
  log::fatal("Unexpected: action={}, position={}"sv, action, position);
}

Product &MarketData::find_product(const json::InstrumentItem &item) {
  auto iter = product_cache_.find(item.symbol);
  if (iter == std::end(product_cache_)) {
    iter = product_cache_.emplace(item.symbol, item).first;
  } else {
    (*iter).second.update(item);
  }
  assert(iter != std::end(product_cache_));
  return (*iter).second;
}

Product &MarketData::find_product(const json::FundingItem &item) {
  auto iter = product_cache_.find(item.symbol);
  if (iter == std::end(product_cache_)) {
    log::warn<1>(R"(Create product symbol="{}" from funding (no reference data))"sv, item.symbol);
    iter = product_cache_.emplace(item.symbol, item).first;
  } else {
    (*iter).second.update(item);
  }
  assert(iter != std::end(product_cache_));
  return (*iter).second;
}

void MarketData::publish_market_by_price(
    const TraceInfo &trace_info,
    bool is_last,
    const std::string_view &symbol,
    const std::span<MBPUpdate> &bids,
    const std::span<MBPUpdate> &asks,
    bool snapshot) {
  assert(!(std::empty(bids) && std::empty(asks)));
  if (snapshot)
    log::info<1>(R"(Received market data snapshot for symbol="{}")"sv, symbol);
  const MarketByPriceUpdate market_by_price_update{
      .stream_id = stream_id_,
      .exchange = Flags::exchange(),
      .symbol = symbol,
      .bids = bids,
      .asks = asks,
      .update_type = snapshot ? UpdateType::SNAPSHOT : UpdateType::INCREMENTAL,
      .exchange_time_utc = {},
      .exchange_sequence = {},
      .price_decimals = {},
      .quantity_decimals = {},
      .checksum = {},
  };
  log::info<3>("market_by_price_update={}"sv, market_by_price_update);
  try {
    create_trace_and_dispatch(handler_, trace_info, market_by_price_update, is_last, false);
  } catch (BadState &) {
    resubscribe_order_book_l2(symbol);
  }
}

void MarketData::resubscribe_order_book_l2(const std::string_view &symbol) {
  log::warn<1>(R"(*** RESUBSCRIBE *** (symbol="{}"))"sv, symbol);
  /* this does not work because we subscribe everything
  send_unsubscribe("orderBookL2"sv, symbol);
  latch_[symbol] = false;
  send_subscribe("orderBookL2"sv, symbol);
  */
  send_unsubscribe("orderBookL2"sv);
  partial_received_.order_book_l2 = false;
  send_subscribe("orderBookL2"sv);
  // note! we should maybe also reset all MbP books here...
}

}  // namespace bitmex
}  // namespace roq
