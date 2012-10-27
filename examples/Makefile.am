INCLUDES = -I$(top_builddir)                    \
	   -I/usr/include/apr-1.0/              \
           -I$(top_srcdir)/lucene               \
           -I$(top_srcdir)/include              \
           -I$(top_srcdir)                      \
           -I/usr/local/apr/include/apr-1


noinst_PROGRAMS=indexinfo

indexinfo_SOURCES=indexinfo.c indexinfo_func.c
indexinfo_CFLAGS = -Wall -D_LARGEFILE64_SOURCE
indexinfo_LDADD = $(top_builddir)/lucene/liblucene.la
