/*
	Virtual screen cookie definitions

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

#ifndef VIRTUALSCREEN_H
#define VIRTUALSCREEN_H

/*--- Types ---*/

/* VSCR cookie points to this structure */

typedef struct {
	unsigned char	id[4];		/* "VSCR" */
	unsigned char	client[4];	/* ID for program providing virtual screen */
	unsigned short	version;
	unsigned short	posx,posy;	/* Position in virtual screen */
	unsigned short	width,height;	/* Size of window (physical screen) */
} vscr_cookie_t;

#endif /* VIRTUALSCREEN_H */
