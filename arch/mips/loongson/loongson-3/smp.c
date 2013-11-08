/*
 * Copyright (C) 2010, 2011, 2012, Lemote, Inc.
 * Author: Chen Huacai, chenhc@lemote.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <linux/init.h>
#include <linux/cpu.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/cpufreq.h>
#include <asm/processor.h>
#include <asm/time.h>
#include <asm/clock.h>
#include <asm/tlbflush.h>
#include <loongson.h>

#include "smp.h"

DEFINE_PER_CPU(int, cpu_state);
DEFINE_PER_CPU(uint32_t, core0_c0count);

static void *ipi_set0_regs[16];
static void *ipi_clear0_regs[16];
static void *ipi_status0_regs[16];
static void *ipi_en0_regs[16];
static void *ipi_mailbox_buf[16];

#ifdef CONFIG_LOONGSON3_CPUAUTOPLUG
extern int autoplug_verbose;
#define verbose autoplug_verbose
#else
#define verbose 1
#endif

/* read a 64bit value from ipi register */
uint64_t loongson3_ipi_read64(void * addr)
{
	return *((volatile uint64_t *)addr);
};

/* write a 64bit value to ipi register */
void loongson3_ipi_write64(uint64_t action, void * addr)
{
	*((volatile uint64_t *)addr) = action;
	__wbflush();
};

/* read a 32bit value from ipi register */
uint32_t loongson3_ipi_read32(void * addr)
{
	return *((volatile uint32_t *)addr);
};

/* write a 32bit value to ipi register */
void loongson3_ipi_write32(uint32_t action, void * addr)
{
	*((volatile uint32_t *)addr) = action;
	__wbflush();
};

static void ipi_set0_regs_init(void)
{
	ipi_set0_regs[0] = (void *)(smp_core_group0_base + smp_core0_offset + SET0);
	ipi_set0_regs[1] = (void *)(smp_core_group0_base + smp_core1_offset + SET0);
	ipi_set0_regs[2] = (void *)(smp_core_group0_base + smp_core2_offset + SET0);
	ipi_set0_regs[3] = (void *)(smp_core_group0_base + smp_core3_offset + SET0);
	ipi_set0_regs[4] = (void *)(smp_core_group1_base + smp_core0_offset + SET0);
	ipi_set0_regs[5] = (void *)(smp_core_group1_base + smp_core1_offset + SET0);
	ipi_set0_regs[6] = (void *)(smp_core_group1_base + smp_core2_offset + SET0);
	ipi_set0_regs[7] = (void *)(smp_core_group1_base + smp_core3_offset + SET0);
	ipi_set0_regs[8] = (void *)(smp_core_group2_base + smp_core0_offset + SET0);
	ipi_set0_regs[9] = (void *)(smp_core_group2_base + smp_core1_offset + SET0);
	ipi_set0_regs[10] = (void *)(smp_core_group2_base + smp_core2_offset + SET0);
	ipi_set0_regs[11] = (void *)(smp_core_group2_base + smp_core3_offset + SET0);
	ipi_set0_regs[12] = (void *)(smp_core_group3_base + smp_core0_offset + SET0);
	ipi_set0_regs[13] = (void *)(smp_core_group3_base + smp_core1_offset + SET0);
	ipi_set0_regs[14] = (void *)(smp_core_group3_base + smp_core2_offset + SET0);
	ipi_set0_regs[15] = (void *)(smp_core_group3_base + smp_core3_offset + SET0);
}

