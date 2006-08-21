/*
	PCI-BIOS demo program

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
#include <mint/cookie.h>

#include "param.h"
#include "endian.h"

#include "pci.h"
#include "pci_bios.h"
#include "pci_xpci.h"
#include "pcinames.h"

/*--- Defines ---*/

#define SCANCODE_ESC	0x01

/*--- Variables ---*/

/* Names of vendor and device, if found */
int names_read;
char vendorname[64];
char devicename[64];

/* Names of classes, if found */
int classes_read;
char classname[64];
char subclassname[64];
char progifname[64];

/*--- Functions prototypes ---*/

void DemoPciLoop(void);
void DemoLoop(void);

/*--- Functions ---*/

int main(int argc, char **argv)
{
	unsigned long cookie_pci;
	unsigned long cookie_xpci;

	PAR_TestRedirect(argc, argv);
	if (output_to_file) {
		PAR_RedirectStdout();
	}

	fprintf(output_handle, "XPCI cookie: ");
	if (Getcookie(C_XPCI, &cookie_xpci) != C_FOUND) {
		fprintf(output_handle, "non present\n");
		cookie_xpci = 0;
	} else {
		fprintf(output_handle, "present\n");
	}

	fprintf(output_handle, "_PCI cookie: ");
	if (Getcookie(C__PCI, &cookie_pci) != C_FOUND) {
		fprintf(output_handle, "non present\n");
		cookie_pci = 0;
	} else {
		fprintf(output_handle, "present\n");
	}

	DemoPciLoop();

	if (!output_to_file) {
		DemoLoop();
	} else {
		fclose(output_handle);
	}
}

