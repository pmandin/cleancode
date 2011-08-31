/*
	Metados CD-ROM demo program

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
#include <mint/cdromio.h>
#include <mint/metados.h>

#include "param.h"

/*--- Defines ---*/

#define SCANCODE_ESC	0x01
#define SCANCODE_SPACE	0x39

/*--- Local variables ---*/

static struct cdrom_tochdr	tochdr;

/*--- Functions prototypes ---*/

void ListMetadosDrives(metainit_t *metainit);
void DemoLoop();

void DisplayDiscInfoOld(int drive);
void DisplayTocEntriesOld(int drive);
int DisplayDiscInfo(int drive);
void DisplayTocEntries(int drive);

/*--- Functions ---*/

int main(int argc, char **argv)
{
	metainit_t metainit={0,0,0,0};

	PAR_TestRedirect(argc, argv);
	if (output_to_file) {
		PAR_RedirectStdout();
	}

	Metainit(&metainit);
	if (metainit.version == NULL) {
		fprintf(output_handle, "Metados not installed\n");
	} else {
		fprintf(output_handle, "Metados driver installed:\n %s\n", metainit.version);
		if (metainit.info != NULL) {
			fprintf(output_handle, " Version: 0x%02x.%02x\n", ((metainit.info->version)>>8) & 0xff, (metainit.info->version) & 0xff);
		}
		if (metainit.drives_map) {
			ListMetadosDrives(&metainit);
		} else {
			fprintf(output_handle, "List of Metados drives empty\n");
		}
	}

	if (!output_to_file) {
		DemoLoop();
	} else {
		fclose(output_handle);
	}
}

void ListMetadosDrives(metainit_t *metainit)
{
	metaopen_t metaopen;
	int handle, i;
	unsigned char gemdos_dev[32];

	fprintf(output_handle, "List of Metados drives: 0x%08x\n", metainit->drives_map);

	memset(gemdos_dev, 0, sizeof(gemdos_dev));
	for (i=0; i<32; i++) {
		int metados_dev = metainit->info->log2phys[i];
		if ((metados_dev>='A') && (metados_dev<='Z')) {
			gemdos_dev[metados_dev - 'A'] = i + 'A';
		}
	}

	for (i='A'; i<='Z'; i++) {
		if (metainit->drives_map & (1<<(i-'A'))) {
			fprintf(output_handle, " Metados device %c (Gemdos device %c): ",
				i, gemdos_dev[i-'A'] ? gemdos_dev[i-'A'] : '-');
			handle = Metaopen(i, &metaopen); 
			if (handle==0) {
				fprintf(output_handle, "%s\n", metaopen.name);

				DisplayDiscInfoOld(i);
				DisplayTocEntriesOld(i);

				if (DisplayDiscInfo(i)>=0) {
					DisplayTocEntries(i);
				}

				Metaclose(i);
			} else {
				fprintf(output_handle, "Can not open drive\n");
			}
		}
	}
}

void DisplayDiscInfoOld(int drive)
{
	long errorcode;
	metadiscinfo_t discinfo;

	fprintf(output_handle, "  MetaDiscInfo():\n");

	errorcode = Metadiscinfo(drive, &discinfo);
	if (errorcode<0) {
		fprintf(output_handle, "   Can not read disc info\n");
		return;
	}

	fprintf(output_handle, "   Disc type: %s\n", (discinfo.disctype==0) ? "audio" : "data");
	fprintf(output_handle, "   Tracks: first=%d, last=%d, current=%d\n",
		discinfo.first, discinfo.last, discinfo.current
	);
	fprintf(output_handle, "   Relative position: 0x%02x, %02x:%02x:%02x\n",
		discinfo.relative.reserved, discinfo.relative.minute,
		discinfo.relative.second, discinfo.relative.frame
	);
	fprintf(output_handle, "   Absolute position: 0x%02x, %02x:%02x:%02x\n",
		discinfo.absolute.reserved, discinfo.absolute.minute,
		discinfo.absolute.second, discinfo.absolute.frame
	);
	fprintf(output_handle, "   End position: 0x%02x, %02x:%02x:%02x\n",
		discinfo.end.reserved, discinfo.end.minute,
		discinfo.end.second, discinfo.end.frame
	);
	fprintf(output_handle, "   Index: 0x%02x\n", discinfo.index);
}

void DisplayTocEntriesOld(int drive)
{
	long errorcode;
	unsigned char *buffer;
	metatocentry_t *tocentry;
	int i;

	fprintf(output_handle, "  MetaGetToc():\n");

	buffer = (unsigned char *)Malloc(100*sizeof(metatocentry_t));
	if (buffer == NULL) {
		fprintf(output_handle, "   Can not allocate memory for TOC\n");
		return;
	}

	errorcode = Metagettoc(drive, 0, buffer);
	if (errorcode<0) {
		fprintf(output_handle, "   Can not read TOC\n");
		Mfree(buffer);
		return;
	}

	tocentry = (metatocentry_t *) (buffer);
	i=0;
	for (;;) {
		if ((tocentry[i].track==0) && (tocentry[i].minute==0) && 
			(tocentry[i].second==0) && (tocentry[i].frame==0)
			) {
			break;
		}

		if (tocentry[i].track != CDROM_LEADOUT_CDAR) {
			fprintf(output_handle,
				"   Track %d, id 0x%02x, from %02x:%02x:%02x\n", i, tocentry[i].track,
				tocentry[i].minute,
				tocentry[i].second,
				tocentry[i].frame
			);
		} else {
			fprintf(output_handle,
				"   Total: %02x:%02x:%02x\n",
				tocentry[i].minute,
				tocentry[i].second,
				tocentry[i].frame
			);
		}

		i++;
	}

	Mfree(buffer);
}

int DisplayDiscInfo(int drive)
{
	struct cdrom_mcn	mcn;

	fprintf(output_handle, "  MetaIoctl(CDROMREADTOCHDR):\n");
	if (Metaioctl(drive, METADOS_IOCTL_MAGIC, CDROMREADTOCHDR, &tochdr)<0) {
		fprintf(output_handle, "   Can not do ioctl()\n");
		return -1;
	}
	fprintf(output_handle, "   Tracks: start=%d, end=%d\n", tochdr.cdth_trk0, tochdr.cdth_trk1);

	fprintf(output_handle, "  MetaIoctl(CDROMGETMCN):\n");
	if (Metaioctl(drive, METADOS_IOCTL_MAGIC, CDROMGETMCN, &mcn)<0) {
		fprintf(output_handle, "   Can not do ioctl()\n");
		return 0;
	}
	fprintf(output_handle, "   Media catalog number: %s\n", mcn.mcn_mcn);

	return 0;
}

void DisplayTocEntries(int drive)
{
	struct cdrom_tocentry tocentry;
	int numtracks, i;

	fprintf(output_handle, "  MetaIoctl(CDROMREADTOCENTRY):\n");

	numtracks = tochdr.cdth_trk1 - tochdr.cdth_trk0 + 1;
	for (i=0; i<=numtracks; i++) {
		tocentry.cdte_track = tochdr.cdth_trk0+i;
		if (i == numtracks) {
			tocentry.cdte_track = CDROM_LEADOUT;
		}
		tocentry.cdte_format = CDROM_MSF;

		if (Metaioctl(drive, METADOS_IOCTL_MAGIC, CDROMREADTOCENTRY, &tocentry)<0) {
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

void DemoLoop()
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
