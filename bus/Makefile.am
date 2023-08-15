bin_PROGRAMS = bupc.ttp busc.ttp

lib_LIBRARIES = libpci.a libscsi.a

libpci_a_SOURCES = pci.c pcinames.c pci_bios.c pci_xpci.c
libscsi_a_SOURCES = scsidrv.c scsi.c scsinames.c

bupc_ttp_SOURCES = bus_pci.c 
bupc_ttp_LDADD = $(top_builddir)/common/libparam.a \
	libpci.a

busc_ttp_SOURCES = bus_scsi.c
busc_ttp_LDADD = $(top_builddir)/common/libparam.a \
	libscsi.a

AM_CPPFLAGS = -I$(top_srcdir)/common

libpciincludedir = $(includedir)
libpciinclude_HEADERS = pci.h pcinames.h

libscsiincludedir = $(includedir)
libscsiinclude_HEADERS = scsidrv.h scsi.h scsinames.h

EXTRA_DIST = pci.ids pci_bios.h pci_xpci.h
