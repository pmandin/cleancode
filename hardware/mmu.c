/*
	MMU tree dumps

	Copyright (C) 2006	Patrice Mandin

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

/*--- Includes ---*/

#include <stdio.h>

#include "param.h"
#include "hw_cpu.h"

/*--- Constants ---*/

static char *descriptorType030[4]={
	"invalid",
	"page descriptor",
	"next page has 4-bytes descriptors",
	"next page has 8-bytes descriptor"
};

static char *defaultCacheMode040[4]={
	"writethrough, cacheable",
	"copyback, cacheable",
	"cache inhibited, precise exception model",
	"cache inhibited, imprecise exception model"
};

static char *ttrCacheMode040[4]={
	"cacheable, writethrough",
	"cacheable, copyback",
	"non cacheable, serialized",
	"non cacheable"
};

/*--- Functions prototypes ---*/

static void printTc030(void);
static void printRp030(unsigned long rp0, unsigned long rp1);
static void printTtr030(unsigned long ttr);

static void printTc040(void);
static void printRp040(unsigned long rp);
static void printTtr040(unsigned long ttr);

/*--- Functions ---*/

void DisplayMmuTree030_851(void)
{
	fprintf(output_handle, " tc : 0x%08x\n", hw_mmu_tcr);
		printTc030();
	fprintf(output_handle, " crp: 0x%08x:0x%08x\n", hw_mmu_urp[0], hw_mmu_urp[1]);
		printRp030(hw_mmu_urp[0], hw_mmu_urp[1]);
	fprintf(output_handle, " srp: 0x%08x:0x%08x\n", hw_mmu_srp[0], hw_mmu_srp[1]);
		printRp030(hw_mmu_srp[0], hw_mmu_srp[1]);
	fprintf(output_handle, " tt0: 0x%08x\n", hw_mmu_dttr0);
		printTtr030(hw_mmu_dttr0);
	fprintf(output_handle, " tt1: 0x%08x\n", hw_mmu_dttr1);
		printTtr030(hw_mmu_dttr1);
}

static void printRp030(unsigned long rp0, unsigned long rp1)
{
	fprintf(output_handle, "  limit direction: %s\n", (rp0 & (1<<31)) ? "up" : "down");
	fprintf(output_handle, "  limit: %d\n", (rp0>>16)&0x7fff);
	fprintf(output_handle, "  dt: %d (%s)\n", rp0 & 3, descriptorType030[rp0 & 3]);
	fprintf(output_handle, "  base address: 0x%08\n", rp1 & 0xfffffff0UL);
}

static void printTc030(void)
{
	fprintf(output_handle, "  enable: %s\n", (hw_mmu_tcr & (1<<31)) ? "true" : "false");
	fprintf(output_handle, "  supervisor root enable: %s\n", (hw_mmu_tcr & (1<<25)) ? "true" : "false");
	fprintf(output_handle, "  function code lookup: %s\n", (hw_mmu_tcr & (1<<24)) ? "true" : "false");
	fprintf(output_handle, "  page size: %d bytes\n", 1<<((hw_mmu_tcr>>20)&15));
	fprintf(output_handle, "  initial shift: %d bits\n", (hw_mmu_tcr>>16)&15);
	fprintf(output_handle, "  table index a: %d bits\n", (hw_mmu_tcr>>12)&15);
	fprintf(output_handle, "  table index b: %d bits\n", (hw_mmu_tcr>>8)&15);
	fprintf(output_handle, "  table index c: %d bits\n", (hw_mmu_tcr>>4)&15);
	fprintf(output_handle, "  table index d: %d bits\n", hw_mmu_tcr&15);
}

