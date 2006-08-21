/*
	Videl Inside 2 definitions

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

#ifndef VIDELINSIDE_H
#define VIDELINSIDE_H

/*--- Types ---*/

typedef struct {
	unsigned short	freqh_mini;
	unsigned short	freqh_maxi;
	unsigned short	freqv_mini;	/* Value*10 */
	unsigned short	freqv_maxi;	/* Value*10 */
} vi_limits_t;

/* VI2 cookie points to this structure */

typedef struct {
	unsigned short	version;
	unsigned short	length;	/* constant = 0x46 */
	unsigned short	dummy[(0x23)-2];	/* 0x23 = length/2 */
	void			(*setmode)(void);	/* Function to set a new mode */
} __attribute__((packed)) vi_cookie_t;

/* VI2.DAT file definition */

typedef struct {
	unsigned char	name[8];	/* "DATFILE\0" */
	unsigned short	version;	/* 0x0103 */

	unsigned short	freqh_mini;	/* Freq limits for a multisync monitor */
	unsigned short	freqh_maxi;	/*  The freqh stored are divided by 100 */
	unsigned short	freqv_mini;	/*  The freqv stored are multiplied by 10 */
	unsigned short	freqv_maxi;

	unsigned short	multisync;	/* Multisync monitor ? */
								/*  bit 0: 0=no, 1=yes */

	unsigned short	num_modes;	/* Number of video modes in this file */

	unsigned short	intfreq1;	/* Videl clocks defined */
	unsigned short	intfreq2;	/*  Freqs stored are divided by 100000 */
	unsigned short	extfreq;
} __attribute__((packed)) vi_header_t;

typedef struct {
	unsigned short	modecode;	/* Video mode definition */
								/*  bit 1: interlaced mode */ 
								/*   0=no, 1=yes */
								/*  bits 3,2: pixel clock predivisor */ 
								/*   0=divide by 4 */
								/*   1=divide by 2 */
								/*   2=divide by 1 */
								/*  bits 6,5,4: bits per pixel */ 
								/*   0=1 bpp */
								/*   1=2 bpp */
								/*   2=4 bpp */
								/*   3=8 bpp */
								/*   4=16 bpp */
								/*  bits 8,7: videl clock to use */
								/*   0=internal 1 */
								/*   1=internal 2 */
								/*   2=external */
								/*  bits 11,10,9: index for 0xffff8266.w value */ 
	unsigned short	freqh_div;	/* Horizontal frequency divisor */
	unsigned short	width;		/* Horizontal resolution */
	unsigned short	dummy2;
	unsigned short	freqv_div;	/* Vertical frequency divisor */
	unsigned short	height;		/* Vertical resolution */
	unsigned short	dummy3[19];
} __attribute__((packed)) vi_mode_t;

/*--- Variables ---*/

extern const vi_limits_t vi_limits[4];

/*--- Functions prototypes ---*/

vi_header_t *VI_LoadModes(void);

#endif /* VIDELINSIDE_H */
