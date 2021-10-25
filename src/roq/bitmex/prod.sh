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
	--client_listen_address $CWD/$NAME.sock \
	--metrics_listen_address 1234 \
	--config_file "$CONFIG_FILE" \
	--ws_uri "$WS_URI" \
	--rest_uri "$REST_URI" \
	$@
