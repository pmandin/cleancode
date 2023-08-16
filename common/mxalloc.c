/*
	Memory allocation

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

#include <string.h>

#include <mint/osbind.h>
#include <mint/sysvars.h>

/*--- Variables ---*/

static int atari_mxalloc_avail=-1;

/*--- Functions prototypes ---*/

static unsigned short ReadTosVersion(void);

/*--- Functions ---*/

static unsigned short ReadTosVersion(void)
{
	void *oldstack;
	OSHEADER *os_hdr;
	unsigned short r = 0x0100;

	oldstack = (void *) Super(NULL);

	os_hdr = (OSHEADER *) *_sysbase;
	r = os_hdr->os_version;

	Super(oldstack);

	return(r);
}

void *Atari_SysMalloc(unsigned long size, unsigned short alloc_type)
{
	/* Test if Mxalloc() available */
	if (atari_mxalloc_avail<0) {
		atari_mxalloc_avail = ((Sversion()>=0x1900) && (ReadTosVersion()>=0x300));
	}

	if (atari_mxalloc_avail) {
		return (void *) Mxalloc(size, alloc_type);
	} else {
		return (void *) Malloc(size);
	}
}
