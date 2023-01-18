/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/bitmex/web_socket.hpp"

#include <algorithm>
#include <utility>

#include "roq/mask.hpp"
#include "roq/utils/update.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/web/socket/client_factory.hpp"

#include "roq/bitmex/flags.hpp"
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

auto create_connection(auto &handler, auto &context, auto &&create_upgrade_headers) {
  auto uri = Flags::ws_uri();
  auto config = web::socket::Client::Config{
      .always_reconnect = true,
      .connection_timeout = server::Flags::net_connection_timeout(),
      .disconnect_on_idle_timeout = {},
      .validate_certificate = server::Flags::net_tls_validate_certificate(),
      .uris = {&uri, 1},
      .query = {},
      .ping_frequency = Flags::ws_ping_freq(),
      .read_buffer_size = Flags::decode_buffer_size(),
      .encode_buffer_size = Flags::encode_buffer_size(),
  };
  return web::socket::ClientFactory::create(handler, context, config, std::move(create_upgrade_headers));
}

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(auto const &group, auto const &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};
}  // namespace

// === IMPLEMENTATION ===

WebSocket::WebSocket(Handler &handler, io::Context &context, uint16_t stream_id, Security &security, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_, security.get_account())},
      connection_{create_connection(*this, context, [this]() { return create_upgrade_headers(); })},
      decode_buffer_{Flags::decode_buffer_size()},
      counter_{
          .disconnect = create_metrics(name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(name_, "parse"sv),
          .create_order = create_metrics(name_, "create_order"sv),
          .modify_order = create_metrics(name_, "modify_order"sv),
          .cancel_order = create_metrics(name_, "cancel_order"sv),
          .cancel_all_orders = create_metrics(name_, "cancel_all_orders"sv),
          .cancel_all_after = create_metrics(name_, "cancel_all_after"sv),
          .error = create_metrics(name_, "error"sv),
          .execution = create_metrics(name_, "execution"sv),
          .handshake = create_metrics(name_, "handshake"sv),
          .margin = create_metrics(name_, "margin"sv),
          .order = create_metrics(name_, "order"sv),
          .position = create_metrics(name_, "position"sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"sv),
          .heartbeat = create_metrics(name_, "heartbeat"sv),
      },
      security_{security}, shared_{shared}, download_{Flags::ws_request_timeout(), [this](auto state) {
                                                        return download(state);
                                                      }} {
}

void WebSocket::operator()(Event<Start> const &) {
  (*connection_).start();
}

void WebSocket::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void WebSocket::operator()(Event<Timer> const &event) {
  if (!(*connection_).refresh(event.value.now))
    return;
  if (Flags::ws_cancel_on_disconnect() && Flags::ws_cancel_all_after().count() && ready_ &&
      next_cancel_all_after_ <= event.value.now) {
    next_cancel_all_after_ = event.value.now + Flags::ws_cancel_all_after() / 4;
    send_cancel_all_after(Flags::ws_cancel_all_after());
  }
}

void WebSocket::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::COUNTER)
      // profile
      .write(profile_.parse, metrics::PROFILE)
      .write(profile_.create_order, metrics::PROFILE)
      .write(profile_.modify_order, metrics::PROFILE)
      .write(profile_.cancel_order, metrics::PROFILE)
      .write(profile_.cancel_all_orders, metrics::PROFILE)
      .write(profile_.cancel_all_after, metrics::PROFILE)
      .write(profile_.error, metrics::PROFILE)
      .write(profile_.execution, metrics::PROFILE)
      .write(profile_.handshake, metrics::PROFILE)
      .write(profile_.margin, metrics::PROFILE)
      .write(profile_.order, metrics::PROFILE)
      .write(profile_.position, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY)
      .write(latency_.heartbeat, metrics::LATENCY);
}

uint16_t WebSocket::operator()(
    Event<CreateOrder> const &event, oms::Order const &order, std::string_view const &request_id) {
  create_order(event, order, request_id);
  return stream_id_;
}

uint16_t WebSocket::operator()(
    Event<ModifyOrder> const &event,
    oms::Order const &order,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  modify_order(event, order, request_id, previous_request_id);
  return stream_id_;
}

uint16_t WebSocket::operator()(
    Event<CancelOrder> const &event,
    oms::Order const &order,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  cancel_order(event, order, request_id, previous_request_id);
  return stream_id_;
}

uint16_t WebSocket::operator()(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  cancel_all_orders(event, request_id);
  return stream_id_;
}

void WebSocket::operator()(web::socket::Client::Connected const &) {
  // note! don't notify gateway: wait for ready
}

void WebSocket::operator()(web::socket::Client::Disconnected const &) {
  ready_ = false;
  next_cancel_all_after_ = {};
  partial_received_ = {};
  download_.reset();
  (*this)(ConnectionStatus::DISCONNECTED);
}

void WebSocket::operator()(web::socket::Client::Ready const &) {
  // note! don't notify gateway: wait for handshake
  (*this)(ConnectionStatus::LOGIN_SENT);
}

