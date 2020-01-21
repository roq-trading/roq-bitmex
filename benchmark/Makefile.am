TESTS = roq-coinbase-pro-benchmark
bin_PROGRAMS = roq-coinbase-pro-benchmark
roq_coinbase_pro_benchmark_LDADD = \
	$(top_builddir)/src/roq/coinbase/json/libroq-coinbase-json.la \
	$(top_builddir)/src/roq/coinbase/fix/libroq-coinbase-fix.la \
	$(top_builddir)/src/roq/coinbase/api/libroq-coinbase-api.la \
	$(DATE_LIBS) \
	$(ROQ_API_LIBS) \
	$(ROQ_CORE_LIBS) \
	$(FMT_LIBS) \
	$(BENCHMARK_LIBS)  \
	$(RT_LIBS) \
	-lpthread
roq_coinbase_pro_benchmark_CFLAGS = -pthread
roq_coinbase_pro_benchmark_CPPFLAGS = \
	-I$(top_builddir)/src \
	$(ROQ_API_CFLAGS) \
	$(ROQ_CORE_CFLAGS) \
	$(BENCHMARK_CFLAGS) \
	$(FMT_CFLAGS) \
	$(DATE_CFLAGS)
nodist_roq_coinbase_pro_benchmark_SOURCES = \
	utils.cpp \
	fix_execution_report.cpp \
	fix_heartbeat.cpp \
	fix_logon.cpp \
	fix_logout.cpp \
	fix_new_order_single.cpp \
	fix_order_cancel_reject.cpp \
	fix_order_cancel_request.cpp \
	fix_reject.cpp \
	fix_test_request.cpp \
	fmt_error.cpp \
	fmt_heartbeat.cpp \
	fmt_received.cpp \
	json_activate.cpp \
	json_change.cpp \
	json_currency.cpp \
	json_done.cpp \
	json_error.cpp \
	json_fill.cpp \
	json_fills.cpp \
	json_heartbeat.cpp \
	json_l2update.cpp \
	json_last_match.cpp \
	json_match.cpp \
	json_open.cpp \
	json_order.cpp \
	json_orders.cpp \
	json_product.cpp \
	json_products.cpp \
	json_received.cpp \
	json_snapshot.cpp \
	json_status.cpp \
	json_subscriptions.cpp \
	json_ticker.cpp \
	main.cpp
