/*
	HSModem RSVF list of ports

	Copyright (C) 2005	Patrice Mandin

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <string.h>

#include <mint/osbind.h>
#include <mint/cookie.h>

#include "param.h"
#include "rsvf.h"

/*--- Defines ---*/

#define SCANCODE_ESC	0x01

/*--- Types ---*/

/*--- Variables ---*/

long cookie_rsvf;

/*--- Functions prototypes ---*/

void DemoLoop(void);
void RsvfList(rsvf_object_t *list);

/*--- Functions ---*/

int main(int argc, char **argv)
{
	PAR_TestRedirect(argc, argv);
	if (output_to_file) {
		PAR_RedirectStdout();
	}

	/* Read cookie */

	fprintf(output_handle, "RSVF cookie: ");
	if (Getcookie(C_RSVF, &cookie_rsvf) != C_FOUND) {
		fprintf(output_handle, "non present\n");
		cookie_rsvf = 0;
	} else {
		fprintf(output_handle, "present: 0x%08x\n", cookie_rsvf);
	}

	if (cookie_rsvf) {
		RsvfList((rsvf_object_t *)cookie_rsvf);
	}

	if (!output_to_file) {
		DemoLoop();
	} else {
		fclose(output_handle);
	}

	return 0;
}

void DemoLoop(void)
{
	unsigned long key_pressed;
	unsigned char scancode;

	printf("Press ESC to quit\n");

	key_pressed = scancode = 0;
	while (scancode != SCANCODE_ESC) {

		/* Read key pressed, if needed */
		if (Cconis()) {
			key_pressed = Cnecin();
			scancode = (key_pressed >>16) & 0xff;

			/* Wait key release */
			while (Cconis()!=0) { }
		}
	}
}

void RsvfList(rsvf_object_t *list)
{
	for(;;) {
		if ((list->type & RSVF_INTERFACE)==0) {
			if (list->pointer.next == NULL) {
				break;
			} else {
				list = (rsvf_object_t *)list->pointer.next;
				continue;
			}
		}

		/* We have a serial interface */
		fprintf(output_handle, "Device name: U:\\DEV\\%s\n", list->pointer.name);
		fprintf(output_handle, " BIOS device number: %d\n", list->bios_number);
		if (list->type & RSVF_GEMDOS) {
			fprintf(output_handle, " Device has GEMDOS functions\n");
		}
		if (list->type & RSVF_BIOS) {
			fprintf(output_handle, " Device has BIOS functions\n");
		}
		if (list->type & RSVF_MAGIC3) {
			fprintf(output_handle, " Device has Magic3 Dcntl functions\n");
		}

		list++;
	}
}
