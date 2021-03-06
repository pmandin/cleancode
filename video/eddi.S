/*
	Read EdDI version

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

	.globl	_get_EdDI_version

/*--- Vector installer ---*/

_get_EdDI_version:
	movel	sp@(4),a0	/* Value of EdDI cookie */

	movel	d2,sp@-

	/* Call EdDI function #0 */
	clrw	d0
	jsr	(a0)

	movel	sp@+,d2

	rts
