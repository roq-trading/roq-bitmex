/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/bitmex/drop_copy.hpp"

#include <algorithm>
#include <utility>

#include "roq/mask.hpp"

#include "roq/utils/update.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/tools/exception.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/bitmex/order_update.hpp"
#include "roq/bitmex/utils.hpp"

#include "roq/bitmex/json/utils.hpp"

using namespace std::literals;
using namespace std::chrono_literals;  // NOLINT

namespace roq {
namespace bitmex {

// === CONSTANTS ===

namespace {
auto const NAME = "ex"sv;

auto const SUPPORTS = Mask{
    SupportType::ORDER_ACK,
    SupportType::ORDER,
    SupportType::TRADE,
    SupportType::POSITION,
};

auto const REQUEST_EXPIRES = 5s;
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id, auto const &account) {
  return fmt::format("{}:{}:{}"sv, stream_id, NAME, account);
}

auto create_connection(auto &handler, auto &settings, auto &context, auto &&create_upgrade_headers) {
  auto uri = settings.ws.uri;
  auto config = web::socket::Client::Config{
      // connection
      .interface = {},
      .uris = {&uri, 1},
      .host = {},
      .validate_certificate = settings.net.tls_validate_certificate,
      // connection manager
      .connection_timeout = settings.net.connection_timeout,
      .disconnect_on_idle_timeout = {},
      .always_reconnect = true,
      // proxy
      .proxy = {},
      // http
      .query = {},
      .user_agent = ROQ_PACKAGE_NAME,
      .request_timeout = {},
      .ping_frequency = settings.ws.ping_freq,
      // implementation
      .decode_buffer_size = settings.misc.decode_buffer_size,
      .encode_buffer_size = settings.misc.encode_buffer_size,
  };
  return web::socket::Client::create(handler, context, config, std::move(create_upgrade_headers));
}

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(auto &settings, auto const &group, auto const &function)
      : core::metrics::Factory(settings.app.name, group, function) {}
};
}  // namespace

// === IMPLEMENTATION ===

DropCopy::DropCopy(Handler &handler, io::Context &context, uint16_t stream_id, Account &account, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_, account.get_name())},
      connection_{create_connection(*this, shared.settings, context, [this]() { return create_upgrade_headers(); })},
      decode_buffer_(shared.settings.misc.decode_buffer_size),
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(shared.settings, name_, "parse"sv),
          .cancel_all_after = create_metrics(shared.settings, name_, "cancel_all_after"sv),
          .error = create_metrics(shared.settings, name_, "error"sv),
          .execution = create_metrics(shared.settings, name_, "execution"sv),
          .handshake = create_metrics(shared.settings, name_, "handshake"sv),
          .margin = create_metrics(shared.settings, name_, "margin"sv),
          .order = create_metrics(shared.settings, name_, "order"sv),
          .position = create_metrics(shared.settings, name_, "position"sv),
          .subscribe = create_metrics(shared.settings, name_, "subscribe"sv),
          .unsubscribe = create_metrics(shared.settings, name_, "unsubscribe"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
          .heartbeat = create_metrics(shared.settings, name_, "heartbeat"sv),
      },
      account_{account}, shared_{shared},
      download_{shared.settings.ws.request_timeout, [this](auto state) { return download(state); }} {
}

void DropCopy::operator()(Event<Start> const &) {
  (*connection_).start();
}

void DropCopy::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void DropCopy::operator()(Event<Timer> const &event) {
  if (!(*connection_).refresh(event.value.now))
    return;
  if (shared_.settings.ws.cancel_on_disconnect && shared_.settings.ws.cancel_all_after.count() && ready_ &&
      next_cancel_all_after_ <= event.value.now) {
    next_cancel_all_after_ = event.value.now + shared_.settings.ws.cancel_all_after / 4;
    send_cancel_all_after(shared_.settings.ws.cancel_all_after);
  }
}

