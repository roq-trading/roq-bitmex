/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/core/promise.h"

#include "roq/core/utils/buffer.h"

#include "roq/core/metrics/counter.h"
#include "roq/core/metrics/latency.h"
#include "roq/core/metrics/profile.h"

#include "roq/core/ssl/ssl.h"

#include "roq/core/event/base.h"
#include "roq/core/event/dns_base.h"

#include "roq/core/web/client.h"

#include "roq/server.h"

#include "roq/bitmex/config.h"
#include "roq/bitmex/random.h"

#include "roq/bitmex/json/order.h"
#include "roq/bitmex/json/order_item.h"

namespace roq {
namespace bitmex {

class Rest final : public core::web::Client::Handler {
 public:
  struct Handler {
    virtual void operator()(const Rest&) = 0;
  };

  Rest(
      Handler& handler,
      const Config& config,
      Random& random,
      core::event::Base& base,
      core::event::DNSBase& dns_base,
      core::ssl::Context& ssl_context);

  Rest(Rest&&) = delete;
  Rest(const Rest&) = delete;

  bool ready() const;

  void operator()(const server::StartEvent&);
  void operator()(const server::StopEvent&);
  void operator()(const server::TimerEvent&);

  void operator()(Metrics& metrics);

  void create_order(
      const CreateOrder& create_order,
      const std::string_view& cl_ord_id,
      std::function<void(const core::Promise<json::OrderItem>&)>&& callback);

  void modify_order(
      const ModifyOrder& modify_order,
      const std::string_view& request_id,
      const server::OMS_Order& order,
      std::function<void(const core::Promise<json::OrderItem>&)>&& callback);

  void cancel_order(
      const CancelOrder& cancel_order,
      const std::string_view& request_id,
      const server::OMS_Order& order,
      std::function<void(const core::Promise<json::Order>&)>&& callback);

  template <typename T>
  void get(std::function<void(const core::Promise<T>&)>&&);

 protected:
  // core::web::Client::Handler

  void operator()(const core::web::Client::Connected&) override;
  void operator()(const core::web::Client::Disconnected&) override;
  void operator()(const core::web::Client::Latency&) override;

 private:
  Handler& _handler;
  // authentication
  Random& _random;
  // connection
  core::web::Client _connection;
  // buffers
  core::utils::Buffer _decode_buffer;
  // metrics
  struct {
    core::metrics::Counter
      disconnect;
  } _counter;
  struct {
    core::metrics::Profile
      products,
      accounts,
      create_order,
      modify_order,
      cancel_order;
  } _profile;
  struct {
    core::metrics::Latency
      ping;
  } _latency;
};

}  // namespace bitmex
}  // namespace roq
