/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/bitmex/flags.h"

#include <absl/flags/declare.h>
#include <absl/flags/flag.h>

#include <string>

#include "roq/core/flags/non_empty.h"
#include "roq/core/flags/non_zero.h"
#include "roq/core/flags/uri.h"

using namespace roq::literals;

ABSL_FLAG(  //
    roq::core::flags::NonEmpty<std::string>,
    config_file,
    {},
    "config file (path)"_sv);

ABSL_FLAG(  //
    roq::core::flags::NonEmpty<std::string>,
    exchange,
    "bitmex"_s,
    "exchange identifier (string)"_sv);

// rest

ABSL_FLAG(  //
    roq::core::flags::URI<std::string>,
    rest_uri,
    "https://testnet.bitmex.com"_s,
    "REST end-point (URI)"_sv);

ABSL_FLAG(  //
    uint32_t,
    rest_ping_freq_secs,
    uint32_t{5},
    "ping frequency (seconds)"_sv);

ABSL_FLAG(  //
    roq::core::flags::URI<std::string>,
    rest_ping_path,
    "/"_s,
    "URI path used for REST connection keep-alive messages"_sv);

ABSL_FLAG(  //
    uint32_t,
    rest_request_queue_depth,
    uint32_t{5},
    "request: max queue depth"_sv);

ABSL_FLAG(  //
    uint32_t,
    rest_request_timeout_secs,
    uint32_t{30},
    "request: timeout (seconds)"_sv);

ABSL_FLAG(  //
    uint32_t,
    rest_rate_limit_interval_secs,
    uint32_t{60},
    "rate limit: monitor interval (seconds)"_sv);

ABSL_FLAG(  //
    uint32_t,
    rest_rate_limit_max_requests,
    uint32_t{60},
    "rate limit: max requests (per interval)"_sv);

ABSL_FLAG(  //
    uint32_t,
    rest_expires_timeout_secs,
    uint32_t{1},
    "expires time-out (seconds)"_sv);

ABSL_FLAG(  //
    bool,
    rest_allow_order_updates,
    false,
    "allow inconsistent order updates? (bool)"_sv);

// ws

ABSL_FLAG(  //
    roq::core::flags::URI<std::string>,
    ws_uri,
    "wss://testnet.bitmex.com/realtime"_s,
    "WebSocket end-point (URI)"_sv);

ABSL_FLAG(  //
    uint32_t,
    ws_ping_freq_secs,
    uint32_t{5},
    "ping frequency (seconds)"_sv);

ABSL_FLAG(  //
    uint32_t,
    ws_request_timeout_secs,
    uint32_t{15},
    "request time-out (seconds)"_sv);

ABSL_FLAG(  //
    bool,
    ws_cancel_on_disconnect,
    true,
    "cancel orders on disconnect? (bool)"_sv);

ABSL_FLAG(  //
    uint32_t,
    ws_cancel_all_after_secs,
    uint32_t{15},
    "cancel all after (seconds), requires cancel-on-disconnect"_sv);

// XXX review

ABSL_FLAG(  //
    roq::core::flags::NonZero<uint32_t>,
    encode_buffer_size,
    uint32_t{1048576},
    "encode buffer size"_sv);

ABSL_FLAG(  //
    roq::core::flags::NonZero<uint32_t>,
    decode_buffer_size,
    uint32_t{10485760},
    "decode buffer size"_sv);

// external

ABSL_DECLARE_FLAG(roq::core::flags::NonEmpty<std::string>, name);
ABSL_DECLARE_FLAG(roq::core::flags::NonZero<uint32_t>, cache_mbp_max_depth);
ABSL_DECLARE_FLAG(roq::core::flags::NonZero<uint32_t>, cache_trades_max_depth);
ABSL_DECLARE_FLAG(roq::core::flags::NonZero<uint32_t>, cache_fills_max_depth);

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
  static const uint32_t result = absl::GetFlag(FLAGS_rest_rate_limit_interval_secs);
  return result;
}

uint32_t Flags::rest_rate_limit_max_requests() {
  static const uint32_t result = absl::GetFlag(FLAGS_rest_rate_limit_max_requests);
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
