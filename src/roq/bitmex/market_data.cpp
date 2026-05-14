/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/bitmex/market_data.hpp"

#include <algorithm>
#include <utility>

#include "roq/mask.hpp"

#include "roq/utils/update.hpp"

#include "roq/utils/exceptions/unhandled.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/bitmex/json/map.hpp"
#include "roq/bitmex/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace bitmex {

// === CONSTANTS ===

namespace {
auto const NAME = "md"sv;

auto const SUPPORTS = Mask{
    SupportType::REFERENCE_DATA,
    SupportType::MARKET_STATUS,
    SupportType::TOP_OF_BOOK,
    SupportType::MARKET_BY_PRICE,
    SupportType::TRADE_SUMMARY,
    SupportType::STATISTICS,
};

size_t const MAX_DECODE_BUFFER_DEPTH = 1;
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id) {
  return fmt::format("{}:{}"sv, stream_id, NAME);
}

auto create_connection(auto &handler, auto &settings, auto &context) {
  auto uri = settings.ws.uri;
  auto config = web::socket::Client::Config{
      // connection
      .interface = {},
      .uris = {&uri, 1},
      .host = {},
      .validate_certificate = settings.net.tls_validate_certificate,
      // connection manager
      .connection_timeout = settings.net.connection_timeout,
      .disconnect_on_idle_timeout = settings.net.disconnect_on_idle_timeout,
      .always_reconnect = true,
      // proxy
      .proxy = {},
      // http
      .user_agent = ROQ_PACKAGE_NAME,
      .request_timeout = {},
      .ping_frequency = settings.ws.ping_freq,
      // implementation
      .decode_buffer_size = settings.misc.decode_buffer_size,
      .encode_buffer_size = settings.misc.encode_buffer_size,
  };
  return web::socket::Client::create(handler, context, config, []() { return std::string(); });
}

struct create_metrics final : public utils::metrics::Factory {
  create_metrics(auto &settings, auto &group, auto const &function) : utils::metrics::Factory{settings.app.name, group, function} {}
};
}  // namespace

// === IMPLEMENTATION ===

MarketData::MarketData(Handler &handler, io::Context &context, uint16_t stream_id, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_)}, connection_{create_connection(*this, shared.settings, context)},
      decode_buffer_{shared.settings.misc.decode_buffer_size, MAX_DECODE_BUFFER_DEPTH},
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(shared.settings, name_, "parse"sv),
          .welcome = create_metrics(shared.settings, name_, "welcome"sv),
          .error = create_metrics(shared.settings, name_, "error"sv),
          .subscribe = create_metrics(shared.settings, name_, "subscribe"sv),
          .unsubscribe = create_metrics(shared.settings, name_, "unsubscribe"sv),
          .instrument = create_metrics(shared.settings, name_, "instrument"sv),
          .quote = create_metrics(shared.settings, name_, "quote"sv),
          .order_book_l2 = create_metrics(shared.settings, name_, "order_book_l2"sv),
          .trade = create_metrics(shared.settings, name_, "trade"sv),
          .funding = create_metrics(shared.settings, name_, "funding"sv),
          .liquidation = create_metrics(shared.settings, name_, "liquidation"sv),
          .settlement = create_metrics(shared.settings, name_, "settlement"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
          .heartbeat = create_metrics(shared.settings, name_, "heartbeat"sv),
      },
      shared_{shared}, download_{shared.settings.ws.request_timeout, [this](auto state) { return download(state); }} {
}

void MarketData::operator()(Event<Start> const &) {
  (*connection_).start();
}

void MarketData::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void MarketData::operator()(Event<Timer> const &event) {
  (*connection_).refresh(event.value.now);
}

void MarketData::operator()(metrics::Writer &writer) const {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.parse, metrics::Type::PROFILE)
      .write(profile_.welcome, metrics::Type::PROFILE)
      .write(profile_.error, metrics::Type::PROFILE)
      .write(profile_.subscribe, metrics::Type::PROFILE)
      .write(profile_.unsubscribe, metrics::Type::PROFILE)
      .write(profile_.instrument, metrics::Type::PROFILE)
      .write(profile_.quote, metrics::Type::PROFILE)
      .write(profile_.order_book_l2, metrics::Type::PROFILE)
      .write(profile_.trade, metrics::Type::PROFILE)
      .write(profile_.funding, metrics::Type::PROFILE)
      .write(profile_.liquidation, metrics::Type::PROFILE)
      .write(profile_.settlement, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY)
      .write(latency_.heartbeat, metrics::Type::LATENCY);
}