void WebSocket::operator()(web::socket::Client::Close const &) {
}

void WebSocket::operator()(web::socket::Client::Latency const &latency) {
  TraceInfo trace_info;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = security_.get_account(),
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void WebSocket::operator()(web::socket::Client::Text const &text) {
  parse(text.payload);
}

void WebSocket::operator()(web::socket::Client::Binary const &) {
  log::fatal("Unexpected"sv);
}

void WebSocket::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    TraceInfo trace_info;
    auto stream_status = StreamStatus{
        .stream_id = stream_id_,
        .account = security_.get_account(),
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

// create-order

void WebSocket::create_order(
    Event<CreateOrder> const &, oms::Order const &, [[maybe_unused]] std::string_view const &request_id) {
  profile_.create_order([&]() {
    if (!ready())
      throw oms::NotReady{"not ready"sv};
  });
}

// modify-order

void WebSocket::modify_order(
    Event<ModifyOrder> const &,
    oms::Order const &,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  profile_.modify_order([&]() {
    if (!ready())
      throw oms::NotReady{"not ready"sv};
  });
}

// cancel-order

void WebSocket::cancel_order(
    Event<CancelOrder> const &,
    oms::Order const &,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  profile_.cancel_order([&]() {
    if (!ready())
      throw oms::NotReady{"not ready"sv};
  });
}

// cancel-all-orders

void WebSocket::cancel_all_orders(Event<CancelAllOrders> const &, [[maybe_unused]] std::string_view const &request_id) {
  profile_.cancel_all_orders([&]() {});
}

// cancel-all-after

void WebSocket::send_cancel_all_after(std::chrono::nanoseconds timeout) {
  auto message = fmt::format(
      R"({{)"
      R"("op":"cancelAllAfter",)"
      R"("args":{})"
      R"(}})"sv,
      std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());
  (*connection_).send_text(message);
}

uint32_t WebSocket::download(WebSocketState state) {
  switch (state) {
    using enum WebSocketState;
    case UNDEFINED:
      assert(false);
      break;
    case AUTHENTICATE:
      return {};
    case DONE:
      (*this)(ConnectionStatus::READY);
      assert(!ready_);
      ready_ = true;
      return {};
  }
  assert(false);
  return {};
}

void WebSocket::parse(std::string_view const &message) {
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

void WebSocket::parse_helper(std::string_view const &message) {
  TraceInfo trace_info;
  core::json::Buffer buffer{decode_buffer_};
  json::StreamParser::dispatch(*this, message, buffer, trace_info);
}

void WebSocket::operator()(Trace<json::CancelAllAfter> const &event) {
  profile_.cancel_all_after([&]() {
    auto &[trace_info, cancel_all_after] = event;
    log::info<2>("cancel_all_after={}"sv, cancel_all_after);
  });
}

void WebSocket::operator()(Trace<json::Error> const &event) {
  profile_.error([&]() {
    auto &[trace_info, error] = event;
    log::warn("error={}"sv, error);
  });
  (*connection_).close();
}

void WebSocket::operator()(Trace<json::Handshake> const &event) {
  profile_.handshake([&]() {
    auto &[trace_info, handshake] = event;
    log::info<2>("handshake={}"sv, handshake);
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
    if (!Flags::ws_cancel_on_disconnect() || Flags::ws_cancel_all_after().count() == 0)
      send_cancel_all_after(std::chrono::seconds{});
  });
}

void WebSocket::operator()(Trace<json::Subscribe> const &) {
}

void WebSocket::operator()(Trace<json::Unsubscribe> const &) {
}

void WebSocket::operator()(Trace<json::Execution> const &event, json::Action action) {
  profile_.execution([&]() {
    auto &trace_info = event.trace_info;
    auto &execution = event.value;
    log::info<2>("event={{action={}, execution={}}}"sv, action, execution);
    shared_.fills.clear();
    auto emplace_back = [](auto &result, auto &value) {
      auto fill = Fill{
          .external_trade_id = value.trd_match_id,
          .quantity = value.last_qty,
          .price = value.last_px,
          .liquidity = {},
      };
      result.emplace_back(std::move(fill));
    };
    size_t index = {};
    for (auto &item : execution.data) {
      auto last = std::size(execution.data) == ++index;
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
      auto response = oms::Response{
          .type = request_type,
          .origin = Origin::EXCHANGE,
          .status = request_status,
          .error = error,
          .text = item.text,
          .version = {},
          .request_id = request_id,
          .quantity = item.order_qty,
          .price = item.price,
      };
      auto order_update = oms::OrderUpdate{
          .account = security_.get_account(),
          .exchange = Flags::exchange(),
          .symbol = item.symbol,
          .side = side,
          .position_effect = {},
          .max_show_quantity = NaN,
          .order_type = order_type,
          .time_in_force = time_in_force,
          .execution_instructions = {},
          .order_template = {},
          .create_time_utc = {},
          .update_time_utc = item.timestamp,
          .external_account = external_account,
          .external_order_id = item.order_id,
          .status = order_status,
          .quantity = item.order_qty,
          .price = item.price,
          .stop_price = item.stop_px,
          .remaining_quantity = item.leaves_qty,
          .traded_quantity = item.cum_qty,
          .average_traded_price = item.avg_px,
          .last_traded_quantity = item.last_qty,
          .last_traded_price = item.last_px,
          .last_liquidity = last_liquidity,
          .update_type = {},
      };
      if (shared_.update_order(item.cl_ord_id, stream_id_, trace_info, response, order_update, [&](auto &order) {
            if (item.exec_type == json::ExecType::TRADE) {
              emplace_back(shared_.fills, item);
            }
            if (last && !std::empty(shared_.fills)) {
              auto trade_update = TradeUpdate{
                  .stream_id = stream_id_,
                  .account = order.account,
                  .order_id = order.order_id,
                  .exchange = order.exchange,
                  .symbol = order.symbol,
                  .side = order.side,
                  .position_effect = order.position_effect,
                  .create_time_utc = item.timestamp,
                  .update_time_utc = item.timestamp,
                  .external_account = external_account,
                  .external_order_id = order.external_order_id,
                  .fills = shared_.fills,
                  .routing_id = order.routing_id,
                  .update_type = {},
                  .user = {},
              };
              create_trace_and_dispatch(handler_, trace_info, trade_update, true, order.user_id);
            }
          })) {
      } else {
        log::warn<1>("*** EXTERNAL ORDER ***"sv);
      }
    }
  });
}

void WebSocket::operator()(Trace<json::Margin> const &event, json::Action action) {
  profile_.margin([&]() {
    auto &[trace_info, margin] = event;
    log::info<2>("event={{action={}, margin={}}}"sv, action, margin);
    // not used
  });
}

void WebSocket::operator()(Trace<json::Order> const &event, json::Action action) {
  profile_.order([&]() {
    auto &[trace_info, order] = event;
    log::info<2>("event={{action={}, order={}}}"sv, action, order);
    auto download = !partial_received_.order && action == json::Action::PARTIAL;
    OrderUpdate{shared_, stream_id_, security_.get_account()}(order, trace_info, download);
    // state management
    if (download) {
      partial_received_.order = true;
      // release download state
      // download_.check_relaxed(WebSocketState::SUBSCRIBE);
    }
  });
}

void WebSocket::operator()(Trace<json::Position> const &event, json::Action action) {
  profile_.position([&]() {
    auto &[trace_info, position] = event;
    log::info<2>("event={{action={}, position={}}}"sv, action, position);
    for (auto &item : position.data) {
      auto external_account = item.account ? fmt::format("{}"sv, item.account) : std::string{};
      auto long_quantity = std::max(0.0, item.current_qty);
      auto short_quantity = std::max(0.0, -item.current_qty);
      const PositionUpdate position_update{
          .stream_id = stream_id_,
          .account = security_.get_account(),
          .exchange = Flags::exchange(),
          .symbol = item.symbol,
          .external_account = external_account,
          .long_quantity = long_quantity,
          .short_quantity = short_quantity,
          .long_quantity_begin = NaN,
          .short_quantity_begin = NaN,
      };
      create_trace_and_dispatch(handler_, trace_info, position_update, false);
    }
  });
}

void WebSocket::operator()(Trace<json::Funding> const &event, json::Action action) {
  auto &[trace_info, funding] = event;
  log::fatal("Unexpected: action={}, funding={}"sv, action, funding);
}

void WebSocket::operator()([[maybe_unused]] Trace<json::Instrument> const &event, json::Action action) {
#if defined(__clang__)
  // note! compile-time formatting doesn't work
  log::fatal("Unexpected: action={}"sv, action);
#else
  auto &[trace_info, instrument] = event;
  log::fatal("Unexpected: action={}, instrument={}"sv, action, instrument);
#endif
}

void WebSocket::operator()(Trace<json::Liquidation> const &event, json::Action action) {
  auto &[trace_info, liquidation] = event;
  log::fatal("Unexpected: action={}, liquidation={}"sv, action, liquidation);
}

void WebSocket::operator()(Trace<json::OrderBookL2> const &event, json::Action action) {
  auto &[trace_info, order_book_l2] = event;
  log::fatal("Unexpected: action={}, order_book_l2={}"sv, action, order_book_l2);
}

void WebSocket::operator()(Trace<json::Quote> const &event, json::Action action) {
  auto &[trace_info, quote] = event;
  log::fatal("Unexpected: action={}, quote={}"sv, action, quote);
}

void WebSocket::operator()(Trace<json::Settlement> const &event, json::Action action) {
  auto &[trace_info, settlement] = event;
  log::fatal("Unexpected: action={}, settlement={}"sv, action, settlement);
}

void WebSocket::operator()(Trace<json::Trade> const &event, json::Action action) {
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

std::string WebSocket::create_upgrade_headers() {
  auto expires = compute_expires();
  return security_.create_headers(expires, web::http::Method::GET, "/realtime"sv, {});
}

}  // namespace bitmex
}  // namespace roq
