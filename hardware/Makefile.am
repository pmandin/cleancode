bin_PROGRAMS = hw.ttp hwrs.ttp

hw_ttp_SOURCES = hardware.c
hw_ttp_LDADD = $(top_builddir)/common/libparam.a \
	libhw.a

hwrs_ttp_SOURCES = rsvfdemo.c
hwrs_ttp_LDADD = $(top_builddir)/common/libparam.a

INCLUDES = -I$(top_srcdir)/common

noinst_LIBRARIES = libhw.a

libhw_a_SOURCES = hw_cpu.S hw_regs.S

rsvfincludedir = $(includedir)
rsvfinclude_HEADERS = rsvf.h

EXTRA_DIST = hw_cpu.h hw_regs.h
