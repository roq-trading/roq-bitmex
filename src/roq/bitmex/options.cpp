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
    "https://testnet.bitmex.com/api/v1",
    "REST end-point (URI)");

DEFINE_string(ws_uri,
    "wss://testnet.bitmex.com/realtime",
    "WebSocket end-point (URI)");

DEFINE_uint64(ping_freq_secs,
    uint64_t{5},
    "ping frequency (seconds)");

DEFINE_string(exchange,
    "bitmex",
    "exchange identifier (string)");

DEFINE_bool(cancel_on_disconnect,
    true,
    "cancel orders on disconnect? (bool)");

DEFINE_uint32(max_trades,
  uint32_t{16384},
  "maximum trades for trade summary");

DEFINE_uint32(encode_buffer_size,
    uint32_t{1048576},
    "encode buffer size");

DEFINE_uint32(decode_buffer_size,
    uint32_t{10485760},
    "decode buffer size");

DEFINE_uint64(reconnect_secs,
    uint64_t{3},
    "time before reconnect (seconds)");

DEFINE_uint32(cancel_all_after_secs,
    uint32_t{60},
    "cancel all after (seconds)");
