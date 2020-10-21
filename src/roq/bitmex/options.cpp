/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/options.h"

DEFINE_string(config_file, "", "config file (path)");

DEFINE_string(exchange, "bitmex", "exchange identifier (string)");

DEFINE_uint32(download_timeout_secs, 15, "download time-out (seconds)");

DEFINE_string(rest_uri, "https://testnet.bitmex.com", "REST end-point (URI)");

DEFINE_uint32(rest_ping_freq_secs, 5, "ping frequency (seconds)");

DEFINE_string(
    rest_ping_path,
    "/",
    "URI path used for REST connection keep-alive messages");

DEFINE_uint32(rest_request_queue_depth, 5, "request: max queue depth");

DEFINE_uint32(rest_request_timeout_secs, 30, "request: timeout (seconds)");

DEFINE_uint32(
    rest_rate_limit_interval_secs,
    60,
    "rate limit: monitor interval (seconds)");

DEFINE_uint32(
    rest_rate_limit_max_requests,
    60,
    "rate limit: max requests (per interval)");

DEFINE_uint32(rest_expires_timeout_secs, 1, "expires time-out (seconds)");

DEFINE_string(
    ws_uri, "wss://testnet.bitmex.com/realtime", "WebSocket end-point (URI)");

DEFINE_uint32(ws_ping_freq_secs, 5, "ping frequency (seconds)");

DEFINE_bool(
    ws_cancel_on_disconnect, true, "cancel orders on disconnect? (bool)");

DEFINE_uint32(
    ws_cancel_all_after_secs,
    15,
    "cancel all after (seconds), requires cancel-on-disconnect");

DEFINE_uint32(encode_buffer_size, 1048576, "encode buffer size");

DEFINE_uint32(decode_buffer_size, 10485760, "decode buffer size");

DEFINE_bool(
    allow_inconsistent_order_updates,
    false,
    "allow inconsistent order updates? (bool)");
