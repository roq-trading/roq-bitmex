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

REST_URI="https://$URI/api/v1"
WS_URI="wss://$URI/realtime"

$PREFIX ./roq-bitmex \
	--name "bitmex" \
	--config-file "$CONFIG_FILE" \
	--ws-uri "$WS_URI" \
	--rest-uri "$REST_URI" \
	--listen $CWD/$NAME.sock \
	--metrics 1234 \
	$@
