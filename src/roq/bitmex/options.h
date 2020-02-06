/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <gflags/gflags.h>

DECLARE_string(listen);
DECLARE_string(config_file);

DECLARE_string(rest_uri);
DECLARE_string(ws_uri);
DECLARE_uint64(ping_freq_secs);
DECLARE_string(exchange);
DECLARE_bool(cancel_on_disconnect);
DECLARE_uint32(max_trades);
DECLARE_uint32(encode_buffer_size);
DECLARE_uint32(decode_buffer_size);
DECLARE_uint64(reconnect_secs);

DECLARE_uint32(cancel_all_after_secs);

// external

DECLARE_string(name);
DECLARE_uint32(max_depth);
