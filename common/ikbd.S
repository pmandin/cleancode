/*
	IKBD 6301 interrupt routine

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

	.globl	_IkbdInstall
	.globl	_IkbdUninstall

	.globl	_Ikbd_keyboard
	.globl	_Ikbd_mouseb
	.globl	_Ikbd_mousex
	.globl	_Ikbd_mousey
	.globl	_Ikbd_joystick

/*--- Install our IKBD vector ---*/

_IkbdInstall:
#if defined(__mcoldfire__)
	lea	sp@(-16),sp
	moveml	d0-d1/a0-a1,sp@
#else
	moveml	d0-d1/a0-a1,sp@-
#endif

	| Disable all interrupts

	movew	#0x2700,sr

	| Save MFP registers used for ACIA interrupt

	lea	0xfffffa00:w,a0
	btst	#6,a0@(0x09)
#if defined(__mcoldfire__)
	sne	d0
	move.b	d0,ikbd_ierb
#else
	sne	ikbd_ierb
#endif
	btst	#6,a0@(0x15)
#if defined(__mcoldfire__)
	sne	d0
	move.b	d0,ikbd_imrb
#else
	sne	ikbd_imrb
#endif

	| Set our MFP routine

#if defined(__mcoldfire__)
	movel	0x118:w,d0
	movel	d0,old_ikbd
	lea	ikbd,a0
	movel	a0,0x118:w
	moveq	#6,d0
	bset	d0,0xfffffa09:w	| IERB
	bset	d0,0xfffffa15:w	| IMRB
#else
	movel	0x118:w,old_ikbd
	movel	#ikbd,0x118:w
	bset	#6,0xfffffa09:w	| IERB
	bset	#6,0xfffffa15:w	| IMRB
#endif

	| Set relative mouse motion mode
	| needed because running a .tos or .ttp program
	| disable the mouse (entering VT52 textmode)

#if defined(__mcoldfire__)
	moveq	#8,d0
	moveb	d0,0xfffffc02.w
#else
	moveb	#8,0xfffffc02.w
#endif

	| Re-enable interrupts

	movew	#0x2300,sr

#if defined(__mcoldfire__)
	moveml	sp@,d0-d1/a0-a1
	lea	sp@(16),sp
#else
	moveml	sp@+,d0-d1/a0-a1
#endif
	rts

/*--- Uninstall our IKBD vector ---*/

_IkbdUninstall:
	movel	a0,sp@-

	| Disable interrupts

	movew	#0x2700,sr

	| Restore previous MFP registers

	lea	0xfffffa00:w,a0

	bclr	#6,a0@(0x09)
	tstb	ikbd_ierb
	beqs	ikbd_restoreierb
	bset	#6,a0@(0x09)
ikbd_restoreierb:

	bclr	#6,a0@(0x15)
	tstb	ikbd_imrb
	beqs	ikbd_restoreimrb
	bset	#6,a0@(0x15)
ikbd_restoreimrb:

#if defined(__mcoldfire__)
	movel	old_ikbd,a0
	movel	a0,0x118:w
#else
	movel	old_ikbd,0x118:w
#endif

	| Clear keyboard buffer

	lea	0xfffffc00:w,a0
ikbd_videbuffer:
	btst	#0,a0@
	beqs	ikbd_finbuffer
	tstb	a0@(0x02)
	bras	ikbd_videbuffer
ikbd_finbuffer:

	| Re-enable interrupts

	movew	#0x2300,sr

	movel	sp@+,a0
	rts

	.bss

	.even
	.comm	ikbd_ierb,1
	.comm	ikbd_imrb,1

/*--- Our custom IKBD vector ---*/

	.text
	.even
	.ascii	"XBRA"
	.ascii	"LSDL"
	.comm	old_ikbd,4
ikbd:
	| test if byte coming from IKBD or MIDI

	btst	#0,0xfffffc00.w
	beqs	ikbd_endit

	moveml	d0-d1/a0,sp@-

	moveb	0xfffffc02:w,d0

	| Joystick packet ?
	
	cmpb	#0xff,d0
	beqs	ikbd_yes_joystick

	| Mouse packet ?

	cmpb	#0xf8,d0
	bmis	ikbd_no_mouse
	cmpb	#0xfc,d0
	bpls	ikbd_no_mouse

ikbd_yes_mouse:
	andw	#3,d0
	movew	d0,_Ikbd_mouseb

	movel	#ikbd_mousex,0x118:w
	bras	ikbd_endit_stack

ikbd_yes_joystick:
	movel	#ikbd_joystick,0x118:w
	bras	ikbd_endit_stack

	| Keyboard press/release

ikbd_no_mouse:
	moveb	d0,d1
	lea	_Ikbd_keyboard,a0
	andl	#0x7f,d1
	tas	d0
	spl	a0@(0,d1:w)

ikbd_endit_stack:
	moveml	sp@+,d0-d1/a0
ikbd_endit:
	bclr	#6,0xfffffa11:w
	rte

ikbd_mousex:
	| test if byte coming from IKBD or MIDI

	btst	#0,0xfffffc00.w
	beqs	ikbd_endit

	movew	d0,sp@-

	moveb	0xfffffc02:w,d0
	extw	d0
	addw	d0,_Ikbd_mousex

	movew	sp@+,d0

	movel	#ikbd_mousey,0x118:w
	bras	ikbd_endit

ikbd_mousey:
	| test if byte coming from IKBD or MIDI

	btst	#0,0xfffffc00.w
	beqs	ikbd_endit

	movew	d0,sp@-

	moveb	0xfffffc02:w,d0
	extw	d0
	addw	d0,_Ikbd_mousey

	movew	sp@+,d0

	movel	#ikbd,0x118:w
	bras	ikbd_endit

ikbd_joystick:
	| test if byte coming from IKBD or MIDI

	btst	#0,0xfffffc00.w
	beqs	ikbd_endit

	moveb	0xfffffc02:w,_Ikbd_joystick+1

	movel	#ikbd,0x118:w
	bras	ikbd_endit

	.bss
	
	.even
	.comm	_Ikbd_keyboard,128
	.comm	_Ikbd_mousex,2
	.comm	_Ikbd_mousey,2
	.comm	_Ikbd_mouseb,2
	.comm	_Ikbd_joystick,2
