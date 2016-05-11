bin_PROGRAMS = egcpops
lib_LTLIBRARIES = libegc.la
include_HEADERS = ${top_srcdir)/include/egclib.h

libegc_la_SOURCES = EGC_comm.c EGC_event.c EGC_lib.c EGC_utils.c \
          ASP_debug.c \
          CFR_base.c \
          PMG_base.c \
          EGC_comm_file.c EGC_comm_sock.c 
libegc_la_CFLAGS = -Wall -I$(top_srcdir)/include -DDISPLAY_ERROR_LOCATION
libegc_la_LDFLAGS = -version-info $(libegc_version)
libegc_la_LIBADD = -lrt -levent

egcpops_SOURCES = EGC_server.c
egcpops_CFLAGS = -Wall -I$(top_srcdir)/include -DDISPLAY_ERROR_LOCATION
egcpops_LDADD = libegc.la
egcpops_DEPENDENCIES = libegc.la