static void ipi_clear0_regs_init(void)
{
	ipi_clear0_regs[0] = (void *)(smp_core_group0_base + smp_core0_offset + CLEAR0);
	ipi_clear0_regs[1] = (void *)(smp_core_group0_base + smp_core1_offset + CLEAR0);
	ipi_clear0_regs[2] = (void *)(smp_core_group0_base + smp_core2_offset + CLEAR0);
	ipi_clear0_regs[3] = (void *)(smp_core_group0_base + smp_core3_offset + CLEAR0);
	ipi_clear0_regs[4] = (void *)(smp_core_group1_base + smp_core0_offset + CLEAR0);
	ipi_clear0_regs[5] = (void *)(smp_core_group1_base + smp_core1_offset + CLEAR0);
	ipi_clear0_regs[6] = (void *)(smp_core_group1_base + smp_core2_offset + CLEAR0);
	ipi_clear0_regs[7] = (void *)(smp_core_group1_base + smp_core3_offset + CLEAR0);
	ipi_clear0_regs[8] = (void *)(smp_core_group2_base + smp_core0_offset + CLEAR0);
	ipi_clear0_regs[9] = (void *)(smp_core_group2_base + smp_core1_offset + CLEAR0);
	ipi_clear0_regs[10] = (void *)(smp_core_group2_base + smp_core2_offset + CLEAR0);
	ipi_clear0_regs[11] = (void *)(smp_core_group2_base + smp_core3_offset + CLEAR0);
	ipi_clear0_regs[12] = (void *)(smp_core_group3_base + smp_core0_offset + CLEAR0);
	ipi_clear0_regs[13] = (void *)(smp_core_group3_base + smp_core1_offset + CLEAR0);
	ipi_clear0_regs[14] = (void *)(smp_core_group3_base + smp_core2_offset + CLEAR0);
	ipi_clear0_regs[15] = (void *)(smp_core_group3_base + smp_core3_offset + CLEAR0);
}

static void ipi_status0_regs_init(void)
{
	ipi_status0_regs[0] = (void *)(smp_core_group0_base + smp_core0_offset + STATUS0);
	ipi_status0_regs[1] = (void *)(smp_core_group0_base + smp_core1_offset + STATUS0);
	ipi_status0_regs[2] = (void *)(smp_core_group0_base + smp_core2_offset + STATUS0);
	ipi_status0_regs[3] = (void *)(smp_core_group0_base + smp_core3_offset + STATUS0);
	ipi_status0_regs[4] = (void *)(smp_core_group1_base + smp_core0_offset + STATUS0);
	ipi_status0_regs[5] = (void *)(smp_core_group1_base + smp_core1_offset + STATUS0);
	ipi_status0_regs[6] = (void *)(smp_core_group1_base + smp_core2_offset + STATUS0);
	ipi_status0_regs[7] = (void *)(smp_core_group1_base + smp_core3_offset + STATUS0);
	ipi_status0_regs[8] = (void *)(smp_core_group2_base + smp_core0_offset + STATUS0);
	ipi_status0_regs[9] = (void *)(smp_core_group2_base + smp_core1_offset + STATUS0);
	ipi_status0_regs[10] = (void *)(smp_core_group2_base + smp_core2_offset + STATUS0);
	ipi_status0_regs[11] = (void *)(smp_core_group2_base + smp_core3_offset + STATUS0);
	ipi_status0_regs[12] = (void *)(smp_core_group3_base + smp_core0_offset + STATUS0);
	ipi_status0_regs[13] = (void *)(smp_core_group3_base + smp_core1_offset + STATUS0);
	ipi_status0_regs[14] = (void *)(smp_core_group3_base + smp_core2_offset + STATUS0);
	ipi_status0_regs[15] = (void *)(smp_core_group3_base + smp_core3_offset + STATUS0);
}

