/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/bitmex/drop_copy.hpp"

#include <algorithm>
#include <utility>

#include "roq/mask.hpp"

#include "roq/utils/update.hpp"

#include "roq/utils/exceptions/unhandled.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/bitmex/order_update.hpp"
#include "roq/bitmex/utils.hpp"

#include "roq/bitmex/json/map.hpp"
#include "roq/bitmex/json/utils.hpp"

using namespace std::literals;
using namespace std::chrono_literals;

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

size_t const MAX_DECODE_BUFFER_DEPTH = 1;
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id, auto &account) {
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
      .user_agent = ROQ_PACKAGE_NAME,
      .request_timeout = {},
      .ping_frequency = settings.ws.ping_freq,
      // implementation
      .decode_buffer_size = settings.misc.decode_buffer_size,
      .encode_buffer_size = settings.misc.encode_buffer_size,
  };
  return web::socket::Client::create(handler, context, config, std::move(create_upgrade_headers));
}

struct create_metrics final : public utils::metrics::Factory {
  create_metrics(auto &settings, auto &group, auto const &function) : utils::metrics::Factory{settings.app.name, group, function} {}
};

auto compute_expires() {
  auto now = clock::get_realtime();
  auto expires = now + REQUEST_EXPIRES;
  return std::chrono::ceil<std::chrono::seconds>(expires);
}
}  // namespace

// === IMPLEMENTATION ===

DropCopy::DropCopy(Handler &handler, io::Context &context, uint16_t stream_id, Account &account, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_, account.name)},
      connection_{create_connection(*this, shared.settings, context, [this]() { return create_upgrade_headers(); })},
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
          .cancel_all_after = create_metrics(shared.settings, name_, "cancel_all_after"sv),
          .order = create_metrics(shared.settings, name_, "order"sv),
          .execution = create_metrics(shared.settings, name_, "execution"sv),
          .margin = create_metrics(shared.settings, name_, "margin"sv),
          .position = create_metrics(shared.settings, name_, "position"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
          .heartbeat = create_metrics(shared.settings, name_, "heartbeat"sv),
      },
      account_{account}, shared_{shared}, download_{shared.settings.ws.request_timeout, [this](auto state) { return download(state); }} {
}

void DropCopy::operator()(Event<Start> const &) {
  (*connection_).start();
}

void DropCopy::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void DropCopy::operator()(Event<Timer> const &event) {
  if (!(*connection_).refresh(event.value.now)) {
    return;
  }
  if (shared_.settings.ws.cancel_on_disconnect && shared_.settings.ws.cancel_all_after.count() && ready_ && next_cancel_all_after_ <= event.value.now) {
    next_cancel_all_after_ = event.value.now + shared_.settings.ws.cancel_all_after / 4;
    send_cancel_all_after(shared_.settings.ws.cancel_all_after);
  }
}

void DropCopy::operator()(metrics::Writer &writer) const {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.parse, metrics::Type::PROFILE)
      .write(profile_.welcome, metrics::Type::PROFILE)
      .write(profile_.error, metrics::Type::PROFILE)
      .write(profile_.subscribe, metrics::Type::PROFILE)
      .write(profile_.unsubscribe, metrics::Type::PROFILE)
      .write(profile_.cancel_all_after, metrics::Type::PROFILE)
      .write(profile_.order, metrics::Type::PROFILE)
      .write(profile_.execution, metrics::Type::PROFILE)
      .write(profile_.margin, metrics::Type::PROFILE)
      .write(profile_.position, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY)
      .write(latency_.heartbeat, metrics::Type::LATENCY);
}

// web::socket::Client::Handler

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
      .account = account_.name,
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

