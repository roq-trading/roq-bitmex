#!/usr/bin/env bash

if [ "$1" == "debug" ]; then
  PREFIX="gdb --args"
else
  PREFIX=
fi

NAME="bitmex"

CONFIG="${CONFIG:-$NAME-prod}"

CONFIG_FILE="$ROQ_CONFIG_PATH/roq-bitmex/$CONFIG.toml"

URI="bitmex.com"

REST_URI="https://www.$URI"
WS_URI="wss://ws.$URI/realtime"

$PREFIX ./roq-bitmex \
  --name "$NAME" \
  --config_file "$CONFIG_FILE" \
  --cache_dir "$HOME/var/lib/roq/cache" \
  --event_log_dir "$HOME/var/lib/roq/data" \
  --event_log_symlink true \
  --client_listen_address "$HOME/run/$NAME.sock" \
  --service_listen_address "$HOME/run/metrics/${NAME}.sock" \
  --ws_uri "$WS_URI" \
  --rest_uri "$REST_URI" \
  $@
