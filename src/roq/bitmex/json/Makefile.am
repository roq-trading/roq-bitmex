noinst_LTLIBRARIES = libroq-bitmex-json.la
libroq_bitmex_json_la_LDFLAGS = \
	-static \
	$(LIBCONFIG_LIBS) \
	$(ROQ_LIBS)
libroq_bitmex_json_la_CPPFLAGS = \
	-I$(top_srcdir)/src \
	$(LIBCONFIG_CFLAGS) \
	$(ROQ_CFLAGS)
BUILT_SOURCES = \
	action.cpp \
	action.h \
	cancel_all_after.cpp \
	cancel_all_after.h \
	error.cpp \
	error.h \
	exec_inst.cpp \
	exec_inst.h \
	execution.cpp \
	execution.h \
	execution_item.cpp \
	execution_item.h \
	field.cpp \
	field.h \
	funding.cpp \
	funding.h \
	funding_item.cpp \
	funding_item.h \
	handshake.cpp \
	handshake.h \
	instrument.cpp \
	instrument.h \
	instrument_item.cpp \
	instrument_item.h \
	liquidation.cpp \
	liquidation.h \
	liquidation_item.cpp \
	liquidation_item.h \
	margin.cpp \
	margin.h \
	margin_item.cpp \
	margin_item.h \
	multi_leg_reporting_type.cpp \
	multi_leg_reporting_type.h \
	ord_status.cpp \
	ord_status.h \
	ord_type.cpp \
	ord_type.h \
	order_book_l2.cpp \
	order_book_l2.h \
	order_book_l2_item.cpp \
	order_book_l2_item.h \
	order.cpp \
	order.h \
	order_item.cpp \
	order_item.h \
	position.cpp \
	position.h \
	position_item.cpp \
	position_item.h \
	quote.cpp \
	quote.h \
	quote_item.cpp \
	quote_item.h \
	settlement.cpp \
	settlement.h \
	settlement_item.cpp \
	settlement_item.h \
	settlement_type.cpp \
	settlement_type.h \
	side.cpp \
	side.h \
	state.cpp \
	state.h \
	subscribe.cpp \
	subscribe.h \
	table.cpp \
	table.h \
	time_in_force.cpp \
	time_in_force.h \
	trade.cpp \
	trade.h \
	trade_item.cpp \
	trade_item.h \
	typ.cpp \
	typ.h
CLEANFILES = $(BUILT_SOURCES)
libroq_bitmex_json_la_SOURCES = \
	action.cpp \
	cancel_all_after.cpp \
	error.cpp \
	exec_inst.cpp \
	execution.cpp \
	execution_item.cpp \
	field.cpp \
	funding.cpp \
	funding_item.cpp \
	handshake.cpp \
	instrument.cpp \
	instrument_item.cpp \
	liquidation.cpp \
	liquidation_item.cpp \
	margin.cpp \
	margin_item.cpp \
	multi_leg_reporting_type.cpp \
	ord_status.cpp \
	ord_type.cpp \
	order_book_l2.cpp \
	order_book_l2_item.cpp \
	order.cpp \
	order_item.cpp \
	parser.cpp \
	position.cpp \
	position_item.cpp \
	quote.cpp \
	quote_item.cpp \
	settlement.cpp \
	settlement_item.cpp \
	settlement_type.cpp \
	side.cpp \
	state.cpp \
	subscribe.cpp \
	table.cpp \
	time_in_force.cpp \
	trade.cpp \
	trade_item.cpp \
	typ.cpp \
	utils.cpp

.json.h:
	@abs_top_builddir@/autogen/autogen.py \
		--namespace=roq/bitmex/json \
		--output-type json \
		--file-type h \
		$< > $@

.json.cpp:
	@abs_top_builddir@/autogen/autogen.py \
		--namespace=roq/bitmex/json \
		--output-type json \
		--file-type cpp \
		$< > $@
