/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <gflags/gflags.h>

DECLARE_string(listen);
DECLARE_string(config_file);

DECLARE_string(rest_uri);
DECLARE_string(ws_uri);
DECLARE_uint32(rate_limit_interval_secs);
DECLARE_uint32(rate_limit_max_requests);
DECLARE_uint32(ping_freq_secs);
DECLARE_uint32(download_timeout_secs);
DECLARE_string(exchange);
DECLARE_bool(cancel_on_disconnect);
DECLARE_uint32(max_trades);
DECLARE_uint32(encode_buffer_size);
DECLARE_uint32(decode_buffer_size);
DECLARE_uint32(reconnect_secs);

DECLARE_uint32(cancel_all_after_secs);
DECLARE_uint32(request_expires_secs);

DECLARE_string(rest_ping_path);

// external

DECLARE_string(name);
DECLARE_uint32(max_depth);
