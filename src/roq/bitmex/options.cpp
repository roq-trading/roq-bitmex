/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/options.h"

DEFINE_string(listen,
    "",
    "bind address (path)");
// DEFINE_validator(listen, ...);

DEFINE_string(config_file,
    "",
    "config file (path)");

DEFINE_string(rest_uri,
    "https://api-public.sandbox.pro.bitmex.com",
    "REST end-point (URI)");

DEFINE_string(ws_uri,
    "wss://ws-feed-public.sandbox.pro.bitmex.com",
    "WebSocket end-point (URI)");

DEFINE_string(fix_uri,
    "tcp+ssl://fix-public.sandbox.pro.bitmex.com:4198",
    "FIX end-point (URI)");

DEFINE_uint64(ping_freq_secs,
    uint64_t{5},
    "ping frequency (seconds)");

DEFINE_string(exchange,
    "bitmex-pro",
    "exchange identifier (string)");

DEFINE_bool(cancel_on_disconnect,
    true,
    "cancel orders on disconnect? (bool)");

DEFINE_uint32(encode_buffer_size,
    uint32_t{1048576},
    "encode buffer size");

DEFINE_uint32(decode_buffer_size,
    uint32_t{10485760},
    "decode buffer size");

DEFINE_uint64(reconnect_secs,
    uint64_t{3},
    "time before reconnect (seconds)");

DEFINE_bool(log_fix,
    false,
    "log fix messages?");
