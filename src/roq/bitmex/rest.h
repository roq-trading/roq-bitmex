/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <list>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include "roq/core/utils/buffer.h"

#include "roq/core/metrics/counter.h"
#include "roq/core/metrics/latency.h"
#include "roq/core/metrics/profile.h"

#include "roq/core/uri.h"

#include "roq/core/ssl/ssl.h"

#include "roq/core/event/base.h"
#include "roq/core/event/buffer.h"
#include "roq/core/event/buffer_event.h"
#include "roq/core/event/dns_base.h"

#include "roq/core/http/method.h"
#include "roq/core/http/request.h"
#include "roq/core/http/response.h"

#include "roq/core/ws/decoder.h"

#include "roq/bitmex/config.h"
#include "roq/bitmex/random.h"

namespace roq {
namespace bitmex {

using success_t = std::function<void(const std::string_view&)>;
using failure_t = std::function<void(const core::http::Status&)>;

class HTTPConnection final : public core::http::Response::Handler {
 public:
  struct connected_t final {
  };
  struct disconnected_t final {
  };
  struct status_t final {
    core::http::Status status;
  };
  struct header_t final {
    core::http::Header header;
    std::string_view value;
  };
  struct body_t final {
    const void *data;
    size_t length;
  };
  struct Handler {
    virtual void operator()(const connected_t&) = 0;
    virtual void operator()(const disconnected_t&) = 0;
    virtual void operator()(const status_t&) = 0;
    virtual void operator()(const header_t&) = 0;
    virtual void operator()(const body_t&) = 0;
  };
  HTTPConnection(
      Handler& handler,
      const core::URI& uri,
      core::ssl::Context& ssl_context,
      core::event::Base& base);

  HTTPConnection(const HTTPConnection&) = delete;
  HTTPConnection(HTTPConnection&&) = delete;

  void connect(
      core::event::DNSBase& dns_base,
      const core::URI& uri);

  void write(const void *data, size_t length);

  void stop();

 protected:
  void on_read();
  void on_error(int events);

 protected:
  void operator()(const core::http::Response::MessageBegin&) override;
  void operator()(const core::http::Response::URL&) override;
  void operator()(const core::http::Response::Status&) override;
  void operator()(const core::http::Response::HeaderField&) override;
  void operator()(const core::http::Response::HeaderValue&) override;
  void operator()(const core::http::Response::HeadersComplete&) override;
  void operator()(const core::http::Response::ChunkHeader&) override;
  void operator()(const core::http::Response::Body&) override;
  void operator()(const core::http::Response::ChunkComplete&) override;
  void operator()(const core::http::Response::MessageComplete&) override;

 private:
  Handler& _handler;
  core::ssl::Connection _ssl_connection;
  core::event::BufferEvent _buffer_event;
  core::event::Buffer _buffer;
  enum State {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
  } _state = State::DISCONNECTED;
  // http-parser
  core::http::Response _response;
  core::http::Status _status = core::http::Status::UNKNOWN;
  core::http::Header _header = core::http::Header::UNKNOWN;
  std::vector<std::byte> _chunk_buffer;
  size_t _buffer_offset = 0;
};

class Gateway;

class Rest final : public HTTPConnection::Handler {
  enum State {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    DISCONNECTING,
  };

 public:
  Rest(
      Gateway& gateway,
      const Config& config,
      Random& random,
      core::event::Base& base,
      core::event::DNSBase& dns_base,
      core::ssl::Context& ssl_context);

  Rest(const Rest&) = delete;
  Rest(Rest&&) = delete;

  void operator()(const StartEvent&);
  void operator()(const StopEvent&);
  void operator()(const TimerEvent&);

  void operator()(Metrics& metrics);

  void create_order(const CreateOrder& create_order);

  void get_products();
  void get_accounts();

 private:
  void on_timer();

  void check_timeout();

  void connect();

  void process_pending();

  void get_time();

  void get(
      const std::string_view& uri,
      success_t&& success,
      failure_t&& failure);

  void post(
      const std::string_view& uri,
      const std::string_view& body,
      success_t&& success,
      failure_t&& failure);

  bool request(
      core::http::Method method,
      const std::string_view& path,
      const std::string_view& body);

  bool throttle();

  void make_pending(
      core::http::Method,
      const std::string_view& uri,
      const std::string_view& body,
      std::chrono::nanoseconds create_time,
      success_t&& success,
      failure_t&& failure);

  void make_sent(
      std::chrono::nanoseconds create_time,
      std::chrono::nanoseconds send_time,
      success_t&& success,
      failure_t&& failure);

  bool remove_one(
      const core::http::Status& status,
      const std::string_view& body);
  void remove_all();

 protected:
  void operator()(const HTTPConnection::connected_t&);
  void operator()(const HTTPConnection::disconnected_t&);
  void operator()(const HTTPConnection::status_t&);
  void operator()(const HTTPConnection::header_t&);
  void operator()(const HTTPConnection::body_t&);

 private:
  Gateway& _gateway;
  // config
  const core::URI _uri;
  // authentication
  Random& _random;
  // async
  core::event::Base& _base;
  core::event::DNSBase& _dns_base;
  // crypto
  core::ssl::Context& _ssl_context;
  // connection
  std::unique_ptr<HTTPConnection> _connection;
  // buffers
  core::utils::Buffer _decode_buffer;
  // metrics
  struct {
    core::metrics::Counter
      disconnect;
  } _counter;
  struct {
    core::metrics::Profile
      success,
      failure,
      products,
      accounts,
      create_order;
  } _profile;
  struct {
    core::metrics::Latency
      ping;
  } _latency;
  // session
  State _state = State::DISCONNECTED;
  // request pipeline
  std::list<
    std::tuple<
      core::http::Method,
      std::string,  // uri
      std::string,  // body
      std::chrono::nanoseconds,
      success_t,
      failure_t> > _waiting;
  std::list<
    std::tuple<
      std::chrono::nanoseconds,
      std::chrono::nanoseconds,
      success_t,
      failure_t> > _sent;
  // throttling
  std::chrono::nanoseconds _window = {};
  int _request_count = 0;
  // XXX weird to have here ...
  core::http::Status _status = core::http::Status::UNKNOWN;
};

}  // namespace bitmex
}  // namespace roq