static void ipi_en0_regs_init(void)
{
	ipi_en0_regs[0] = (void *)(smp_core_group0_base + smp_core0_offset + EN0);
	ipi_en0_regs[1] = (void *)(smp_core_group0_base + smp_core1_offset + EN0);
	ipi_en0_regs[2] = (void *)(smp_core_group0_base + smp_core2_offset + EN0);
	ipi_en0_regs[3] = (void *)(smp_core_group0_base + smp_core3_offset + EN0);
	ipi_en0_regs[4] = (void *)(smp_core_group1_base + smp_core0_offset + EN0);
	ipi_en0_regs[5] = (void *)(smp_core_group1_base + smp_core1_offset + EN0);
	ipi_en0_regs[6] = (void *)(smp_core_group1_base + smp_core2_offset + EN0);
	ipi_en0_regs[7] = (void *)(smp_core_group1_base + smp_core3_offset + EN0);
	ipi_en0_regs[8] = (void *)(smp_core_group2_base + smp_core0_offset + EN0);
	ipi_en0_regs[9] = (void *)(smp_core_group2_base + smp_core1_offset + EN0);
	ipi_en0_regs[10] = (void *)(smp_core_group2_base + smp_core2_offset + EN0);
	ipi_en0_regs[11] = (void *)(smp_core_group2_base + smp_core3_offset + EN0);
	ipi_en0_regs[12] = (void *)(smp_core_group3_base + smp_core0_offset + EN0);
	ipi_en0_regs[13] = (void *)(smp_core_group3_base + smp_core1_offset + EN0);
	ipi_en0_regs[14] = (void *)(smp_core_group3_base + smp_core2_offset + EN0);
	ipi_en0_regs[15] = (void *)(smp_core_group3_base + smp_core3_offset + EN0);
}

static void ipi_mailbox_buf_init(void)
{
	ipi_mailbox_buf[0] = (void *)(smp_core_group0_base + smp_core0_offset + BUF);
	ipi_mailbox_buf[1] = (void *)(smp_core_group0_base + smp_core1_offset + BUF);
	ipi_mailbox_buf[2] = (void *)(smp_core_group0_base + smp_core2_offset + BUF);
	ipi_mailbox_buf[3] = (void *)(smp_core_group0_base + smp_core3_offset + BUF);
	ipi_mailbox_buf[4] = (void *)(smp_core_group1_base + smp_core0_offset + BUF);
	ipi_mailbox_buf[5] = (void *)(smp_core_group1_base + smp_core1_offset + BUF);
	ipi_mailbox_buf[6] = (void *)(smp_core_group1_base + smp_core2_offset + BUF);
	ipi_mailbox_buf[7] = (void *)(smp_core_group1_base + smp_core3_offset + BUF);
	ipi_mailbox_buf[8] = (void *)(smp_core_group2_base + smp_core0_offset + BUF);
	ipi_mailbox_buf[9] = (void *)(smp_core_group2_base + smp_core1_offset + BUF);
	ipi_mailbox_buf[10] = (void *)(smp_core_group2_base + smp_core2_offset + BUF);
	ipi_mailbox_buf[11] = (void *)(smp_core_group2_base + smp_core3_offset + BUF);
	ipi_mailbox_buf[12] = (void *)(smp_core_group3_base + smp_core0_offset + BUF);
	ipi_mailbox_buf[13] = (void *)(smp_core_group3_base + smp_core1_offset + BUF);
	ipi_mailbox_buf[14] = (void *)(smp_core_group3_base + smp_core2_offset + BUF);
	ipi_mailbox_buf[15] = (void *)(smp_core_group3_base + smp_core3_offset + BUF);
}

/*
 * Simple enough, just poke the appropriate ipi register
 */
static void loongson3_send_ipi_single(int cpu, unsigned int action)
{
	loongson3_ipi_write32((u32)action, ipi_set0_regs[cpu]);
}

static void loongson3_send_ipi_mask(const struct cpumask *mask, unsigned int action)
{
	unsigned int i;

	for_each_cpu(i, mask)
		loongson3_ipi_write32((u32)action, ipi_set0_regs[i]);
}

void loongson3_ipi_interrupt(struct pt_regs *regs)
{
	int i, cpu = smp_processor_id();
	unsigned int action, c0count;

	/* Load the ipi register to figure out what we're supposed to do */
	action = loongson3_ipi_read32(ipi_status0_regs[cpu]);

	/* Clear the ipi register to clear the interrupt */
	loongson3_ipi_write32((u32)action, ipi_clear0_regs[cpu]);

	if (action & SMP_RESCHEDULE_YOURSELF) {
		scheduler_ipi();
	}

	if (action & SMP_CALL_FUNCTION) {
		smp_call_function_interrupt();
	}

	if (action & SMP_ASK_C0COUNT) {
		BUG_ON(cpu != 0);
		c0count = read_c0_count();
		for (i=1; i < nr_cpus_loongson; i++)
			per_cpu(core0_c0count, i) = c0count;
	}
}

