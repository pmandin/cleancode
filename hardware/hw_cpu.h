/*
	Hardware CPU test

	Copyright (C) 2003	Patrice Mandin

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

#ifndef HW_CPU_H
#define HW_CPU_H

/*--- Variables ---*/

extern unsigned long hw_cpu, hw_mmu, hw_fpu;
extern unsigned long hw_mmu_urp[2], hw_mmu_srp[2], hw_mmu_tcr;
extern unsigned long hw_mmu_dttr0, hw_mmu_dttr1, hw_mmu_ittr0, hw_mmu_ittr1;

/*--- Functions prototypes ---*/

void HW_Detect(unsigned long cookie_cpu);
void HW_CpuDetect(void);

#endif /* HW_CPU_H */
