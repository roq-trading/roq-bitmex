/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/bitmex/shared.hpp"

using namespace std::literals;

namespace roq {
namespace bitmex {

// === IMPLEMENTATION ===

Shared::Shared(server::Dispatcher &dispatcher, Settings const &settings) : dispatcher_{dispatcher}, settings{settings} {
}

std::string_view Shared::next_request_id() {
  auto request_id = ++request_id_;
  request_id_encode_buffer_.clear();
  fmt::format_to(std::back_inserter(request_id_encode_buffer_), "roq-{}"sv, request_id);
  return request_id_encode_buffer_;
}

}  // namespace bitmex
}  // namespace roq