#define MAX_LOOPS 1250
/*
 * SMP init and finish on secondary CPUs
 */
void __cpuinit loongson3_init_secondary(void)
{
	int i;
	uint32_t initcount;
	unsigned int cpu = smp_processor_id();
	unsigned int imask = STATUSF_IP7 | STATUSF_IP6 |
			     STATUSF_IP3 | STATUSF_IP2;

	/* Set interrupt mask, but don't enable */
	change_c0_status(ST0_IM, imask);

	for (i = 0; i < nr_cpus_loongson; i++) {
		loongson3_ipi_write32(0xffffffff, ipi_en0_regs[i]);
	}

	cpu_data[cpu].package = cpu / cores_per_package;
	cpu_data[cpu].core = cpu % cores_per_package;
	per_cpu(cpu_state, cpu) = CPU_ONLINE;

	i = 0;
	__get_cpu_var(core0_c0count) = 0;
	loongson3_send_ipi_single(0, SMP_ASK_C0COUNT);
	while (!__get_cpu_var(core0_c0count)) {
		i++;
		cpu_relax();
	}

	if (i > MAX_LOOPS)
		i = MAX_LOOPS;
	initcount = __get_cpu_var(core0_c0count) + i;
	write_c0_count(initcount);
}

void __cpuinit loongson3_smp_finish(void)
{
	write_c0_compare(read_c0_count() + mips_hpt_frequency/HZ);
	local_irq_enable();
	loongson3_ipi_write64(0, (void *)(ipi_mailbox_buf[smp_processor_id()]+0x0));
	if (verbose || system_state == SYSTEM_BOOTING)
		printk(KERN_INFO "CPU#%d finished, CP0_ST=%x\n",
			smp_processor_id(), read_c0_status());
}

void __init loongson3_smp_setup(void)
{
	int i, num;

	init_cpu_possible(cpu_none_mask);
	set_cpu_possible(0, true);

	__cpu_number_map[0] = 0;
	__cpu_logical_map[0] = 0;

	/* For unified kernel, NR_CPUS is the maximum possible value,
	 * nr_cpus_loongson is the really present value */
	for (i = 1, num = 0; i < nr_cpus_loongson; i++) {
		set_cpu_possible(i, true);
		__cpu_number_map[i] = ++num;
		__cpu_logical_map[num] = i;
	}
	ipi_set0_regs_init();
	ipi_clear0_regs_init();
	ipi_status0_regs_init();
	ipi_en0_regs_init();
	ipi_mailbox_buf_init();
	printk(KERN_INFO "Detected %i available secondary CPU(s)\n", num);
}

void __init loongson3_prepare_cpus(unsigned int max_cpus)
{
	init_cpu_present(cpu_possible_mask);
	per_cpu(cpu_state, smp_processor_id()) = CPU_ONLINE;
}

/*
 * Setup the PC, SP, and GP of a secondary processor and start it runing!
 */
void __cpuinit loongson3_boot_secondary(int cpu, struct task_struct *idle)
{
	volatile unsigned long startargs[4];

	if (verbose || system_state == SYSTEM_BOOTING)
		printk(KERN_INFO "Booting CPU#%d...\n", cpu);

	/* startargs[] are initial PC, SP and GP for secondary CPU */
	startargs[0] = (unsigned long)&smp_bootstrap;
	startargs[1] = (unsigned long)__KSTK_TOS(idle);
	startargs[2] = (unsigned long)task_thread_info(idle);
	startargs[3] = 0;

	if (verbose || system_state == SYSTEM_BOOTING)
		printk(KERN_DEBUG "CPU#%d, func_pc=%lx, sp=%lx, gp=%lx\n",
			cpu, startargs[0], startargs[1], startargs[2]);

	loongson3_ipi_write64(startargs[3], (void *)(ipi_mailbox_buf[cpu]+0x18));
	loongson3_ipi_write64(startargs[2], (void *)(ipi_mailbox_buf[cpu]+0x10));
	loongson3_ipi_write64(startargs[1], (void *)(ipi_mailbox_buf[cpu]+0x8));
	loongson3_ipi_write64(startargs[0], (void *)(ipi_mailbox_buf[cpu]+0x0));
}

