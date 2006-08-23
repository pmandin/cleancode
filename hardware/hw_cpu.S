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

/*
	Main detection code taken from freemint kernel from Draco
*/

	.text

	.globl	_hw_cpu
	.globl	_hw_mmu
	.globl	_hw_fpu

	.globl	_hw_mmu_urp
	.globl	_hw_mmu_srp
	.globl	_hw_mmu_tcr
	.globl	_hw_mmu_dttr0
	.globl	_hw_mmu_dttr1
	.globl	_hw_mmu_ittr0
	.globl	_hw_mmu_ittr1

	.globl	_HW_Detect
	.globl	_HW_CpuDetect
	.globl	_HW_FpuDetect
	.globl	_HW_MmuDetect

/*--- Defines ---*/

#define BUSERROR_VECTOR	2<<2
#define ILLEGAL_VECTOR	4<<2
#define LINEF_VECTOR	11<<2
#define UNIMPLEMENTED_INTEGER_VECTOR	61<<2

#define FPU_PERIPHERAL	0x00000001
#define FPU_68881_82	0x00020000
#define FPU_68881	0x00040000
#define FPU_68882	0x00060000
#define FPU_040		0x00080000
#define FPU_060		0x00100000

/*--- Detection ---*/

_HW_Detect:
	movel	sp@(4),d0	| cookie_cpu

	/* Check VBR ? */
	moveq	#0,d1

	cmp	#10,d0
	bmis	novbr
	.chip	68010
	movec	vbr,d1
	.chip	68000
novbr:
	movel	d1,current_vbr

	movel	d2,sp@-
	bsr	_HW_CpuDetect
	movel	sp@+,d2
	bsr	_HW_FpuDetect
	bsr	_HW_MmuDetect

	rts

/*--- CPU detection ---*/

	.text
_HW_CpuDetect:
	/*--- 68000 as default ---*/

	clrl	_hw_cpu

	/* Save stack pointer */
	movel	sp,a1

	/* Save current vectors, and redirect it to end of routine */
	movel	current_vbr,a0

	movel	a0@(ILLEGAL_VECTOR),save_vector0
	movel	#CpuDetectEnd,a0@(ILLEGAL_VECTOR)

	movel	a0@(UNIMPLEMENTED_INTEGER_VECTOR),save_vector1
	movel	#CpuDetectEnd,a0@(UNIMPLEMENTED_INTEGER_VECTOR)

	nop

	/*--- Check for 68010 ---*/

	.chip	68010
	move	ccr,d0		| Legal on 68010 and higher
	.chip	68000
	movel	#10,_hw_cpu

	/*--- Check for 68020 ---*/

	.chip	68020
	movec	cacr,d1		| get cache control register
	movel	d1,d0		| hold a copy for later
	or	#0x8100,d1	| enable '030 data and '040 instruction caches
	movec	d1,cacr		| set new cache controls
	movec	cacr,d1		| read it back to check the effects
	movec	d0,cacr		| restore original CACR settings
	.chip	68000
	tst	d1		| if 15th bit was set, this is a 68040+
	bmis	CpuIs040OrHigher

	movel	#20,_hw_cpu

	/*--- Check for 68030 ---*/

	btst	#0x08,d1	| check if 68030 data cache was enabled
	beqs	CpuDetectEnd	| a zero here means no data cache, i.e. 68020

	movel	#30,_hw_cpu
	bras	CpuDetectEnd

	/*--- Check for 68060 ---*/

CpuIs040OrHigher:
	movel	#60,_hw_cpu
	.chip	68020
	mulul	d0,d2:d1	| instruction implemented means 040
	.chip	68000

	/*--- Check for 68040 ---*/

	movel	#40,_hw_cpu

CpuDetectEnd:
	movel	save_vector0,a0@(ILLEGAL_VECTOR)
	movel	save_vector1,a0@(UNIMPLEMENTED_INTEGER_VECTOR)
	movel	a1,sp

	nop
	rts

/*--- FPU detection ---*/

	.text
_HW_FpuDetect:

	/*--- no fpu as default ---*/

	clrl	_hw_fpu

	/* Save stack pointer */
	movel	sp,a1

	/* Save current vectors, and redirect it to end of routine */
	movel	current_vbr,a0

	movel	a0@(ILLEGAL_VECTOR),save_vector0
	movel	#FpuDetectEnd,a0@(ILLEGAL_VECTOR)

	movel	a0@(LINEF_VECTOR),save_vector1
	movel	#FpuDetectEnd,a0@(LINEF_VECTOR)

	movel	a0@(BUSERROR_VECTOR),save_vector2
	movel	#FpuDetectEnd,a0@(BUSERROR_VECTOR)

	nop

	movel	_hw_cpu,d0
	cmp	#10,d0
	bpls	FpuIs020OrHigher

	tstw	0xfffffa40.w
	movel	#FPU_PERIPHERAL,_hw_fpu
	bras	FpuDetectEnd

