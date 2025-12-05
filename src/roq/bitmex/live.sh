#!/usr/bin/env bash

if [ "$1" == "debug" ]; then
  PREFIX="gdb --args"
else
  PREFIX=
fi

NAME="bitmex"

CONFIG="${CONFIG:-$NAME-prod}"

CONFIG_FILE="$ROQ_CONFIG_PATH/roq-bitmex/$CONFIG.toml"

FLAGFILE="../../../share/flags/prod/flags.cfg"

$PREFIX ./roq-bitmex \
  --name "$NAME" \
  --config_file "$CONFIG_FILE" \
  --flagfile "$FLAGFILE" \
  --cache_dir "$HOME/var/lib/roq/cache" \
  --event_log_dir "$HOME/var/lib/roq/data" \
  --client_listen_address "$HOME/run/$NAME.sock" \
  --service_listen_address "$HOME/run/metrics/${NAME}.sock" \
  --cache_all_reference_data=true \
  $@
