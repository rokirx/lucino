INCLUDES = -I$(top_srcdir)/lucenecpp          \
           -I/usr/local/apr/include/apr-1


TESTS=test_all
check_PROGRAMS = $(TESTS)

test_all_SOURCES = main.cpp                          \
                   test.cpp                          \
                   testsuite.cpp                     \
                   termquerytest.cpp                 \
                   documenttest.cpp                  \
                   ../lucenecpp/analyzer.cpp         \
                   ../lucenecpp/analyzermap.cpp      \
                   ../lucenecpp/autoreleasepool.cpp  \
                   ../lucenecpp/booleanquery.cpp     \
                   ../lucenecpp/directory.cpp        \
                   ../lucenecpp/document.cpp         \
                   ../lucenecpp/documentdumpiterator.cpp         \
                   ../lucenecpp/exception.cpp        \
                   ../lucenecpp/field.cpp            \
                   ../lucenecpp/hits.cpp             \
                   ../lucenecpp/indexreader.cpp      \
                   ../lucenecpp/lucenecpp.cpp        \
                   ../lucenecpp/prefixquery.cpp      \
                   ../lucenecpp/query.cpp            \
                   ../lucenecpp/string.cpp           \
                   ../lucenecpp/searcher.cpp         \
                   ../lucenecpp/term.cpp             \
		   ../lucenecpp/indexsearcher.cpp    \
                   ../lucenecpp/termquery.cpp        

test_all_CFLAGS  = -Wall -D_LARGEFILE64_SOURCE -D_LCNTRACE -fno-strict-aliasing -O0
