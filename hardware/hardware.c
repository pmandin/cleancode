/*
	Hardware registers test

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
#include <mint/cookie.h>

#include "param.h"
#include "hw_cpu.h"
#include "hw_regs.h"
#include "mmu.h"

/*--- Defines ---*/

#define SCANCODE_ESC	0x01

#define HW_INIT(address,name) \
	{(void *)address, name, 0}

/*--- Types ---*/

typedef struct {
	void *address;
	unsigned char *name;
	int present;
} hardware_t;

/*--- Variables ---*/

long cookie_cpu, cookie_fpu;

hardware_t hardware[]={
	HW_INIT(0xfff00000, "Falcon: IDE controller"),
	HW_INIT(0xffff8000, "MMU configuration"),
	HW_INIT(0xffff8007, "Falcon: Bus control"),
	HW_INIT(0xffff8200, "Video chip"),
	HW_INIT(0xffff8262, "TT: Shifter"),
	HW_INIT(0xffff827e, "Stacy: Shifter"),
	HW_INIT(0xffff8280, "Falcon: Videl"),
	HW_INIT(0xffff8604, "Floppy disk/Hard disk DMA"),
	HW_INIT(0xffff8700, "TT: SCSI DMA"),
	HW_INIT(0xffff8780, "TT: SCSI controller"),
	HW_INIT(0xffff8800, "YM-2149 sound generator"),
	HW_INIT(0xffff8901, "Audio DMA"),
	HW_INIT(0xffff8922, "Microwire interface"),
	HW_INIT(0xffff8930, "Falcon: Sound matrix"),
	HW_INIT(0xffff8960, "Real time clock"),
	HW_INIT(0xffff8a00, "Blitter"),
	HW_INIT(0xffff8c00, "Scc DMA"),
	HW_INIT(0xffff8c80, "Scc controller"),
	HW_INIT(0xffff8e00, "VME System control unit"),
	HW_INIT(0xffff8e20, "MegaSTE: Processor control"),
	HW_INIT(0xffff9200, "Joypad"),
	HW_INIT(0xffff9800, "Falcon: Videl palette registers"),
	HW_INIT(0xffffa200, "Falcon: DSP 56K host interface"),
	HW_INIT(0xfffffa01, "Mfp0"),
	HW_INIT(0xfffffa40, "68881: Peripheral mode"),
	HW_INIT(0xfffffa80, "Mfp1"),
	HW_INIT(0xfffffc00, "Acia0"),
	HW_INIT(0xfffffc04, "Acia1"),
	HW_INIT(0xfffffc20, "MegaST: Real time clock"),
	HW_INIT(NULL, NULL)
};

hardware_t hardware_ct60[]={
	HW_INIT(0x30000000, "CT60: Supervidel"),
	HW_INIT(0x80000000, "CT60: Ethernat"),
	HW_INIT(0xe0000000, "CT60: CTPCI"),
	HW_INIT(NULL, NULL)
};

/*--- Functions prototypes ---*/

void DemoLoop(void);
void DemoHwLoop(void);

void detectHardware(void);

/*--- Functions ---*/

