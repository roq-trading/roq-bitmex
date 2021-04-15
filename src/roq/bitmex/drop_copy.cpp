/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/bitmex/drop_copy.h"

#include "roq/utils/mask.h"
#include "roq/utils/update.h"

#include "roq/core/back_emplacer.h"

#include "roq/core/metrics/factory.h"

#include "roq/bitmex/flags.h"
#include "roq/bitmex/order_update.h"

#include "roq/bitmex/json/utils.h"

using namespace roq::literals;

namespace roq {
namespace bitmex {

namespace {
static const auto NAME = "ex"_sv;
static const auto SUPPORTS = utils::Mask{
    SupportType::ORDER_ACK,
    SupportType::ORDER,
    SupportType::TRADE,
    SupportType::POSITION,
};

static const auto REQUEST_EXPIRES = std::chrono::seconds{5};

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(const std::string_view &group, const std::string_view &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

template <typename T>
void emplace(Fill &result, const T &value, uint32_t trade_id) {
  new (&result) Fill{
      .quantity = value.last_qty,
      .price = value.last_px,
      .trade_id = trade_id,
      .gateway_trade_id = trade_id,
      .external_trade_id = value.trd_match_id,
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
      name_(roq::format("{}:{}:{}"_fmt, stream_id_, NAME, security.get_account())),
      connection_(
          *this,
          context,
          core::URI(Flags::ws_uri()),
          {},  // query
          Flags::ws_ping_freq(),
          Flags::decode_buffer_size(),  // XXX need read buffer size
          Flags::encode_buffer_size(),
          [this]() { return create_upgrade_headers(); }),
      decode_buffer_(Flags::decode_buffer_size()),
      counter_{
          .disconnect = create_metrics(name_, "disconnect"_sv),
      },
      profile_{
          .parse = create_metrics(name_, "parse"_sv),
          .cancel_all_after = create_metrics(name_, "cancel_all_after"_sv),
          .error = create_metrics(name_, "error"_sv),
          .execution = create_metrics(name_, "execution"_sv),
          .handshake = create_metrics(name_, "handshake"_sv),
          .margin = create_metrics(name_, "margin"_sv),
          .order = create_metrics(name_, "order"_sv),
          .position = create_metrics(name_, "position"_sv),
          .subscribe = create_metrics(name_, "subscribe"_sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"_sv),
          .heartbeat = create_metrics(name_, "heartbeat"_sv),
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
      // latency
      .write(latency_.ping, metrics::LATENCY)
      .write(latency_.heartbeat, metrics::LATENCY);
}

void DropCopy::operator()(const core::web::Socket::Connected &) {
  // note! don't notify gateway: wait for ready
}

void DropCopy::operator()(const core::web::Socket::Disconnected &) {
  ready_ = false;
  next_cancel_all_after_ = {};
  partial_received_ = {};
  download_.reset();
  (*this)(ConnectionStatus::DISCONNECTED);
}

void DropCopy::operator()(const core::web::Socket::Ready &) {
  // note! don't notify gateway: wait for handshake
  (*this)(ConnectionStatus::LOGIN_SENT);
}

void DropCopy::operator()(const core::web::Socket::Close &) {
}

void DropCopy::operator()(const core::web::Socket::Latency &latency) {
  server::TraceInfo trace_info;
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .latency = latency.sample,
  };
  server::create_trace_and_dispatch(trace_info, external_latency, handler_);
  latency_.ping.update(latency.sample);
}

void DropCopy::operator()(const core::web::Socket::Text &text) {
  parse(text.payload);
}

void DropCopy::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    server::TraceInfo trace_info;
    StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = security_.get_account(),
        .supports = SUPPORTS.get(),
        .status = status_,
        .type = StreamType::WEB_SOCKET,
        .priority = Priority::PRIMARY,
    };
    log::info("stream_status={}"_fmt, stream_status);
    server::create_trace_and_dispatch(trace_info, stream_status, handler_);
  }
}

void DropCopy::send_cancel_all_after(std::chrono::seconds seconds) {
  auto message = roq::format(
      R"({{)"
      R"("op":"cancelAllAfter",)"
      R"("args":{})"
      R"(}})"_fmt,
      std::chrono::duration_cast<std::chrono::milliseconds>(seconds).count());
  connection_.send_text(message);
}

void DropCopy::send_subscribe(const std::string_view &topic) {
  auto message = roq::format(
      R"({{)"
      R"("op":"subscribe",)"
      R"("args":"{}")"
      R"(}})"_fmt,
      topic);
  log::debug(R"(DEBUG: message="{}")"_fmt, message);
  connection_.send_text(message);
}

void DropCopy::send_subscribe(const roq::span<std::string_view> &topics) {
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
    log::debug(R"(DEBUG: message="{}")"_fmt, message);
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
      return 1u;
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
      "execution"_sv,
      "order"_sv,
      "margin"_sv,
      "position"_sv,
  };
  send_subscribe(topics);
  // XXX other topics?
  // cancelAllAfter
  // authKeyExpires
}

void DropCopy::parse(const std::string_view &message) {
  log::trace_4(R"(message="{}")"_fmt, message);
  profile_.parse([&]() {
    try {
      parse_helper(message);
    } catch (...) {
      log::warn(R"(message="{}")"_fmt, message);
      core::tools::UnhandledException::terminate();
    }
  });
}

void DropCopy::parse_helper(const std::string_view &message) {
  server::TraceInfo trace_info;
  core::json::Buffer buffer(decode_buffer_);
  json::Parser::dispatch(*this, message, buffer, trace_info);
}

void DropCopy::operator()(const json::CancelAllAfter &cancel_all_after) {
  profile_.cancel_all_after([&]() { log::trace_1("cancel_all_after={}"_fmt, cancel_all_after); });
}

void DropCopy::operator()(const json::Error &error) {
  profile_.error([&]() { log::warn("error={}"_fmt, error); });
  connection_.close();
}

void DropCopy::operator()(const json::Handshake &handshake) {
  profile_.handshake([&]() {
    log::trace_1("handshake={}"_fmt, handshake);
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
    if (!Flags::ws_cancel_on_disconnect() || Flags::ws_cancel_all_after().count() == 0)
      send_cancel_all_after(std::chrono::seconds{});
  });
}

void DropCopy::operator()(const json::Subscribe &subscribe) {
  profile_.subscribe([&]() {
    log::trace_1("subscribe={}"_fmt, subscribe);
    if (subscribe.success) {
      assert(!subscribe.failure);
      log::info(R"(Successfully subscribed to topic="{}")"_fmt, subscribe.subscribe);
    } else if (subscribe.failure) {
      assert(!subscribe.success);
      log::warn(R"(Failed to subscribe topic="{}")"_fmt, subscribe.subscribe);
    } else {
      log::fatal("Expected success or failure"_sv);
    }
    // TODO(thraneh): clear timeout
  });
}

namespace {
auto compute_request_status(RequestType request_type, json::ExecType exec_type) {
  switch (exec_type) {
    case json::ExecType::UNDEFINED:
    case json::ExecType::UNKNOWN:
      break;
    case json::ExecType::NEW: {
      switch (request_type) {
        case RequestType::UNDEFINED:
          log::warn("*** EXTERNAL ACTION ***"_sv);
          break;
        case RequestType::CREATE_ORDER:
          return RequestStatus::ACCEPTED;
        case RequestType::MODIFY_ORDER:
        case RequestType::CANCEL_ORDER:
          log::fatal("DEBUG: UNEXPECTED"_sv);
          break;
      }
      break;
    }
    case json::ExecType::REPLACED: {
      switch (request_type) {
        case RequestType::UNDEFINED:
          log::warn("*** EXTERNAL ACTION ***"_sv);
          break;
        case RequestType::MODIFY_ORDER:
          return RequestStatus::ACCEPTED;
        case RequestType::CREATE_ORDER:
        case RequestType::CANCEL_ORDER:
          log::fatal("DEBUG: UNEXPECTED"_sv);
          break;
      }
      break;
    }
    case json::ExecType::CANCELED: {
      switch (request_type) {
        case RequestType::UNDEFINED:
          log::warn("*** EXTERNAL ACTION ***"_sv);
          break;
        case RequestType::CANCEL_ORDER:
          return RequestStatus::ACCEPTED;
        case RequestType::CREATE_ORDER:
        case RequestType::MODIFY_ORDER:
          log::fatal("DEBUG: UNEXPECTED"_sv);
          break;
      }
      break;
    }
    case json::ExecType::TRADE:
      break;
    case json::ExecType::FUNDING:
      break;
  }
  return RequestStatus::UNDEFINED;
}
}  // namespace

void DropCopy::operator()(
    const json::Action action,
    const json::Execution &execution,
    const server::TraceInfo &trace_info) {
  profile_.execution([&]() {
    log::trace_1("action={}, execution={}"_fmt, action, execution);
    core::back_emplacer fills(shared_.fills);
    size_t index = {};
    for (auto &item : execution.data) {
      auto last = execution.data.size() == ++index;
      server::OMS_Lookup order_lookup{
          .symbol = item.symbol,
          .side = json::map(item.side),
          .status = json::map(item.ord_status),
          .price = item.price,
          .remaining_quantity = item.leaves_qty,
          .traded_quantity = item.cum_qty,
          .timestamp = item.timestamp,  // XXX transact_time?
          .external_account = {},
          .external_order_id = item.order_id,
      };
      auto found = shared_.find_order(
          item.order_id,
          item.cl_ord_id,
          order_lookup,
          trace_info,
          [&](const auto &order, auto &result) {
            result.request_status = compute_request_status(order.request_type, item.exec_type);
            if (result.request_status != RequestStatus::UNDEFINED) {
              result.origin = Origin::EXCHANGE;
              result.error = item.ord_rej_reason.empty() ? Error::UNDEFINED : Error::UNKNOWN,
              result.text = item.ord_rej_reason;
            }
            if (item.exec_type == json::ExecType::TRADE) {
              fills.emplace_back([&](auto &result) {
                auto trade_id = shared_.next_trade_id();
                emplace(result, item, trade_id);
              });
            }
            if (last && !fills.empty()) {
              TradeUpdate trade_update{
                  .stream_id = stream_id_,
                  .account = order.account,
                  .order_id = order.user_order_id,
                  .exchange = order.exchange,
                  .symbol = order.symbol,
                  .side = order.side,
                  .position_effect = {},
                  .order_template = {},
                  .create_time_utc = item.timestamp,  // XXX transact_time?
                  .update_time_utc = item.timestamp,  // XXX transact_time?
                  .gateway_order_id = order.gateway_order_id,
                  .external_account = {},
                  .external_order_id = order.external_order_id,
                  .routing_id = order.routing_id,
                  .fills = fills,
              };
              server::create_trace_and_dispatch(
                  trace_info, trade_update, handler_, true, order.user_id);
            }
          });
      if (!found) {
        log::warn("*** EXTERNAL ORDER ***"_sv);
        log::warn("action={}, execution={}"_fmt, action, execution);
      }
    }
  });
}

void DropCopy::operator()(
    const json::Action action, const json::Margin &margin, const server::TraceInfo &) {
  profile_.margin([&]() {
    log::trace_2("action={}, margin={}"_fmt, action, margin);
    /// XXX not used
  });
}

void DropCopy::operator()(
    const json::Action action, const json::Order &order, const server::TraceInfo &trace_info) {
  profile_.order([&]() {
    log::trace_1("action={}, order={}"_fmt, action, order);
    OrderUpdate{shared_}(order, trace_info);
    // state management
    if (!partial_received_.order && action == json::Action::PARTIAL) {
      partial_received_.order = true;
      // release download state
      download_.check_relaxed(DropCopyState::SUBSCRIBE);
    }
  });
}

void DropCopy::operator()(
    const json::Action action,
    const json::Position &position,
    const server::TraceInfo &trace_info) {
  profile_.position([&]() {
    log::trace_2("action={}, position={}"_fmt, action, position);
    for (auto &item : position.data) {
      PositionUpdate position_update{
          .stream_id = stream_id_,
          .account = security_.get_account(),
          .exchange = Flags::exchange(),
          .symbol = item.symbol,
          .side = {},
          .position = item.current_qty,
          .last_trade_id = {},
          .position_cost = 0.0,
          .position_yesterday = 0.0,
          .position_cost_yesterday = 0.0,
          .external_account = {},
      };
      server::create_trace_and_dispatch(trace_info, position_update, handler_, false);
    }
  });
}

void DropCopy::operator()(
    const json::Action action, const json::Funding &funding, const server::TraceInfo &) {
  log::fatal("Unexpected: action={}, funding={}"_fmt, action, funding);
}

void DropCopy::operator()(
    const json::Action action, const json::Instrument &instrument, const server::TraceInfo &) {
  log::fatal("Unexpected: action={}, instrument={}"_fmt, action, instrument);
}

void DropCopy::operator()(
    const json::Action action, const json::Liquidation &liquidation, const server::TraceInfo &) {
  log::fatal("Unexpected: action={}, liquidation={}"_fmt, action, liquidation);
}

void DropCopy::operator()(
    const json::Action action, const json::OrderBookL2 &order_book_l2, const server::TraceInfo &) {
  log::fatal("Unexpected: action={}, order_book_l2={}"_fmt, action, order_book_l2);
}

void DropCopy::operator()(
    const json::Action action, const json::Quote &quote, const server::TraceInfo &) {
  log::fatal("Unexpected: action={}, quote={}"_fmt, action, quote);
}

void DropCopy::operator()(
    const json::Action action, const json::Settlement &settlement, const server::TraceInfo &) {
  log::fatal("Unexpected: action={}, settlement={}"_fmt, action, settlement);
}

void DropCopy::operator()(
    const json::Action action, const json::Trade &trade, const server::TraceInfo &) {
  log::fatal("Unexpected: action={}, trade={}"_fmt, action, trade);
}

std::string DropCopy::create_upgrade_headers() {
  auto expires = std::chrono::duration_cast<std::chrono::seconds>(
      core::get_realtime_clock() + REQUEST_EXPIRES);
  return security_.create_headers(
      expires, core::http::Method::GET, "/realtime"_sv, std::string_view{});
}

}  // namespace bitmex
}  // namespace roq
