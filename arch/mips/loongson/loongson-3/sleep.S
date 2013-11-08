/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Sleep helper for Loongson-3A sleep mode, derived from Au1xxx.
 *
 * Copyright (C) 2011 Huacai Chen <chenhc@lemote.com>
 */

#include <asm/asm.h>
#include <asm/asmmacro.h>
#include <asm/mipsregs.h>
#include <asm/stackframe.h>

	.extern cmos_write64

	.text
	.set noreorder
	.set noat
	.align	5
	.set	mips64

/* preparatory stuff */
.macro	SETUP_SLEEP
	dsubu	sp, PT_SIZE
	sd	$1, PT_R1(sp)
	sd	$2, PT_R2(sp)
	sd	$3, PT_R3(sp)
	sd	$4, PT_R4(sp)
	sd	$5, PT_R5(sp)
	sd	$6, PT_R6(sp)
	sd	$7, PT_R7(sp)
	sd	$16, PT_R16(sp)
	sd	$17, PT_R17(sp)
	sd	$18, PT_R18(sp)
	sd	$19, PT_R19(sp)
	sd	$20, PT_R20(sp)
	sd	$21, PT_R21(sp)
	sd	$22, PT_R22(sp)
	sd	$23, PT_R23(sp)
	sd	$26, PT_R26(sp)
	sd	$27, PT_R27(sp)
	sd	$28, PT_R28(sp)
	sd	$30, PT_R30(sp)
	sd	$31, PT_R31(sp)
	mfc0	k0, CP0_STATUS
	sw	k0, PT_R8(sp)
	mfc0	k0, CP0_CONTEXT
	sd	k0, PT_R9(sp)
	mfc0	k0, CP0_XCONTEXT
	sd	k0, PT_R10(sp)
	mfc0	k0, CP0_PAGEMASK, 0  /* Loongson-3 has sub-registers */
	sw	k0, PT_R11(sp)
	mfc0	k0, CP0_PAGEMASK, 1  /* Loongson-3 has sub-registers */
	sw	k0, PT_R12(sp)
	mfc0	k0, CP0_CONFIG, 0
	sw	k0, PT_R13(sp)

	/* Now set up the "wakeup vector" in  RTC space so the boot rom will
	 * return to this point upon wakeup.
	 * 0x40 : RA,  0x48 : SP
	 */
	daddi	a0, sp, 0
	li      a1, 0x48
	jal     cmos_write64
	nop
	dla	a0, wakeup_start	/* resume path */
	li      a1, 0x40
	jal     cmos_write64
	nop
.endm

/* sleep code for Loongson-3A memory controller type */
LEAF(loongson_suspend_lowlevel)
	SETUP_SLEEP

	li t0, 0x80000000
	li t1, 512
flushL1:
	cache 0, 0(t0)
	cache 0, 1(t0)
	cache 0, 2(t0)
	cache 0, 3(t0)
	cache 1, 0(t0)
	cache 1, 1(t0)
	cache 1, 2(t0)
	cache 1, 3(t0)
	addiu t0, t0, 0x20
	bnez  t1, flushL1
	addiu t1, t1, -1

	li t0, 0x80000000
	li t1, 32768
flushL2:
	cache 3, 0(t0)
	cache 3, 1(t0)
	cache 3, 2(t0)
	cache 3, 3(t0)
	addiu t0, t0, 0x20
	bnez  t1, flushL2
	addiu t1, t1, -1

	dli v0, 0xffffffffbfc00500 /* Call PMON's STR sleep routine */
	jr  v0
	nop
END(loongson_suspend_lowlevel)

	/* This is where we return upon wakeup.
	 * Reload all of the registers and return.
	 */
LEAF(wakeup_start)
	lw	k0, PT_R8(sp)
	mtc0	k0, CP0_STATUS
	ld	k0, PT_R9(sp)
	mtc0	k0, CP0_CONTEXT
	ld	k0, PT_R10(sp)
	mtc0	k0, CP0_XCONTEXT
	lw	k0, PT_R11(sp)
	mtc0	k0, CP0_PAGEMASK, 0
	lw	k0, PT_R12(sp)
	mtc0	k0, CP0_PAGEMASK, 1
	lw	k0, PT_R13(sp)
	mtc0	k0, CP0_CONFIG, 0

	nop
	ld	$1, PT_R1(sp)
	ld	$2, PT_R2(sp)
	ld	$3, PT_R3(sp)
	ld	$4, PT_R4(sp)
	ld	$5, PT_R5(sp)
	ld	$6, PT_R6(sp)
	ld	$7, PT_R7(sp)
	ld	$16, PT_R16(sp)
	ld	$17, PT_R17(sp)
	ld	$18, PT_R18(sp)
	ld	$19, PT_R19(sp)
	ld	$20, PT_R20(sp)
	ld	$21, PT_R21(sp)
	ld	$22, PT_R22(sp)
	ld	$23, PT_R23(sp)
	ld	$26, PT_R26(sp)
	ld	$27, PT_R27(sp)
	ld	$28, PT_R28(sp)
	ld	$30, PT_R30(sp)
	ld	$31, PT_R31(sp)

	jr	ra
	daddiu	sp, PT_SIZE
END(wakeup_start)
	.set mips0