/*
 * Final cleanup after all secondaries booted
 */
void __init loongson3_cpus_done(void)
{
}

#ifdef CONFIG_HOTPLUG_CPU

extern void fixup_irqs(void);
extern void (*flush_cache_all)(void);

static int loongson3_cpu_disable(void)
{
	unsigned long flags;
	unsigned int cpu = smp_processor_id();

	if (cpu == 0)
		return -EBUSY;

	set_cpu_online(cpu, false);
	cpu_clear(cpu, cpu_callin_map);
	local_irq_save(flags);
	fixup_irqs();
	local_irq_restore(flags);
	flush_cache_all();
	local_flush_tlb_all();

	return 0;
}


static void loongson3_cpu_die(unsigned int cpu)
{
	while (per_cpu(cpu_state, cpu) != CPU_DEAD)
		cpu_relax();

	mb();
}

/* To shutdown a core in Loongson 3, the target core should go to CKSEG1 and
 * flush all L1 entries at first. Then, another core (usually Core 0) can
 * safely disable the clock of the target core. loongson3_play_dead() is
 * called via CKSEG1 (uncached and unmmaped) */
void loongson3a_play_dead(int *state_addr)
{
	__asm__ __volatile__(
		"      .set push                         \n"
		"      .set noreorder                    \n"
		"      li $t0, 0x80000000                \n" /* KSEG0 */
		"      li $t1, 512                       \n" /* num of L1 entries */
		"ls3a_flush_loop:                        \n" /* flush L1 */
		"      cache 0, 0($t0)                   \n" /* ICache */
		"      cache 0, 1($t0)                   \n"
		"      cache 0, 2($t0)                   \n"
		"      cache 0, 3($t0)                   \n"
		"      cache 1, 0($t0)                   \n" /* DCache */
		"      cache 1, 1($t0)                   \n"
		"      cache 1, 2($t0)                   \n"
		"      cache 1, 3($t0)                   \n"
		"      addiu $t0, $t0, 0x20              \n"
		"      bnez  $t1, ls3a_flush_loop        \n"
		"      addiu $t1, $t1, -1                \n"
		"      li    $t0, 0x7                    \n" /* *state_addr = CPU_DEAD; */
		"      sw    $t0, 0($a0)                 \n"
		"      sync                              \n"
		"      cache 21, 0($a0)                  \n" /* flush entry of *state_addr */
		"      .set pop                          \n");

	__asm__ __volatile__(
		"      .set push                         \n"
		"      .set noreorder                    \n"
		"      .set mips64                       \n"
		"      mfc0  $t2, $15, 1                 \n"
		"      andi  $t2, 0x3ff                  \n"
		"      .set mips3                        \n"
		"      dli   $t0, 0x900000003ff01000     \n"
		"      andi  $t3, $t2, 0x3               \n"
		"      sll   $t3, 8                      \n"  /* get cpu id */
		"      or    $t0, $t0, $t3               \n"
		"      andi  $t1, $t2, 0xc               \n"
		"      dsll  $t1, 42                     \n"  /* get node id */
		"      or    $t0, $t0, $t1               \n"
		"ls3a_wait_for_init:                     \n"
		"      li    $a0, 0x100                  \n"
		"ls3a_idle_loop:                         \n"
		"      bnez  $a0, ls3a_idle_loop         \n"
		"      addiu $a0, -1                     \n"
		"      lw    $v0, 0x20($t0)              \n"  /* get PC via mailbox */
		"      nop                               \n"
		"      beqz  $v0, ls3a_wait_for_init     \n"
		"      nop                               \n"
		"      ld    $sp, 0x28($t0)              \n"  /* get SP via mailbox */
		"      nop                               \n"
		"      ld    $gp, 0x30($t0)              \n"  /* get GP via mailbox */
		"      nop                               \n"
		"      ld    $a1, 0x38($t0)              \n"
		"      nop                               \n"
		"      jr  $v0                           \n"  /* jump to initial PC */
		"      nop                               \n"
		"      .set pop                          \n");
}

