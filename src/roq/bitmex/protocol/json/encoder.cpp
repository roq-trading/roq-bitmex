/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/bitmex/protocol/json/encoder.hpp"

#include "roq/bitmex/protocol/json/map.hpp"
#include "roq/bitmex/protocol/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace bitmex {
namespace protocol {
namespace json {

// === IMPLEMENTATION ===

std::string_view Encoder::place_order(
    std::string &buffer, CreateOrder const &create_order, server::oms::Order const &, server::oms::RefData const &, std::string_view const &request_id) {
  buffer.clear();
  auto side = map(create_order.side).template get<protocol::json::Side>();
  auto ord_type = map(create_order.order_type).template get<protocol::json::OrdType>();
  auto time_in_force = map(create_order.time_in_force).template get<protocol::json::TimeInForce>();
  auto exec_inst =
      std::empty(create_order.execution_instructions) ? std::string_view{} : protocol::json::map(create_order.execution_instructions).as_raw_text();
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("clOrdID":"{}",)"
      R"("symbol":"{}",)"
      R"("side":"{}",)"
      R"("price":{},)"
      R"("orderQty":{},)"
      R"("ordType":"{}",)"
      R"("timeInForce":"{}",)"
      R"("execInst":"{}")"
      R"(}})"sv,
      request_id,
      create_order.symbol,
      side.as_raw_text(),
      create_order.price,
      create_order.quantity,
      ord_type.as_raw_text(),
      time_in_force.as_raw_text(),
      exec_inst);
  return buffer;
}

std::string_view Encoder::modify_order(
    std::string &buffer,
    ModifyOrder const &modify_order,
    server::oms::Order const &,
    server::oms::RefData const &,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("clOrdID":"{}",)"
      R"("origClOrdID":"{}",)"
      R"("orderQty":{},)"
      R"("price":{})"
      R"(}})"sv,
      request_id,
      previous_request_id,
      modify_order.quantity,
      modify_order.price);
  return buffer;
}

std::string_view Encoder::cancel_order(
    std::string &buffer,
    CancelOrder const &,
    server::oms::Order const &order,
    server::oms::RefData const &,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("orderID":"{}")"
      R"(}})"sv,
      order.external_order_id);
  return buffer;
}

std::string_view Encoder::cancel_all_orders(std::string &buffer, CancelAllOrders const &, [[maybe_unused]] std::string_view const &request_id) {
  buffer.clear();
  return buffer;
}

}  // namespace json
}  // namespace protocol
}  // namespace bitmex
}  // namespace roq
