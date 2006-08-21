/*
	XPCI calling functions

	Copyright (C) 2003	Patrice Mandin
	Some parts (in the utility functions), from the Linux kernel

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
*/

#include <stdio.h>
#include <stdlib.h>

#include <mint/osbind.h>
#include <mint/cookie.h>
#include <mint/falcon.h> /* for some trap14_*() macros */

#include "pci.h"
#include "pci_xpci.h"
#include "endian.h"

#ifndef OSBIND_CLOBBER_LIST
#define OSBIND_CLOBBER_LIST "d1", "d2", "a0", "a1", "a2"
#endif

/*--- XPCI function calls ---*/

#ifndef trap_14_wlw
#define trap_14_wlw(n,a,b)	\
__extension__	\
({	\
	register long retvalue __asm__("d0");	\
	long _a = (long)(a);	\
	short _b = (short)(b);	\
	\
	__asm__ volatile (	\
		"movw	%3,sp@-\n\t"	\
		"movl	%2,sp@-\n\t"	\
		"movw	%1,sp@-\n\t"	\
		"trap	#14\n\t"	\
		"addql	#8,sp"	\
		: "=r"(retvalue)	\
		: "g"(n), "r"(_a), "r"(_b)	\
		: OSBIND_CLOBBER_LIST	\
	);	\
	retvalue;	\
})
#endif

#ifndef trap_14_wlww
#define trap_14_wlww(n,a,b,c)	\
__extension__	\
({	\
	register long retvalue __asm__("d0");	\
	long _a = (long)(a);	\
	short _b = (short)(b);	\
	short _c = (short)(c);	\
	\
	__asm__ volatile (	\
		"movw	%4,sp@-\n\t"	\
		"movw	%3,sp@-\n\t"	\
		"movl	%2,sp@-\n\t"	\
		"movw	%1,sp@-\n\t"	\
		"trap	#14\n\t"	\
		"lea	sp@(10),sp"	\
		: "=r"(retvalue)	\
		: "g"(n), "r"(_a), "r"(_b), "r"(_c)	\
		: OSBIND_CLOBBER_LIST	\
	);	\
	retvalue;	\
})
#endif

/*--- Types ---*/

/*--- Variables ---*/

static unsigned long cookie_xpci;

/*--- Functions prototypes ---*/

/*--- Functions ---*/

static long xpci_find_device(unsigned long device_handle, unsigned short index)
{
	return (long)trap_14_wlw((short)300,(long)device_handle,(short)index);
}

static long xpci_find_classcode(unsigned long class_code, unsigned short index)
{
	return (long)trap_14_wlw((short)301,(long)class_code,(short)index);
}

static long xpci_read_config_byte(unsigned long device_handle, void *data, unsigned char num_register)
{
	return (long)trap_14_wlwl((short)302,(long)device_handle,(short)num_register,(long)data);
}

static long xpci_read_config_word(unsigned long device_handle, void *data, unsigned char num_register)
{
	return (long)trap_14_wlwl((short)303,(long)device_handle,(short)num_register,(long)data);
}

static long xpci_read_config_long(unsigned long device_handle, void *data, unsigned char num_register)
{
	return (long)trap_14_wlwl((short)304,(long)device_handle,(short)num_register,(long)data);
}

static unsigned char xpci_read_config_byte_fast(unsigned long device_handle, unsigned char num_register)
{
	return (unsigned char)trap_14_wlw((short)305,(long)device_handle,(short)num_register);
}

static unsigned short xpci_read_config_word_fast(unsigned long device_handle, unsigned char num_register)
{
	return (unsigned short)trap_14_wlw((short)306,(long)device_handle,(short)num_register);
}

static unsigned long xpci_read_config_long_fast(unsigned long device_handle, unsigned char num_register)
{
	return (unsigned long)trap_14_wlw((short)307,(long)device_handle,(short)num_register);
}

static long xpci_write_config_byte(unsigned long device_handle, unsigned char num_register, unsigned char value)
{
	return (long)trap_14_wlww((short)308,(long)device_handle,(short)num_register,(short)value);
}

static long xpci_write_config_word(unsigned long device_handle, unsigned char num_register, unsigned short value)
{
	return (long)trap_14_wlww((short)309,(long)device_handle,(short)num_register,(short)value);
}

static long xpci_write_config_long(unsigned long device_handle, unsigned char num_register, unsigned long value)
{
	return (long)trap_14_wlwl((short)310,(long)device_handle,(short)num_register,(long)value);
}