void loongson3b_play_dead(int *state_addr)
{
	__asm__ __volatile__(
		"      .set push                         \n"
		"      .set noreorder                    \n"
		"      li $t0, 0x80000000                \n" /* KSEG0 */
		"      li $t1, 512                       \n" /* num of L1 entries */
		"ls3b_flush_loop:                        \n" /* flush L1 */
		"      cache 0, 0($t0)                   \n" /* ICache */
		"      cache 0, 1($t0)                   \n"
		"      cache 0, 2($t0)                   \n"
		"      cache 0, 3($t0)                   \n"
		"      cache 1, 0($t0)                   \n" /* DCache */
		"      cache 1, 1($t0)                   \n"
		"      cache 1, 2($t0)                   \n"
		"      cache 1, 3($t0)                   \n"
		"      addiu $t0, $t0, 0x20              \n"
		"      bnez  $t1, ls3b_flush_loop        \n"
		"      addiu $t1, $t1, -1                \n"
		"      li    $t0, 0x7                    \n" /* *state_addr = CPU_DEAD; */
		"      sw    $t0, 0($a0)                 \n"
		"      sync                              \n"
		"      cache 21, 0($a0)                  \n" /* flush entry of *state_addr */
		"      .set pop                          \n");

	__asm__ __volatile__(
		"      .set push                         \n"
		"      .set noreorder                    \n"
		"      .set mips64                       \n"
		"      mfc0  $t2, $15, 1                 \n"
		"      andi  $t2, 0x3ff                  \n"
		"      .set mips3                        \n"
		"      dli   $t0, 0x900000003ff01000     \n"
		"      andi  $t3, $t2, 0x3               \n"
		"      sll   $t3, 8                      \n"  /* get cpu id */
		"      or    $t0, $t0, $t3               \n"
		"      andi  $t1, $t2, 0xc               \n"
		"      dsll  $t1, 42                     \n"  /* get node id */
		"      or    $t0, $t0, $t1               \n"
		"      dsrl  $t1, 30                     \n"  /* 15:14 */
		"      or    $t0, $t0, $t1               \n"
		"ls3b_wait_for_init:                     \n"
		"      li    $a0, 0x100                  \n"
		"ls3b_idle_loop:                         \n"
		"      bnez  $a0, ls3b_idle_loop         \n"
		"      addiu $a0, -1                     \n"
		"      lw    $v0, 0x20($t0)              \n"  /* get PC via mailbox */
		"      nop                               \n"
		"      beqz  $v0, ls3b_wait_for_init     \n"
		"      nop                               \n"
		"      ld    $sp, 0x28($t0)              \n"  /* get SP via mailbox */
		"      nop                               \n"
		"      ld    $gp, 0x30($t0)              \n"  /* get GP via mailbox */
		"      nop                               \n"
		"      ld    $a1, 0x38($t0)              \n"
		"      nop                               \n"
		"      jr  $v0                           \n"  /* jump to initial PC */
		"      nop                               \n"
		"      .set pop                          \n");
}