void DemoPciLoop(void)
{
	int num_device;
	int class_device, subclass_device;
	long machineid, device_handle;
	pci_config_t device_config;
	long classcode;
	char **pci_subclass_devices;
	unsigned short vendor_id, device_id, command, status;
	unsigned short subvendor_id, subdevice_id;
	unsigned long address, size;

	/* Init library */
	if (pci_init()!=PCI_SUCCESSFUL) {
		fprintf(output_handle, "Unable to initialize PCI library\n");
		return;
	}
	fprintf(output_handle, "PCI library initialized\n\n");

	/* Machine ID */
	fprintf(output_handle, "PCI Machine ID: ");
	machineid=pci_get_machine_id();
	if (machineid>=0) {
		unsigned long serialnumber, machnumber;

		serialnumber = machineid & 0xffffffL;
		machnumber = (machineid>>24) & 0xff;

		if (machnumber<3) {
			switch(machnumber) {
				case 0:
					fprintf(output_handle, "Medusa or Hades\n");
					break;
				case 1:
					fprintf(output_handle, "Milan\n");
					break;
				case 2:
					fprintf(output_handle, "PAK/Panther 2\n");
					break;
			}
			fprintf(output_handle, " Machine serial number: 0x%06x\n",serialnumber);
		} else {
			fprintf(output_handle, "Unknown: 0x%08x\n", machineid);
		}
	} else {
		fprintf(output_handle, "Error reading value\n");
	}

	/* List devices and infos */
	num_device=-1;
	for(;;) {
		int x,y,i;
		long result;

		num_device++;
		
		result = pci_find_device(0x0000ffffL, num_device);
		if (result<0) {
			printf("\nPCI listing done.\n");
			break;
		}
		device_handle = result;

		fprintf(output_handle, "\nDevice %d:\n",num_device);

		/* Display configuration bytes */
		fprintf(output_handle, " Configuration registers:\n");
		for (y=0; y<4; y++) {
			fprintf(output_handle, "  %02x:", y*16);
			for (x=0; x<16; x++) {
				unsigned char value;

				result=pci_read_config_byte(device_handle, &value, y*16+x);
				if (result<0) {
					fprintf(output_handle, " --");
				} else {
					fprintf(output_handle, " %02x", value);
				}
			}
			fprintf(output_handle, "\n");
		}

		/* Read configuration registers */
		result = pci_read_config(device_handle, &device_config);
		if (result<0) {
			fprintf(output_handle, "Error reading configuration registers in block\n");
			continue;
		}

		/* Vendor and device ID */
		vendor_id = GET_LE_WORD_S(device_config,vendor_id);
		device_id = GET_LE_WORD_S(device_config,device_id);

		names_read = pci_device_getname(
			vendor_id, vendorname, sizeof(vendorname),
			device_id, devicename, sizeof(devicename)
		);

		fprintf(output_handle, " Vendor ID: 0x%04x", vendor_id);
		if (names_read>=0) {
			fprintf(output_handle, ", %s\n", vendorname);
		} else {
			fprintf(output_handle, "\n");
		}

		fprintf(output_handle, " Device ID: 0x%04x", device_id);
		if (names_read>=0) {
			fprintf(output_handle, ", %s\n", devicename);
		} else {
			fprintf(output_handle, "\n");
		}

		/* Classcode */
		classcode=device_config.classcode[2]<<16;
		classcode|=device_config.classcode[1]<<8;
		classcode|=device_config.classcode[0];
		fprintf(output_handle, " Classcode: 0x%06x\n",classcode);

		classes_read= pci_class_getname(
			classcode,
			classname, sizeof(classname),
			subclassname, sizeof(subclassname),
			progifname, sizeof(progifname)
		);

		if (classes_read>=0) {
			fprintf(output_handle, "  Class: %s\n", classname);
			fprintf(output_handle, "  Subclass: %s\n", subclassname);
			fprintf(output_handle, "  Progif: %s\n", progifname);
		}

		fprintf(output_handle, " Revision: %d\n",device_config.revision_id);

		fprintf(output_handle, " Cache line size: 0x%02x\n",device_config.cache_line_size);
		fprintf(output_handle, " Latency timer: 0x%02x\n",device_config.latency_timer);
		fprintf(output_handle, " Header type: 0x%02x\n",device_config.header_type);
		fprintf(output_handle, " ARE: 0x%02x\n",device_config.are);

		address = GET_LE_LONG_S(device_config,address);
		if (address!=0) {
			int memtype;

			address = pci_read_mem_base(address, &memtype);
			size = pci_read_mem_size(device_handle, 0x10);

			if (memtype) {
				fprintf(output_handle, " Memory space: ");
			} else {
				fprintf(output_handle, " I/O space: ");
			}
			fprintf(output_handle, "at 0x%08x, size 0x%08x\n", address, size);
		}

		switch(device_config.header_type) {
			case 0:
				fprintf(output_handle, " Normal PCI device:\n");

				for (i=0; i<5; i++) {
					address = device_config.pci_config_header.header0.address[i*4];
					address |= device_config.pci_config_header.header0.address[i*4+1]<<8;
					address |= device_config.pci_config_header.header0.address[i*4+2]<<16;
					address |= device_config.pci_config_header.header0.address[i*4+3]<<24;

					if (address!=0) {
						int memtype;

						address = pci_read_mem_base(address, &memtype);
						size = pci_read_mem_size(device_handle, 0x10);
						fprintf(output_handle, "  Region %d:", i);
						if (memtype) {
							fprintf(output_handle, " Memory space: ");
						} else {
							fprintf(output_handle, " I/O space: ");
						}
						fprintf(output_handle, "at 0x%08x, size 0x%08x\n", address, size);
					}
				}

				subvendor_id = GET_LE_WORD_S(device_config.pci_config_header.header0,subvendor_id);
				subdevice_id = GET_LE_WORD_S(device_config.pci_config_header.header0,subdevice_id);

				if ((subvendor_id!=0) && (subdevice_id!=0)) {
					names_read = pci_subdevice_getname(
						vendor_id, device_id,
						subvendor_id, vendorname, sizeof(vendorname),
						subdevice_id, devicename, sizeof(devicename)
					);

					fprintf(output_handle, "  SubVendor ID: 0x%04x", subvendor_id);
					if (names_read>=0) {
						fprintf(output_handle, ", %s\n", vendorname);
					} else {
						fprintf(output_handle, "\n");
					}

					fprintf(output_handle, "  SubDevice ID: 0x%04x", subdevice_id);
					if (names_read>=0) {
						fprintf(output_handle, ", %s\n", devicename);
					} else {
						fprintf(output_handle, "\n");
					}
				}

				address = GET_LE_LONG_S(device_config.pci_config_header.header0,rom_address);
				if (address!=0) {
					fprintf(output_handle, "  ROM at 0x%08x\n", address);
				}

				fprintf(output_handle,
					"  Interrupt: line=%d, pin=%d, min. grant=%d, max. lat=%d\n",
					device_config.pci_config_header.header0.int_line,
					device_config.pci_config_header.header0.int_pin,
					device_config.pci_config_header.header0.min_gnt,
					device_config.pci_config_header.header0.max_lat
				);
				break;
			case 1:
				fprintf(output_handle, " PCI-to-PCI bridge device\n");
				
				address = GET_LE_LONG_S(device_config.pci_config_header.header1, address);
				if (address!=0) {
					fprintf(output_handle, "  Memory at 0x%08x\n", address);
				}

				fprintf(output_handle,
					"  Bus: primary=%d, secondary=%d, subordinate=%d, sec. latency=%d\n",
					device_config.pci_config_header.header1.primary_bus,
					device_config.pci_config_header.header1.secondary_bus,
					device_config.pci_config_header.header1.subordinate_bus,
					device_config.pci_config_header.header1.sec_latency_timer
				);

				address = GET_LE_LONG_S(device_config.pci_config_header.header1, rom_address);
				if (address!=0) {
					fprintf(output_handle, "  ROM at 0x%08x\n", address);
				}
				break;
			case 2:
				fprintf(output_handle, " CardBus bridge device\n");

				fprintf(output_handle,
					"  Bus: primary=%d, card=%d, subordinate=%d, latency=%d\n",
					device_config.pci_config_header.header2.primary_bus,
					device_config.pci_config_header.header2.card_bus,
					device_config.pci_config_header.header2.subordinate_bus,
					device_config.pci_config_header.header2.latency_timer
				);

				subvendor_id = GET_LE_WORD_S(device_config.pci_config_header.header2,subvendor_id);
				subdevice_id = GET_LE_WORD_S(device_config.pci_config_header.header2,subdevice_id);

				if ((subvendor_id!=0) && (subdevice_id!=0)) {
					names_read = pci_subdevice_getname(
						vendor_id, device_id,
						subvendor_id, vendorname, sizeof(vendorname),
						subdevice_id, devicename, sizeof(devicename)
					);

					fprintf(output_handle, "  SubVendor ID: 0x%04x", subvendor_id);
					if (names_read>=0) {
						fprintf(output_handle, ", %s\n", vendorname);
					} else {
						fprintf(output_handle, "\n");
					}

					fprintf(output_handle, "  SubDevice ID: 0x%04x", subdevice_id);
					if (names_read>=0) {
						fprintf(output_handle, ", %s\n", devicename);
					} else {
						fprintf(output_handle, "\n");
					}
				}
				break;
		}

		/* Command bits */
		command = GET_LE_WORD_S(device_config,command);
		fprintf(output_handle, " Device commands:\n");
		if (command & PCI_COMMAND_IO) {
			fprintf(output_handle, "  Device respond to I/O space\n");
		}
		if (command & PCI_COMMAND_MEMORY) {
			fprintf(output_handle, "  Device respond to memory space\n");
		}
		if (command & PCI_COMMAND_MASTER) {
			fprintf(output_handle, "  Device can bus master\n");
		}
		if (command & PCI_COMMAND_SPECIAL) {
			fprintf(output_handle, "  Device respond to special cycles\n");
		}
		if (command & PCI_COMMAND_INVALIDATE) {
			fprintf(output_handle, "  Device can invalidate memory writes\n");
		}
		if (command & PCI_COMMAND_VGA_PALETTE) {
			fprintf(output_handle, "  Device snoops VGA palette\n");
		}
		if (command & PCI_COMMAND_PARITY) {
			fprintf(output_handle, "  Device can check parity\n");
		}
		if (command & PCI_COMMAND_WAIT) {
			fprintf(output_handle, "  Device operates with address DATA stepping\n");
		}
		if (command & PCI_COMMAND_SERR) {
			fprintf(output_handle, "  Device can signal system error\n");
		}
		if (command & PCI_COMMAND_FAST_BACK) {
			fprintf(output_handle, "  Device can do back-to-back writes\n");
		}

		/* Status bits */
		status = GET_LE_WORD_S(device_config,status);
		fprintf(output_handle, " Device status:\n");
		if (status & PCI_STATUS_CAP_LIST) {
			fprintf(output_handle, "  Device has capability list\n");
		}
		if (status & PCI_STATUS_66MHZ) {
			fprintf(output_handle, "  Device supports 66MHz PCI 2.1 bus\n");
		}
		if (status & PCI_STATUS_UDF) {
			fprintf(output_handle, "  Device supports user-definable features (obsolete)\n");
		}
		if (status & PCI_STATUS_FAST_BACK) {
			fprintf(output_handle, "  Device accepts back-to-back\n");
		}
		if (status & PCI_STATUS_PARITY) {
			fprintf(output_handle, "  Device has detected parity error with master access\n");
		}
		if ((status & PCI_STATUS_DEVSEL_MASK)==PCI_STATUS_DEVSEL_FAST) {
			fprintf(output_handle, "  Device has fast selection speed\n");
		}
		if ((status & PCI_STATUS_DEVSEL_MASK)==PCI_STATUS_DEVSEL_MEDIUM) {
			fprintf(output_handle, "  Device has medium selection speed\n");
		}
		if ((status & PCI_STATUS_DEVSEL_MASK)==PCI_STATUS_DEVSEL_SLOW) {
			fprintf(output_handle, "  Device has slow selection speed\n");
		}
		if (status & PCI_STATUS_SIG_TARGET_ABORT) {
			fprintf(output_handle, "  Device signaled aborted target transaction\n");
		}
		if (status & PCI_STATUS_REC_TARGET_ABORT) {
			fprintf(output_handle, "  Device received aborted target transaction\n");
		}
		if (status & PCI_STATUS_REC_MASTER_ABORT) {
			fprintf(output_handle, "  Device received aborted master transaction\n");
		}
		if (status & PCI_STATUS_SIG_SYSTEM_ERROR) {
			fprintf(output_handle, "  Device signaled a system error\n");
		}
		if (status & PCI_STATUS_DETECTED_PARITY) {
			fprintf(output_handle, "  Device has detected parity error\n");
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
