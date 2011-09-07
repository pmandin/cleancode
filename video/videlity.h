/*
	Videlity definitions

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

#ifndef VIDELITY_H
#define VIDELITY_H

/*--- Types ---*/

typedef struct {
	short modecode;
	unsigned short bpp;
	unsigned short width;
	unsigned short height;
} vdly_modeinfo_t;

typedef struct {
	void (*readmodes)(vdly_modeinfo_t *vdly_modeinfo);
	void *video_registers;
	void (*undef2)(void);
	long (*addmode)(unsigned short modecode, void *vid_registers);
	long (*getmode)(unsigned short modecode, void *vid_registers);
	void (*setmode)(unsigned short modecode, void *vid_registers);
	unsigned char *name;
	void (*undef7)(void);
} cookie_vdly_t;

/*--- Functions prototypes ---*/

long VDLY_Init(void);
void VDLY_ReadModes(vdly_modeinfo_t *vdly_modeinfo);
void VDLY_AddMode(unsigned short modecode, void *vid_registers);
long VDLY_GetMode(unsigned short modecode,void *vid_registers);
void VDLY_SetMode(void *vid_registers);

#endif /* VIDELITY_H */
