# SO 35998856
check_LTLIBRARIES = libgtest.la
libgtest_la_SOURCES = \
        $(top_srcdir)/third_party/googletest/googletest/src/gtest-all.cc
libgtest_la_CPPFLAGS = \
        -I$(top_srcdir)/third_party/googletest/googletest/include \
        -I$(top_srcdir)/third_party/googletest/googletest
libgtest_la_LDFLAGS = -pthread

TESTS = roq-coinbase-pro-test
check_PROGRAMS = roq-coinbase-pro-test

roq_coinbase_pro_test_LDADD = \
	libgtest.la \
	$(top_builddir)/src/roq/coinbase/json/libroq-coinbase-json.la \
	$(top_builddir)/src/roq/coinbase/fix/libroq-coinbase-fix.la \
	$(top_builddir)/src/roq/coinbase/api/libroq-coinbase-api.la \
	$(DATE_LIBS) \
	$(ROQ_CORE_LIBS) \
	$(ROQ_API_LIBS) \
	$(FMT_LIBS) \
	$(DATE_LIBS)
roq_coinbase_pro_test_CPPFLAGS = \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/third_party/googletest/googletest/include \
	$(ROQ_API_CFLAGS) \
	$(ROQ_CORE_CFLAGS) \
	$(FMT_CFLAGS) \
	$(DATE_CFLAGS)
nodist_roq_coinbase_pro_test_SOURCES = \
	fix_execution_report.cpp \
	fix_heartbeat.cpp \
	fix_logon.cpp \
	fix_logout.cpp \
	fix_new_order_single.cpp \
	fix_order_cancel_reject.cpp \
	fix_order_cancel_request.cpp \
	fix_reject.cpp \
	fix_test_request.cpp \
	fmt_enums.cpp \
	json_account.cpp \
	json_accounts.cpp \
	json_activate.cpp \
	json_change.cpp \
	json_currency.cpp \
	json_done.cpp \
	json_error.cpp \
	json_fill.cpp \
	json_fills.cpp \
	json_heartbeat.cpp \
	json_last_match.cpp \
	json_l2book.cpp \
	json_l2update.cpp \
	json_l3book.cpp \
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
	json_time.cpp \
	main.cpp
