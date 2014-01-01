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

/*--- Coldfire/m68k macros ---*/

#if defined(__mcoldfire__)

# define SAVESTACK_D0D1A0A1	\
	lea	sp@(-16),sp;	\
	moveml	d0-d1/a0-a1,sp@

# define RESTORESTACK_D0D1A0A1	\
	moveml	sp@,d0-d1/a0-a1;	\
	lea	sp@(16),sp

# define SAVESTACK_D0D1A0	\
	lea	sp@(-12),sp;	\
	moveml	d0-d1/a0,sp@

# define RESTORESTACK_D0D1A0	\
	moveml	sp@,d0-d1/a0;	\
	lea	sp@(12),sp

# define MOVEL_REG(reg,src,dst)	\
	movel	src,reg;	\
	movel	reg,dst

# define MOVEB_REG(reg,src,dst)	\
	moveb	src,reg;	\
	moveb	reg,dst

# define BTST_REG(reg,num,addr)	\
	moveql	num,reg;	\
	btst	reg,addr

# define BCLR_REG(reg,num,addr)	\
	moveql	num,reg;	\
	bclr	reg,addr

# define ADDW_REG(reg,src,dst)	\
	movew	dst,reg;	\
	addl	src,reg;	\
	movew	reg,dst
	
#else

# define SAVESTACK_D0D1A0A1	\
	moveml	d0-d1/a0-a1,sp@-

# define RESTORESTACK_D0D1A0A1	\
	moveml	sp@+,d0-d1/a0-a1

# define SAVESTACK_D0D1A0	\
	moveml	d0-d1/a0,sp@-

# define RESTORESTACK_D0D1A0	\
	moveml	sp@+,d0-d1/a0

# define MOVEL_REG(reg,src,dst)	\
	movel	src,dst

# define MOVEB_REG(reg,src,dst)	\
	moveb	src,dst

# define BTST_REG(reg,num,addr)	\
	btst	num,addr

# define BCLR_REG(reg,num,addr)	\
	bclr	num,addr

# define ADDW_REG(reg,src,dst)	\
	addw	src,dst

#endif

/*--- Install our IKBD vector ---*/

_IkbdInstall:
	SAVESTACK_D0D1A0A1

	| Disable all interrupts

	movew	#0x2700,sr

	| Save MFP registers used for ACIA interrupt

	lea	0xfffffa00:w,a0
	btst	#6,a0@(0x09)
	sne	d0
	move.b	d0,ikbd_ierb

	btst	#6,a0@(0x15)
	sne	d0
	move.b	d0,ikbd_imrb

	| Set our MFP routine

	MOVEL_REG(d0,0x118:w,old_ikbd)
	MOVEL_REG(d0,#ikbd,0x118:w)

	moveq	#6,d0
	bset	d0,0xfffffa09:w	| IERB
	bset	d0,0xfffffa15:w	| IMRB

	| Set relative mouse motion mode
	| needed because running a .tos or .ttp program
	| disable the mouse (entering VT52 textmode)

	MOVEB_REG(d0,#8,0xfffffc02.w)

	| Re-enable interrupts

	movew	#0x2300,sr

	clrl	_Ikbd_packets

	RESTORESTACK_D0D1A0A1
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

	MOVEL_REG(a0,old_ikbd,0x118:w)

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
	SAVESTACK_D0D1A0

	| Check if source is IKBD or MIDI
	BTST_REG(d0,#0,0xfffffc00.w)
	beqs	ikbd_oldmidi

	moveb	0xfffffc02:w,d0

	| Joystick packet ?
	
	cmpb	#0xff,d0
	beqs	ikbd_yes_joystick

	| Mouse packet ?

	cmpb	#0xf8,d0
	bmis	ikbd_no_mouse
	cmpb	#0xfc,d0
	bpls	ikbd_no_mouse

	| Mouse packet, byte #1

ikbd_yes_mouse:
#if defined(__mcoldfire__)
	andl	#3,d0
#else
	andw	#3,d0
#endif
	movew	d0,_Ikbd_mouseb

	MOVEL_REG(d0,#ikbd_mousex,0x118:w)
	bras	ikbd_endit_stack

	| Joystick packet, byte #1

ikbd_yes_joystick:
	MOVEL_REG(d0,#ikbd_joystick,0x118:w)
	bras	ikbd_endit_stack

	| Keyboard press/release

ikbd_no_mouse:
	moveb	d0,d1
	lea	_Ikbd_keyboard,a0
#if defined(__mcoldfire__)
	andl	#0x7f,d1
	btst	#7,d0
	spl	d0
	moveb	d0,a0@(0,d1:l)
#else
	andw	#0x7f,d1
	tas	d0
	spl	a0@(0,d1:w)
#endif

	| End of interrupt

ikbd_endit_stack:
	BCLR_REG(d0,#6,0xfffffa11:w)

	RESTORESTACK_D0D1A0
	rte

	| Call old MIDI interrupt

ikbd_oldmidi:
	RESTORESTACK_D0D1A0

	movel	old_ikbd,sp@-
	rts

	| Mouse packet, byte #2

ikbd_mousex:
	SAVESTACK_D0D1A0

	| Check if source is IKBD or MIDI
	BTST_REG(d0,#0,0xfffffc00.w)
	beqs	ikbd_oldmidi

	moveb	0xfffffc02:w,d0
	extw	d0
	ADDW_REG(d1,d0,_Ikbd_mousex)

	MOVEL_REG(d0,#ikbd_mousey,0x118:w)
	bras	ikbd_endit_stack

	| Mouse packet, byte #3

ikbd_mousey:
	SAVESTACK_D0D1A0

	| Check if source is IKBD or MIDI
	BTST_REG(d0,#0,0xfffffc00.w)
	beqs	ikbd_oldmidi

	moveb	0xfffffc02:w,d0
	extw	d0
	ADDW_REG(d1,d0,_Ikbd_mousey)

	MOVEL_REG(d0,#ikbd,0x118:w)
	bra	ikbd_endit_stack

	| Joystick packet, byte #2

ikbd_joystick:
	SAVESTACK_D0D1A0

	| Check if source is IKBD or MIDI
	BTST_REG(d0,#0,0xfffffc00.w)
	beq	ikbd_oldmidi

	MOVEB_REG(d0,0xfffffc02:w,_Ikbd_joystick+1)

	MOVEL_REG(d0,#ikbd,0x118:w)
	bra	ikbd_endit_stack

	.bss
	
	.even
	.comm	_Ikbd_keyboard,128
	.comm	_Ikbd_mousex,2
	.comm	_Ikbd_mousey,2
	.comm	_Ikbd_mouseb,2
	.comm	_Ikbd_joystick,2
