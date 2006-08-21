/*
	PCIBIOS calling functions

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
#include "endian.h"

/*--- Defines ---*/

#define PCIBIOS_BEGIN(stack) \
	stack=NULL; \
	if (Super(1)==0) { \
		stack=(void *)Super(NULL); \
	}

#define PCIBIOS_END(stack) \
	if (stack!=NULL) { \
		Super(stack); \
	}

/*--- PCI bios function calls ---*/

#define PCIBIOSCALL_l_d0ld1w(func_ptr,p1,p2)	\
__extension__	\
({	\
	register long retvalue __asm__("d0");	\
	\
	__asm__ volatile (	\
		"movw	%3,d1\n\t"	\
		"movl	%2,d0\n\t"	\
		"jbsr	%1@"	\
		: "=r"(retvalue)	\
		: "a"(func_ptr), "g"(p1), "g"(p2)	\
		: "d1", "memory"	\
	);	\
	retvalue;	\
})

#define PCIBIOSCALL_l_d0la0ld1b(func_ptr,p1,p2,p3)	\
__extension__	\
({	\
	register long retvalue __asm__("d0");	\
	\
	__asm__ volatile (	\
		"movb	%4,d1\n\t"	\
		"movl	%3,a0\n\t"	\
		"movl	%2,d0\n\t"	\
		"jbsr	%1@"	\
		: "=r"(retvalue)	\
		: "a"(func_ptr), "g"(p1), "g"(p2), "g"(p3)	\
		: "d1", "a0", "memory"	\
	);	\
	retvalue;	\
})

#define PCIBIOSCALL_l_d0ld1b(func_ptr,p1,p2)	\
__extension__	\
({	\
	register long retvalue __asm__("d0");	\
	\
	__asm__ volatile (	\
		"movb	%3,d1\n\t"	\
		"movl	%2,d0\n\t"	\
		"jbsr	%1@"	\
		: "=r"(retvalue)	\
		: "a"(func_ptr), "g"(p1), "g"(p2)	\
		: "d1", "memory"	\
	);	\
	retvalue;	\
})

#define PCIBIOSCALL_l_d0ld1bd2(func_ptr,p1,p2,p3)	\
__extension__	\
({	\
	register long retvalue __asm__("d0");	\
	\
	__asm__ volatile (	\
		"movl	%4,d2\n\t"	\
		"movb	%3,d1\n\t"	\
		"movl	%2,d0\n\t"	\
		"jbsr	%1@"	\
		: "=r"(retvalue)	\
		: "a"(func_ptr), "g"(p1), "g"(p2), "g"(p3)	\
		: "d1", "d2", "memory"	\
	);	\
	retvalue;	\
})

#define PCIBIOSCALL_l_d0la0ld1l(func_ptr,p1,p2,p3)	\
__extension__	\
({	\
	register long retvalue __asm__("d0");	\
	\
	__asm__ volatile (	\
		"movl	%4,d1\n\t"	\
		"movl	%3,a0\n\t"	\
		"movl	%2,d0\n\t"	\
		"jbsr	%1@"	\
		: "=r"(retvalue)	\
		: "a"(func_ptr), "g"(p1), "g"(p2), "g"(p3)	\
		: "d1", "a0", "memory"	\
	);	\
	retvalue;	\
})

#define PCIBIOSCALL_l_d0l(func_ptr,p1)	\
__extension__	\
({	\
	register long retvalue __asm__("d0");	\
	\
	__asm__ volatile (	\
		"movl	%2,d0\n\t"	\
		"jbsr	%1@"	\
		: "=r"(retvalue)	\
		: "a"(func_ptr), "g"(p1)	\
		: "memory"	\
	);	\
	retvalue;	\
})

#define PCIBIOSCALL_l_d0bd1l(func_ptr,p1,p2)	\
__extension__	\
({	\
	register long retvalue __asm__("d0");	\
	\
	__asm__ volatile (	\
		"movl	%3,d1\n\t"	\
		"movb	%2,d0\n\t"	\
		"jbsr	%1@"	\
		: "=r"(retvalue)	\
		: "a"(func_ptr), "g"(p1), "g"(p2)	\
		: "d1", "memory"	\
	);	\
	retvalue;	\
})

