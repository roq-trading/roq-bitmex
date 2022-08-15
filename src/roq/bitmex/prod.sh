#!/usr/bin/env bash

CWD="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"

if [ "$1" == "debug" ]; then
	PREFIX="gdb --args"
else
	PREFIX=
fi

NAME="bitmex-prod"

CONFIG_FILE="$CWD/config/$NAME.toml"

URI="bitmex.com"

REST_URI="https://www.$URI"
WS_URI="wss://ws.$URI/realtime"

$PREFIX ./roq-bitmex \
	--name "bitmex" \
	--config_file "$CONFIG_FILE" \
  --event_log_dir "$HOME/var/lib/roq/data" \                                                                            
  --event_log_symlink \                                                                                                 
  --client_listen_address "$HOME/run/$NAME.sock" \                                                                      
  --metrics_listen_address "$HOME/run/${NAME}_metrics.sock" \
	--ws_uri "$WS_URI" \
	--rest_uri "$REST_URI" \
	$@
