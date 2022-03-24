/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/core/buffer.hpp"

#include "roq/core/metrics/counter.hpp"
#include "roq/core/metrics/latency.hpp"
#include "roq/core/metrics/profile.hpp"

#include "roq/core/io/context.hpp"

#include "roq/core/web/client.hpp"

#include "roq/server.hpp"

#include "roq/bitmex/security.hpp"
#include "roq/bitmex/shared.hpp"

#include "roq/bitmex/json/order.hpp"
#include "roq/bitmex/json/order_item.hpp"

namespace roq {
namespace bitmex {

class OrderEntry final : public core::web::Client::Handler {
 public:
  struct Handler {
    virtual void operator()(const Trace<StreamStatus> &) = 0;
    virtual void operator()(const Trace<ExternalLatency> &) = 0;
  };

  OrderEntry(Handler &, core::io::Context &, uint16_t stream_id, Security &, Shared &);

  OrderEntry(OrderEntry &&) = delete;
  OrderEntry(const OrderEntry &) = delete;

  bool ready() const { return status_ == ConnectionStatus::READY; }

  void operator()(const Event<Start> &);
  void operator()(const Event<Stop> &);
  void operator()(const Event<Timer> &);

  void operator()(metrics::Writer &);

  uint16_t operator()(
      const Event<CreateOrder> &, const oms::Order &, const std::string_view &request_id);
  uint16_t operator()(
      const Event<ModifyOrder> &,
      const oms::Order &,
      const std::string_view &request_id,
      const std::string_view &previous_request_id);
  uint16_t operator()(
      const Event<CancelOrder> &,
      const oms::Order &,
      const std::string_view &request_id,
      const std::string_view &previous_request_id);

  uint16_t operator()(const Event<CancelAllOrders> &, const std::string_view &request_id);

 protected:
  void operator()(const core::web::Client::Connected &) override;
  void operator()(const core::web::Client::Disconnected &) override;
  void operator()(const core::web::Client::Latency &) override;

 private:
  void operator()(ConnectionStatus);

  void create_order(
      const Event<CreateOrder> &, const oms::Order &, const std::string_view &request_id);
  void create_order_ack(
      const Trace<core::web::Response> &,
      uint8_t user_id,
      uint32_t order_id,
      uint32_t version);

  void modify_order(
      const Event<ModifyOrder> &,
      const oms::Order &,
      const std::string_view &request_id,
      const std::string_view &previous_request_id);
  void modify_order_ack(
      const Trace<core::web::Response> &,
      uint8_t user_id,
      uint32_t order_id,
      uint32_t version);

  void cancel_order(
      const Event<CancelOrder> &,
      const oms::Order &,
      const std::string_view &request_id,
      const std::string_view &previous_request_id);
  void cancel_order_ack(
      const Trace<core::web::Response> &,
      uint8_t user_id,
      uint32_t order_id,
      uint32_t version);

  void cancel_all_orders(const Event<CancelAllOrders> &, const std::string_view &request_id);
  void cancel_all_orders_ack(const Trace<core::web::Response> &);

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
    core::metrics::Profile create_order, create_order_ack,  //
        modify_order, modify_order_ack,                     //
        cancel_order, cancel_order_ack,                     //
        cancel_all_orders, cancel_all_orders_ack;
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
