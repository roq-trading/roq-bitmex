/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/bitmex/market_data.h"

#include "roq/core/back_emplacer.h"
#include "roq/core/update.h"

#include "roq/core/metrics/factory.h"

#include "roq/bitmex/flags.h"

#include "roq/bitmex/json/utils.h"

using namespace roq::literals;

namespace roq {
namespace bitmex {

namespace {
static const auto CONNECTION = "md"_sv;

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(const std::string_view &group, const std::string_view &function)
      : core::metrics::Factory(Flags::name(), group, function) {}
};

template <typename T>
void emplace(MBPUpdate &result, const T &value) {
  new (&result) MBPUpdate{
      .price = value.first,
      .quantity = value.second,
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
    : handler_(handler), stream_id_(stream_id),
      name_(roq::format("{}_{}"_fmt, CONNECTION, stream_id_)),
      connection_(
          *this,
          context,
          core::URI(Flags::ws_uri()),
          std::string_view(),  // query
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
  (*this)(GatewayStatus::DISCONNECTED);
}

void MarketData::operator()(const core::web::Socket::Ready &) {
  // note! don't notify gateway: wait for handshake
  (*this)(GatewayStatus::LOGIN_SENT);
}

void MarketData::operator()(const core::web::Socket::Close &) {
}

void MarketData::operator()(const core::web::Socket::Latency &latency) {
  server::TraceInfo trace_info;
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .name = name_,
      .latency = latency.sample,
  };
  server::create_trace_and_dispatch(trace_info, external_latency, handler_);
  latency_.ping.update(latency.sample);
}

void MarketData::operator()(const core::web::Socket::Text &text) {
  parse(text.payload);
}

void MarketData::operator()(GatewayStatus status) {
  if (core::update(status_, status)) {
    server::TraceInfo trace_info;
    MarketDataStatus market_data_status{
        .stream_id = stream_id_,
        .status = status_,
    };
    LOG(INFO)("market_data_status={}"_fmt, market_data_status);
    server::create_trace_and_dispatch(trace_info, market_data_status, handler_);
  }
}

void MarketData::send_subscribe(const std::string_view &topic) {
  auto message = roq::format(
      R"({{)"
      R"("op":"subscribe",)"
      R"("args":"{}")"
      R"(}})"_fmt,
      topic);
  DLOG(INFO)(R"(DEBUG: message="{}")"_fmt, message);
  connection_.send_text(message);
}

void MarketData::send_subscribe(const roq::span<std::string_view> &topics) {
  assert(!topics.empty());
  if (std::size(topics) == 1u) {
    send_subscribe(topics[0]);
  } else {
    auto message = roq::format(
        R"({{)"
        R"("op":"subscribe",)"
        R"("args":["{}"])"
        R"(}})"_fmt,
        roq::join(topics, R"(",")"_sv));
    DLOG(INFO)(R"(DEBUG: message="{}")"_fmt, message);
    connection_.send_text(message);
  }
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
      return 1u;
    case MarketDataState::ORDER_BOOK_L2:
      subscribe_order_book_l2();
      return 1u;
    case MarketDataState::DONE:
      (*this)(GatewayStatus::READY);
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
  VLOG(4)(R"(message={})"_fmt, message);
  profile_.parse([&]() {
    try {
      parse_helper(message);
    } catch (std::exception &e) {
      LOG(WARNING)(R"(message="{}")"_fmt, message);
      LOG(FATAL)(R"(ERROR what="{}")"_fmt, e.what());
    }
  });
}

void MarketData::parse_helper(const std::string_view &message) {
  server::TraceInfo trace_info;
  core::json::Buffer buffer(decode_buffer_);
  json::Parser::dispatch(*this, message, buffer, trace_info);
}

void MarketData::operator()(const json::CancelAllAfter &cancel_all_after) {
  profile_.cancel_all_after([&]() { VLOG(1)(R"(cancel_all_after={})"_fmt, cancel_all_after); });
}

void MarketData::operator()(const json::Error &error) {
  profile_.error([&]() { LOG(WARNING)(R"(error={})"_fmt, error); });
  connection_.close();
}

void MarketData::operator()(const json::Handshake &handshake) {
  profile_.handshake([&]() {
    VLOG(1)(R"(handshake={})"_fmt, handshake);
    (*this)(GatewayStatus::DOWNLOADING);
    download_.begin();
  });
}

void MarketData::operator()(const json::Subscribe &subscribe) {
  profile_.subscribe([&]() {
    VLOG(1)(R"(subscribe={})"_fmt, subscribe);
    if (subscribe.success) {
      assert(subscribe.failure == false);
      LOG(INFO)
      (R"(Successfully subscribed to topic="{}")"_fmt, subscribe.subscribe);
    } else if (subscribe.failure) {
      assert(subscribe.success == false);
      LOG(WARNING)(R"(Failed to subscribe topic="{}")"_fmt, subscribe.subscribe);
    } else {
      LOG(FATAL)("Expected success or failure"_sv);
    }
    // TODO(thraneh): clear timeout
  });
}

void MarketData::operator()(
    const json::Action action, const json::Funding &funding, const server::TraceInfo &) {
  profile_.funding([&]() {
    VLOG(2)(R"(action={}, funding={})"_fmt, action, funding);
    // XXX not used
  });
}

void MarketData::operator()(
    const json::Action action,
    const json::Instrument &instrument,
    const server::TraceInfo &trace_info) {
  profile_.instrument([&]() {
    VLOG(2)(R"(action={}, instrument={})"_fmt, action, instrument);
    // note!
    //   first partial update will include *all* instruments
    //   drop everything received before partial (as per bitmex documentation)
    switch (action) {
      case json::Action::UNDEFINED:
      case json::Action::UNKNOWN:
        LOG(FATAL)("Unexpected"_sv);
        break;
      case json::Action::PARTIAL:
        if (partial_received_.instrument) {
          DLOG(INFO)("DEBUG: action={}, instrument={})"_fmt, action, instrument);
          assert(false);  // didn't expect this
        } else {
          partial_received_.instrument = true;
          size_t security_count = {};
          for (auto &item : instrument.data) {
            if (shared_.discard_symbol(item.symbol)) {
              VLOG(1)(R"(Drop symbol="{}")"_fmt, item.symbol);
              continue;
            }
            ++security_count;
            auto &product = find_product(item);
            auto reference_data = product.reference_data(item, stream_id_);
            server::create_trace_and_dispatch(trace_info, reference_data, handler_, false);
            auto market_status = product.market_status(item, stream_id_);
            server::create_trace_and_dispatch(trace_info, market_status, handler_, false);
            auto statistics_update = product.statistics_update(item, stream_id_);
            server::create_trace_and_dispatch(trace_info, statistics_update, handler_, true);
          }
          VLOG(2)
          (R"(- securities: {} (/{}))"_fmt, security_count, instrument.data.size());
          // release download state
          download_.check_relaxed(MarketDataState::INSTRUMENT);
        }
        break;
      case json::Action::INSERT:
        DLOG(INFO)("DEBUG: action={}, instrument={})"_fmt, action, instrument);
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
              auto market_status = product.market_status(item, stream_id_);
              server::create_trace_and_dispatch(trace_info, market_status, handler_, true);
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
    VLOG(2)(R"(action={}, liquidation={})"_fmt, action, liquidation);
    // don't use
  });
}

void MarketData::operator()(
    const json::Action action,
    const json::OrderBookL2 &order_book_l2,
    const server::TraceInfo &trace_info) {
  profile_.order_book_l2([&]() {
    VLOG(3)(R"(action={}, order_book_l2={})"_fmt, action, order_book_l2);
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
        assert(!(bids.empty() && asks.empty()));
        LOG_IF(INFO, snapshot)
        (R"(Received market data snapshot for symbol="{}")"_fmt, previous);
        MarketByPriceUpdate market_by_price_update{
            .stream_id = stream_id_,
            .exchange = Flags::exchange(),
            .symbol = previous,
            .bids = bids,
            .asks = asks,
            .snapshot = snapshot,
            .exchange_time_utc = {},
        };
        VLOG(3)(R"(market_by_price_update={})"_fmt, market_by_price_update);
        server::create_trace_and_dispatch(trace_info, market_by_price_update, handler_, false);
        previous = item.symbol;
        bids.clear();
        asks.clear();
      }
      auto price_size = shared_.price_cache(action, item.id, item.price, item.size);
      if (std::isfinite(price_size.first)) {
        switch (item.side) {
          case json::Side::BUY:
            bids.emplace_back([&price_size](auto &result) { emplace(result, price_size); });
            break;
          case json::Side::SELL:
            asks.emplace_back([&price_size](auto &result) { emplace(result, price_size); });
            break;
          default:
            LOG(FATAL)("Unexpected"_sv);
        }
      } else {
        LOG(WARNING)
        (R"(Closing web-socket: )"
         R"(unexpected action={} id={} price={} size={})"_fmt,
         action,
         item.id,
         item.price,
         item.size);
        connection_.close();
        return;
      }
    }
    assert(!previous.empty());
    assert(!(bids.empty() && asks.empty()));
    LOG_IF(INFO, snapshot)
    (R"(Received market data snapshot for symbol="{}")"_fmt, previous);
    MarketByPriceUpdate market_by_price_update{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = previous,
        .bids = bids,
        .asks = asks,
        .snapshot = snapshot,
        .exchange_time_utc = {},
    };
    VLOG(3)(R"(market_by_price_update={})"_fmt, market_by_price_update);
    server::create_trace_and_dispatch(trace_info, market_by_price_update, handler_, true);
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
    VLOG(3)(R"(action={}, quote={})"_fmt, action, quote);
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
          .snapshot = false,  // XXX ???
          .exchange_time_utc = item.timestamp,
      };
      server::create_trace_and_dispatch(trace_info, top_of_book, handler_, true);
    }
  });
}

void MarketData::operator()(
    const json::Action action, const json::Settlement &settlement, const server::TraceInfo &) {
  profile_.settlement([&]() {
    VLOG(3)(R"(action={}, settlement={})"_fmt, action, settlement);
    // do nothing
  });
}

void MarketData::operator()(
    const json::Action action, const json::Trade &trade, const server::TraceInfo &trace_info) {
  profile_.trade([&]() {
    VLOG(2)(R"(action={}, trade={})"_fmt, action, trade);
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
  LOG(FATAL)(R"(Unexpected: action={}, execution={})"_fmt, action, execution);
}

void MarketData::operator()(
    const json::Action action, const json::Margin &margin, const server::TraceInfo &) {
  LOG(FATAL)(R"(Unexpected: action={}, margin={})"_fmt, action, margin);
}

void MarketData::operator()(
    const json::Action action, const json::Order &order, const server::TraceInfo &) {
  LOG(FATAL)(R"(Unexpected: action={}, order={})"_fmt, action, order);
}

void MarketData::operator()(
    const json::Action action, const json::Position &position, const server::TraceInfo &) {
  LOG(FATAL)(R"(Unexpected: action={}, position={})"_fmt, action, position);
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

}  // namespace bitmex
}  // namespace roq
