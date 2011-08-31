/*
	RSVF definitions

	Copyright (C) 2005	Patrice Mandin

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

#ifndef _RSVF_H
#define _RSVF_H

/*--- Defines ---*/

#define RSVF_INTERFACE	(1<<7)
#define RSVF_GEMDOS		(1<<6)
#define RSVF_BIOS		(1<<5)
#define RSVF_MAGIC3		(1<<0)

/*--- Types ---*/

typedef struct {
	union {
		unsigned char *name;	/* Pointer to device name */
		void *next;				/* Pointer to next list of objects */
	} pointer;
	unsigned char type;			/* Type of object */
	unsigned char reserved1;
	unsigned char bios_number;	/* Device number for BIOS functions */
	unsigned char reserved2;
} rsvf_object_t;

#endif /* _RSVF_H */
