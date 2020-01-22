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

#define PREFIX "[REST] "

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
  LOG(INFO)(PREFIX
      "Connecting to host=\"{}\", port={}",
      uri.host, uri.get_port_with_default());
  assert(_state == State::DISCONNECTED);
  _buffer_event.connect(
      dns_base,
      AF_INET,
      uri.host,
      uri.get_port_with_default());
  _state = State::CONNECTING;
}

void HTTPConnection::write(const void *data, size_t length) {
  VLOG(4)(PREFIX
      "send(length={})", length);
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
    LOG(ERROR)(PREFIX "Exception: what=\"{}\"", e.what());
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
    core::event::Base& base,
    core::event::DNSBase& dns_base,
    core::ssl::Context& ssl_context)
    : _gateway(gateway),
      _access_key(config.get_api_key()),
      _access_secret(config.get_secret()),
      _uri(FLAGS_rest_uri),
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
      },
      _latency {
        .ping = create_latency("ping"),
      } {
  LOG_IF(FATAL, _uri.scheme.compare("https") != 0)(
      "Expected URI scheme to be \"https\" (got \"{}\")",
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

void Rest::get_products() {
  get(
      "/products",
      false,
      [this](const std::string_view& body) {
        _profile.products(
            [&]() {
              /*
              core::json::Buffer buffer(_decode_buffer);
              auto products = json::Products::parse(
                  body,
                  buffer);
              VLOG(1)(PREFIX "products={}", products);
              _gateway(products);
              */
            });
      },
      [](auto& status) {
        LOG(WARNING)(PREFIX "HTTP status={}", status);
        LOG(WARNING)(PREFIX "Unable to get products");
        LOG(FATAL)(PREFIX "Unexpected -- now what?");  // FIXME(thraneh): ...
      });
  // DEBUG
  get_time();
}

void Rest::get_accounts() {
  get(
      "/accounts",
      true,
      [this](const std::string_view& body) {
        _profile.accounts(
            [&]() {
              /*
              core::json::Buffer buffer(_decode_buffer);
              auto accounts = json::Accounts::parse(
                  body,
                  buffer);
              VLOG(1)(PREFIX "accounts={}", accounts);
              _gateway(accounts);
              */
            });
      },
      [this](auto& status) {
        LOG(WARNING)(PREFIX "HTTP status={}", status);
        LOG(WARNING)(PREFIX "Unable to get accounts");
        LOG(FATAL)(PREFIX "Unexpected -- now what?");  // FIXME(thraneh): ...
      });
}

void Rest::get_time() {
  get(
      "/time",
      false,
      [this](const std::string_view& body) {
        _profile.products(
            [&]() {
              /*
              auto time = json::Time::parse(body);
              VLOG(1)(PREFIX "time={}", time);
              */
            });
      },
      [](auto& status) {
        LOG(WARNING)(PREFIX "HTTP status={}", status);
        LOG(WARNING)(PREFIX "Unable to get products");
        LOG(FATAL)(PREFIX "Unexpected -- now what?");  // FIXME(thraneh): ...
      });
}

void Rest::get(
    const std::string_view& uri,
    bool authenticate,
    success_t&& success,
    failure_t&& failure) {
  LOG(INFO)(PREFIX "GET {}", uri);
  auto create_time = core::get_system_clock();
  switch (_state) {
    case State::DISCONNECTED:
      connect();
      [[ fallthrough ]];
    case State::DISCONNECTING:  // will fail in order
    case State::CONNECTING:
      make_pending(
          uri,
          authenticate,
          create_time,
          std::move(success),
          std::move(failure));
      break;
    case State::CONNECTED:
      if (_waiting.empty() &&
          request(
            core::http::Method::GET,
            uri,
            authenticate)) {
        make_sent(
            create_time,
            create_time,
            std::move(success),
            std::move(failure));
      } else {
        make_pending(
            uri,
            authenticate,
            create_time,
            std::move(success),
            std::move(failure));
      }
      break;
    default:
      LOG(FATAL)(PREFIX "Unexpected");
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
      break;
    case State::DISCONNECTING:
      assert(_connection);
      LOG(INFO)(PREFIX "Disconnected");
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
  LOG(INFO)(PREFIX "Disconnecting...");
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
    if (request(
          core::http::Method::GET,
          std::get<0>(front),
          std::get<1>(front)) == false)
      return;
    make_sent(
        std::get<2>(front),
        core::get_system_clock(),
        std::move(std::get<3>(front)),
        std::move(std::get<4>(front)));
    _waiting.pop_front();
  }
}

bool Rest::request(
    const core::http::Method& method,
    const std::string_view& path,
    bool authenticate) {
  LOG(INFO)(PREFIX "Sending method={} path=\"{}\"", method, path);
  assert(_state == State::CONNECTED);
  assert(_connection);
  if (throttle()) {
    LOG(WARNING)(PREFIX "Request is pending due to throttling");
    return false;
  }
  std::string headers;
  if (authenticate) {
    // *must* be seconds (see bitmex-pro api documentation)
    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        core::get_realtime_clock());
    Random random(
        _access_key,
        _access_secret);
    headers = random.create_headers(
        now,
        core::http::Method::GET,
        path);
  }
  fmt::memory_buffer buffer;
  fmt::format_to(
      buffer,
      "{} {} HTTP/1.1\r\n"
      "Host: {}\r\n"
      "User-Agent: roq-bitmex-pro/{}\r\n"
      "Accept: */*\r\n"
      "{}"
      "\r\n",
      method,
      path,
      _uri.host,
      ROQ_VERSION,
      headers);
  std::string_view request(
      buffer.data(),
      buffer.size());
  VLOG(1)(PREFIX "{}", request);
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
    const std::string_view& uri,
    bool authenticate,
    std::chrono::nanoseconds create_time,
    success_t&& success,
    failure_t&& failure) {
  _waiting.push_back(
      std::make_tuple(
          std::string(uri),
          authenticate,
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
    LOG(WARNING)(PREFIX
        "Unexpected body={}", body);
    return false;
  }
  auto now = core::get_system_clock();
  VLOG(3)("body=\"{}\"", body);
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
            std::get<3>(front)(status);  // failure handler
          });
    }
  } catch (std::exception& e) {
    LOG(WARNING)("exception, what=\"{}\"", e.what());
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