static long xpci_hook_interrupt(unsigned long device_handle, void (*data)(), unsigned long parameter)
{
	return (long)trap_14_wlll((short)311,(long)device_handle,(long)data,(long)parameter);
}

static long xpci_unhook_interrupt(unsigned long device_handle)
{
	return (long)trap_14_wl((short)312,(long)device_handle);
}

static long xpci_special_cycle(unsigned char num_bus, unsigned long data)
{
	return (long)trap_14_wwl((short)313,(short)num_bus,(long)data);
}

/*
static long xpci_get_routing(unsigned long device_handle)
{
	result = (long)trap_14_wl((short)314,(long)device_handle);
}

static long xpci_set_interrupt(unsigned long device_handle)
{
	result = (long)trap_14_wl((short)315,(long)device_handle);
}
*/

static long xpci_get_resource(unsigned long device_handle)
{
	return (long)trap_14_wl((short)316,(long)device_handle);
}

static long xpci_get_card_used(unsigned long device_handle, unsigned long *callback)
{
	return (long)trap_14_wll((short)317,(long)device_handle,(long)callback);
}

static long xpci_set_card_used(unsigned long device_handle, unsigned long callback)
{
	return (long)trap_14_wll((short)318,(long)device_handle,(long)callback);
}

static long xpci_read_mem_byte(unsigned long device_handle, unsigned long pci_address, unsigned char *data)
{
	return (long)trap_14_wlll((short)319,(long)device_handle,(long)pci_address,(long)data);
}

static long xpci_read_mem_word(unsigned long device_handle, unsigned long pci_address, unsigned short *data)
{
	return (long)trap_14_wlll((short)320,(long)device_handle,(long)pci_address,(long)data);
}

static long xpci_read_mem_long(unsigned long device_handle, unsigned long pci_address, unsigned long *data)
{
	return (long)trap_14_wlll((short)321,(long)device_handle,(long)pci_address,(long)data);
}

static unsigned char xpci_read_mem_byte_fast(unsigned long device_handle, unsigned long pci_address)
{
	return (long)trap_14_wll((short)322,(long)device_handle,(long)pci_address);
}

static unsigned short xpci_read_mem_word_fast(unsigned long device_handle, unsigned long pci_address)
{
	return (long)trap_14_wll((short)323,(long)device_handle,(long)pci_address);
}

static unsigned long xpci_read_mem_long_fast(unsigned long device_handle, unsigned long pci_address)
{
	return (long)trap_14_wll((short)324,(long)device_handle,(long)pci_address);
}

static long xpci_write_mem_byte(unsigned long device_handle, unsigned long pci_address, unsigned char value)
{
	return (long)trap_14_wllw((short)325,(long)device_handle,(long)pci_address,(short)value);
}

static long xpci_write_mem_word(unsigned long device_handle, unsigned long pci_address, unsigned short value)
{
	return (long)trap_14_wllw((short)326,(long)device_handle,(long)pci_address,(short)value);
}

static long xpci_write_mem_long(unsigned long device_handle, unsigned long pci_address, unsigned long value)
{
	return (long)trap_14_wlll((short)327,(long)device_handle,(long)pci_address,(long)value);
}

static long xpci_read_io_byte(unsigned long device_handle, unsigned long pci_address, unsigned char *data)
{
	return (long)trap_14_wlll((short)328,(long)device_handle,(long)pci_address,(long)data);
}

static long xpci_read_io_word(unsigned long device_handle, unsigned long pci_address, unsigned short *data)
{
	return (long)trap_14_wlll((short)329,(long)device_handle,(long)pci_address,(long)data);
}

static long xpci_read_io_long(unsigned long device_handle, unsigned long pci_address, unsigned long *data)
{
	return (long)trap_14_wlll((short)330,(long)device_handle,(long)pci_address,(long)data);
}

static unsigned char xpci_read_io_byte_fast(unsigned long device_handle, unsigned long pci_address)
{
	return (unsigned char)trap_14_wll((short)331,(long)device_handle,(long)pci_address);
}

static unsigned short xpci_read_io_word_fast(unsigned long device_handle, unsigned long pci_address)
{
	return (unsigned short)trap_14_wll((short)332,(long)device_handle,(long)pci_address);
}

static unsigned long xpci_read_io_long_fast(unsigned long device_handle, unsigned long pci_address)
{
	return (unsigned long)trap_14_wll((short)333,(long)device_handle,(long)pci_address);
}

