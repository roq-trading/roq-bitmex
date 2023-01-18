/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/bitmex/shared.hpp"

#include "roq/bitmex/flags.hpp"

using namespace std::literals;

namespace roq {
namespace bitmex {

// === IMPLEMENTATION ===

Shared::Shared(server::Dispatcher &dispatcher) : dispatcher_{dispatcher} {
}

std::string_view Shared::next_request_id() {
  auto request_id = ++request_id_;
  stack_buffer_.clear();
  fmt::format_to(std::back_inserter(stack_buffer_), "roq-{}"sv, request_id);
  return {std::data(stack_buffer_), std::size(stack_buffer_)};
}

}  // namespace bitmex
}  // namespace roq
