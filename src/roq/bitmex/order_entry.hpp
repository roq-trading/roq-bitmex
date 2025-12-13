/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/rest/client.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/bitmex/account.hpp"
#include "roq/bitmex/shared.hpp"

#include "roq/bitmex/json/order.hpp"

namespace roq {
namespace bitmex {

struct OrderEntry final : public web::rest::Client::Handler {
  struct Handler {
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &) = 0;
  };

  OrderEntry(Handler &, io::Context &, uint16_t stream_id, Account &, Shared &, bool master);

  OrderEntry(OrderEntry const &) = delete;

  bool ready() const { return status_ == ConnectionStatus::READY; }

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &) const;

  uint16_t operator()(Event<CreateOrder> const &, server::oms::Order const &, std::string_view const &request_id);
  uint16_t operator()(Event<ModifyOrder> const &, server::oms::Order const &, std::string_view const &request_id, std::string_view const &previous_request_id);
  uint16_t operator()(Event<CancelOrder> const &, server::oms::Order const &, std::string_view const &request_id, std::string_view const &previous_request_id);

  uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id);

 protected:
  // web::rest::Client::Handler

  void operator()(Trace<web::rest::Client::Connected> const &) override;
  void operator()(Trace<web::rest::Client::Disconnected> const &) override;
  void operator()(Trace<web::rest::Client::Latency> const &) override;

 private:
  void operator()(ConnectionStatus);

  // create-order

  void create_order(Event<CreateOrder> const &, server::oms::Order const &, std::string_view const &request_id);
  void create_order_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint64_t order_id, uint32_t version);

  // modify-order

  void modify_order(Event<ModifyOrder> const &, server::oms::Order const &, std::string_view const &request_id, std::string_view const &previous_request_id);
  void modify_order_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint64_t order_id, uint32_t version);

  // cancel-order

  void cancel_order(Event<CancelOrder> const &, server::oms::Order const &, std::string_view const &request_id, std::string_view const &previous_request_id);
  void cancel_order_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint64_t order_id, uint32_t version);

  // cancel-all-orders

  void cancel_all_orders(Event<CancelAllOrders> const &, std::string_view const &request_id);
  void cancel_all_orders_ack(Trace<web::rest::Response> const &, std::string_view const &request_id);

  // helpers

  void operator()(json::OrderDataItem const &);
  void operator()(json::Order const &);

  template <typename SuccessHandler, typename ErrorHandler>
  void process_response(web::rest::Response const &, SuccessHandler, ErrorHandler);

  template <typename... Args>
  void operator()(Trace<server::oms::Response> const &, uint8_t user_id, uint64_t order_id, Args &&...);

  void operator()(Trace<server::oms::OrderUpdate> const &, std::string_view const &client_order_id);

 private:
  Handler &handler_;
  // config
  uint16_t const stream_id_;
  std::string const name_;
  bool const master_;
  // connection
  std::unique_ptr<web::rest::Client> const connection_;
  // buffers
  core::json::BufferStack decode_buffer_;
  std::string encode_buffer_;
  // metrics
  struct {
    utils::metrics::Counter disconnect;
  } counter_;
  struct {
    utils::metrics::Profile  //
        create_order,
        create_order_ack,                //
        modify_order, modify_order_ack,  //
        cancel_order, cancel_order_ack,  //
        cancel_all_orders, cancel_all_orders_ack;
  } profile_;
  struct {
    utils::metrics::Latency ping;
  } latency_;
  // account
  Account &account_;
  // cache
  Shared &shared_;
  // state
  ConnectionStatus status_ = {};
};

}  // namespace bitmex
}  // namespace roq