static long xpci_write_io_byte(unsigned long device_handle, unsigned long pci_address, unsigned char value)
{
	return (long)trap_14_wllw((short)334,(long)device_handle,(long)pci_address,(short)value);
}

static long xpci_write_io_word(unsigned long device_handle, unsigned long pci_address, unsigned short value)
{
	return (long)trap_14_wllw((short)335,(long)device_handle,(long)pci_address,(short)value);
}

static long xpci_write_io_long(unsigned long device_handle, unsigned long pci_address, unsigned long value)
{
	return (long)trap_14_wlll((short)336,(long)device_handle,(long)pci_address,(long)value);
}

static long xpci_get_machine_id(void)
{
	return (long)trap_14_w((short)337);
}

static unsigned long xpci_get_pagesize(void)
{
	return (unsigned long)trap_14_w((short)338);
}

static long xpci_virt_to_bus(unsigned long device_handle, void *virt_cpu_address, void *data)
{
	return (long)trap_14_wlll((short)339,(long)device_handle,(long)virt_cpu_address,(long)data);
}

static long xpci_bus_to_virt(unsigned long device_handle, unsigned long pci_address, void *data)
{
	return (long)trap_14_wlll((short)340,(long)device_handle,(long)pci_address,(long)data);
}

static long xpci_virt_to_phys(void *virt_cpu_address, void *data)
{
	return (long)trap_14_wll((short)341,(long)virt_cpu_address,(long)data);
}

static long xpci_phys_to_virt(void *phys_cpu_address, void *data)
{
	return (long)trap_14_wll((short)342,(long)phys_cpu_address,(long)data);
}

long xpci_init(void)
{
	/* Check XPCI cookie */
	if (Getcookie(C_XPCI, (unsigned long *)&cookie_xpci) != C_FOUND) {
		return PCI_FUNC_NOT_SUPPORTED;
	}

	pci_find_device = xpci_find_device;
	pci_find_classcode = xpci_find_classcode;
	pci_read_config_byte = xpci_read_config_byte;
	pci_read_config_word = xpci_read_config_word;
	pci_read_config_long = xpci_read_config_long;
	pci_read_config_byte_fast = xpci_read_config_byte_fast;
	pci_read_config_word_fast = xpci_read_config_word_fast;
	pci_read_config_long_fast = xpci_read_config_long_fast;
	pci_write_config_byte = xpci_write_config_byte;
	pci_write_config_word = xpci_write_config_word;
	pci_write_config_long = xpci_write_config_long;
	pci_hook_interrupt = xpci_hook_interrupt;
	pci_unhook_interrupt = xpci_unhook_interrupt;
	pci_special_cycle = xpci_special_cycle;
/* pci_get_routing */
/* pci_set_interrupt */
	pci_get_resource = xpci_get_resource;
	pci_get_card_used = xpci_get_card_used;
	pci_set_card_used = xpci_set_card_used;
	pci_read_mem_byte = xpci_read_mem_byte;
	pci_read_mem_word = xpci_read_mem_word;
	pci_read_mem_long = xpci_read_mem_long;
	pci_read_mem_byte_fast = xpci_read_mem_byte_fast;
	pci_read_mem_word_fast = xpci_read_mem_word_fast;
	pci_read_mem_long_fast = xpci_read_mem_long_fast;
	pci_write_mem_byte = xpci_write_mem_byte;
	pci_write_mem_word = xpci_write_mem_word;
	pci_write_mem_long = xpci_write_mem_long;
	pci_read_io_byte = xpci_read_io_byte;
	pci_read_io_word = xpci_read_io_word;
	pci_read_io_long = xpci_read_io_long;
	pci_read_io_byte_fast = xpci_read_io_byte_fast;
	pci_read_io_word_fast = xpci_read_io_word_fast;
	pci_read_io_long_fast = xpci_read_io_long_fast;
	pci_write_io_byte = xpci_write_io_byte;
	pci_write_io_word = xpci_write_io_word;
	pci_write_io_long = xpci_write_io_long;
	pci_get_machine_id = xpci_get_machine_id;
	pci_get_pagesize = xpci_get_pagesize;
	pci_virt_to_bus = xpci_virt_to_bus;
	pci_bus_to_virt = xpci_bus_to_virt;
	pci_virt_to_phys = xpci_virt_to_phys;
	pci_phys_to_virt = xpci_phys_to_virt;
	
	return PCI_SUCCESSFUL;
}