static void printTtr030(unsigned long ttr)
{
	fprintf(output_handle, "  logical address base: 0x%02x000000\n", (ttr>>24)&0xff);
	fprintf(output_handle, "  logical address mask: 0x00%02x0000\n", (ttr>>16)&0xff);
	fprintf(output_handle, "  enable: %s\n", (ttr&(1<<15)) ? "true" : "false");
	fprintf(output_handle, "  cache inhibited: %s\n", (ttr&(1<<10)) ? "true" : "false");
	fprintf(output_handle, "  read/write: %s\n", (ttr & (1<<9)) ? "true" : "false");
	fprintf(output_handle, "  read/write mask: %s\n", (ttr & (1<<8)) ? "true" : "false");
	fprintf(output_handle, "  fc base: %d\n", (ttr>>4)&3);
	fprintf(output_handle, "  fc mask: %d\n", (ttr>>0)&3);
}

void DisplayMmuTree040_060(void)
{
	fprintf(output_handle, " tc  : 0x%08x\n", hw_mmu_tcr);
		printTc040();
	fprintf(output_handle, " urp : 0x%08x\n", hw_mmu_urp);
		fprintf(output_handle, "  base address: 0x%08x\n", hw_mmu_urp & 0xfffffe00UL);
	fprintf(output_handle, " srp : 0x%08x\n", hw_mmu_srp);
		fprintf(output_handle, "  base address: 0x%08x\n", hw_mmu_srp & 0xfffffe00UL);
	fprintf(output_handle, " dtt0: 0x%08x\n", hw_mmu_dttr0);
		printTtr040(hw_mmu_dttr0);
	fprintf(output_handle, " dtt1: 0x%08x\n", hw_mmu_dttr1);
		printTtr040(hw_mmu_dttr1);
	fprintf(output_handle, " itt0: 0x%08x\n", hw_mmu_ittr0);
		printTtr040(hw_mmu_ittr0);
	fprintf(output_handle, " itt1: 0x%08x\n", hw_mmu_ittr1);
		printTtr040(hw_mmu_ittr1);
}

static void printTc040(void)
{
	fprintf(output_handle, "  enable: %s\n", (hw_mmu_tcr & (1<<15)) ? "true" : "false");
	fprintf(output_handle, "  page size: %d bytes\n", 4096<<((hw_mmu_tcr>>14)&1));
	fprintf(output_handle, "  no allocate mode (data atc): %s\n", (hw_mmu_tcr & (1<<13)) ? "true" : "false");
	fprintf(output_handle, "  no allocate mode (inst atc): %s\n", (hw_mmu_tcr & (1<<12)) ? "true" : "false");
	fprintf(output_handle, "  half cache mode (data atc): %s\n", (hw_mmu_tcr & (1<<11)) ? "true" : "false");
	fprintf(output_handle, "  half cache mode (inst atc): %s\n", (hw_mmu_tcr & (1<<10)) ? "true" : "false");
	fprintf(output_handle, "  default cache mode (data cache): %s\n", defaultCacheMode040[(hw_mmu_tcr>>8)&3]);
	fprintf(output_handle, "  default upa bits (data cache): %d\n", (hw_mmu_tcr>>6)&3);
	fprintf(output_handle, "  default write protect (data cache): %s\n", (hw_mmu_tcr & (1<<5)) ? "read only" : "read/write");
	fprintf(output_handle, "  default cache mode (inst cache): %s\n", defaultCacheMode040[(hw_mmu_tcr>>3)&3]);
	fprintf(output_handle, "  default upa bits (inst cache): %d\n", (hw_mmu_tcr>>1)&3);
}

static void printTtr040(unsigned long ttr)
{
	fprintf(output_handle, "  logical address base: 0x%02x000000\n", (ttr>>24)&0xff);
	fprintf(output_handle, "  logical address mask: 0x00%02x0000\n", (ttr>>16)&0xff);
	fprintf(output_handle, "  enable: %s\n", (ttr&(1<<15)) ? "true" : "false");
	fprintf(output_handle, "  supervisor mode: %d\n", (ttr>>13)&3);
	fprintf(output_handle, "  upa bits: %d\n", (ttr>>8)&3);
	fprintf(output_handle, "  cache mode: %s\n", ttrCacheMode040[(ttr>>5)&3]);
	fprintf(output_handle, "  write protect: %s\n", (ttr & (1<<2)) ? "read only" : "read/write");
}
