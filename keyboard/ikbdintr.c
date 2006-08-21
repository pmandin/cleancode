/*
	Atari keyboard manager, using IKBD (interrupt method)

	Copyright (C) 2002	Patrice Mandin

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

#include "ikbd.h"

/*--- Defines ---*/

#define SCANCODE_ESC	0x01

#define KEY_PRESSED		0xff
#define KEY_UNDEFINED	0x80
#define KEY_RELEASED	0x00

/*--- Functions ---*/

int main(int argc, char **argv)
{
	int i, quit;

	/* Availability: all Atari machines, Hades, Medusa */

	memset(Ikbd_keyboard, KEY_UNDEFINED, sizeof(Ikbd_keyboard));
	Ikbd_mousex = Ikbd_mousey = Ikbd_mouseb = Ikbd_joystick = 0;

	/* Install our asm handler */
	Supexec(IkbdInstall);

	printf("Press ESC to quit\n");

	/* Wait till ESC key pressed */
	quit = 0;
	while (!quit) {
		for (i=0; i<128; i++) {
			if (Ikbd_keyboard[i]==KEY_PRESSED) {
				printf("Key with scancode 0x%02x pressed\n", i);
				Ikbd_keyboard[i]=KEY_UNDEFINED;
				if (i==SCANCODE_ESC) {
					quit=1;
				}
			}
			if (Ikbd_keyboard[i]==KEY_RELEASED) {
				printf("Key with scancode 0x%02x released\n", i);
				Ikbd_keyboard[i]=KEY_UNDEFINED;
			}
		}
	}

	/* Uninstall our asm handler */
	Supexec(IkbdUninstall);
}
