noinst_LTLIBRARIES = libroq-bitmex-json.la
libroq_bitmex_json_la_LDFLAGS = \
	-static \
	$(LIBCONFIG_LIBS) \
	$(ROQ_LIBS)
libroq_bitmex_json_la_CPPFLAGS = \
	-I$(top_srcdir)/src \
	$(LIBCONFIG_CFLAGS) \
	$(ROQ_CFLAGS)
AUTOGEN_CPP = \
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
	typ.cpp
AUTOGEN_H = \
	action.h \
	cancel_all_after.h \
	error.h \
	exec_inst.h \
	execution.h \
	execution_item.h \
	field.h \
	funding.h \
	funding_item.h \
	handshake.h \
	instrument.h \
	instrument_item.h \
	liquidation.h \
	liquidation_item.h \
	margin.h \
	margin_item.h \
	multi_leg_reporting_type.h \
	ord_status.h \
	ord_type.h \
	order_book_l2.h \
	order_book_l2_item.h \
	order.h \
	order_item.h \
	position.h \
	position_item.h \
	quote.h \
	quote_item.h \
	settlement.h \
	settlement_item.h \
	settlement_type.h \
	side.h \
	state.h \
	subscribe.h \
	table.h \
	time_in_force.h \
	trade.h \
	trade_item.h \
	typ.h
BUILT_SOURCES = $(AUTOGEN_CPP) $(AUTOGEN_H) .gitignore
CLEANFILES = $(BUILT_SOURCES)
libroq_bitmex_json_la_SOURCES = \
	$(AUTOGEN_CPP) \
	parser.cpp \
	utils.cpp

.gitignore:  Makefile.am
	echo $(AUTOGEN_CPP) $(AUTOGEN_H) | tr " " "\n" | sort > $@

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
