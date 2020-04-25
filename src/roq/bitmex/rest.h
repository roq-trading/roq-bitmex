/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/core/utils/buffer.h"

#include "roq/core/metrics/counter.h"
#include "roq/core/metrics/latency.h"
#include "roq/core/metrics/profile.h"

#include "roq/core/ssl/ssl.h"

#include "roq/core/event/base.h"
#include "roq/core/event/dns_base.h"

#include "roq/core/web/client.h"

#include "roq/bitmex/config.h"
#include "roq/bitmex/random.h"

namespace roq {
namespace bitmex {

class Gateway;

class Rest final : public core::web::Client::Handler {
 public:
  Rest(
      Gateway& gateway,
      const Config& config,
      Random& random,
      core::event::Base& base,
      core::event::DNSBase& dns_base,
      core::ssl::Context& ssl_context);

  Rest(Rest&&) = delete;
  Rest(const Rest&) = delete;

  bool ready() const;

  void close();

  void operator()(const StartEvent&);
  void operator()(const StopEvent&);
  void operator()(const TimerEvent&);

  void operator()(Metrics& metrics);

  void create_order(
      const CreateOrder& create_order,
      const std::string_view& cl_ord_id,
      std::function<void(const core::web::Response&)>&& callback);

  void modify_order(
      const ModifyOrder& modify_order,
      const std::string_view& request_id,
      const server::OMS_Order& order,
      std::function<void(const core::web::Response&)>&& callback);

  void cancel_order(
      const CancelOrder& cancel_order,
      const std::string_view& request_id,
      const server::OMS_Order& order,
      std::function<void(const core::web::Response&)>&& callback);

  void get_accounts(
      std::function<void(const core::web::Response&)>&& callback);

 protected:
  void operator()(const core::web::Client::Connected&) override;
  void operator()(const core::web::Client::Disconnected&) override;
  void operator()(const core::web::Client::Latency&) override;

 private:
  Gateway& _gateway;
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
      success,
      failure,
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