void DropCopy::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.parse, metrics::Type::PROFILE)
      .write(profile_.cancel_all_after, metrics::Type::PROFILE)
      .write(profile_.error, metrics::Type::PROFILE)
      .write(profile_.execution, metrics::Type::PROFILE)
      .write(profile_.handshake, metrics::Type::PROFILE)
      .write(profile_.margin, metrics::Type::PROFILE)
      .write(profile_.order, metrics::Type::PROFILE)
      .write(profile_.position, metrics::Type::PROFILE)
      .write(profile_.subscribe, metrics::Type::PROFILE)
      .write(profile_.unsubscribe, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY)
      .write(latency_.heartbeat, metrics::Type::LATENCY);
}

void DropCopy::operator()(web::socket::Client::Connected const &) {
  // note! don't notify gateway: wait for ready
}

void DropCopy::operator()(web::socket::Client::Disconnected const &) {
  ready_ = false;
  next_cancel_all_after_ = {};
  partial_received_ = {};
  download_.reset();
  (*this)(ConnectionStatus::DISCONNECTED);
}

void DropCopy::operator()(web::socket::Client::Ready const &) {
  // note! don't notify gateway: wait for handshake
  (*this)(ConnectionStatus::LOGIN_SENT);
}

void DropCopy::operator()(web::socket::Client::Close const &) {
}

