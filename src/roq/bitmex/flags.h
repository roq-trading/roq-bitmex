/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <cstdint>
#include <string_view>

namespace roq {
namespace bitmex {

struct Flags final {
  static std::string_view config_file();
  static std::string_view exchange();
  static std::string_view rest_uri();
  static uint32_t rest_ping_freq_secs();
  static std::string_view rest_ping_path();
  static uint32_t rest_request_queue_depth();
  static uint32_t rest_request_timeout_secs();
  static uint32_t rest_rate_limit_interval_secs();
  static uint32_t rest_rate_limit_max_requests();
  static uint32_t rest_expires_timeout_secs();
  static bool rest_allow_order_updates();
  static std::string_view ws_uri();
  static uint32_t ws_ping_freq_secs();
  static uint32_t ws_request_timeout_secs();
  static bool ws_cancel_on_disconnect();
  static uint32_t ws_cancel_all_after_secs();
  static uint32_t encode_buffer_size();
  static uint32_t decode_buffer_size();
  // external
  static std::string_view name();
  static uint32_t cache_mbp_max_depth();
  static uint32_t cache_trades_max_depth();
  static uint32_t cache_fills_max_depth();
};

}  // namespace bitmex
}  // namespace roq
