#!/usr/bin/env bash

CWD="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"

if [ "$1" == "debug" ]; then
	PREFIX="libtool --mode=execute gdb --args"
else
	PREFIX=
fi

NAME="bitmex-prod"

CONFIG_FILE="$CWD/config/$NAME.toml"

URI="www.bitmex.com"

REST_URI="https://$URI"
WS_URI="wss://$URI/realtime"

$PREFIX ./roq-bitmex \
	--name "bitmex" \
	--client-listen-address $CWD/$NAME.sock \
	--metrics-listen-address 1234 \
	--config-file "$CONFIG_FILE" \
	--ws-uri "$WS_URI" \
	--rest-uri "$REST_URI" \
	$@
