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
	instrument.cpp \
	instruments.cpp \
	market_by_price.cpp \
	order.cpp \
	order_book.cpp \
	parser.cpp \
	position.cpp \
	utils.cpp
