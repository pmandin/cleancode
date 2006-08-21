bin_PROGRAMS = viae.gtp vivd.ttp vixb.ttp

viae_gtp_SOURCES = aes.c vdi_com.c eddi.S rgb.c
viae_gtp_LDADD = $(top_builddir)/common/libparam.a \
	$(top_builddir)/common/libmxalloc.a

vivd_ttp_SOURCES = vdi.c vdi_com.c eddi.S rgb.c
vivd_ttp_LDADD = $(top_builddir)/common/libparam.a \
	$(top_builddir)/common/libmxalloc.a

vixb_ttp_SOURCES = xbios.c rgb.c
vixb_ttp_LDADD = $(top_builddir)/common/libparam.a \
	$(top_builddir)/common/libmxalloc.a \
	libsblast2.a libsblast3.a libmvdi.a libvinside.a libvidelity.a libnova.a

INCLUDES = -I$(top_srcdir)/common

lib_LIBRARIES = libsblast2.a libsblast3.a libmvdi.a libvinside.a libnova.a \
	libvidelity.a

libsblast2_a_SOURCES = sblast2.c

libsblast3_a_SOURCES = sblast3.c

libmvdi_a_SOURCES = mvdi.c

libvinside_a_SOURCES = vinside.c

libnova_a_SOURCES = nova.c

libvidelity_a_SOURCES = videlity.c

libvideoincludedir = $(includedir)
libvideoinclude_HEADERS = mvdi.h cscreen.h vinside.h blowup.h nova.h sblast3.h \
	sblast2.h videlity.h vscr.h

EXTRA_DIST = eddi_s.h rgb.h vdi_com.h overscan.h