#define PCIBIOSCALL_l_d0la0l(func_ptr,p1,p2)	\
__extension__	\
({	\
	register long retvalue __asm__("d0");	\
	\
	__asm__ volatile (	\
		"movl	%3,a0\n\t"	\
		"movl	%2,d0\n\t"	\
		"jbsr	%1@"	\
		: "=r"(retvalue)	\
		: "a"(func_ptr), "g"(p1), "g"(p2)	\
		: "a0", "memory"	\
	);	\
	retvalue;	\
})

#define PCIBIOSCALL_l_d0ld1l(func_ptr,p1,p2)	\
__extension__	\
({	\
	register long retvalue __asm__("d0");	\
	\
	__asm__ volatile (	\
		"movl	%3,d1\n\t"	\
		"movl	%2,d0\n\t"	\
		"jbsr	%1@"	\
		: "=r"(retvalue)\
		: "a"(func_ptr), "g"(p1), "g"(p2)\
		: "d1", "memory"	\
	);	\
	retvalue;	\
})

#define PCIBIOSCALL_l_d0ld1ld2(func_ptr,p1,p2,p3)	\
__extension__	\
({	\
	register long retvalue __asm__("d0");	\
	\
	__asm__ volatile (	\
		"movl	%4,d2\n\t"	\
		"movl	%3,d1\n\t"	\
		"movl	%2,d0\n\t"	\
		"jbsr	%1@"	\
		: "=r"(retvalue)	\
		: "a"(func_ptr), "g"(p1), "g"(p2), "g"(p3)	\
		: "d1", "d2", "memory"	\
	);	\
	retvalue;	\
})

#define PCIBIOSCALL_l_void(func_ptr)	\
__extension__	\
({	\
	register long retvalue __asm__("d0");	\
	\
	__asm__ volatile (	\
		"jbsr	%1@"	\
		: "=r"(retvalue)	\
		: "a"(func_ptr)	\
		: "memory"	\
	);	\
	retvalue;	\
})

/*--- Types ---*/

typedef void (*pci_routine_t)(void);

typedef struct {
	unsigned long *subcookie;
	unsigned long version;
	pci_routine_t pci_routines[43];
} __attribute__((packed)) pci_cookie_t;

/*--- Variables ---*/

static pci_cookie_t *cookie_pci;

/*--- Functions ---*/

static long pcibios_find_device(unsigned long device_handle, unsigned short index)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0ld1w(cookie_pci->pci_routines[0], device_handle, index);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_find_classcode(unsigned long class_code, unsigned short index)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0ld1w(cookie_pci->pci_routines[1], class_code, index);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_read_config_byte(unsigned long device_handle, void *data, unsigned char num_register)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0la0ld1b(cookie_pci->pci_routines[2], device_handle, data, num_register);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_read_config_word(unsigned long device_handle, void *data, unsigned char num_register)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0la0ld1b(cookie_pci->pci_routines[3], device_handle, data, num_register);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_read_config_long(unsigned long device_handle, void *data, unsigned char num_register)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0la0ld1b(cookie_pci->pci_routines[4], device_handle, data, num_register);
	PCIBIOS_END(oldstack);
	return result;
}

static unsigned char pcibios_read_config_byte_fast(unsigned long device_handle, unsigned char num_register)
{
	unsigned char result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0ld1b(cookie_pci->pci_routines[5], device_handle, num_register);
	PCIBIOS_END(oldstack);
	return result;
}

static unsigned short pcibios_read_config_word_fast(unsigned long device_handle, unsigned char num_register)
{
	unsigned short result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0ld1b(cookie_pci->pci_routines[6], device_handle, num_register);
	PCIBIOS_END(oldstack);
	return result;
}

