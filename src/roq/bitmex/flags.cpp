/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/flags.h"

#include <absl/flags/declare.h>
#include <absl/flags/flag.h>

#include <string>

ABSL_FLAG(std::string, config_file, "", "config file (path)");

ABSL_FLAG(std::string, exchange, "bitmex", "exchange identifier (string)");

// rest

ABSL_FLAG(
    std::string,
    rest_uri,
    "https://testnet.bitmex.com",
    "REST end-point (URI)");

ABSL_FLAG(uint32_t, rest_ping_freq_secs, 5, "ping frequency (seconds)");

ABSL_FLAG(
    std::string,
    rest_ping_path,
    "/",
    "URI path used for REST connection keep-alive messages");

ABSL_FLAG(uint32_t, rest_request_queue_depth, 5, "request: max queue depth");

ABSL_FLAG(
    uint32_t, rest_request_timeout_secs, 30, "request: timeout (seconds)");

ABSL_FLAG(
    uint32_t,
    rest_rate_limit_interval_secs,
    60,
    "rate limit: monitor interval (seconds)");

ABSL_FLAG(
    uint32_t,
    rest_rate_limit_max_requests,
    60,
    "rate limit: max requests (per interval)");

ABSL_FLAG(uint32_t, rest_expires_timeout_secs, 1, "expires time-out (seconds)");

ABSL_FLAG(
    bool,
    rest_allow_order_updates,
    false,
    "allow inconsistent order updates? (bool)");

// ws

ABSL_FLAG(
    std::string,
    ws_uri,
    "wss://testnet.bitmex.com/realtime",
    "WebSocket end-point (URI)");

ABSL_FLAG(uint32_t, ws_ping_freq_secs, 5, "ping frequency (seconds)");

ABSL_FLAG(uint32_t, ws_request_timeout_secs, 15, "request time-out (seconds)");

ABSL_FLAG(
    bool, ws_cancel_on_disconnect, true, "cancel orders on disconnect? (bool)");

ABSL_FLAG(
    uint32_t,
    ws_cancel_all_after_secs,
    15,
    "cancel all after (seconds), requires cancel-on-disconnect");

// XXX review

ABSL_FLAG(uint32_t, encode_buffer_size, 1048576, "encode buffer size");

ABSL_FLAG(uint32_t, decode_buffer_size, 10485760, "decode buffer size");

// external

ABSL_DECLARE_FLAG(std::string, name);
ABSL_DECLARE_FLAG(uint32_t, cache_mbp_max_depth);
ABSL_DECLARE_FLAG(uint32_t, cache_trades_max_depth);
ABSL_DECLARE_FLAG(uint32_t, cache_fills_max_depth);

namespace roq {
namespace bitmex {

std::string_view Flags::config_file() {
  static const std::string result = absl::GetFlag(FLAGS_config_file);
  return result;
}

std::string_view Flags::exchange() {
  static const std::string result = absl::GetFlag(FLAGS_exchange);
  return result;
}

std::string_view Flags::rest_uri() {
  static const std::string result = absl::GetFlag(FLAGS_rest_uri);
  return result;
}

uint32_t Flags::rest_ping_freq_secs() {
  static const uint32_t result = absl::GetFlag(FLAGS_rest_ping_freq_secs);
  return result;
}

std::string_view Flags::rest_ping_path() {
  static const std::string result = absl::GetFlag(FLAGS_rest_ping_path);
  return result;
}

uint32_t Flags::rest_request_queue_depth() {
  static const uint32_t result = absl::GetFlag(FLAGS_rest_request_queue_depth);
  return result;
}

uint32_t Flags::rest_request_timeout_secs() {
  static const uint32_t result = absl::GetFlag(FLAGS_rest_request_timeout_secs);
  return result;
}

uint32_t Flags::rest_rate_limit_interval_secs() {
  static const uint32_t result =
      absl::GetFlag(FLAGS_rest_rate_limit_interval_secs);
  return result;
}

uint32_t Flags::rest_rate_limit_max_requests() {
  static const uint32_t result =
      absl::GetFlag(FLAGS_rest_rate_limit_max_requests);
  return result;
}

uint32_t Flags::rest_expires_timeout_secs() {
  static const uint32_t result = absl::GetFlag(FLAGS_rest_expires_timeout_secs);
  return result;
}

bool Flags::rest_allow_order_updates() {
  static const bool result = absl::GetFlag(FLAGS_rest_allow_order_updates);
  return result;
}

std::string_view Flags::ws_uri() {
  static const std::string result = absl::GetFlag(FLAGS_ws_uri);
  return result;
}

uint32_t Flags::ws_ping_freq_secs() {
  static const uint32_t result = absl::GetFlag(FLAGS_ws_ping_freq_secs);
  return result;
}

uint32_t Flags::ws_request_timeout_secs() {
  static const uint32_t result = absl::GetFlag(FLAGS_ws_request_timeout_secs);
  return result;
}

bool Flags::ws_cancel_on_disconnect() {
  static const bool result = absl::GetFlag(FLAGS_ws_cancel_on_disconnect);
  return result;
}

uint32_t Flags::ws_cancel_all_after_secs() {
  static const uint32_t result = absl::GetFlag(FLAGS_ws_cancel_all_after_secs);
  return result;
}

uint32_t Flags::encode_buffer_size() {
  static const uint32_t result = absl::GetFlag(FLAGS_encode_buffer_size);
  return result;
}

uint32_t Flags::decode_buffer_size() {
  static const uint32_t result = absl::GetFlag(FLAGS_decode_buffer_size);
  return result;
}

std::string_view Flags::name() {
  static const std::string result = absl::GetFlag(FLAGS_name);
  return result;
}

uint32_t Flags::cache_mbp_max_depth() {
  static const uint32_t result = absl::GetFlag(FLAGS_cache_mbp_max_depth);
  return result;
}

uint32_t Flags::cache_trades_max_depth() {
  static const uint32_t result = absl::GetFlag(FLAGS_cache_trades_max_depth);
  return result;
}

uint32_t Flags::cache_fills_max_depth() {
  static const uint32_t result = absl::GetFlag(FLAGS_cache_fills_max_depth);
  return result;
}

}  // namespace bitmex
}  // namespace roq
