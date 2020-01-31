noinst_LTLIBRARIES = libroq-bitmex-json.la
libroq_bitmex_json_la_LDFLAGS = \
	-static \
	$(LIBCONFIG_LIBS) \
	$(ROQ_LIBS)
libroq_bitmex_json_la_CPPFLAGS = \
	-I$(top_srcdir)/src \
	$(LIBCONFIG_CFLAGS) \
	$(ROQ_CFLAGS)
libroq_bitmex_json_la_SOURCES = \
	action.cpp \
	error.cpp \
	funding.cpp \
	funding_item.cpp \
	handshake.cpp \
	instrument.cpp \
	instrument_item.cpp \
	liquidation.cpp \
	liquidation_item.cpp \
	order.cpp \
	order_book_l2.cpp \
	order_book_l2_item.cpp \
	parser.cpp \
	position.cpp \
	quote.cpp \
	quote_item.cpp \
	settlement.cpp \
	settlement_item.cpp \
	settlement_type.cpp \
	side.cpp \
	state.cpp \
	subscribe.cpp \
	table.cpp \
	trade.cpp \
	trade_item.cpp \
	typ.cpp \
	utils.cpp