static unsigned long pcibios_read_config_long_fast(unsigned long device_handle, unsigned char num_register)
{
	unsigned long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0ld1b(cookie_pci->pci_routines[7], device_handle, num_register);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_write_config_byte(unsigned long device_handle, unsigned char num_register, unsigned char value)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0ld1bd2(cookie_pci->pci_routines[8], device_handle, num_register, value);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_write_config_word(unsigned long device_handle, unsigned char num_register, unsigned short value)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0ld1bd2(cookie_pci->pci_routines[9], device_handle, num_register, value);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_write_config_long(unsigned long device_handle, unsigned char num_register, unsigned long value)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0ld1bd2(cookie_pci->pci_routines[10], device_handle, num_register, value);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_hook_interrupt(unsigned long device_handle, void (*data)(), unsigned long parameter)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0la0ld1l(cookie_pci->pci_routines[11], device_handle, data, parameter);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_unhook_interrupt(unsigned long device_handle)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0l(cookie_pci->pci_routines[12], device_handle);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_special_cycle(unsigned char num_bus, unsigned long data)
{
	long result=PCI_FUNC_NOT_SUPPORTED;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0bd1l(cookie_pci->pci_routines[13], num_bus, data);
	PCIBIOS_END(oldstack);
	return result;
}

/* pci_get_routing() */

/* pci_set_interrupt() */

static long pcibios_get_resource(unsigned long device_handle)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0l(cookie_pci->pci_routines[16], device_handle);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_get_card_used(unsigned long device_handle, unsigned long *callback)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0la0l(cookie_pci->pci_routines[17], device_handle, callback);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_set_card_used(unsigned long device_handle, unsigned long callback)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0la0l(cookie_pci->pci_routines[18], device_handle, callback);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_read_mem_byte(unsigned long device_handle, unsigned long pci_address, unsigned char *data)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0la0ld1l(cookie_pci->pci_routines[19], device_handle, data, pci_address);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_read_mem_word(unsigned long device_handle, unsigned long pci_address, unsigned short *data)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0la0ld1l(cookie_pci->pci_routines[20], device_handle, data, pci_address);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_read_mem_long(unsigned long device_handle, unsigned long pci_address, unsigned long *data)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0la0ld1l(cookie_pci->pci_routines[21], device_handle, data, pci_address);
	PCIBIOS_END(oldstack);
	return result;
}

static unsigned char pcibios_read_mem_byte_fast(unsigned long device_handle, unsigned long pci_address)
{
	unsigned char result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0ld1l(cookie_pci->pci_routines[22], device_handle, pci_address);
	PCIBIOS_END(oldstack);
	return result;
}

static unsigned short pcibios_read_mem_word_fast(unsigned long device_handle, unsigned long pci_address)
{
	unsigned short result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0ld1l(cookie_pci->pci_routines[23], device_handle, pci_address);
	PCIBIOS_END(oldstack);
	return result;
}

static unsigned long pcibios_read_mem_long_fast(unsigned long device_handle, unsigned long pci_address)
{
	unsigned long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0ld1l(cookie_pci->pci_routines[24], device_handle, pci_address);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_write_mem_byte(unsigned long device_handle, unsigned long pci_address, unsigned char value)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0ld1ld2(cookie_pci->pci_routines[25], device_handle, pci_address, value);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_write_mem_word(unsigned long device_handle, unsigned long pci_address, unsigned short value)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0ld1ld2(cookie_pci->pci_routines[26], device_handle, pci_address, value);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_write_mem_long(unsigned long device_handle, unsigned long pci_address, unsigned long value)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0ld1ld2(cookie_pci->pci_routines[27], device_handle, pci_address, value);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_read_io_byte(unsigned long device_handle, unsigned long pci_address, unsigned char *data)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0la0ld1l(cookie_pci->pci_routines[28], device_handle, data, pci_address);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_read_io_word(unsigned long device_handle, unsigned long pci_address, unsigned short *data)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0la0ld1l(cookie_pci->pci_routines[29], device_handle, data, pci_address);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_read_io_long(unsigned long device_handle, unsigned long pci_address, unsigned long *data)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0la0ld1l(cookie_pci->pci_routines[30], device_handle, data, pci_address);
	PCIBIOS_END(oldstack);
	return result;
}

static unsigned char pcibios_read_io_byte_fast(unsigned long device_handle, unsigned long pci_address)
{
	unsigned char result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0ld1l(cookie_pci->pci_routines[31], device_handle, pci_address);
	PCIBIOS_END(oldstack);
	return result;
}

static unsigned short pcibios_read_io_word_fast(unsigned long device_handle, unsigned long pci_address)
{
	unsigned long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0ld1l(cookie_pci->pci_routines[32], device_handle, pci_address);
	PCIBIOS_END(oldstack);
	return result;
}

static unsigned long pcibios_read_io_long_fast(unsigned long device_handle, unsigned long pci_address)
{
	unsigned long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0ld1l(cookie_pci->pci_routines[33], device_handle, pci_address);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_write_io_byte(unsigned long device_handle, unsigned long pci_address, unsigned char value)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0ld1ld2(cookie_pci->pci_routines[34], device_handle, pci_address, value);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_write_io_word(unsigned long device_handle, unsigned long pci_address, unsigned short value)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0ld1ld2(cookie_pci->pci_routines[35], device_handle, pci_address, value);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_write_io_long(unsigned long device_handle, unsigned long pci_address, unsigned long value)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0ld1ld2(cookie_pci->pci_routines[36], device_handle, pci_address, value);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_get_machine_id(void)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_void(cookie_pci->pci_routines[37]);
	PCIBIOS_END(oldstack);
	return result;
}

