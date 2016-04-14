#
# Copyright (c) 2016 KOBAYASHI Taizo
# $COPYRIGHT$
#

AM_CPPFLAGS = -I$(top_builddir)/include -I$(top_srcdir)/include
AM_LDFLAGS = -lrt -levent
LDADD = \
        $(top_builddir)/src/libegc.la

noinst_PROGRAMS = \
               clientA \
               clientB

clientA_DEPENDENCIES = $(LDADD)
clientA_SOURCES = clientA.c

clientB_DEPENDENCIES = $(LDADD)
clientB_SOURCES = clientB.c

	