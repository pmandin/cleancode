/*
	ScreenBlaster 3 definitions

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

#ifndef SBLAST3_H
#define SBLAST3_H

/*--- Defines ---*/

#define C_SCPN 0x5343504EL

#define SCPN_DEV_1BPP	0
#define SCPN_DEV_2BPP	1
#define SCPN_DEV_4BPP	2
#define SCPN_DEV_8BPP	3
#define SCPN_DEV_16BPP	4

extern const int scpn_planes_device[];

/*--- Types ---*/

typedef struct {
	unsigned short	virtual_width;	/* Virtual screen width */
	unsigned short	virtual_height;	/* Virtual screen height */
	unsigned short	visible_width;	/* Visible width */
	unsigned short	visible_height;	/* Visible height */
	unsigned short	h_pos;	/* Horizontal position in virtual screen */
	unsigned short	v_pos;	/* Vertical position in virtual screen */
	unsigned short	dummy;
	unsigned long	size;	/* Size of screen in bytes */
	unsigned short	device;	/* Device number to find planes = getRez() */
							/* = Index in scpn_planes_device[] */
} scpn_screeninfo_t;

typedef struct {
	unsigned long	magic;	/* just a BRA assembler jump */
	unsigned short	version;
	void			*dummy1;
	unsigned short	ptsout0_1;
	unsigned short	ptsout0_2;
	unsigned short	dummy3;
	unsigned char	date[8];	/* Date of program build */
	unsigned char	asm_string[30];	/* 10 times the 'ASM' string */
	unsigned short	dummy4;
	scpn_screeninfo_t	*screen_info;
	unsigned short	dummy6;
} scpn_cookie_t;

#endif /* SBLAST3_H */