FpuIs020OrHigher:

	cmp	#60,d0
	bmis	FpuIs040OrLess

	.chip	68060
	movec	pcr,d1
	.chip	68000
	swap	d1
	cmp	#0x0431,d1
	beqs	FpuIs040OrLess
	bclr	#1,d1
	.chip	68060
	movec	d1,pcr
	.chip	68000

FpuIs040OrLess:

	.chip	68040
	fnop
	clrl	sp@-
	clrl	sp@-
	clrl	sp@-
	frestore	sp@+
	fnop
	fsave	sp@-
	.chip	68000

	movel	#FPU_060,_hw_fpu
	cmp	#60,d0
	beqs	FpuDetectEnd

	movel	#FPU_040,_hw_fpu
	cmp	#40,d0
	beqs	FpuDetectEnd

	movel	#FPU_68882,_hw_fpu
	move.b	(sp)+,d1
	cmp	#0x1f,d1
	beqs	FpuDetectEnd

	movel	#FPU_68881,_hw_fpu

FpuDetectEnd:
	movel	save_vector0,a0@(ILLEGAL_VECTOR)
	movel	save_vector1,a0@(LINEF_VECTOR)
	movel	save_vector2,a0@(BUSERROR_VECTOR)
	movel	a1,sp

	nop
	rts

/*--- MMU detection ---*/

	.text
_HW_MmuDetect:

	/*--- no mmu as default ---*/

	clrl	_hw_mmu

	clrl	_hw_mmu_urp
	clrl	_hw_mmu_srp
	clrl	_hw_mmu_tcr
	clrl	_hw_mmu_dttr0
	clrl	_hw_mmu_dttr1
	clrl	_hw_mmu_ittr0
	clrl	_hw_mmu_ittr1

	/* Save stack pointer */
	movel	sp,a1

	/* Save current vectors, and redirect it to end of routine */
	movel	current_vbr,a0

	movel	a0@(ILLEGAL_VECTOR),save_vector0
	movel	#MmuDetectEnd,a0@(ILLEGAL_VECTOR)

	movel	a0@(LINEF_VECTOR),save_vector1
	movel	#MmuDetectEnd,a0@(LINEF_VECTOR)

	nop

	movel	_hw_cpu,d0
	cmp	#20,d0
	blt	MmuDetectEnd	/* 68000 or 68010: no mmu */

	cmp	#30,d0
	bles	Mmu030OrLower

	/* Check for 040 or 060 */
	.chip	68040
	movec	tc,d1

	movec	urp,d1
	movel	d1,_hw_mmu_urp
	movec	srp,d1
	movel	d1,_hw_mmu_srp
	movec	tc,d1
	movel	d1,_hw_mmu_tcr

	movec	dtt0,d1
	movel	d1,_hw_mmu_dttr0
	movec	dtt1,d1
	movel	d1,_hw_mmu_dttr1
	movec	itt0,d1
	movel	d1,_hw_mmu_ittr0
	movec	itt1,d1
	movel	d1,_hw_mmu_ittr1
	.chip	68000

	bras	MmuIsCpu

	/* Check for 020+68851 or 030 */
Mmu030OrLower:
	lea	current_mmusr,a0
	.chip	68030
	pmove	psr,a0@

	pmove	crp,_hw_mmu_urp
	pmove	srp,_hw_mmu_srp
	pmove	tc,_hw_mmu_tcr

	pmove	tt0,d1
	movel	d1,_hw_mmu_dttr0
	movel	d1,_hw_mmu_ittr0
	pmove	tt1,d1
	movel	d1,_hw_mmu_dttr1
	movel	d1,_hw_mmu_ittr1
	.chip	68000

	cmp	#30,d0
	beqs	MmuIsCpu
	movel	#851,d0

MmuIsCpu:
	movel	d0,_hw_mmu

MmuDetectEnd:
	movel	save_vector0,a0@(ILLEGAL_VECTOR)
	movel	save_vector1,a0@(LINEF_VECTOR)
	movel	a1,sp

	nop

	rts

/*--- Variables ---*/

	.data
	.even
	.comm	_hw_cpu,4
	.comm	_hw_mmu,4
	.comm	_hw_fpu,4

	.comm	save_vector0,4	| Illegal instruction
	.comm	save_vector1,4	| Line F or unimplemented integer instruction
	.comm	save_vector2,4	| Bus error instruction

	.comm	current_vbr,4
	.comm	current_mmusr,4

	.comm	_hw_mmu_urp,4
	.comm	_hw_mmu_srp,4
	.comm	_hw_mmu_tcr,4
	.comm	_hw_mmu_dttr0,4
	.comm	_hw_mmu_dttr1,4
	.comm	_hw_mmu_ittr0,4
	.comm	_hw_mmu_ittr1,4
