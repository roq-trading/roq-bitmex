#!/usr/bin/env bash

CWD="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"

if [ "$1" == "debug" ]; then
	PREFIX="gdb --args"
else
	PREFIX=
fi

NAME="bitmex-testnet"

CONFIG_FILE="$CWD/config/$NAME.toml"

URI="testnet.bitmex.com"

REST_URI="https://$URI"
WS_URI="wss://ws.$URI/realtime"

$PREFIX ./roq-bitmex \
	--name "bitmex" \
	--config_file "$CONFIG_FILE" \
	--client_listen_address "$HOME/run/bitmex.sock" \
	--metrics_listen_address 1234 \
	--ws_uri "$WS_URI" \
	--rest_uri "$REST_URI" \
	$@