void MarketData::operator()(web::socket::Client::Connected const &) {
  // note! don't notify gateway: wait for ready
}

void MarketData::operator()(web::socket::Client::Disconnected const &) {
  ready_ = false;
  partial_received_ = {};
  download_.reset();
  (*this)(ConnectionStatus::DISCONNECTED);
}

void MarketData::operator()(web::socket::Client::Ready const &) {
  // note! don't notify gateway: wait for handshake
  (*this)(ConnectionStatus::LOGIN_SENT);
}

void MarketData::operator()(web::socket::Client::Close const &) {
}

void MarketData::operator()(web::socket::Client::Latency const &latency) {
  TraceInfo trace_info;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = {},
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void MarketData::operator()(web::socket::Client::Text const &text) {
  parse(text.payload);
}

void MarketData::operator()(web::socket::Client::Binary const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(ConnectionStatus connection_status, std::string_view const &reason) {
  connection_status_ = connection_status;
  TraceInfo trace_info;
  auto stream_status = StreamStatus{
      .stream_id = stream_id_,
      .account = {},
      .supports = SUPPORTS,
      .transport = Transport::TCP,
      .protocol = Protocol::WS,
      .encoding = {Encoding::JSON},
      .priority = Priority::PRIMARY,
      .connection_status = connection_status_,
      .reason = reason,
      .interface = (*connection_).get_interface(),
      .authority = (*connection_).get_current_authority(),
      .path = (*connection_).get_current_path(),
      .proxy = (*connection_).get_proxy(),
  };
  log::info("stream_status={}"sv, stream_status);
  create_trace_and_dispatch(handler_, trace_info, stream_status);
}

void MarketData::send_subscribe(std::string_view const &topic) {
  auto message = fmt::format(
      R"({{)"
      R"("op":"subscribe",)"
      R"("args":"{}")"
      R"(}})"sv,
      topic);
  (*connection_).send_text(message);
}

void MarketData::send_unsubscribe(std::string_view const &topic) {
  auto message = fmt::format(
      R"({{)"
      R"("op":"unsubscribe",)"
      R"("args":"{}")"
      R"(}})"sv,
      topic);
  (*connection_).send_text(message);
}

void MarketData::send_subscribe(std::span<std::string_view> const &topics) {
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
    (*connection_).send_text(message);
  }
}

void MarketData::send_subscribe(std::string_view const &topic, std::string_view const &symbol) {
  auto message = fmt::format(
      R"({{)"
      R"("op":"subscribe",)"
      R"("args":"{}:{}")"
      R"(}})"sv,
      topic,
      symbol);
  (*connection_).send_text(message);
}

void MarketData::send_unsubscribe(std::string_view const &topic, std::string_view const &symbol) {
  auto message = fmt::format(
      R"({{)"
      R"("op":"unsubscribe",)"
      R"("args":"{}:{}")"
      R"(}})"sv,
      topic,
      symbol);
  (*connection_).send_text(message);
}

uint32_t MarketData::download(State state) {
  switch (state) {
    using enum State;
    case UNDEFINED:
      assert(false);
      break;
    case ACCOUNTS:
      (*this)(ConnectionStatus::DOWNLOADING, "accounts"sv);
      return 0;
    case INSTRUMENT:
      (*this)(ConnectionStatus::DOWNLOADING, "instrument"sv);
      subscribe_instrument();
      return 1;
    case ORDER_BOOK_L2:
      (*this)(ConnectionStatus::DOWNLOADING, "order-book-l2"sv);
      subscribe_order_book_l2();
      return 1;
    case DONE:
      (*this)(ConnectionStatus::READY);
      assert(!ready_);
      ready_ = true;
      return 0;
  }
  assert(false);
  return 0;
}

void MarketData::subscribe_instrument() {
  send_subscribe("instrument"sv);
}

// note! actually "everything else" (than instrument)
void MarketData::subscribe_order_book_l2() {
  std::array<std::string_view, 6> topics{{
      "quote"sv,
      "orderBookL2"sv,
      "trade"sv,
      "funding"sv,
      "liquidation"sv,
      "settlement"sv,
  }};
  send_subscribe(topics);
}

void MarketData::parse(std::string_view const &message) {
  profile_.parse([&]() {
    auto log_message = [&]() { log::warn(R"(*** PLEASE REPORT *** message="{}")"sv, message); };
    try {
      TraceInfo trace_info;
      if (!json::Parser::dispatch(*this, message, decode_buffer_, trace_info, shared_.settings.experimental.allow_unknown_event_types)) {
        log_message();
      }
    } catch (...) {
      log_message();
      utils::exceptions::Unhandled::terminate();
    }
  });
}

void MarketData::operator()(Trace<json::Welcome> const &event) {
  profile_.welcome([&]() {
    auto &[trace_info, welcome] = event;
    log::info<2>("welcome={}"sv, welcome);
    (*connection_).touch(trace_info.source_receive_time);
    download_.begin();
  });
}

void MarketData::operator()(Trace<json::Error> const &event) {
  profile_.error([&]() {
    auto &[trace_info, error] = event;
    log::warn("error={}"sv, error);
  });
  (*connection_).close();
}

void MarketData::operator()(Trace<json::Subscribe> const &event) {
  profile_.subscribe([&]() {
    auto &[trace_info, subscribe] = event;
    log::info<2>("subscribe={}"sv, subscribe);
    if (subscribe.success) {
      log::info(R"(Successfully subscribed to topic="{}")"sv, subscribe.subscribe);
    } else {
      log::warn(R"(Failed to subscribe topic="{}")"sv, subscribe.subscribe);
    }
  });
}

void MarketData::operator()(Trace<json::Unsubscribe> const &event) {
  profile_.unsubscribe([&]() {
    auto &[trace_info, unsubscribe] = event;
    log::info<2>("unsubscribe={}"sv, unsubscribe);
    if (unsubscribe.success) {
      log::info(R"(Successfully unsubscribed from topic="{}")"sv, unsubscribe.unsubscribe);
    } else {
      log::warn(R"(Failed to unsubscribe topic="{}")"sv, unsubscribe.unsubscribe);
    }
  });
}

void MarketData::operator()(Trace<json::Instrument> const &event) {
  profile_.instrument([&]() {
    auto &[trace_info, instrument] = event;
    log::info<4>("instrument={}"sv, instrument);
    (*connection_).touch(trace_info.source_receive_time);
    // note!
    //   first partial update will include *all* instruments
    //   drop everything received before partial (as per bitmex documentation)
    switch (instrument.action) {
      using enum json::Action::type_t;
      case UNDEFINED_INTERNAL:
      case UNKNOWN_INTERNAL:
        log::fatal("Unexpected"sv);
        break;
      case PARTIAL:
        if (partial_received_.instrument) {
          assert(false);  // didn't expect this
        } else {
          partial_received_.instrument = true;
          size_t security_count = {};
          for (auto &item : instrument.data) {
            auto discard = shared_.discard_symbol(item.symbol);
            auto &product = find_product(item);
            auto reference_data = product.reference_data(item, stream_id_, discard);
            create_trace_and_dispatch(handler_, trace_info, reference_data, true);
            if (discard) {
              log::info<2>(R"(Drop symbol="{}")"sv, item.symbol);
              continue;
            }
            ++security_count;
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
          download_.check_relaxed(State::INSTRUMENT);
        }
        break;
      case INSERT:
        assert(false);  // XXX should we just drop these updates?
        break;
      case UPDATE: {
        // only process after "partial" (as per bitmex documentation)
        if (partial_received_.instrument) {
          for (auto &item : instrument.data) {
            if (shared_.discard_symbol(item.symbol)) {
              continue;
            }
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

void MarketData::operator()(Trace<json::Quote> const &event) {
  profile_.quote([&]() {
    auto &[trace_info, quote] = event;
    log::info<4>("quote={}"sv, quote);
    (*connection_).touch(trace_info.source_receive_time);
    for (auto &item : quote.data) {
      if (shared_.discard_symbol(item.symbol)) {
        continue;
      }
      auto top_of_book = TopOfBook{
          .stream_id = stream_id_,
          .exchange = shared_.settings.exchange,
          .symbol = item.symbol,
          .layer{
              .bid_price = item.bid_price,
              .bid_quantity = item.bid_size,
              .ask_price = item.ask_price,
              .ask_quantity = item.ask_size,
          },
          .update_type = UpdateType::SNAPSHOT,
          .exchange_time_utc = item.timestamp,  // XXX not sure
          .exchange_sequence = {},
          .sending_time_utc = {},
      };
      create_trace_and_dispatch(handler_, trace_info, top_of_book, true);
    }
  });
}

void MarketData::operator()(Trace<json::OrderBookL2> const &event) {
  profile_.order_book_l2([&]() {
    auto &[trace_info, order_book_l2] = event;
    log::info<4>("order_book_l2={}"sv, order_book_l2);
    (*connection_).touch(trace_info.source_receive_time);
    assert(order_book_l2.action != json::Action::UNKNOWN_INTERNAL);
    auto snapshot = order_book_l2.action == json::Action::PARTIAL;
    // note!
    //   first partial update will include *all* instruments
    //   drop everything received before partial (as per bitmex documentation)
    if (!snapshot && !partial_received_.order_book_l2) {
      return;
    }
    bool discard = true;
    std::string_view previous;
    std::chrono::nanoseconds timestamp = {};
    shared_.bids.clear();
    shared_.asks.clear();
    auto emplace_back = [](auto &result, auto price, auto size) {
      auto mbp_update = MBPUpdate{
          .price = price,
          .quantity = size,
          .implied_quantity = NaN,
          .number_of_orders = {},
          .update_action = {},
          .price_level = {},
      };
      result.emplace_back(std::move(mbp_update));
    };
    auto publish = [&]() {
      if (!std::empty(previous) && !(std::empty(shared_.bids) && std::empty(shared_.asks))) {
        publish_market_by_price(trace_info, false, previous, shared_.bids, shared_.asks, snapshot, timestamp);
      }
    };
    for (auto &item : order_book_l2.data) {
      if (std::empty(previous)) {
        previous = item.symbol;
        discard = shared_.discard_symbol(previous);
      } else if (previous.compare(item.symbol) != 0) {
        publish();
        previous = item.symbol;
        discard = shared_.discard_symbol(previous);
        timestamp = {};
        shared_.bids.clear();
        shared_.asks.clear();
      }
      if (discard) {
        continue;
      }
      utils::update_max(timestamp, item.timestamp);
      auto [price, size] = shared_.price_cache(order_book_l2.action, item.id, item.price, item.size);
      if (!std::isnan(price)) {
        switch (item.side) {
          using enum json::Side::type_t;
          case BUY:
            emplace_back(shared_.bids, price, size);
            break;
          case SELL:
            emplace_back(shared_.asks, price, size);
            break;
          default:
            log::fatal("Unexpected"sv);
        }
      } else {
        log::warn("Unexpected: action={}, item={}, cache={{price={}, size={}}}"sv, order_book_l2.action, item, price, size);
        (*connection_).close();
        log::warn("Closing web-socket"sv);
        return;
      }
    }
    if (!std::empty(previous) && !(std::empty(shared_.bids) && std::empty(shared_.asks))) {
      publish();
    }
    // state management
    if (snapshot) {
      partial_received_.order_book_l2 = true;
      // release download state
      download_.check_relaxed(State::ORDER_BOOK_L2);
    }
  });
}

void MarketData::operator()(Trace<json::Trade> const &event) {
  profile_.trade([&]() {
    auto &trace_info = event.trace_info;
    auto &trade = event.value;
    log::info<4>("trade={}"sv, trade);
    (*connection_).touch(trace_info.source_receive_time);
    if (trade.action != json::Action::INSERT) {
      return;
    }
    shared_.trades.clear();
    auto emplace_back = [](auto &result, auto &value) {
      auto trade = Trade{
          .side = map(value.side),
          .price = value.price,
          .quantity = value.size,
          .trade_id = value.trd_match_id,
          .taker_order_id = {},
          .maker_order_id = {},
      };
      result.emplace_back(std::move(trade));
    };
    std::chrono::nanoseconds timestamp = {};
    auto dispatch = [&](auto &symbol, auto is_last) {
      auto trade_summary = TradeSummary{
          .stream_id = stream_id_,
          .exchange = shared_.settings.exchange,
          .symbol = symbol,
          .trades = shared_.trades,
          .exchange_time_utc = timestamp,  // XXX not sure
          .exchange_sequence = {},
          .sending_time_utc = {},
      };
      create_trace_and_dispatch(handler_, trace_info, trade_summary, is_last);
    };
    std::string_view previous;
    for (auto &item : trade.data) {
      timestamp = std::max(timestamp, std::chrono::duration_cast<decltype(timestamp)>(item.timestamp));
      if (item.symbol.compare(previous) != 0) {
        if (!std::empty(previous) && !std::empty(shared_.trades)) {
          if (!shared_.discard_symbol(previous)) {
            dispatch(previous, false);
          }
        }
        previous = item.symbol;
        shared_.trades.clear();
        timestamp = {};
      }
      emplace_back(shared_.trades, item);
    }
    if (!std::empty(previous) && !std::empty(shared_.trades)) {
      if (!shared_.discard_symbol(previous)) {
        dispatch(previous, true);
      }
    }
  });
}

void MarketData::operator()(Trace<json::Funding> const &event) {
  profile_.funding([&]() {
    auto &[trace_info, funding] = event;
    log::info<4>("funding={}"sv, funding);
    (*connection_).touch(trace_info.source_receive_time);
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

void MarketData::operator()(Trace<json::Liquidation> const &event) {
  profile_.liquidation([&]() {
    auto &[trace_info, liquidation] = event;
    log::info<4>("liquidation={}"sv, liquidation);
    (*connection_).touch(trace_info.source_receive_time);
    // don't use
  });
}

void MarketData::operator()(Trace<json::Settlement> const &event) {
  profile_.settlement([&]() {
    auto &[trace_info, settlement] = event;
    log::info<4>("settlement={}"sv, settlement);
    (*connection_).touch(trace_info.source_receive_time);
    // do nothing
  });
}

void MarketData::operator()(Trace<json::Margin> const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(Trace<json::Position> const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(Trace<json::Order> const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(Trace<json::Execution> const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(Trace<json::CancelAllAfter> const &) {
  log::fatal("Unexpected"sv);
}

Product &MarketData::find_product(json::InstrumentDataItem const &item) {
  auto iter = product_cache_.find(item.symbol);
  if (iter == std::end(product_cache_)) {
    iter = product_cache_.try_emplace(item.symbol, shared_, item).first;
  } else {
    (*iter).second.update(item);
  }
  assert(iter != std::end(product_cache_));
  return (*iter).second;
}

Product &MarketData::find_product(json::FundingDataItem const &item) {
  auto iter = product_cache_.find(item.symbol);
  if (iter == std::end(product_cache_)) {
    log::warn<1>(R"(Create product symbol="{}" from funding (no reference data))"sv, item.symbol);
    iter = product_cache_.try_emplace(item.symbol, shared_, item).first;
  } else {
    (*iter).second.update(item);
  }
  assert(iter != std::end(product_cache_));
  return (*iter).second;
}

void MarketData::publish_market_by_price(
    TraceInfo const &trace_info,
    bool is_last,
    std::string_view const &symbol,
    std::span<MBPUpdate> const &bids,
    std::span<MBPUpdate> const &asks,
    bool snapshot,
    std::chrono::nanoseconds const exchange_time_utc) {
  assert(!(std::empty(bids) && std::empty(asks)));
  if (shared_.discard_symbol(symbol)) {
    return;
  }
  if (snapshot) {
    log::info<1>(R"(Received market data snapshot for symbol="{}")"sv, symbol);
  }
  auto market_by_price_update = MarketByPriceUpdate{
      .stream_id = stream_id_,
      .exchange = shared_.settings.exchange,
      .symbol = symbol,
      .bids = bids,
      .asks = asks,
      .update_type = snapshot ? UpdateType::SNAPSHOT : UpdateType::INCREMENTAL,
      .exchange_time_utc = exchange_time_utc,
      .exchange_sequence = {},
      .sending_time_utc = {},
      .price_precision = {},
      .quantity_precision = {},
      .checksum = {},
  };
  log::info<3>("market_by_price_update={}"sv, market_by_price_update);
  try {
    create_trace_and_dispatch(handler_, trace_info, market_by_price_update, is_last);
  } catch (BadState &) {
    resubscribe_order_book_l2(symbol);
  }
}

void MarketData::resubscribe_order_book_l2(std::string_view const &symbol) {
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
