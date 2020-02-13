#!/usr/bin/env bash

AUTOGEN="../../../../../autogen.py"
DEFAULTS="--namespace=bitmex/json"

files=(
	action
	execution_item
	funding_item
	instrument_item
	liquidation_item
	margin_item
	order_book_l2_item
	order_item
	position_item
	quote_item
	settlement_item
	settlement_type
	side
	state
	table
	trade_item
	typ
)

for f in "${files[@]}"; do
	echo "$f.h"
	"$AUTOGEN" "$DEFAULTS" --type h --spec "$f.json" >"$f.h"
	echo "$f.cpp"
	"$AUTOGEN" "$DEFAULTS" --type cpp --spec "$f.json" >"$f.cpp"
done
