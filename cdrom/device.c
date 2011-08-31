/*
	/dev/cdrom demo program

	Copyright (C) 2003	Patrice Mandin

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
#include <mint/cdromio.h>

#include "param.h"

/*--- Defines ---*/

#define SCANCODE_ESC	0x01

/*--- Local variables ---*/

static struct cdrom_tochdr	tochdr;

/*--- Functions prototypes ---*/

void DemoLoop(void);

void ListCdromInfo(char *device);
int DisplayDiscInfo(int drive);
void DisplayTocEntries(int drive);

/*--- Functions ---*/

int main(int argc, char **argv)
{
	int p;
	char *device="/dev/cdrom";

	PAR_TestRedirect(argc, argv);
	if (output_to_file) {
		PAR_RedirectStdout();
	}

	p = PAR_Check("-device", argc, argv);
	if ((p>=0) && (p<argc-1)) {
		device = argv[p+1];
	}

	ListCdromInfo(device);

	if (!output_to_file) {
		DemoLoop();
	} else {
		fclose(output_handle);
	}
}

void ListCdromInfo(char *device)
{
	int handle;
	
	fprintf(output_handle, "Device: %s\n", device);

	handle = open(device, 0);
	if (handle<0) {
		fprintf(output_handle, " Can not open device: error 0x%08x\n", handle);
		return;
	}

	if (DisplayDiscInfo(handle)>=0) {
		DisplayTocEntries(handle);
	}

	close(handle);
}

int DisplayDiscInfo(int drive)
{
	struct cdrom_mcn	mcn;

	fprintf(output_handle, "  Ioctl(CDROMREADTOCHDR):\n");
	if (ioctl(drive, CDROMREADTOCHDR, &tochdr)<0) {
		fprintf(output_handle, "   Can not do ioctl()\n");
		return -1;
	}
	fprintf(output_handle, "   Tracks: start=%d, end=%d\n", tochdr.cdth_trk0, tochdr.cdth_trk1);

	fprintf(output_handle, "  Ioctl(CDROMGETMCN):\n");
	if (ioctl(drive, CDROMGETMCN, &mcn)<0) {
		fprintf(output_handle, "   Can not do ioctl()\n");
		return -1;
	}
	fprintf(output_handle, "   Media catalog number: %s\n", mcn.mcn_mcn);

	return 0;
}

void DisplayTocEntries(int drive)
{
	struct cdrom_tocentry tocentry;
	int numtracks, i;

	fprintf(output_handle, "  Ioctl(CDROMREADTOCENTRY):\n");

	numtracks = tochdr.cdth_trk1 - tochdr.cdth_trk0 + 1;
	for (i=0; i<=numtracks; i++) {
		tocentry.cdte_track = tochdr.cdth_trk0+i;
		if (i == numtracks) {
			tocentry.cdte_track = CDROM_LEADOUT;
		}
		tocentry.cdte_format = CDROM_MSF;

		if (ioctl(drive, CDROMREADTOCENTRY, &tocentry)<0) {
			fprintf(output_handle, "   Can not do ioctl()\n");
			continue;
		}

		if (tocentry.cdte_track != CDROM_LEADOUT) {
			fprintf(output_handle, "   Track %d: id=0x%02x, type %s, position %02d:%02d:%02d\n",
				i,
				tocentry.cdte_track,
				(tocentry.cdte_ctrl & CDROM_DATA_TRACK) ? "data" : "audio",
				tocentry.cdte_addr.msf.minute,
				tocentry.cdte_addr.msf.second,
				tocentry.cdte_addr.msf.frame
			);
		} else {
			fprintf(output_handle, "   Total: %02d:%02d:%02d\n",
				tocentry.cdte_addr.msf.minute,
				tocentry.cdte_addr.msf.second,
				tocentry.cdte_addr.msf.frame
			);
		}
	}
}

/*--- Little demo loop ---*/

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