void play_dead(void)
{
	int *state_addr;
	unsigned int cpu = smp_processor_id();
	void (*play_dead_at_ckseg1)(int *);

	idle_task_exit();
	switch (cputype) {
	case Loongson_3A:
	default:
		play_dead_at_ckseg1 = (void *)CKSEG1ADDR((unsigned long)loongson3a_play_dead);
		break;
	case Loongson_3B:
		play_dead_at_ckseg1 = (void *)CKSEG1ADDR((unsigned long)loongson3b_play_dead);
		break;
	}
	state_addr = &per_cpu(cpu_state, cpu);
	mb();
	play_dead_at_ckseg1(state_addr);
}

void loongson3_disable_clock(int cpu)
{
	unsigned int node = cpu / cores_per_node;
	unsigned int core_id = cpu % cores_per_node;

	if (cputype == Loongson_3A) {
		if (node == 0)
			LOONGSON_CHIPCFG0 &= ~(1 << (12 + core_id));
#ifdef CONFIG_NUMA
		else if (node == 1)
			LOONGSON_CHIPCFG1 &= ~(1 << (12 + core_id));
		else if (node == 2)
			LOONGSON_CHIPCFG2 &= ~(1 << (12 + core_id));
		else if (node == 3)
			LOONGSON_CHIPCFG3 &= ~(1 << (12 + core_id));
#endif
	}
	else if(cputype == Loongson_3B) {
		if (!cpuhotplug_workaround)
			LOONGSON_FREQCTRL &= ~(1 << (cpu * 4 + 3));
	}
}

void loongson3_enable_clock(int cpu)
{
	unsigned int node = cpu / cores_per_node;
	unsigned int core_id = cpu % cores_per_node;

	if (cputype == Loongson_3A) {
		if (node == 0)
			LOONGSON_CHIPCFG0 |= 1 << (12 + core_id);
#ifdef CONFIG_NUMA
		else if (node == 1)
			LOONGSON_CHIPCFG1 |= 1 << (12 + core_id);
		else if (node == 2)
			LOONGSON_CHIPCFG2 |= 1 << (12 + core_id);
		else if (node == 3)
			LOONGSON_CHIPCFG3 |= 1 << (12 + core_id);
#endif
	}
	else if(cputype == Loongson_3B){
		if (!cpuhotplug_workaround)
			LOONGSON_FREQCTRL |= 1 << (cpu * 4 + 3);
	}
}

#define CPU_POST_DEAD_FROZEN	(CPU_POST_DEAD | CPU_TASKS_FROZEN)
static int __cpuinit loongson3_cpu_callback(struct notifier_block *nfb,
	unsigned long action, void *hcpu)
{
	unsigned int cpu = (unsigned long)hcpu;

	switch (action) {
	case CPU_POST_DEAD:
	case CPU_POST_DEAD_FROZEN:
		if (verbose || system_state == SYSTEM_BOOTING)
			printk(KERN_INFO "Disable clock for CPU#%d\n", cpu);
		loongson3_disable_clock(cpu);
		break;
	case CPU_UP_PREPARE:
	case CPU_UP_PREPARE_FROZEN:
		if (verbose || system_state == SYSTEM_BOOTING)
			printk(KERN_INFO "Enable clock for CPU#%d\n", cpu);
		loongson3_enable_clock(cpu);
		break;
	}

	return NOTIFY_OK;
}

static int __cpuinit register_loongson3_notifier(void)
{
	hotcpu_notifier(loongson3_cpu_callback, 0);
	return 0;
}
early_initcall(register_loongson3_notifier);

#endif

struct plat_smp_ops loongson3_smp_ops = {
	.send_ipi_single = loongson3_send_ipi_single,
	.send_ipi_mask = loongson3_send_ipi_mask,
	.init_secondary = loongson3_init_secondary,
	.smp_finish = loongson3_smp_finish,
	.cpus_done = loongson3_cpus_done,
	.boot_secondary = loongson3_boot_secondary,
	.smp_setup = loongson3_smp_setup,
	.prepare_cpus = loongson3_prepare_cpus,
#ifdef CONFIG_HOTPLUG_CPU
	.cpu_disable = loongson3_cpu_disable,
	.cpu_die = loongson3_cpu_die,
#endif
};
