TESTS = roq-bitmex-benchmark
bin_PROGRAMS = roq-bitmex-benchmark
roq_bitmex_benchmark_LDADD = \
	$(top_builddir)/src/roq/bitmex/json/libroq-bitmex-json.la \
	$(top_builddir)/src/roq/bitmex/api/libroq-bitmex-api.la \
	$(DATE_LIBS) \
	$(ROQ_API_LIBS) \
	$(ROQ_CORE_LIBS) \
	$(FMT_LIBS) \
	$(BENCHMARK_LIBS)  \
	$(RT_LIBS) \
	-lpthread
roq_bitmex_benchmark_CFLAGS = -pthread
roq_bitmex_benchmark_CPPFLAGS = \
	-I$(top_builddir)/src \
	$(ROQ_API_CFLAGS) \
	$(ROQ_CORE_CFLAGS) \
	$(BENCHMARK_CFLAGS) \
	$(FMT_CFLAGS) \
	$(DATE_CFLAGS)
nodist_roq_bitmex_benchmark_SOURCES = \
	main.cpp