static unsigned long pcibios_get_pagesize(void)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_void(cookie_pci->pci_routines[38]);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_virt_to_bus(unsigned long device_handle, void *virt_cpu_address, void *data)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0la0ld1l(cookie_pci->pci_routines[39], device_handle, data, virt_cpu_address);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_bus_to_virt(unsigned long device_handle, unsigned long pci_address, void *data)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0la0ld1l(cookie_pci->pci_routines[40], device_handle, data, pci_address);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_virt_to_phys(void *virt_cpu_address, void *data)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0la0l(cookie_pci->pci_routines[41], virt_cpu_address, data);
	PCIBIOS_END(oldstack);
	return result;
}

static long pcibios_phys_to_virt(void *phys_cpu_address, void *data)
{
	long result;
	void *oldstack;

	PCIBIOS_BEGIN(oldstack);
	result = PCIBIOSCALL_l_d0la0l(cookie_pci->pci_routines[42], phys_cpu_address, data);
	PCIBIOS_END(oldstack);
	return result;
}

long pcibios_init(void)
{
	/* Check _PCI cookie */
	if (Getcookie(C__PCI, (unsigned long *)&cookie_pci) != C_FOUND) {
		return PCI_FUNC_NOT_SUPPORTED;
	}

	if ( (((cookie_pci->version)>>16)&0xffff) != 0x0001) {
		return PCI_FUNC_NOT_SUPPORTED;
	}

	pci_find_device = pcibios_find_device;
	pci_find_classcode = pcibios_find_classcode;
	pci_read_config_byte = pcibios_read_config_byte;
	pci_read_config_word = pcibios_read_config_word;
	pci_read_config_long = pcibios_read_config_long;
	pci_read_config_byte_fast = pcibios_read_config_byte_fast;
	pci_read_config_word_fast = pcibios_read_config_word_fast;
	pci_read_config_long_fast = pcibios_read_config_long_fast;
	pci_write_config_byte = pcibios_write_config_byte;
	pci_write_config_word = pcibios_write_config_word;
	pci_write_config_long = pcibios_write_config_long;
	pci_hook_interrupt = pcibios_hook_interrupt;
	pci_unhook_interrupt = pcibios_unhook_interrupt;
	pci_special_cycle = pcibios_special_cycle;
/* pci_get_routing */
/* pci_set_interrupt */
	pci_get_resource = pcibios_get_resource;
	pci_get_card_used = pcibios_get_card_used;
	pci_set_card_used = pcibios_set_card_used;
	pci_read_mem_byte = pcibios_read_mem_byte;
	pci_read_mem_word = pcibios_read_mem_word;
	pci_read_mem_long = pcibios_read_mem_long;
	pci_read_mem_byte_fast = pcibios_read_mem_byte_fast;
	pci_read_mem_word_fast = pcibios_read_mem_word_fast;
	pci_read_mem_long_fast = pcibios_read_mem_long_fast;
	pci_write_mem_byte = pcibios_write_mem_byte;
	pci_write_mem_word = pcibios_write_mem_word;
	pci_write_mem_long = pcibios_write_mem_long;
	pci_read_io_byte = pcibios_read_io_byte;
	pci_read_io_word = pcibios_read_io_word;
	pci_read_io_long = pcibios_read_io_long;
	pci_read_io_byte_fast = pcibios_read_io_byte_fast;
	pci_read_io_word_fast = pcibios_read_io_word_fast;
	pci_read_io_long_fast = pcibios_read_io_long_fast;
	pci_write_io_byte = pcibios_write_io_byte;
	pci_write_io_word = pcibios_write_io_word;
	pci_write_io_long = pcibios_write_io_long;
	pci_get_machine_id = pcibios_get_machine_id;
	pci_get_pagesize = pcibios_get_pagesize;
	pci_virt_to_bus = pcibios_virt_to_bus;
	pci_bus_to_virt = pcibios_bus_to_virt;
	pci_virt_to_phys = pcibios_virt_to_phys;
	pci_phys_to_virt = pcibios_phys_to_virt;
	
	return PCI_SUCCESSFUL;
}