void DropCopy::operator()(ConnectionStatus connection_status, std::string_view const &reason) {
  connection_status_ = connection_status;
  TraceInfo trace_info;
  auto stream_status = StreamStatus{
      .stream_id = stream_id_,
      .account = account_.name,
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
      (*this)(ConnectionStatus::DOWNLOADING, "subscribe");
      subscribe();
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

void DropCopy::subscribe() {
  std::array<std::string_view, 4> topics{{
      "execution"sv,
      "order"sv,
      "margin"sv,
      "position"sv,
  }};
  send_subscribe(topics);
  // XXX other topics?
  // cancelAllAfter
  // authKeyExpires
}

void DropCopy::parse(std::string_view const &message) {
  profile_.parse([&]() {
    log::debug("{}"sv, message);
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

void DropCopy::operator()(Trace<json::Welcome> const &event) {
  profile_.welcome([&]() {
    auto &[trace_info, welcome] = event;
    log::info<2>("welcome={}"sv, welcome);
    download_.begin();
    if (!shared_.settings.ws.cancel_on_disconnect || shared_.settings.ws.cancel_all_after.count() == 0) {
      send_cancel_all_after(std::chrono::seconds{});
    }
  });
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

void DropCopy::operator()(Trace<json::Subscribe> const &event) {
  profile_.subscribe([&]() {
    auto &[trace_info, subscribe] = event;
    log::info<2>("subscribe={}"sv, subscribe);
    if (subscribe.success) {
      log::info(R"(Successfully subscribed to topic="{}")"sv, subscribe.subscribe);
    } else {
      log::warn(R"(Failed to subscribe topic="{}")"sv, subscribe.subscribe);
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

void DropCopy::operator()(Trace<json::Instrument> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::Quote> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::OrderBookL2> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::Trade> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::Funding> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::Liquidation> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::Settlement> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::Order> const &event) {
  profile_.order([&]() {
    auto &[trace_info, order] = event;
    log::info<2>("order={}"sv, order);
    auto download = !partial_received_.order && order.action == json::Action::PARTIAL;
    OrderUpdate{shared_, stream_id_, account_.name}(order, trace_info, download);
    // state management
    if (download) {
      partial_received_.order = true;
      // release download state
      download_.check_relaxed(DropCopyState::SUBSCRIBE);
    }
  });
}

void DropCopy::operator()(Trace<json::Execution> const &event) {
  profile_.execution([&]() {
    auto &[trace_info, execution] = event;
    log::info<2>("execution={}"sv, execution);
    for (auto &item : execution.data) {
      auto external_account = item.account ? fmt::format("{}"sv, item.account) : std::string{};
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
          .external_order_id = {},
          .quantity = item.order_qty,
          .price = item.price,
      };
      auto order_update = server::oms::OrderUpdate{
          .account = account_.name,
          .exchange = shared_.settings.exchange,
          .symbol = item.symbol,
          .side = map(item.side),
          .position_effect = {},
          .margin_mode = {},
          .max_show_quantity = NaN,
          .order_type = map(item.ord_type),
          .time_in_force = map(item.time_in_force),
          .execution_instructions = {},
          .create_time_utc = {},
          .update_time_utc = item.timestamp,
          .external_account = external_account,
          .external_order_id = item.order_id,
          .client_order_id = {},
          .order_status = map(item.ord_status),
          .error = {},
          .text = {},
          .quantity = item.order_qty,
          .price = item.price,
          .stop_price = item.stop_px,
          .leverage = NaN,
          .remaining_quantity = item.leaves_qty,
          .traded_quantity = item.cum_qty,
          .average_traded_price = item.avg_px,
          .last_traded_quantity = item.last_qty,
          .last_traded_price = item.last_px,
          .last_liquidity = map(item.last_liquidity_ind),
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
      if (item.exec_type != json::ExecType::TRADE) {
        continue;
      }
      auto fill = Fill{
          .external_trade_id = item.trd_match_id,
          .quantity = item.last_qty,
          .price = item.last_px,
          .liquidity = {},
          .commission_amount = NaN,
          .commission_currency = {},
          .base_amount = NaN,
          .quote_amount = NaN,
          .profit_loss_amount = NaN,
      };
      auto trade_update = TradeUpdate{
          .stream_id = stream_id_,
          .account = account_.name,
          .order_id = order_id,
          .exchange = shared_.settings.exchange,
          .symbol = item.symbol,
          .side = map(item.side),
          .position_effect = {},
          .margin_mode = {},
          .quantity_type = {},
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

void DropCopy::operator()(Trace<json::Margin> const &event) {
  profile_.margin([&]() {
    auto &[trace_info, margin] = event;
    log::info<2>("margin={}"sv, margin);
    // not used
  });
}

void DropCopy::operator()(Trace<json::Position> const &event) {
  profile_.position([&]() {
    auto &[trace_info, position] = event;
    log::info<2>("position={}"sv, position);
    for (auto &item : position.data) {
      auto external_account = item.account ? fmt::format("{}"sv, item.account) : std::string{};
      auto long_quantity = std::max(0.0, item.current_qty);
      auto short_quantity = std::max(0.0, -item.current_qty);
      auto position_update = PositionUpdate{
          .stream_id = stream_id_,
          .account = account_.name,
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

std::string DropCopy::create_upgrade_headers() {
  auto expires = compute_expires();
  return account_.create_headers(expires, web::http::Method::GET, shared_.api.ws.realtime, {});
}

}  // namespace bitmex
}  // namespace roq
