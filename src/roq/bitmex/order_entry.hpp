/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/core/buffer.hpp"

#include "roq/core/metrics/counter.hpp"
#include "roq/core/metrics/latency.hpp"
#include "roq/core/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/rest/client.hpp"

#include "roq/server.hpp"

#include "roq/bitmex/security.hpp"
#include "roq/bitmex/shared.hpp"

#include "roq/bitmex/json/order.hpp"
#include "roq/bitmex/json/order_item.hpp"

namespace roq {
namespace bitmex {

class OrderEntry final : public web::rest::Client::Handler {
 public:
  struct Handler {
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &) = 0;
  };

  OrderEntry(Handler &, io::Context &, uint16_t stream_id, Security &, Shared &);

  OrderEntry(OrderEntry &&) = delete;
  OrderEntry(OrderEntry const &) = delete;

  bool ready() const { return status_ == ConnectionStatus::READY; }

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &);

  uint16_t operator()(Event<CreateOrder> const &, oms::Order const &, std::string_view const &request_id);
  uint16_t operator()(
      Event<ModifyOrder> const &,
      oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);
  uint16_t operator()(
      Event<CancelOrder> const &,
      oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);

  uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id);

 protected:
  void operator()(web::rest::Client::Connected const &) override;
  void operator()(web::rest::Client::Disconnected const &) override;
  void operator()(web::rest::Client::Latency const &) override;

 private:
  void operator()(ConnectionStatus);

  void create_order(Event<CreateOrder> const &, oms::Order const &, std::string_view const &request_id);
  void create_order_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint32_t order_id, uint32_t version);

  void modify_order(
      Event<ModifyOrder> const &,
      oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);
  void modify_order_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint32_t order_id, uint32_t version);

  void cancel_order(
      Event<CancelOrder> const &,
      oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);
  void cancel_order_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint32_t order_id, uint32_t version);

  void cancel_all_orders(Event<CancelAllOrders> const &, std::string_view const &request_id);
  void cancel_all_orders_ack(Trace<web::rest::Response> const &);

  void operator()(json::OrderItem const &);
  void operator()(json::Order const &);

  template <typename SuccessHandler, typename ErrorHandler>
  void process_response(web::rest::Response const &, SuccessHandler, ErrorHandler);

  template <typename... Args>
  void operator()(Trace<oms::Response> const &, uint8_t user_id, uint32_t order_id, Args &&...);

  template <typename... Args>
  void operator()(Trace<oms::OrderUpdate> const &, std::string_view const &client_order_id, Args &&...);

 private:
  Handler &handler_;
  // config
  const uint16_t stream_id_;
  const std::string name_;
  // connection
  std::unique_ptr<web::rest::Client> connection_;
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
