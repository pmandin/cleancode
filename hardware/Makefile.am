bin_PROGRAMS = hw.ttp hwrs.ttp

hw_ttp_SOURCES = hardware.c mmu.c
hw_ttp_LDADD = $(top_builddir)/common/libparam.a \
	libhw.a

hwrs_ttp_SOURCES = rsvfdemo.c
hwrs_ttp_LDADD = $(top_builddir)/common/libparam.a

AM_CPPFLAGS = -I$(top_srcdir)/common

noinst_LIBRARIES = libhw.a

libhw_a_SOURCES = hw_cpu.S hw_regs.S

rsvfincludedir = $(includedir)
rsvfinclude_HEADERS = rsvf.h

EXTRA_DIST = hw_cpu.h hw_regs.h mmu.h
