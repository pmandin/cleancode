/*
	NOVA video cards definitions

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

#ifndef NOVA_H
#define NOVA_H

/*--- Types ---*/

typedef struct {
	unsigned char	name[33];	/* Video mode name */
	unsigned char	dummy1;

	unsigned short	mode;		/* Video mode type */
								/*  0=4 bpp */
								/*  1=1 bpp */
								/*  2=8 bpp */
								/*  3=15 bpp (little endian) */
								/*  4=16 bpp (little endian) */
								/*  5=24 bpp (BGR) */
								/*  6=32 bpp (RGBA) */
	unsigned short	pitch;		/* bpp<8: words/plane /line */
								/*  bpp>=8: bytes /line */
	unsigned short	planes;		/* Bits per pixel */
	unsigned short	colors;		/* Number of colours */
	unsigned short	hc_mode;	/* Hardcopy mode */
								/*  0=1 pixel screen -> 1x1 printer screen */
								/*  1=1 pixel screen -> 2x2 printer screen */
								/*  2=1 pixel screen -> 4x4 printer screen */
	unsigned short	max_x;		/* Max x,y coordinates, values-1 */
	unsigned short	max_y;
	unsigned short	real_x;		/* Real max x,y coordinates, values-1 */
	unsigned short	real_y;

	unsigned short	freq;		/* Pixel clock */
	unsigned char	freq2;		/* Another pixel clock */
	unsigned char	low_res;	/* Half of pixel clock */
	unsigned char	r_3c2;
	unsigned char	r_3d4[25];
	unsigned char	extended[3];
	unsigned char	dummy2;
} nova_resolution_t;

/* cookie IMNE points to this */
typedef struct {
	/*	Version number (in ASCII)	*/
	unsigned char	id[4];

	/*	Pointer to a routine to change resolution	*/
	void			(*p_chres)(nova_resolution_t *nova_res);	

	/*	Video mode type
		0=4 bpp, 1=1 bpp, 2=8 bpp, 3= 15 bpp (little endian)
		4=16 bpp (little endian), 5=24 bpp (BGR), 6=32 bpp (RGBA)	*/
	unsigned short	mode;	

	/*	Line length
		bpp<8: bytes per plane, per line
		bpp>=8: bytes per line	*/
	unsigned short	pitch;

	/*	Bits per pixel	*/
	unsigned short	planes;	

	/*	Number of colours	*/
	unsigned short	colours;	

	/*	Hardcopy mode
		0=1 pixel screen -> 1x1 printer screen
		1=1 pixel screen -> 2x2 printer screen
		2=1 pixel screen -> 4x4 printer screen	*/
	unsigned short	hc;

	unsigned short	max_x, max_y;	/* Maximal virtual screen size (values-1) */
	unsigned short	rmn_x, rmx_x;	/* Min and max of real screen size (values-1) */
	unsigned short	rmn_y, rmx_y;
	void 			*scr_base;	/* Pointer to current screen address */
	void			*reg_base;	/* Pointer to video card I/O registers */
	unsigned char	name[36];	/* Actual video mode name */
	unsigned long	bl_count;	/* Some values for blanking */
	unsigned long	bl_max;
	unsigned short	bl_on;
	unsigned short	card;		/* Video card type */
	unsigned char	speed_tab[12];	/* Mouse accelerator values */
	void			(*chng_vrt)(unsigned short x, unsigned short y);
	unsigned short	boot_drv;	/* Boot drive for resolution file */
	void			(*set_vec)(void);
	void			*main_work;
	void			(*handler)(unsigned short x, unsigned long y);
	void			(*init_col)(void);
	unsigned short	scrn_cnt;	/* Number of screens in video RAM */
	unsigned long	scrn_sze;	/* Size of a screen in video RAM */
	void			*base;		/* Address of screen #0 in video RAM */
	void			(*p_setscr)(void *adr);	/* Pointer to a routine to change physical screen address */
	unsigned short	v_top, v_bottom;		/* Some values for virtual screen */
	unsigned short	v_left, v_right;
	unsigned short	max_light;
	void			(*chng_pos)(nova_resolution_t *nova_res, unsigned short direction, unsigned short offset);
	void			(*chng_edg)(unsigned short edge);
	unsigned short	hw_flags;
	unsigned short	calib_on;	/* Some values for calibration */
	unsigned short	calib_col[6][3];
	void			(*calib_in)(void);
	unsigned char	*cal_tab;
	unsigned char	*recal_tab;
} nova_icb_t;

/* cookie NOVA points to this */
typedef struct {
	unsigned char	version[4];	/* Version number */
	unsigned char	resolution;	/* Resolution number */
	unsigned char	blnk_time;	/* Time before blanking */
	unsigned char	ms_speed;	/* Mouse speed */
	unsigned char	old_res;

	/* Pointer to routine to change resolution */
	void			(*p_chres)(nova_resolution_t *nova_res, unsigned long offset);
	
	unsigned short	mode;		/* Video mode type: */
								/*  0=4 bpp */
								/*  1=1 bpp */
								/*  2=8 bpp */
								/*  3=15 bpp (little endian) */
								/*  4=16 bpp (little endian) */
								/*  5=24 bpp (BGR) */
								/*  6=32 bpp (RGBA) */
	unsigned short	pitch;		/* bpp<8: bytes per plane, per line */
								/*  bpp>=8: bytes per line */
	unsigned short	planes;		/* Bits per pixel */
	unsigned short	colours;	/* Number of colours, unused */
	unsigned short	hc;			/* Hardcopy mode */
								/*  0=1 pixel screen -> 1x1 printer screen */
								/*  1=1 pixel screen -> 2x2 printer screen */
								/*  2=1 pixel screen -> 4x4 printer screen */
	unsigned short	max_x, max_y;		/* Resolution, values-1 */
	unsigned short	rmn_x, rmx_x;
	unsigned short	rmn_y, rmx_y;
	unsigned short	v_top, v_bottom;
	unsigned short	v_left, v_right;

	/* Pointer to routine to set colours */
	void			(*p_setcol)(unsigned short index, unsigned char *colors);	

	void			(*chng_vrt)(unsigned short x, unsigned short y);
	void			(*inst_xbios)(unsigned short on);
	void			(*pic_on)(unsigned short on);
	void			(*chng_pos)(nova_resolution_t *nova_res, unsigned short direction, unsigned short offset);
	void			(*p_setscr)(void *adr);	/* Pointer to routine to change screen address */
	void			*base;		/* Address of screen #0 in video RAM */
	void			*scr_base;	/* Adress of video RAM */
	unsigned short	scrn_cnt;	/* Number of possible screens in video RAM */
	unsigned long	scrn_sze;	/* Size of a screen */
	void			*reg_base;	/* Video card I/O registers base */
	void			(*p_vsync)(void);	/* Pointer to routine to vsync */
	unsigned char	name[36];	/* Video mode name */
	unsigned long	mem_size;	/* Global size of video memory */
} nova_xcb_t;

/*--- Functions prototypes ---*/

nova_resolution_t *nova_LoadModes(int *num_modes);

#endif /* NOVA_H */
