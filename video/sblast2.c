/*
	ScreenBlaster 2 functions

	Copyright (C) 2002	Patrice Mandin

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

#include "sblast2.h"

/*--- Defines ---*/

#define SBLAST2_BEGIN(stack) \
	stack=NULL; \
	if (Super(1)==0) { \
		stack=(void *)Super(NULL); \
	}

#define SBLAST2_END(stack) \
	if (stack!=NULL) { \
		Super(stack); \
	}

/*--- Types ---*/

enum {
	SBLAST2_FUNC_GETPATH=0,
	SBLAST2_FUNC_GETMODES,
	SBLAST2_FUNC_SETVIDMODE,
	SBLAST2_FUNC_GETSTATUS,
	SBLAST2_FUNC_SETVRTSCREEN,
	SBLAST2_FUNC_GETFILE
};

/*--- Variables ---*/

static void (*cookie_osbl)(void);

/*--- Functions prototypes ---*/

#define SBLAST2_CLOBBER_LIST	"d0", "d1", "d2", "a0", "a1", "memory"
#define SBLAST2_CLOBBER_LIST_D	"d1", "d2", "a0", "a1", "memory"
#define SBLAST2_CLOBBER_LIST_A	"d0", "d1", "d2", "a1", "memory"

#define SBLAST2CALL_la_v(func_ptr, opcode)	\
__extension__	\
({	\
	register long retvalue __asm__("a0");	\
	\
	__asm__ volatile (	\
		"movql	%2,d0\n\t"	\
		"jbsr	%1@"	\
		: "=r"(retvalue)	\
		: "a"(func_ptr), "g"(opcode)	\
		: SBLAST2_CLOBBER_LIST_A	\
	);	\
	retvalue;	\
})

#define SBLAST2CALL_v_l(func_ptr, opcode, p1)	\
__extension__	\
({	\
	long _p1 = (long)(p1);	\
	\
	__asm__ volatile (	\
		"movl	%2,a0\n\t"	\
		"movql	%1,d0\n\t"	\
		"jbsr	%0@"	\
		: \
		: "a"(func_ptr), "g"(opcode), "g"(_p1)	\
		: SBLAST2_CLOBBER_LIST	\
	);	\
})

#define SBLAST2CALL_v_ll(func_ptr, opcode, p1, p2)	\
__extension__	\
({	\
	__asm__ volatile (	\
		"movql	%1,d0\n\t"	\
		"jbsr	%0@\n\t"	\
		"movl	a0,%2\n\t"	\
		"movl	a1,%3"	\
		: \
		: "a"(func_ptr), "g"(opcode), "g"(p1), "g"(p2)	\
		: SBLAST2_CLOBBER_LIST	\
	);	\
})

#define SBLAST2CALL_w_v(func_ptr, opcode)	\
__extension__	\
({	\
	register unsigned short retvalue __asm__("d0");	\
	\
	__asm__ volatile (	\
		"movql	%2,d0\n\t"	\
		"jbsr	%1@"	\
		: "=r"(retvalue)	\
		: "a"(func_ptr), "g"(opcode)	\
		: SBLAST2_CLOBBER_LIST_D	\
	);	\
	retvalue;	\
})

/*--- Functions ---*/

long SB2_Init(void)
{
	if (Getcookie(C_OSBL, (long *)&cookie_osbl) != C_FOUND) {
		return -ENOSYS;
	}

	return 0;
}

char *SB2_GetPath(void)
{
	void *oldstack;
	long result;

	SBLAST2_BEGIN(oldstack);
	result=SBLAST2CALL_la_v(*cookie_osbl, SBLAST2_FUNC_GETPATH);
	SBLAST2_END(oldstack);
	return (char *)result;
}

void SB2_GetModes(sblast2_mode_t **first, sblast2_mode_t **current)
{
	void *oldstack;

	SBLAST2_BEGIN(oldstack);
	SBLAST2CALL_v_ll(*cookie_osbl, SBLAST2_FUNC_GETMODES, *current, *first);
	SBLAST2_END(oldstack);
}

void SB2_SetVideoMode(sblast2_mode_t *videomode)
{
	void *oldstack;

	SBLAST2_BEGIN(oldstack);
	SBLAST2CALL_v_l(*cookie_osbl, SBLAST2_FUNC_SETVIDMODE, videomode);
	SBLAST2_END(oldstack);
}

unsigned short SB2_GetStatus(void)
{
	void *oldstack;
	unsigned short result;

	SBLAST2_BEGIN(oldstack);
	result=SBLAST2CALL_w_v(*cookie_osbl, SBLAST2_FUNC_GETSTATUS);
	SBLAST2_END(oldstack);
	return result;
}

void SB2_SetVirtualScreen(sblast2_mode_t *videomode)
{
	void *oldstack;

	SBLAST2_BEGIN(oldstack);
	SBLAST2CALL_v_l(*cookie_osbl, SBLAST2_FUNC_SETVRTSCREEN, videomode);
	SBLAST2_END(oldstack);
}

sblast2_header_t *SB2_GetFile(void)
{
	void *oldstack;
	long result;

	SBLAST2_BEGIN(oldstack);
	result=SBLAST2CALL_la_v(*cookie_osbl, SBLAST2_FUNC_GETFILE);
	SBLAST2_END(oldstack);
	return (sblast2_header_t *) result;
}
