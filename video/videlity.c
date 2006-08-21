/*
	Videlity functions

	Copyright (C) 2003	Patrice Mandin

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

/*--- Includes ---*/

#include <stdlib.h>

#include <mint/cookie.h>
#include <mint/errno.h>
#include <mint/osbind.h>

#include "videlity.h"

/*--- Defines ---*/

#define VDLY_BEGIN(stack) \
	stack=NULL; \
	if (Super(1)==0) { \
		stack=(void *)Super(NULL); \
	}

#define VDLY_END(stack) \
	if (stack!=NULL) { \
		Super(stack); \
	}

/*--- Types ---*/

/*--- Variables ---*/

static cookie_vdly_t *cookie_vdly;

/*--- Functions prototypes ---*/

#define VDLYCALL_v_v(func_ptr)	\
__extension__	\
({	\
	__asm__ volatile (	\
		"jbsr	%0@"	\
		: \
		: "a"(func_ptr)	\
		: "d0", "a0", "memory" \
	);	\
})

#define VDLYCALL_v_l(func_ptr, p1)	\
__extension__	\
({	\
	long _p1 = (long)(p1);	\
	\
	__asm__ volatile (	\
		"movl	%1,a0\n\t"	\
		"jbsr	%0@"	\
		: \
		: "a"(func_ptr), "g"(_p1)	\
		: "d0", "a0", "memory" \
	);	\
})

#define VDLYCALL_v_wl(func_ptr, p1, p2)	\
__extension__	\
({	\
	short _p1 = (short)(p1);	\
	long _p2 = (long)(p2);	\
	\
	__asm__ volatile (	\
		"movl	%2,a0\n\t"	\
		"movw	%1,d0\n\t"	\
		"jbsr	%0@"	\
		: \
		: "a"(func_ptr), "g"(_p1), "g"(_p2)	\
		: "d0", "a0", "memory" \
	);	\
})

#define VDLYCALL_l_wl(func_ptr, p1, p2)	\
__extension__	\
({	\
	short _p1 = (short)(p1);	\
	long _p2 = (long)(p2);	\
	register long retvalue __asm__("d0");	\
	\
	__asm__ volatile (	\
		"movw	%2,d0\n\t"	\
		"jbsr	%1@\n\t"	\
		"movl	a0,%3"	\
		: "=r"(retvalue)	\
		: "a"(func_ptr), "g"(_p1), "g"(_p2)	\
		: "a0", "memory" \
	);	\
	retvalue; \
})

/*--- Functions ---*/

long VDLY_Init(void)
{
	if (Getcookie(C_VDLY, (unsigned long *)&cookie_vdly) != C_FOUND) {
		return -ENOSYS;
	}

	return 0;
}

void VDLY_ReadModes(vdly_modeinfo_t *vdly_modeinfo)
{
	void *oldstack;

	VDLY_BEGIN(oldstack);
	VDLYCALL_v_l(cookie_vdly->readmodes, vdly_modeinfo);
	VDLY_END(oldstack);
}

void VDLY_AddMode(unsigned short modecode, void *vid_registers)
{
	void *oldstack;

	VDLY_BEGIN(oldstack);
	VDLYCALL_v_wl(cookie_vdly->addmode, modecode, vid_registers);
	VDLY_END(oldstack);
}

long VDLY_GetMode(unsigned short modecode, void *vid_registers)
{
	void *oldstack;
	long result;

	VDLY_BEGIN(oldstack);
	result = VDLYCALL_l_wl(cookie_vdly->getmode, modecode, vid_registers);
	VDLY_END(oldstack);
	return result;
}

void VDLY_SetMode(void *vid_registers)
{
	void *oldstack;

	VDLY_BEGIN(oldstack);
	cookie_vdly->video_registers = vid_registers;
	VDLYCALL_v_v(cookie_vdly->setmode);
	VDLY_END(oldstack);
}
