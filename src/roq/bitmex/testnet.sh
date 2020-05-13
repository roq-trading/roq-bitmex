#!/usr/bin/env bash

CWD="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"

if [ "$1" == "debug" ]; then
	PREFIX="libtool --mode=execute gdb --args"
else
	PREFIX=
fi

NAME="bitmex-testnet"

CONFIG_FILE="$CWD/config/$NAME.toml"

URI="testnet.bitmex.com"

REST_URI="https://$URI"
WS_URI="wss://$URI/realtime"

$PREFIX ./roq-bitmex \
	--name "bitmex" \
	--config-file "$CONFIG_FILE" \
	--client-listen-address $CWD/$NAME.sock \
	--metrics-listen-address 1234 \
	--ws-uri "$WS_URI" \
	--rest-uri "$REST_URI" \
	$@
