/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/rest.h"

#include <netinet/tcp.h>

#include <fmt/format.h>
#include <fmt/chrono.h>

#include <utility>

#include "roq/patterns.h"

#include "roq/core/clock.h"

#include "roq/core/http/response.h"

#include "roq/bitmex/gateway.h"
#include "roq/bitmex/options.h"
#include "roq/bitmex/random.h"

#include "roq/bitmex/json/utils.h"

namespace roq {
namespace bitmex {

namespace {
constexpr auto TIMEOUT_SECONDS = std::chrono::seconds{60};
constexpr auto MAX_REQUESTS_PER_SECOND = 3;
}  // namespace

// HTTP CONNECTION

namespace {
static auto create_connection(auto& ssl_context, auto& uri_host) {
  core::ssl::Connection result(ssl_context);
  result.set_host_name(uri_host);
  return result;
}
}  // namespace

HTTPConnection::HTTPConnection(
    Handler& handler,
    const core::URI& uri,
    core::ssl::Context& ssl_context,
    core::event::Base& base)
    : _handler(handler),
      _ssl_connection(create_connection(ssl_context, uri.host)),
      _buffer_event(base, _ssl_connection),
      _response(*this),
      _chunk_buffer(1024*1024) {
  _buffer_event.setcb(
      [this]() { on_read(); },
      [this](int events) { on_error(events); });
  _buffer_event.enable(EV_READ);
}

void HTTPConnection::connect(
    core::event::DNSBase& dns_base,
    const core::URI& uri) {
  LOG(INFO)(
      FMT_STRING("Connecting to host=\"{}\", port={}"),
      uri.host,
      uri.get_port_with_default());
  assert(_state == State::DISCONNECTED);
  _buffer_event.connect(
      dns_base,
      AF_INET,
      uri.host,
      uri.get_port_with_default());
  _state = State::CONNECTING;
}

void HTTPConnection::write(const void *data, size_t length) {
  VLOG(4)(
      FMT_STRING("send(length={})"),
      length);
  _buffer_event.write(data, length);
  _buffer_event.flush(EV_WRITE, BEV_FLUSH);
}

void HTTPConnection::stop() {
  _buffer_event.shutdown(SHUT_RDWR);
}

void HTTPConnection::on_read() {
  try {
    _buffer_event.read(_buffer);
    if (_buffer.empty())
      return;
    auto bytes = _buffer.length();
    auto buffer = _buffer.pullup(bytes);
    auto length = _response.dispatch(
        reinterpret_cast<const char *>(buffer),
        bytes);
    _buffer.drain(length);
  } catch (std::exception& e) {
    LOG(ERROR)(
        FMT_STRING("Exception: what=\"{}\""),
        e.what());
    stop();
  }
}

void HTTPConnection::on_error(int events) {
  if (events & BEV_EVENT_CONNECTED) {
    _buffer_event.setsockopt(IPPROTO_TCP, TCP_NODELAY, int{1});
    _handler(connected_t{});
  } else {
    _handler(disconnected_t{});
  }
}

void HTTPConnection::operator()(
    const core::http::Response::MessageBegin&) {
  assert(_status == core::http::Status::UNKNOWN);
  assert(_header == core::http::Header::UNKNOWN);
  assert(_buffer_offset == 0);
}

void HTTPConnection::operator()(
    const core::http::Response::URL&) {
  assert(false);  // only client
}

void HTTPConnection::operator()(
    const core::http::Response::Status& status) {
  assert(_header == core::http::Header::UNKNOWN);
  assert(_buffer_offset == 0);
  _status = core::http::parse_status(status.code);
  _handler(
      status_t {
        .status = _status,
      });
}

void HTTPConnection::operator()(
    const core::http::Response::HeaderField& header_field) {
  assert(_buffer_offset == 0);
  _header = core::http::parse_header(header_field.text);
}

void HTTPConnection::operator()(
    const core::http::Response::HeaderValue& header_value) {
  assert(_buffer_offset == 0);
  _handler(
      header_t {
        .header = _header,
        .value = header_value.text,
      });
  _header = core::http::Header::UNKNOWN;
}

void HTTPConnection::operator()(
    const core::http::Response::HeadersComplete&) {
  assert(_header == core::http::Header::UNKNOWN);
  assert(_buffer_offset == 0);
}

void HTTPConnection::operator()(
    const core::http::Response::ChunkHeader&) {
  assert(_header == core::http::Header::UNKNOWN);
}

void HTTPConnection::operator()(
    const core::http::Response::Body& body) {
  assert(_header == core::http::Header::UNKNOWN);
  std::memcpy(
      _chunk_buffer.data() + _buffer_offset,
      body.text.data(),
      body.text.length());
  _buffer_offset += body.text.length();
}

void HTTPConnection::operator()(
    const core::http::Response::ChunkComplete&) {
  assert(_header == core::http::Header::UNKNOWN);
}

void HTTPConnection::operator()(
    const core::http::Response::MessageComplete&) {
  assert(_header == core::http::Header::UNKNOWN);
  _handler(
      body_t {
        .data = _chunk_buffer.data(),
        .length = _buffer_offset
      });
  _status = core::http::Status::UNKNOWN;
  _buffer_offset = 0;
}

// REST

constexpr std::string_view CONNECTION("rest");

static auto create_counter(
    const std::string_view& function) {
  return core::metrics::Counter(
      FLAGS_name,
      CONNECTION,
      function);
}

static auto create_profile(
    const std::string_view& function) {
  return core::metrics::Profile(
      FLAGS_name,
      CONNECTION,
      function);
}

static auto create_latency(
    const std::string_view& function) {
  return core::metrics::Latency(
      FLAGS_name,
      CONNECTION,
      function);
}

Rest::Rest(
    Gateway& gateway,
    const Config& config,
    Random& random,
    core::event::Base& base,
    core::event::DNSBase& dns_base,
    core::ssl::Context& ssl_context)
    : _gateway(gateway),
      _uri(FLAGS_rest_uri),
      _random(random),
      _base(base),
      _dns_base(dns_base),
      _ssl_context(ssl_context),
      _decode_buffer(FLAGS_decode_buffer_size),
      _counter {
        .disconnect = create_counter("disconnect"),
      },
      _profile {
        .success = create_profile("success"),
        .failure = create_profile("failure"),
        .products = create_profile("products"),
        .accounts = create_profile("accounts"),
        .create_order = create_profile("create_order"),
      },
      _latency {
        .ping = create_latency("ping"),
      } {
  LOG_IF(FATAL, _uri.scheme.compare("https") != 0)(
      FMT_STRING("Expected URI scheme to be \"https\" (got \"{}\")"),
      _uri.scheme);
}

void Rest::operator()(const StartEvent&) {
}

void Rest::operator()(const StopEvent&) {
}

void Rest::operator()(const TimerEvent&) {
  on_timer();
}

void Rest::operator()(Metrics& metrics) {
  metrics
    // counter
    .write(_counter.disconnect)
    // profile
    .write(_profile.success)
    .write(_profile.failure)
    .write(_profile.products)
    .write(_profile.accounts)
    // latency
    .write(_latency.ping);
}

void Rest::create_order(
    const CreateOrder& create_order,
    const std::string_view& cl_ord_id) {
  // XXX use encode buffer
  auto message = fmt::format(
      FMT_STRING(
        R"({{)"
        R"("clOrdID":"{}",)"
        R"("symbol":"{}",)"
        R"("side":"{}",)"
        R"("price":{},)"
        R"("orderQty":{},)"
        R"("ordType":"{}",)"
        R"("timeInForce":"{}")"
        R"(}})"),
      cl_ord_id,
      create_order.symbol,
      json::c_str(create_order.side),
      create_order.price,
      create_order.quantity,
      json::c_str(create_order.order_type),
      json::c_str(create_order.time_in_force));
  LOG(INFO)(
      FMT_STRING("DEBUG: body=\"{}\""),
      message);
  post(
      "/order",
      message,
      [this](const std::string_view& body) {
        _profile.create_order(
            [&]() {
              LOG(INFO)(
                  FMT_STRING("DEBUG: body=\"{}\""),
                  body);
              auto order_item = core::json::Parser::create<json::OrderItem>(
                  body);
              LOG(INFO)(FMT_STRING("DEBUG: order_item={}"), order_item);
              // _gateway(json::Action::INSERT, order);
            });
      },
      [](auto& status, auto& body) {
        LOG(WARNING)(
            FMT_STRING("HTTP status={} body=\"{}\""),
            status,
            body);
        LOG(WARNING)("Unable to create order");
        LOG(FATAL)("Unexpected -- now what?");  // FIXME(thraneh): ...
      });
}

void Rest::get_products() {
  get(
      "/products",
      [this](const std::string_view& body) {
        _profile.products(
            [&]() {
              /*
              core::json::Buffer buffer(_decode_buffer);
              auto products = json::Products::parse(
                  body,
                  buffer);
              VLOG(1)("products={}", products);
              _gateway(products);
              */
            });
      },
      [](auto& status, auto& body) {
        LOG(WARNING)(
            FMT_STRING("HTTP status={} body=\"{}\""),
            status,
            body);
        LOG(WARNING)("Unable to get products");
        LOG(FATAL)("Unexpected -- now what?");  // FIXME(thraneh): ...
      });
  get_time();  // XXX DEBUG
}

void Rest::get_accounts() {
  get(
      "/accounts",
      [this](const std::string_view& body) {
        _profile.accounts(
            [&]() {
              /*
              core::json::Buffer buffer(_decode_buffer);
              auto accounts = json::Accounts::parse(
                  body,
                  buffer);
              VLOG(1)("accounts={}", accounts);
              _gateway(accounts);
              */
            });
      },
      [this](auto& status, auto& body) {
        LOG(WARNING)(
            FMT_STRING("HTTP status={} body=\"{}\""),
            status,
            body);
        LOG(WARNING)("Unable to get accounts");
        LOG(FATAL)("Unexpected -- now what?");  // FIXME(thraneh): ...
      });
}

void Rest::get_time() {
  get(
      "/time",
      [this](const std::string_view& body) {
        _profile.products(
            [&]() {
              /*
              auto time = json::Time::parse(body);
              VLOG(1)("time={}", time);
              */
            });
      },
      [](auto& status, auto& body) {
        LOG(WARNING)(
            FMT_STRING("HTTP status={} body=\"{}\""),
            status,
            body);
        LOG(WARNING)("Unable to get products");
        LOG(FATAL)("Unexpected -- now what?");  // FIXME(thraneh): ...
      });
}

void Rest::get(
    const std::string_view& uri,
    success_t&& success,
    failure_t&& failure) {
  LOG(INFO)(
      FMT_STRING("GET {}"),
      uri);
  auto create_time = core::get_system_clock();
  switch (_state) {
    case State::DISCONNECTED:
      connect();
      [[ fallthrough ]];
    case State::DISCONNECTING:  // will fail in order
    case State::CONNECTING:
      make_pending(
          core::http::Method::GET,
          uri,
          std::string_view(),
          create_time,
          std::move(success),
          std::move(failure));
      break;
    case State::CONNECTED:
      if (_waiting.empty() &&
          request(
            core::http::Method::GET,
            uri,
            std::string_view())) {
        make_sent(
            create_time,
            create_time,
            std::move(success),
            std::move(failure));
      } else {
        make_pending(
            core::http::Method::GET,
            uri,
            std::string_view(),
            create_time,
            std::move(success),
            std::move(failure));
      }
      break;
    default:
      LOG(FATAL)("Unexpected");
  }
}

void Rest::post(
    const std::string_view& uri,
    const std::string_view& body,
    success_t&& success,
    failure_t&& failure) {
  LOG(INFO)(
      FMT_STRING("POST {}"),
      uri);
  auto create_time = core::get_system_clock();
  switch (_state) {
    case State::DISCONNECTED:
      connect();
      [[ fallthrough ]];
    case State::DISCONNECTING:  // will fail in order
    case State::CONNECTING:
      make_pending(
          core::http::Method::POST,
          uri,
          body,
          create_time,
          std::move(success),
          std::move(failure));
      break;
    case State::CONNECTED:
      if (_waiting.empty() &&
          request(
            core::http::Method::POST,
            uri,
            body)) {
        make_sent(
            create_time,
            create_time,
            std::move(success),
            std::move(failure));
      } else {
        make_pending(
            core::http::Method::POST,
            uri,
            body,
            create_time,
            std::move(success),
            std::move(failure));
      }
      break;
    default:
      LOG(FATAL)("Unexpected");
  }
}

void Rest::on_timer() {
  switch (_state) {
    case State::DISCONNECTED:
    case State::CONNECTING:
      break;
    case State::CONNECTED:
      if (_waiting.empty())
        check_timeout();
      else
        process_pending();
      send_ping();
      break;
    case State::DISCONNECTING:
      assert(_connection);
      LOG(INFO)("Disconnected");
      _connection.reset();
      _state = State::DISCONNECTED;
      break;
  }
}

void Rest::check_timeout() {
  assert(_state != State::DISCONNECTING);
  auto now = core::get_system_clock();
  if ((now - _window) < TIMEOUT_SECONDS)
    return;
  LOG(INFO)("Disconnecting...");
  _state = State::DISCONNECTING;
  _connection->stop();
}

void Rest::connect() {
  assert(!_connection);
  _connection = std::make_unique<HTTPConnection>(
      *this,
      _uri,
      _ssl_context,
      _base);
  _connection->connect(_dns_base, _uri);
  _state = State::CONNECTING;
}

void Rest::process_pending() {
  while (_waiting.empty() == false) {
    auto& front = _waiting.front();
    if (false == request(
          std::get<0>(front),  // method
          std::get<1>(front),  // uri
          std::get<2>(front)))  // body
      return;
    make_sent(
        std::get<3>(front),  // create time
        core::get_system_clock(),  // send time
        std::move(std::get<4>(front)),  // success
        std::move(std::get<5>(front)));  // failure
    _waiting.pop_front();
  }
}

void Rest::send_ping() {
  std::chrono::nanoseconds now = core::get_system_clock();
  if (now < _next_ping)
    return;
  _next_ping = now + std::chrono::seconds{
      FLAGS_cancel_all_after_secs / 4
  };
  LOG(INFO)("PING");
  auto message = fmt::format(
      FMT_STRING(
        R"({{)"
        R"("timeout":"{}")"
        R"(}})"),
      FLAGS_cancel_all_after_secs * 1000);
  post(
      "/order/cancelAllAfter",
      message,
      [this](const std::string_view& body) {
        LOG(INFO)(FMT_STRING("{}"), body);
      },
      [](auto& status, auto& body) {
        LOG(INFO)(FMT_STRING("{} {}"), status, body);
        LOG(WARNING)("Unable to create order");
        LOG(FATAL)("Unexpected -- now what?");  // FIXME(thraneh): ...
      });
}

bool Rest::request(
    core::http::Method method,
    const std::string_view& path,
    const std::string_view& body) {
  LOG(INFO)(
      FMT_STRING("Sending method={} path=\"{}\""),
      method,
      path);
  assert(_state == State::CONNECTED);
  assert(_connection);
  if (throttle()) {
    LOG(WARNING)("Request is pending due to throttling");
    return false;
  }
  // *must* be seconds (see bitmex-pro api documentation)
  auto now = std::chrono::duration_cast<std::chrono::seconds>(
      core::get_realtime_clock());
  auto expires = now + std::chrono::seconds{
    FLAGS_request_expires_secs
  };
  auto full_path = fmt::format("{}{}", _uri.path, path);
  auto headers = _random.create_headers(
      expires,
      method,
      full_path,
      body);
  fmt::memory_buffer buffer;
  if (body.empty()) {
    fmt::format_to(
        buffer,
        "{} {} HTTP/1.1\r\n"
        "Host: {}\r\n"
        "User-Agent: roq-bitmex/{}\r\n"
        "Accept: application/json\r\n"
        "Connection: keep-alive\r\n"
        "{}"
        "\r\n",
        method,
        full_path,
        _uri.host,
        ROQ_VERSION,
        headers);
  } else {
    fmt::format_to(
        buffer,
        "{} {} HTTP/1.1\r\n"
        "Host: {}\r\n"
        "User-Agent: roq-bitmex/{}\r\n"
        "Accept: */*\r\n"
        "Connection: keep-alive\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: {}\r\n"
        "{}"
        "\r\n"
        "{}",
        method,
        full_path,
        _uri.host,
        ROQ_VERSION,
        body.length(),
        headers,
        body);
  }
  std::string_view request(
      buffer.data(),
      buffer.size());
  LOG(INFO)(
      FMT_STRING("DEBUG: {}"),
      request);
  _connection->write(
      request.data(),
      request.length());
  return true;
}

bool Rest::throttle() {
  auto now = core::get_system_clock();
  auto window = std::chrono::floor<std::chrono::seconds>(now);
  if (_window != window) {
    _window = window;
    _request_count = 0;
  }
  if (MAX_REQUESTS_PER_SECOND <= _request_count)
    return true;
  ++_request_count;
  return false;
}

void Rest::make_pending(
    core::http::Method method,
    const std::string_view& uri,
    const std::string_view& body,
    std::chrono::nanoseconds create_time,
    success_t&& success,
    failure_t&& failure) {
  _waiting.push_back(
      std::make_tuple(
        method,
          std::string(uri),  // copy
          std::string(body),  // copy
          create_time,
          std::move(success),
          std::move(failure)));
}

void Rest::make_sent(
    std::chrono::nanoseconds create_time,
    std::chrono::nanoseconds send_time,
    success_t&& success,
    failure_t&& failure) {
  _sent.push_back(
      std::make_tuple(
          create_time,
          send_time,
          std::move(success),
          std::move(failure)));
}

bool Rest::remove_one(
    const core::http::Status& status,
    const std::string_view& body) {
  if (_sent.empty()) {
    LOG(WARNING)(
        FMT_STRING("Unexpected body={}"),
        body);
    return false;
  }
  auto now = core::get_system_clock();
  VLOG(3)(FMT_STRING("body=\"{}\""), body);
  auto& front = _sent.front();
  try {
    auto create_time = std::get<0>(front);
    auto send_time = std::get<1>(front);
    auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now - send_time);
    _latency.ping.update(latency.count());
    if (status == core::http::Status::OK) {
      _profile.success(
          [&]() {
            std::get<2>(front)(body);  // success handler
          });
    } else {
      _profile.failure(
          [&]() {
            std::get<3>(front)(status, body);  // failure handler
          });
    }
  } catch (std::exception& e) {
    LOG(WARNING)(FMT_STRING("exception, what=\"{}\""), e.what());
  }
  _sent.pop_front();
  return true;
}

void Rest::remove_all() {
  core::http::Status status = core::http::Status::UNKNOWN;
  std::string_view body;
  while (remove_one(status, body)) {
  }
}

void Rest::operator()(const HTTPConnection::connected_t&) {
  assert(_state != State::CONNECTED);
  assert(_sent.empty());
  LOG(INFO)("Connected");
  _state = State::CONNECTED;
  _gateway(*this);
  process_pending();
}

void Rest::operator()(const HTTPConnection::disconnected_t&) {
  LOG_IF(INFO, _state != State::DISCONNECTING)("Disconnecting...");
  _state = State::DISCONNECTING;
  _gateway(*this);
  ++_counter.disconnect;
}

void Rest::operator()(const HTTPConnection::status_t& status) {
  _status = status.status;
}

void Rest::operator()(const HTTPConnection::header_t&) {
}

void Rest::operator()(const HTTPConnection::body_t& body) {
  std::string_view body_(
      reinterpret_cast<const char *>(body.data),
      body.length);
  if (!remove_one(_status, body_))
    throw std::runtime_error("Unexpected [process]");
  _status = core::http::Status::UNKNOWN;
}

}  // namespace bitmex
}  // namespace roq
