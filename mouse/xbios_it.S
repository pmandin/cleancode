/*
	XBIOS mouse interrupt

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

	.text

	.globl	_XbiosInstall
	.globl	_XbiosMouse

	.globl	_Xbios_mouseb
	.globl	_Xbios_mousex
	.globl	_Xbios_mousey

/*--- Vector installer ---*/

_XbiosInstall:
	movel	sp@(4),a0

	/* Stop interrupts */

	movew	#0x2700,sr

	/* Save old mouse vector, set our routine */

	movel	sp@(8),d0
	beqs	no_new_mouse_vector
	movel	a0@(16),d1
	movel	d1,oldmousevector
	movel	d0,a0@(16)
no_new_mouse_vector:

	/* Restart interrupts */

	movew	#0x2300,sr

	rts

/*--- Our mouse vector ---*/

	.text
	.even
	.ascii "XBRA"
	.ascii "LSDL"
	.comm	oldmousevector,4*1
_XbiosMouse:
#if defined(__mcoldfire__)
	lea	sp@(-8),sp
	moveml	d0-d1,sp@
#else
	movel	d0,sp@-
#endif

	/* Mouse buttons */
	moveb	(a0),d0
#if defined(__mcoldfire__)
	andl	#3,d0
#else
	andw	#3,d0
#endif
	movew	d0,_Xbios_mouseb

	/* X movement */
	moveb	a0@(1),d0
	extw	d0
#if defined(__mcoldfire__)
	movew	_Xbios_mousex,d1
	addl	d1,d0
	movew	d0,_Xbios_mousex
#else
	addw	d0,_Xbios_mousex
#endif

	/* Y movement */
	moveb	a0@(2),d0
	extw	d0
#if defined(__mcoldfire__)
	movew	_Xbios_mousey,d1
	addl	d1,d0
	movew	d0,_Xbios_mousey
#else
	addw	d0,_Xbios_mousey
#endif

#if defined(__mcoldfire__)
	moveml	sp@,d0-d1
	lea	sp@(8),sp
#else
	movel	sp@+,d0
#endif

	/* Jump through old vector */
	movel	oldmousevector,sp@-
	rts

	.data
	.even
	.comm	_Xbios_mousex,2*1
	.even
	.comm	_Xbios_mousey,2*1
	.even
	.comm	_Xbios_mouseb,2*1
