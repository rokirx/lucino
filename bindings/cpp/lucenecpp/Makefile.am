INCLUDES = -I$(top_builddir)                    \
           -I/usr/local/apr/include/apr-1


includedir=/usr/local/include/lucenecpp

include_HEADERS=                                \
                          autoreleasepool.hpp   \
                          booleanquery.hpp      \
                          directory.hpp         \
                          document.hpp          \
                          exception.hpp         \
                          field.hpp             \
                          hits.hpp              \
                          indexreader.hpp       \
                          indexsearcher.hpp     \
                          list.hpp              \
                          lucenecpp.hpp         \
                          poolobject.hpp        \
			  prefixquery.hpp       \
                          query.hpp             \
                          search.hpp            \
                          searcher.hpp          \
                          string.hpp            \
                          term.hpp              \
                          termquery.hpp         


lib_LTLIBRARIES     = liblucenecpp.la

liblucenecpp_la_CFLAGS = -Wall -D_LARGEFILE64_SOURCE -D_LCNTRACE

liblucenecpp_la_SOURCES =                      \
                          analyzer.cpp         \
                          analyzermap.cpp       \
                          autoreleasepool.cpp  \
                          booleanquery.cpp     \
                          directory.cpp        \
                          exception.cpp        \
                          document.cpp         \
                          documentdumpiterator.cpp \
                          field.cpp            \
                          hits.cpp             \
                          indexreader.cpp      \
                          indexsearcher.cpp    \
                          lucenecpp.cpp        \
                          prefixquery.cpp      \
                          query.cpp            \
                          searcher.cpp         \
                          string.cpp           \
                          term.cpp             \
                          termquery.cpp        
