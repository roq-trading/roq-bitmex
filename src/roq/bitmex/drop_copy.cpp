/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/bitmex/drop_copy.h"

#include <algorithm>
#include <utility>

#include "roq/utils/mask.h"
#include "roq/utils/update.h"

#include "roq/core/back_emplacer.h"

#include "roq/core/metrics/factory.h"

#include "roq/bitmex/flags.h"
#include "roq/bitmex/order_update.h"
#include "roq/bitmex/utils.h"

#include "roq/bitmex/json/utils.h"

using namespace std::literals;
using namespace std::chrono_literals;  // NOLINT

namespace roq {
namespace bitmex {

namespace {
const auto NAME = "ex"sv;
const auto SUPPORTS = utils::Mask{
    SupportType::ORDER_ACK,
    SupportType::ORDER,
    SupportType::TRADE,
    SupportType::POSITION,
};

const auto REQUEST_EXPIRES = 5s;

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(const std::string_view &group, const std::string_view &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

auto create_connection(auto &handler, auto &context, auto &&create_upgrade_headers) {
  core::web::ClientSocket::Config config{
      .validate_certificate = server::Flags::tls_validate_certificate(),
      .uri = Flags::ws_uri(),
      .query = {},
      .ping_frequency = Flags::ws_ping_freq(),
      .read_buffer_size = Flags::decode_buffer_size(),
      .encode_buffer_size = Flags::encode_buffer_size(),
  };
  return core::web::ClientSocket{handler, context, config, std::move(create_upgrade_headers)};
}

template <typename T>
void emplace(Fill &result, const T &value) {
  new (&result) Fill{
      .external_trade_id = value.trd_match_id,
      .quantity = value.last_qty,
      .price = value.last_px,
      .liquidity = {},
  };
}
}  // namespace

DropCopy::DropCopy(
    Handler &handler,
    core::io::Context &context,
    uint16_t stream_id,
    Security &security,
    Shared &shared)
    : handler_(handler), stream_id_(stream_id),
      name_(fmt::format("{}:{}:{}"sv, stream_id_, NAME, security.get_account())),
      connection_(create_connection(*this, context, [this]() { return create_upgrade_headers(); })),
      decode_buffer_(Flags::decode_buffer_size()),
      counter_{
          .disconnect = create_metrics(name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(name_, "parse"sv),
          .cancel_all_after = create_metrics(name_, "cancel_all_after"sv),
          .error = create_metrics(name_, "error"sv),
          .execution = create_metrics(name_, "execution"sv),
          .handshake = create_metrics(name_, "handshake"sv),
          .margin = create_metrics(name_, "margin"sv),
          .order = create_metrics(name_, "order"sv),
          .position = create_metrics(name_, "position"sv),
          .subscribe = create_metrics(name_, "subscribe"sv),
          .unsubscribe = create_metrics(name_, "unsubscribe"sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"sv),
          .heartbeat = create_metrics(name_, "heartbeat"sv),
      },
      security_(security), shared_(shared),
      download_(Flags::ws_request_timeout(), [this](auto state) { return download(state); }) {
}

void DropCopy::operator()(const Event<Start> &) {
  connection_.start();
}

void DropCopy::operator()(const Event<Stop> &) {
  connection_.stop();
}

void DropCopy::operator()(const Event<Timer> &event) {
  if (!connection_.refresh(event.value.now))
    return;
  if (Flags::ws_cancel_on_disconnect() && Flags::ws_cancel_all_after().count() && ready_ &&
      next_cancel_all_after_ <= event.value.now) {
    next_cancel_all_after_ = event.value.now + Flags::ws_cancel_all_after() / 4;
    send_cancel_all_after(Flags::ws_cancel_all_after());
  }
}

void DropCopy::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::COUNTER)
      // profile
      .write(profile_.parse, metrics::PROFILE)
      .write(profile_.cancel_all_after, metrics::PROFILE)
      .write(profile_.error, metrics::PROFILE)
      .write(profile_.execution, metrics::PROFILE)
      .write(profile_.handshake, metrics::PROFILE)
      .write(profile_.margin, metrics::PROFILE)
      .write(profile_.order, metrics::PROFILE)
      .write(profile_.position, metrics::PROFILE)
      .write(profile_.subscribe, metrics::PROFILE)
      .write(profile_.unsubscribe, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY)
      .write(latency_.heartbeat, metrics::LATENCY);
}

void DropCopy::operator()(const core::web::ClientSocket::Connected &) {
  // note! don't notify gateway: wait for ready
}

void DropCopy::operator()(const core::web::ClientSocket::Disconnected &) {
  ready_ = false;
  next_cancel_all_after_ = {};
  partial_received_ = {};
  download_.reset();
  (*this)(ConnectionStatus::DISCONNECTED);
}

void DropCopy::operator()(const core::web::ClientSocket::Ready &) {
  // note! don't notify gateway: wait for handshake
  (*this)(ConnectionStatus::LOGIN_SENT);
}

void DropCopy::operator()(const core::web::ClientSocket::Close &) {
}

void DropCopy::operator()(const core::web::ClientSocket::Latency &latency) {
  auto trace_info = server::create_trace_info();
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .account = security_.get_account(),
      .latency = latency.sample,
  };
  server::create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void DropCopy::operator()(const core::web::ClientSocket::Text &text) {
  parse(text.payload);
}

void DropCopy::operator()(const core::web::ClientSocket::Binary &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    auto trace_info = server::create_trace_info();
    StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = security_.get_account(),
        .supports = SUPPORTS.get(),
        .status = status_,
        .type = StreamType::WEB_SOCKET,
        .priority = Priority::PRIMARY,
    };
    log::info("stream_status={}"sv, stream_status);
    server::create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

void DropCopy::send_cancel_all_after(std::chrono::nanoseconds timeout) {
  auto message = fmt::format(
      R"({{)"
      R"("op":"cancelAllAfter",)"
      R"("args":{})"
      R"(}})"sv,
      std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());
  connection_.send_text(message);
}

void DropCopy::send_subscribe(const std::string_view &topic) {
  auto message = fmt::format(
      R"({{)"
      R"("op":"subscribe",)"
      R"("args":"{}")"
      R"(}})"sv,
      topic);
  log::debug(R"(message="{}")"sv, message);
  connection_.send_text(message);
}

void DropCopy::send_subscribe(const std::span<std::string_view> &topics) {
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

uint32_t DropCopy::download(DropCopyState state) {
  switch (state) {
    case DropCopyState::UNDEFINED:
      assert(false);
      break;
    case DropCopyState::SUBSCRIBE:
      subscribe();
      return 1;
    case DropCopyState::DONE:
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

void DropCopy::parse(const std::string_view &message) {
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

void DropCopy::parse_helper(const std::string_view &message) {
  auto trace_info = server::create_trace_info();
  core::json::Buffer buffer(decode_buffer_);
  json::StreamParser::dispatch(*this, message, buffer, trace_info);
}

void DropCopy::operator()(const server::Trace<json::CancelAllAfter> &event) {
  profile_.cancel_all_after([&]() {
    auto &[trace_info, cancel_all_after] = event;
    log::info<2>("cancel_all_after={}"sv, cancel_all_after);
  });
}

void DropCopy::operator()(const server::Trace<json::Error> &event) {
  profile_.error([&]() {
    auto &[trace_info, error] = event;
    log::warn("error={}"sv, error);
  });
  connection_.close();
}

void DropCopy::operator()(const server::Trace<json::Handshake> &event) {
  profile_.handshake([&]() {
    auto &[trace_info, handshake] = event;
    log::info<2>("handshake={}"sv, handshake);
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
    if (!Flags::ws_cancel_on_disconnect() || Flags::ws_cancel_all_after().count() == 0)
      send_cancel_all_after(std::chrono::seconds{});
  });
}

void DropCopy::operator()(const server::Trace<json::Subscribe> &event) {
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

void DropCopy::operator()(const server::Trace<json::Unsubscribe> &event) {
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

void DropCopy::operator()(const server::Trace<json::Execution> &event, json::Action action) {
  profile_.execution([&]() {
    // auto &[trace_info, execution] = event; // XXX clang13
    auto &trace_info = event.trace_info;
    auto &execution = event.value;
    log::info<2>("event={{action={}, execution={}}}"sv, action, execution);
    core::back_emplacer fills(shared_.fills);
    size_t index = {};
    for (auto &item : execution.data) {
      auto last = std::size(execution.data) == ++index;
      auto order_status = json::map(item.ord_status);
      auto side = json::map(item.side);
      auto external_account = fmt::format("{}"sv, item.account);
      auto order_type = json::map(item.ord_type);
      auto time_in_force = json::map(item.time_in_force);
      auto last_liquidity = json::map(item.last_liquidity_ind);
      auto request_type = compute_request_type(item.exec_type);
      auto request_status = compute_request_status(item.exec_type);
      auto error = json::guess_error(item.ord_rej_reason);
      // cancel order does not allow passing a custom id
      auto request_id =
          request_type != RequestType::CANCEL_ORDER ? item.cl_ord_id : std::string_view{};
      oms::Response response{
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
      oms::OrderUpdate order_update{
          .account = security_.get_account(),
          .exchange = Flags::exchange(),
          .symbol = item.symbol,
          .side = side,
          .position_effect = {},
          .max_show_quantity = NaN,
          .order_type = order_type,
          .time_in_force = time_in_force,
          .execution_instruction = {},
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
      if (shared_.update_order(
              item.cl_ord_id, stream_id_, trace_info, response, order_update, [&](auto &order) {
                if (item.exec_type == json::ExecType::TRADE) {
                  fills.emplace_back([&](auto &result) { emplace(result, item); });
                }
                if (last && !std::empty(fills)) {
                  TradeUpdate trade_update{
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
                      .fills = fills,
                      .routing_id = order.routing_id,
                  };
                  server::create_trace_and_dispatch(
                      handler_, trace_info, trade_update, true, order.user_id);
                }
              })) {
      } else {
        log::warn<1>("*** EXTERNAL ORDER ***"sv);
      }
    }
  });
}

void DropCopy::operator()(const server::Trace<json::Margin> &event, json::Action action) {
  profile_.margin([&]() {
    auto &[trace_info, margin] = event;
    log::info<2>("event={{action={}, margin={}}}"sv, action, margin);
    // not used
  });
}

void DropCopy::operator()(const server::Trace<json::Order> &event, json::Action action) {
  profile_.order([&]() {
    auto &[trace_info, order] = event;
    log::info<2>("event={{action={}, order={}}}"sv, action, order);
    auto download = !partial_received_.order && action == json::Action::PARTIAL;
    OrderUpdate{shared_, stream_id_, security_.get_account()}(order, trace_info, download);
    // state management
    if (download) {
      partial_received_.order = true;
      // release download state
      download_.check_relaxed(DropCopyState::SUBSCRIBE);
    }
  });
}

void DropCopy::operator()(const server::Trace<json::Position> &event, json::Action action) {
  profile_.position([&]() {
    auto &[trace_info, position] = event;
    log::info<2>("event={{action={}, position={}}}"sv, action, position);
    for (auto &item : position.data) {
      auto external_account = fmt::format("{}"sv, item.account);
      auto long_quantity = std::max(0.0, item.current_qty);
      auto short_quantity = std::max(0.0, -item.current_qty);
      PositionUpdate position_update{
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
      server::create_trace_and_dispatch(handler_, trace_info, position_update, false);
    }
  });
}

void DropCopy::operator()(const server::Trace<json::Funding> &event, json::Action action) {
  auto &[trace_info, funding] = event;
  log::fatal("Unexpected: action={}, funding={}"sv, action, funding);
}

void DropCopy::operator()(const server::Trace<json::Instrument> &event, json::Action action) {
  auto &[trace_info, instrument] = event;
  log::fatal("Unexpected: action={}, instrument={}"sv, action, instrument);
}

void DropCopy::operator()(const server::Trace<json::Liquidation> &event, json::Action action) {
  auto &[trace_info, liquidation] = event;
  log::fatal("Unexpected: action={}, liquidation={}"sv, action, liquidation);
}

void DropCopy::operator()(const server::Trace<json::OrderBookL2> &event, json::Action action) {
  auto &[trace_info, order_book_l2] = event;
  log::fatal("Unexpected: action={}, order_book_l2={}"sv, action, order_book_l2);
}

void DropCopy::operator()(const server::Trace<json::Quote> &event, json::Action action) {
  auto &[trace_info, quote] = event;
  log::fatal("Unexpected: action={}, quote={}"sv, action, quote);
}

void DropCopy::operator()(const server::Trace<json::Settlement> &event, json::Action action) {
  auto &[trace_info, settlement] = event;
  log::fatal("Unexpected: action={}, settlement={}"sv, action, settlement);
}

void DropCopy::operator()(const server::Trace<json::Trade> &event, json::Action action) {
  auto &[trace_info, trade] = event;
  log::fatal("Unexpected: action={}, trade={}"sv, action, trade);
}

namespace {
auto compute_expires() {
  auto now = core::clock::GetRealTime();
  auto expires = now + REQUEST_EXPIRES;
  return std::chrono::ceil<std::chrono::seconds>(expires);
}
}  // namespace

std::string DropCopy::create_upgrade_headers() {
  auto expires = compute_expires();
  return security_.create_headers(expires, core::http::Method::GET, "/realtime"sv, {});
}

}  // namespace bitmex
}  // namespace roq
