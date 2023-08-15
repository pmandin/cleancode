bin_PROGRAMS = dixh.ttp

lib_LIBRARIES = libxhdi.a

libxhdi_a_SOURCES = xhdi.c

dixh_ttp_SOURCES = xhdidemo.c
dixh_ttp_LDADD = $(top_builddir)/common/libparam.a \
	libxhdi.a

AM_CPPFLAGS = -I$(top_srcdir)/common

libxhdiincludedir = $(includedir)
libxhdiinclude_HEADERS = xhdi.h
