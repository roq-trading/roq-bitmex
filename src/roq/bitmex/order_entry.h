/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/core/promise.h"

#include "roq/core/buffer.h"

#include "roq/core/metrics/counter.h"
#include "roq/core/metrics/latency.h"
#include "roq/core/metrics/profile.h"

#include "roq/core/io/context.h"

#include "roq/core/web/client.h"

#include "roq/server.h"

#include "roq/bitmex/security.h"
#include "roq/bitmex/shared.h"

#include "roq/bitmex/json/order.h"
#include "roq/bitmex/json/order_item.h"

namespace roq {
namespace bitmex {

class OrderEntry final : public core::web::Client::Handler {
 public:
  struct Handler {
    virtual void operator()(const server::Trace<StreamStatus> &) = 0;
    virtual void operator()(const server::Trace<ExternalLatency> &) = 0;
  };

  OrderEntry(Handler &, core::io::Context &, uint16_t stream_id, Security &, Shared &);

  OrderEntry(OrderEntry &&) = delete;
  OrderEntry(const OrderEntry &) = delete;

  void operator()(const Event<Start> &);
  void operator()(const Event<Stop> &);
  void operator()(const Event<Timer> &);

  void operator()(metrics::Writer &);

  uint16_t operator()(const Event<CreateOrder> &, const std::string_view &request_id);
  uint16_t operator()(
      const Event<ModifyOrder> &, const std::string_view &request_id, const server::OMS_Order &);
  uint16_t operator()(
      const Event<CancelOrder> &, const std::string_view &request_id, const server::OMS_Order &);
  uint16_t operator()(const Event<CancelAllOrders> &);

 protected:
  void operator()(const core::web::Client::Connected &) override;
  void operator()(const core::web::Client::Disconnected &) override;
  void operator()(const core::web::Client::Latency &) override;

 private:
  void operator()(ConnectionStatus);

  void create_order(
      const CreateOrder &,
      const std::string_view &cl_ord_id,
      std::function<void(const core::Promise<json::OrderItem> &)> &&);

  void modify_order(
      const ModifyOrder &,
      const std::string_view &request_id,
      const server::OMS_Order &,
      std::function<void(const core::Promise<json::OrderItem> &)> &&);

  void cancel_order(
      const CancelOrder &,
      const std::string_view &request_id,
      const server::OMS_Order &,
      std::function<void(const core::Promise<json::Order> &)> &&);

  void cancel_all_orders(
      const CancelAllOrders &, std::function<void(const core::Promise<json::Order> &)> &&);

  void operator()(const json::OrderItem &);
  void operator()(const json::Order &);

 private:
  Handler &handler_;
  // config
  const uint16_t stream_id_;
  const std::string name_;
  // connection
  core::web::Client connection_;
  // buffers
  core::Buffer decode_buffer_;
  // metrics
  struct {
    core::metrics::Counter disconnect;
  } counter_;
  struct {
    core::metrics::Profile products, accounts, create_order, modify_order, cancel_order,
        cancel_all_orders;
  } profile_;
  struct {
    core::metrics::Latency ping;
  } latency_;
  // security
  Security &security_;
  // cache
  Shared &shared_;
  // state
  ConnectionStatus status_ = {};
};

}  // namespace bitmex
}  // namespace roq
