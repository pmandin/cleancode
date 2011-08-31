/*
	ScreenBlaster 2 definitions

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

#ifndef SBLAST2_H
#define SBLAST2_H

/*--- Includes ---*/

/*--- Defines ---*/

/*--- Types ---*/

typedef struct {
	unsigned char	id[24];	/* "OverScan Screenblaster" */
	unsigned short	dummy1;
	unsigned short	mon_count;	/* Number of monitor's names */
	unsigned short	dummy2[3];
	unsigned short	mode_count;	/* Number of video modes */
	unsigned short	dummy3[10];
	unsigned short	clock;		/* External clock number */
	unsigned short	dummy4[5];
	unsigned long	mon_info[4];	/* Info for each type of monitor */
} sblast2_header_t;

typedef struct {
	unsigned char	name[32];
} sblast2_name_t;

typedef struct {
	unsigned short	num_regs;	/* Number of video registers */
	/* This structure is followed in memory by an array of sblast2_reg_t */
} sblast2_regheader_t;

typedef struct {
	unsigned long	address;	/* Address of video register */
	unsigned short	value;		/* Value of video register */
} sblast2_reg_t;

typedef struct {
	union {
		unsigned long	offset;		/* Offset to next video mode, replaced by
										pointer to next video mode in memory */
		void			*pointer;
	} next;
	unsigned short	dummy1;
	union {
		unsigned long	offset;	/* Offset to video mode name, replaced by
										pointer to video mode name in memory */
		sblast2_name_t	*pointer;
	} name;
	unsigned short	virt_width;		/* Virtual width - 1 */
	unsigned short	virt_height;	/* Virtual height - 1 */
	unsigned short	phys_width;		/* Physical width - 1 */
	unsigned short	phys_height;	/* Physical height - 1 */
	unsigned short	dummy2[2];
	unsigned short	bpp;			/* Bits per pixel */
	unsigned long	colours;		/* Number of colours */
	unsigned short	pitch;			/* Length of a line in bytes */
	unsigned short	dummy3;
	unsigned short	hfreq;			/* Horizontal frequency * 10 */
	unsigned short	vfreq;			/* Vertical frequency * 10 */
	unsigned short	dummy4[2];
	unsigned long	mon_suitable;	/* Value to check if monitor suitable */
	unsigned short	dummy5[14];
	union {
		unsigned long	offset;	/* Offset to video registers list, replaced
											by pointer to it in memory */
		sblast2_regheader_t	*pointer;
	} regs;
} sblast2_mode_t;

/*--- Functions prototypes ---*/

/* Return 0 if ScreenBlaster 2 present */
/*  or a negative value otherwise */
long SB2_Init(void);	
						
char *SB2_GetPath(void);
void SB2_GetModes(sblast2_mode_t **first, sblast2_mode_t **current);
void SB2_SetVideoMode(sblast2_mode_t *mode);
unsigned short SB2_GetStatus(void);
void SB2_SetVirtualScreen(sblast2_mode_t *mode);
sblast2_header_t *SB2_GetFile(void);

#endif /* SBLAST2_H */