int main(int argc, char **argv)
{
	unsigned char *fpu_type;

	PAR_TestRedirect(argc, argv);
	if (output_to_file) {
		PAR_RedirectStdout();
	}

	/* Read cookies */

	fprintf(output_handle, "Hardware detection using cookies:\n");

	fprintf(output_handle, " _CPU cookie: ");
	if (Getcookie(C__CPU, &cookie_cpu) != C_FOUND) {
		fprintf(output_handle, "non present\n");
		cookie_cpu = 0;
	} else {
		fprintf(output_handle, "present: 0x%08x\n", cookie_cpu);
	}

	fprintf(output_handle, " _FPU cookie: ");
	if (Getcookie(C__FPU, &cookie_fpu) != C_FOUND) {
		fprintf(output_handle, "non present\n");
		cookie_fpu = 0;
	} else {
		fprintf(output_handle, "present: 0x%08x\n", cookie_fpu);
	}

	if (cookie_cpu<=60) {
		fprintf(output_handle, " cpu: %lu\n", 68000+cookie_cpu);
	} else {
		fprintf(output_handle, " cpu: unknown\n");
	}

	if (cookie_fpu & 0xffff) {
		fpu_type="software emulation";
	} else {
		fpu_type="none or unknown";
		switch(cookie_fpu>>16) {
			case 1:
				fpu_type="sfp004";
				break;
			case 2:
				fpu_type="68881/68882";
				break;
			case 3:
				fpu_type="sfp004 + 68881/68882";
				break;
			case 4:
				fpu_type="68881";
				break;
			case 5:
				fpu_type="sfp004 + 68881";
				break;
			case 6:
				fpu_type="68882";
				break;
			case 7:
				fpu_type="sfp004 + 68882";
				break;
			case 8:
				fpu_type="68040";
				break;
			case 9:
				fpu_type="sfp004 + 68040";
				break;
			case 16:
				fpu_type="68060";
				break;
			case 17:
				fpu_type="sfp004 + 68060";
				break;
		}
	}
	fprintf(output_handle, " fpu: %s\n\n", fpu_type);

	DemoHwLoop();

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

void DemoHwLoop(void)
{
	unsigned char *fpu_type;
	hardware_t *curhw;
	unsigned long cookie_ct60;

	Supexec(detectHardware);

	/* Print infos */

	fprintf(output_handle, "Hardware detection using code:\n");

	if (hw_cpu<=60) {
		fprintf(output_handle, " cpu: %lu\n", 68000+hw_cpu);
	} else {
		fprintf(output_handle, " cpu: unknown\n");
	}

	if (hw_mmu!=0) {
		fprintf(output_handle, " mmu: %lu\n", 68000+hw_mmu);
	} else {
		fprintf(output_handle, " mmu: none or unknown\n");
	}

	if (hw_fpu & 0xffff) {
		fpu_type="software emulation";
	} else {
		fpu_type="none or unknown";
		switch(hw_fpu>>16) {
			case 1:
				fpu_type="sfp004";
				break;
			case 2:
				fpu_type="68881/68882";
				break;
			case 3:
				fpu_type="sfp004 + 68881/68882";
				break;
			case 4:
				fpu_type="68881";
				break;
			case 5:
				fpu_type="sfp004 + 68881";
				break;
			case 6:
				fpu_type="68882";
				break;
			case 7:
				fpu_type="sfp004 + 68882";
				break;
			case 8:
				fpu_type="68040";
				break;
			case 9:
				fpu_type="sfp004 + 68040";
				break;
			case 16:
				fpu_type="68060";
				break;
			case 17:
				fpu_type="sfp004 + 68060";
				break;
		}
	}
	fprintf(output_handle, " fpu: %s\n\n", fpu_type);

	curhw=hardware;
	while (curhw->address != NULL) {
		if (curhw->present) {
			fprintf(output_handle, " 0x%08x: %s\n",
				curhw->address,
				curhw->name
			);
		}

		curhw++;
	}

	if (Getcookie(C_CT60, &cookie_ct60) == C_FOUND) {
		curhw=hardware_ct60;
		while (curhw->address != NULL) {
			if (curhw->present) {
				fprintf(output_handle, " 0x%08x: %s\n",
					curhw->address,
					curhw->name
				);
			}

			curhw++;
		}
	}

	if (hw_mmu) {
		fprintf(output_handle, "\nMMU setup:\n");
	} else {
		fprintf(output_handle, "\nNo MMU to display\n");
	}

	switch(hw_mmu) {
		case 30:
		case 851:
			DisplayMmuTree030_851();
			break;
		case 40:
		case 60:
			DisplayMmuTree040_060();
			break;
	}
}

/* Functions called in supervisor mode */

void detectHardware(void)
{
	hardware_t *curhw;

	HW_Detect(cookie_cpu<=60 ? cookie_cpu : 0);

	curhw=hardware;
	while (curhw->address != NULL) {
		curhw->present = HW_RegDetect(hw_cpu, curhw->address);

		curhw++;
	}
}
