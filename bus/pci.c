/*
	PCI functions

	Copyright (C) 2002	Patrice Mandin
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

#include "pci.h"
#include "pci_bios.h"
#include "pci_xpci.h"
#include "endian.h"

/*--- Functions pointers ---*/

long (*pci_find_device)(unsigned long device_vendor_id, unsigned short index);
long (*pci_find_classcode)(unsigned long class_code, unsigned short index);
long (*pci_read_config_byte)(unsigned long device_handle, void *data, unsigned char num_register);
long (*pci_read_config_word)(unsigned long device_handle, void *data, unsigned char num_register);
long (*pci_read_config_long)(unsigned long device_handle, void *data, unsigned char num_register);
unsigned char (*pci_read_config_byte_fast)(unsigned long device_handle, unsigned char num_register);
unsigned short (*pci_read_config_word_fast)(unsigned long device_handle, unsigned char num_register);
unsigned long (*pci_read_config_long_fast)(unsigned long device_handle, unsigned char num_register);
long (*pci_write_config_byte)(unsigned long device_handle, unsigned char num_register, unsigned char value);
long (*pci_write_config_word)(unsigned long device_handle, unsigned char num_register, unsigned short value);
long (*pci_write_config_long)(unsigned long device_handle, unsigned char num_register, unsigned long value);
long (*pci_hook_interrupt)(unsigned long device_handle, void (*data)(), unsigned long parameter); 
long (*pci_unhook_interrupt)(unsigned long device_handle); 
long (*pci_special_cycle)(unsigned char num_bus, unsigned long data); 
/* pci_get_routing */
/* pci_set_interrupt */
long (*pci_get_resource)(unsigned long device_handle); 
long (*pci_get_card_used)(unsigned long device_handle, unsigned long *callback);
long (*pci_set_card_used)(unsigned long device_handle, unsigned long callback);
long (*pci_read_mem_byte)(unsigned long device_handle, unsigned long pci_address, unsigned char *data);
long (*pci_read_mem_word)(unsigned long device_handle, unsigned long pci_address, unsigned short *data);
long (*pci_read_mem_long)(unsigned long device_handle, unsigned long pci_address, unsigned long *data);
unsigned char (*pci_read_mem_byte_fast)(unsigned long device_handle, unsigned long pci_address);
unsigned short (*pci_read_mem_word_fast)(unsigned long device_handle, unsigned long pci_address);
unsigned long (*pci_read_mem_long_fast)(unsigned long device_handle, unsigned long pci_address);
long (*pci_write_mem_byte)(unsigned long device_handle, unsigned long pci_address, unsigned char value);
long (*pci_write_mem_word)(unsigned long device_handle, unsigned long pci_address, unsigned short value);
long (*pci_write_mem_long)(unsigned long device_handle, unsigned long pci_address, unsigned long value);
long (*pci_read_io_byte)(unsigned long device_handle, unsigned long pci_address, unsigned char *data);
long (*pci_read_io_word)(unsigned long device_handle, unsigned long pci_address, unsigned short *data);
long (*pci_read_io_long)(unsigned long device_handle, unsigned long pci_address, unsigned long *data);
unsigned char (*pci_read_io_byte_fast)(unsigned long device_handle, unsigned long pci_address);
unsigned short (*pci_read_io_word_fast)(unsigned long device_handle, unsigned long pci_address);
unsigned long (*pci_read_io_long_fast)(unsigned long device_handle, unsigned long pci_address);
long (*pci_write_io_byte)(unsigned long device_handle, unsigned long pci_address, unsigned char value);
long (*pci_write_io_word)(unsigned long device_handle, unsigned long pci_address, unsigned short value);
long (*pci_write_io_long)(unsigned long device_handle, unsigned long pci_address, unsigned long value);
long (*pci_get_machine_id)(void);
unsigned long (*pci_get_pagesize)(void);
long (*pci_virt_to_bus)(unsigned long device_handle, void *virt_cpu_address, void *data);
long (*pci_bus_to_virt)(unsigned long device_handle, unsigned long pci_address, void *data);
long (*pci_virt_to_phys)(void *virt_cpu_address, void *data);
long (*pci_phys_to_virt)(void *phys_cpu_address, void *data);

/*--- Functions prototypes ---*/

static unsigned long pci_size(unsigned long base, unsigned long mask);

/*--- Functions ---*/

long pci_init(void)
{
	long errorcode;

	errorcode=xpci_init();
	if (errorcode!=PCI_SUCCESSFUL) {
		errorcode=pcibios_init();
	}

	return errorcode;
}

long pci_read_config(unsigned long device_handle, pci_config_t *device_config)
{
	long result=PCI_SUCCESSFUL;
	int i;
	unsigned long *buffer;

	buffer = (unsigned long *)device_config;
	for (i=0; i<0x80/4; i++) {
		unsigned long value;

		result = pci_read_config_long(device_handle, &value, i<<2);
		if (result<0) {
			return i<<2;
		}

		buffer[i] = LE_LONG(value);
	}

	return 0x80;
}

#if 0
static unsigned long pci_size(unsigned long base, unsigned long mask)
{
	unsigned long size;
	
	size = mask & base;		/* Find the significant bits */
	size = size & ~(size-1);	/* Get the lowest of them to find the decode size */
	return size-1;			/* extent = size - 1 */
}
#endif

unsigned long pci_read_mem_size(unsigned long device_handle, int config_register)
{
	unsigned long old_register, size;
	int num_bits;
	
	/* Read current register value */
	pci_read_config_long(device_handle, &old_register, config_register);

	/* Write not(0) */
	pci_write_config_long(device_handle, config_register, 0xffffffffUL);

	/* Read value */
	pci_read_config_long(device_handle, &size, config_register);

	/* Write previous register value */
	pci_write_config_long(device_handle, config_register, old_register);

	if (!size || size== 0xffffffff) {
		return 0;
	}

	if (old_register == 0xffffffff) {
		old_register = 0;
	}

	/* Find first 0 bit */
	num_bits = 31;
	for (num_bits=31; num_bits>0; --num_bits) {
		if ((size & (1<<num_bits))==0) {
			break;
		}
	}

	return 1<<(num_bits+1);
}

unsigned long pci_read_mem_base(unsigned long adr, int *memtype)
{
	*memtype = ((adr & PCI_BASE_ADDRESS_SPACE) == PCI_BASE_ADDRESS_SPACE_MEMORY);

	if (memtype) {
		adr &= PCI_BASE_ADDRESS_MEM_MASK;
	} else {
		adr &= PCI_BASE_ADDRESS_IO_MASK;
	}

	return adr;
}
