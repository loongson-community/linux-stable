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
	.extern loongson_nr_nodes
	.extern loongson_suspend_addr
	.extern loongson_pcache_ways
	.extern loongson_pcache_sets
	.extern loongson_pcache_linesz
	.extern loongson_scache_ways
	.extern loongson_scache_sets
	.extern loongson_scache_linesz

	.text
	.set push
	.set noat
	.set reorder
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
	mfc0	k0, CP0_CONFIG, 0
	sw	k0, PT_R9(sp)
	mfc0	k0, CP0_PAGEMASK, 0  /* Loongson-3 has sub-registers */
	sw	k0, PT_R10(sp)
	mfc0	k0, CP0_PAGEMASK, 1  /* Loongson-3 has sub-registers */
	sw	k0, PT_R11(sp)
	dmfc0	k0, CP0_CONTEXT
	sd	k0, PT_R12(sp)
	dmfc0	k0, CP0_XCONTEXT
	sd	k0, PT_R13(sp)

	/* Now set up the "wakeup vector" in  RTC space so the boot rom will
	 * return to this point upon wakeup.
	 * 0x40 : RA,  0x48 : SP
	 */
	daddi	a0, sp, 0
	li      a1, 0x48
	jal     cmos_write64
	dla	a0, wakeup_start	/* resume path */
	li      a1, 0x40
	jal     cmos_write64
.endm

/* sleep code for Loongson-3A memory controller type */
LEAF(loongson_suspend_lowlevel)
	SETUP_SLEEP

	/* a0:address a1:L1_sets a2:L1_ways a3:L1_linesize */
	li a0, 0x80000000
	lw a1, loongson_pcache_sets
	lw a2, loongson_pcache_ways
	lw a3, loongson_pcache_linesz
flushL1:
	move  t0, a2
1:	cache 0, (a0)
	cache 1, (a0)
	addiu a0, a0, 1
	addiu t0, t0, -1
	bnez  t0, 1b
	subu  a0, a0, a2
	addu  a0, a0, a3
	addiu a1, a1, -1
	bnez  a1, flushL1

	/* a0:nr_nodes a1:address a2:L2_sets a3:L2_ways t8:L2_linesize */
	lw  a0, loongson_nr_nodes
	dli a1, 0x9800000000000000
	lw  a3, loongson_scache_ways
	lw  t8, loongson_scache_linesz
flushL2_all:
	lw  a2, loongson_scache_sets
	dli t9, 0x100000000000
flushL2_node:
	move   t0, a3
1:	cache  3, (a1)
	daddiu a1, a1, 1
	addiu  t0, t0, -1
	bnez   t0, 1b
	dsubu  a1, a1, a3
	daddu  a1, a1, t8
	addiu  a2, a2, -1
	bnez   a2, flushL2_node
	daddu  a1, a1, t9
	addiu  a0, a0, -1
	bnez   a0, flushL2_all

	ld  v0, loongson_suspend_addr /* Call BIOS's STR sleep routine */
	jr  v0
	nop
END(loongson_suspend_lowlevel)

	/* This is where we return upon wakeup.
	 * Reload all of the registers and return.
	 */
LEAF(wakeup_start)
	lw	k0, PT_R8(sp)
	mtc0	k0, CP0_STATUS
	lw	k0, PT_R9(sp)
	mtc0	k0, CP0_CONFIG, 0
	lw	k0, PT_R10(sp)
	mtc0	k0, CP0_PAGEMASK, 0
	lw	k0, PT_R11(sp)
	mtc0	k0, CP0_PAGEMASK, 1
	ld	k0, PT_R12(sp)
	dmtc0	k0, CP0_CONTEXT
	ld	k0, PT_R13(sp)
	dmtc0	k0, CP0_XCONTEXT

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

	daddiu	sp, PT_SIZE
	jr	ra
END(wakeup_start)

	.set pop
