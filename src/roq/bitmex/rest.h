/* Copyright (c) 2017-2021, Hans Erik Thrane */

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
    virtual void operator()(const Rest &) = 0;
    virtual void operator()(const ExternalLatency &, const server::TraceInfo &) = 0;
  };

  Rest(
      Handler &handler,
      const Config &config,
      Random &random,
      core::event::Base &base,
      core::event::DNSBase &dns_base,
      core::ssl::Context &ssl_context);

  Rest(Rest &&) = delete;
  Rest(const Rest &) = delete;

  bool ready() const;

  void operator()(const Event<Start> &);
  void operator()(const Event<Stop> &);
  void operator()(const Event<Timer> &);

  void operator()(metrics::Writer &writer);

  void create_order(
      const CreateOrder &create_order,
      const std::string_view &cl_ord_id,
      std::function<void(const core::Promise<json::OrderItem> &)> &&callback);

  void modify_order(
      const ModifyOrder &modify_order,
      const std::string_view &request_id,
      const server::OMS_Order &order,
      std::function<void(const core::Promise<json::OrderItem> &)> &&callback);

  void cancel_order(
      const CancelOrder &cancel_order,
      const std::string_view &request_id,
      const server::OMS_Order &order,
      std::function<void(const core::Promise<json::Order> &)> &&callback);

  template <typename T>
  void get(std::function<void(const core::Promise<T> &)> &&);

 protected:
  // core::web::Client::Handler

  void operator()(const core::web::Client::Connected &) override;
  void operator()(const core::web::Client::Disconnected &) override;
  void operator()(const core::web::Client::Latency &) override;

 private:
  Handler &handler_;
  // authentication
  Random &random_;
  // connection
  core::web::Client connection_;
  // buffers
  core::utils::Buffer decode_buffer_;
  // metrics
  struct {
    core::metrics::Counter disconnect;
  } counter_;
  struct {
    core::metrics::Profile products, accounts, create_order, modify_order, cancel_order;
  } profile_;
  struct {
    core::metrics::Latency ping;
  } latency_;
};

}  // namespace bitmex
}  // namespace roq