void DropCopy::operator()(web::socket::Client::Latency const &latency) {
  TraceInfo trace_info;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = account_.get_name(),
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void DropCopy::operator()(web::socket::Client::Text const &text) {
  parse(text.payload);
}

void DropCopy::operator()(web::socket::Client::Binary const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    TraceInfo trace_info;
    auto stream_status = StreamStatus{
        .stream_id = stream_id_,
        .account = account_.get_name(),
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::WS,
        .encoding = {Encoding::JSON},
        .priority = Priority::PRIMARY,
        .connection_status = status_,
        .interface = (*connection_).get_interface(),
        .authority = (*connection_).get_current_authority(),
        .path = (*connection_).get_current_path(),
        .proxy = (*connection_).get_proxy(),
    };
    log::info("stream_status={}"sv, stream_status);
    create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

void DropCopy::send_cancel_all_after(std::chrono::nanoseconds timeout) {
  auto message = fmt::format(
      R"({{)"
      R"("op":"cancelAllAfter",)"
      R"("args":{})"
      R"(}})"sv,
      std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());
  (*connection_).send_text(message);
}

void DropCopy::send_subscribe(std::string_view const &topic) {
  auto message = fmt::format(
      R"({{)"
      R"("op":"subscribe",)"
      R"("args":"{}")"
      R"(}})"sv,
      topic);
  log::debug(R"(message="{}")"sv, message);
  (*connection_).send_text(message);
}

void DropCopy::send_subscribe(std::span<std::string_view> const &topics) {
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
    (*connection_).send_text(message);
  }
}

uint32_t DropCopy::download(DropCopyState state) {
  switch (state) {
    using enum DropCopyState;
    case UNDEFINED:
      assert(false);
      break;
    case SUBSCRIBE:
      subscribe();
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

void DropCopy::subscribe() {
  std::string_view topics[] = {
      "execution"sv,
      "order"sv,
      "margin"sv,
      "position"sv,
  };
  send_subscribe(topics);
  // XXX other topics?
  // cancelAllAfter
  // authKeyExpires
}

void DropCopy::parse(std::string_view const &message) {
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

void DropCopy::parse_helper(std::string_view const &message) {
  TraceInfo trace_info;
  json::StreamParser::dispatch(*this, message, decode_buffer_, trace_info);
}

void DropCopy::operator()(Trace<json::CancelAllAfter> const &event) {
  profile_.cancel_all_after([&]() {
    auto &[trace_info, cancel_all_after] = event;
    log::info<2>("cancel_all_after={}"sv, cancel_all_after);
  });
}

void DropCopy::operator()(Trace<json::Error> const &event) {
  profile_.error([&]() {
    auto &[trace_info, error] = event;
    log::warn("error={}"sv, error);
  });
  (*connection_).close();
}

void DropCopy::operator()(Trace<json::Handshake> const &event) {
  profile_.handshake([&]() {
    auto &[trace_info, handshake] = event;
    log::info<2>("handshake={}"sv, handshake);
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
    if (!shared_.settings.ws.cancel_on_disconnect || shared_.settings.ws.cancel_all_after.count() == 0)
      send_cancel_all_after(std::chrono::seconds{});
  });
}

void DropCopy::operator()(Trace<json::Subscribe> const &event) {
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

void DropCopy::operator()(Trace<json::Unsubscribe> const &event) {
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

void DropCopy::operator()(Trace<json::Execution> const &event, json::Action action) {
  profile_.execution([&]() {
    auto &trace_info = event.trace_info;
    auto &execution = event.value;
    log::info<2>("event={{action={}, execution={}}}"sv, action, execution);
    for (auto &item : execution.data) {
      auto order_status = json::map(item.ord_status);
      auto side = json::map(item.side);
      auto external_account = item.account ? fmt::format("{}"sv, item.account) : std::string{};
      auto order_type = json::map(item.ord_type);
      auto time_in_force = json::map(item.time_in_force);
      auto last_liquidity = json::map(item.last_liquidity_ind);
      auto request_type = compute_request_type(item.exec_type);
      auto request_status = compute_request_status(item.exec_type);
      auto error = json::guess_error(item.ord_rej_reason);
      // cancel order does not allow passing a custom id
      auto request_id = request_type != RequestType::CANCEL_ORDER ? item.cl_ord_id : std::string_view{};
      auto response = server::oms::Response{
          .request_type = request_type,
          .origin = Origin::EXCHANGE,
          .request_status = request_status,
          .error = error,
          .text = item.text,
          .version = {},
          .request_id = request_id,
          .quantity = item.order_qty,
          .price = item.price,
      };
      auto order_update = server::oms::OrderUpdate{
          .account = account_.get_name(),
          .exchange = shared_.settings.exchange,
          .symbol = item.symbol,
          .side = side,
          .position_effect = {},
          .margin_mode = {},
          .max_show_quantity = NaN,
          .order_type = order_type,
          .time_in_force = time_in_force,
          .execution_instructions = {},
          .create_time_utc = {},
          .update_time_utc = item.timestamp,
          .external_account = external_account,
          .external_order_id = item.order_id,
          .client_order_id = {},
          .order_status = order_status,
          .quantity = item.order_qty,
          .price = item.price,
          .stop_price = item.stop_px,
          .remaining_quantity = item.leaves_qty,
          .traded_quantity = item.cum_qty,
          .average_traded_price = item.avg_px,
          .last_traded_quantity = item.last_qty,
          .last_traded_price = item.last_px,
          .last_liquidity = last_liquidity,
          .routing_id = {},
          .max_request_version = {},
          .max_response_version = {},
          .max_accepted_version = {},
          .update_type = UpdateType::INCREMENTAL,  // XXX not sure if this is correct...
          .sending_time_utc = {},
      };
      auto user_id = SOURCE_NONE;
      auto order_id = ORDER_ID_NONE;
      auto strategy_id = STRATEGY_ID_NONE;
      if (shared_.update_order(item.cl_ord_id, stream_id_, trace_info, response, order_update, [&](auto &order) {
            user_id = order.user_id;
            order_id = order.order_id;
            strategy_id = order.strategy_id;
          })) {
      } else {
        log::warn<1>("*** EXTERNAL ORDER ***"sv);
      }
      if (item.exec_type != json::ExecType::TRADE)
        continue;
      auto fill = Fill{
          .external_trade_id = item.trd_match_id,
          .quantity = item.last_qty,
          .price = item.last_px,
          .liquidity = {},
      };
      auto trade_update = TradeUpdate{
          .stream_id = stream_id_,
          .account = account_.get_name(),
          .order_id = order_id,
          .exchange = shared_.settings.exchange,
          .symbol = item.symbol,
          .side = side,
          .position_effect = {},
          .margin_mode = {},
          .create_time_utc = item.timestamp,
          .update_time_utc = item.timestamp,
          .external_account = external_account,
          .external_order_id = item.order_id,
          .client_order_id = {},
          .fills = {&fill, 1},
          .routing_id = {},
          .update_type = UpdateType::INCREMENTAL,
          .sending_time_utc = {},
          .user = {},
          .strategy_id = strategy_id,
      };
      create_trace_and_dispatch(handler_, trace_info, trade_update, true, user_id, item.cl_ord_id);
    }
  });
}

void DropCopy::operator()(Trace<json::Margin> const &event, json::Action action) {
  profile_.margin([&]() {
    auto &[trace_info, margin] = event;
    log::info<2>("event={{action={}, margin={}}}"sv, action, margin);
    // not used
  });
}

void DropCopy::operator()(Trace<json::Order> const &event, json::Action action) {
  profile_.order([&]() {
    auto &[trace_info, order] = event;
    log::info<2>("event={{action={}, order={}}}"sv, action, order);
    auto download = !partial_received_.order && action == json::Action::PARTIAL;
    OrderUpdate{shared_, stream_id_, account_.get_name()}(order, trace_info, download);
    // state management
    if (download) {
      partial_received_.order = true;
      // release download state
      download_.check_relaxed(DropCopyState::SUBSCRIBE);
    }
  });
}

void DropCopy::operator()(Trace<json::Position> const &event, json::Action action) {
  profile_.position([&]() {
    auto &[trace_info, position] = event;
    log::info<2>("event={{action={}, position={}}}"sv, action, position);
    for (auto &item : position.data) {
      auto external_account = item.account ? fmt::format("{}"sv, item.account) : std::string{};
      auto long_quantity = std::max(0.0, item.current_qty);
      auto short_quantity = std::max(0.0, -item.current_qty);
      auto position_update = PositionUpdate{
          .stream_id = stream_id_,
          .account = account_.get_name(),
          .exchange = shared_.settings.exchange,
          .symbol = item.symbol,
          .margin_mode = {},
          .external_account = external_account,
          .long_quantity = long_quantity,
          .short_quantity = short_quantity,
          .update_type = UpdateType::INCREMENTAL,
          .exchange_time_utc = item.timestamp,  // ???
          .sending_time_utc = {},
      };
      create_trace_and_dispatch(handler_, trace_info, position_update, false);
    }
  });
}

void DropCopy::operator()(Trace<json::Funding> const &event, json::Action action) {
  auto &[trace_info, funding] = event;
  log::fatal("Unexpected: action={}, funding={}"sv, action, funding);
}

void DropCopy::operator()([[maybe_unused]] Trace<json::Instrument> const &event, json::Action action) {
  auto &[trace_info, instrument] = event;
  log::fatal("Unexpected: action={}, instrument={}"sv, action, instrument);
}

void DropCopy::operator()(Trace<json::Liquidation> const &event, json::Action action) {
  auto &[trace_info, liquidation] = event;
  log::fatal("Unexpected: action={}, liquidation={}"sv, action, liquidation);
}

void DropCopy::operator()(Trace<json::OrderBookL2> const &event, json::Action action) {
  auto &[trace_info, order_book_l2] = event;
  log::fatal("Unexpected: action={}, order_book_l2={}"sv, action, order_book_l2);
}

void DropCopy::operator()(Trace<json::Quote> const &event, json::Action action) {
  auto &[trace_info, quote] = event;
  log::fatal("Unexpected: action={}, quote={}"sv, action, quote);
}

void DropCopy::operator()(Trace<json::Settlement> const &event, json::Action action) {
  auto &[trace_info, settlement] = event;
  log::fatal("Unexpected: action={}, settlement={}"sv, action, settlement);
}

void DropCopy::operator()(Trace<json::Trade> const &event, json::Action action) {
  auto &[trace_info, trade] = event;
  log::fatal("Unexpected: action={}, trade={}"sv, action, trade);
}

namespace {
auto compute_expires() {
  auto now = clock::get_realtime();
  auto expires = now + REQUEST_EXPIRES;
  return std::chrono::ceil<std::chrono::seconds>(expires);
}
}  // namespace

std::string DropCopy::create_upgrade_headers() {
  auto expires = compute_expires();
  return account_.create_headers(expires, web::http::Method::GET, "/realtime"sv, {});
}

}  // namespace bitmex
}  // namespace roq
