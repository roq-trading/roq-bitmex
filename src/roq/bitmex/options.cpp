/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/options.h"

#include <absl/flags/flag.h>

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
