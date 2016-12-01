/*
 * loongson-specific suspend support
 *
 *  Copyright (C) 2009 - 2012 Lemote Inc.
 *  Author: Wu Zhangjin <wuzhangjin@gmail.com>
 *          Huacai Chen <chenhc@lemote.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <linux/suspend.h>
#include <linux/interrupt.h>
#include <linux/pm.h>

#include <asm/i8259.h>
#include <asm/mipsregs.h>
#include <asm/bootinfo.h>

#include <loongson.h>
#include <mc146818rtc.h>

static unsigned int __maybe_unused cached_master_mask;	/* i8259A */
static unsigned int __maybe_unused cached_slave_mask;
static unsigned int __maybe_unused cached_bonito_irq_mask; /* bonito */
static unsigned int __maybe_unused cached_autoplug_enabled;

u32 loongson_nr_nodes;
u64 loongson_suspend_addr;
u32 loongson_pcache_ways;
u32 loongson_scache_ways;
u32 loongson_pcache_sets;
u32 loongson_scache_sets;
u32 loongson_pcache_linesz;
u32 loongson_scache_linesz;

uint64_t cmos_read64(unsigned long addr)
{
	unsigned char bytes[8];
	int i;

	for (i=0; i<8; i++)
		bytes[i] = CMOS_READ(addr + i);

	return *(uint64_t *)bytes;
}

void cmos_write64(uint64_t data, unsigned long addr)
{
	int i;
	unsigned char * bytes = (unsigned char *)&data;

	for (i=0; i<8; i++)
		CMOS_WRITE(bytes[i], addr + i);
}
void arch_suspend_disable_irqs(void)
{
	/* disable all mips events */
	local_irq_disable();

#ifdef CONFIG_I8259
	/* disable all events of i8259A */
	cached_slave_mask = inb(PIC_SLAVE_IMR);
	cached_master_mask = inb(PIC_MASTER_IMR);

	outb(0xff, PIC_SLAVE_IMR);
	inb(PIC_SLAVE_IMR);
	outb(0xff, PIC_MASTER_IMR);
	inb(PIC_MASTER_IMR);
#endif
	/* disable all events of bonito */
	cached_bonito_irq_mask = LOONGSON_INTEN;
	LOONGSON_INTENCLR = 0xffff;
	(void)LOONGSON_INTENCLR;
}

void arch_suspend_enable_irqs(void)
{
	/* enable all mips events */
	local_irq_enable();
#ifdef CONFIG_I8259
	/* only enable the cached events of i8259A */
	outb(cached_slave_mask, PIC_SLAVE_IMR);
	outb(cached_master_mask, PIC_MASTER_IMR);
#endif
	/* enable all cached events of bonito */
	LOONGSON_INTENSET = cached_bonito_irq_mask;
	(void)LOONGSON_INTENSET;
}

/*
 * Setup the board-specific events for waking up loongson from wait mode
 */
void __weak setup_wakeup_events(void)
{
}

/*
 * Check wakeup events
 */
int __weak wakeup_loongson(void)
{
	return 1;
}

/*
 * If the events are really what we want to wakeup the CPU, wake it up
 * otherwise put the CPU asleep again.
 */
static void wait_for_wakeup_events(void)
{
	while (!wakeup_loongson())
		switch (read_c0_prid() & PRID_REV_MASK) {
		case PRID_REV_LOONGSON2E:
		case PRID_REV_LOONGSON2F:
		case PRID_REV_LOONGSON3A_R1:
		default:
			LOONGSON_CHIPCFG(0) &= ~0x7;
			break;
		case PRID_REV_LOONGSON3A_R2:
		case PRID_REV_LOONGSON3B_R1:
		case PRID_REV_LOONGSON3B_R2:
			LOONGSON_FREQCTRL(0) &= ~0x7;
			break;
		}
}

/*
 * Stop all perf counters
 *
 * $24 is the control register of Loongson perf counter
 */
static inline void stop_perf_counters(void)
{
	switch (read_c0_prid() & PRID_REV_MASK) {
	case PRID_REV_LOONGSON2E:
	case PRID_REV_LOONGSON2F:
		__write_64bit_c0_register($24, 0, 0);
		break;
	case PRID_REV_LOONGSON3A_R1:
	case PRID_REV_LOONGSON3B_R1:
	case PRID_REV_LOONGSON3B_R2:
		__write_64bit_c0_register($25, 0, 0xc0000000);
		__write_64bit_c0_register($25, 2, 0x40000000);
		break;
	case PRID_REV_LOONGSON3A_R2:
		__write_64bit_c0_register($25, 0, 0xc0000000);
		__write_64bit_c0_register($25, 2, 0xc0000000);
		__write_64bit_c0_register($25, 4, 0xc0000000);
		__write_64bit_c0_register($25, 6, 0x40000000);
		break;
	}
}


static void loongson_suspend_enter(void)
{
	static unsigned int cached_cpu_freq;

	/* setup wakeup events via enabling the IRQs */
	setup_wakeup_events();

	stop_perf_counters();

	switch (read_c0_prid() & PRID_REV_MASK) {
	case PRID_REV_LOONGSON2E:
	case PRID_REV_LOONGSON2F:
	case PRID_REV_LOONGSON3A_R1:
		cached_cpu_freq = LOONGSON_CHIPCFG(0);
		/* Put CPU into wait mode */
		LOONGSON_CHIPCFG(0) &= ~0x7;
		/* wait for the given events to wakeup cpu from wait mode */
		wait_for_wakeup_events();
		LOONGSON_CHIPCFG(0) = cached_cpu_freq;
		break;
	case PRID_REV_LOONGSON3A_R2:
	case PRID_REV_LOONGSON3B_R1:
	case PRID_REV_LOONGSON3B_R2:
		cached_cpu_freq = LOONGSON_FREQCTRL(0);
		/* Put CPU into wait mode */
		LOONGSON_FREQCTRL(0) &= ~0x7;
		/* wait for the given events to wakeup cpu from wait mode */
		wait_for_wakeup_events();
		LOONGSON_FREQCTRL(0) = cached_cpu_freq;
		break;
	}
	mmiowb();
}

void __weak mach_suspend(suspend_state_t state)
{
}

void __weak mach_resume(suspend_state_t state)
{
}

static int loongson_pm_enter(suspend_state_t state)
{
	mach_suspend(state);

	/* processor specific suspend */
	switch(state){
	case PM_SUSPEND_STANDBY:
		loongson_suspend_enter();
		break;
	case PM_SUSPEND_MEM:
#ifdef CONFIG_CPU_LOONGSON3
		loongson_nr_nodes = loongson_sysconf.nr_nodes;
		loongson_suspend_addr = loongson_sysconf.suspend_addr;
		loongson_pcache_ways = cpu_data[0].dcache.ways;
		loongson_scache_ways = cpu_data[0].scache.ways;
		loongson_pcache_sets = cpu_data[0].dcache.sets;
		loongson_scache_sets = cpu_data[0].scache.sets*4;
		loongson_pcache_linesz = cpu_data[0].dcache.linesz;
		loongson_scache_linesz = cpu_data[0].scache.linesz;
		loongson_suspend_lowlevel();
		cmos_write64(0x0, 0x40);  /* clear pc in cmos */
		cmos_write64(0x0, 0x48);  /* clear sp in cmos */
		pm_set_resume_via_firmware();
#else
		loongson_suspend_enter();
#endif
		break;
	}

	mach_resume(state);

	return 0;
}

static int loongson_pm_valid_state(suspend_state_t state)
{
	switch (state) {
	case PM_SUSPEND_ON:
		return 1;

	case PM_SUSPEND_STANDBY:
	case PM_SUSPEND_MEM:
		switch (mips_machtype) {
		case MACH_LEMOTE_ML2F7:
		case MACH_LEMOTE_YL2F89:
			return 1;
		case MACH_LOONGSON_GENERIC:
			return !!loongson_sysconf.suspend_addr;
		default:
			return 0;
		}

	default:
		return 0;
	}
}

static int loongson_pm_begin(suspend_state_t state)
{
#ifdef CONFIG_LOONGSON3_CPUAUTOPLUG
	extern int autoplug_enabled;
	cached_autoplug_enabled = autoplug_enabled;
	autoplug_enabled = 0;
#endif
	if (state == PM_SUSPEND_MEM)
		pm_set_suspend_via_firmware();

	return 0;
}

static void loongson_pm_wake(void)
{
#ifdef CONFIG_CPU_LOONGSON3
	disable_unused_cpus();
#endif
}

static void loongson_pm_end(void)
{
#ifdef CONFIG_LOONGSON3_CPUAUTOPLUG
	extern int autoplug_enabled;
	autoplug_enabled = cached_autoplug_enabled;
#endif
}

static const struct platform_suspend_ops loongson_pm_ops = {
	.valid	= loongson_pm_valid_state,
	.begin	= loongson_pm_begin,
	.enter	= loongson_pm_enter,
	.wake	= loongson_pm_wake,
	.end	= loongson_pm_end,
};

static int __init loongson_pm_init(void)
{
	suspend_set_ops(&loongson_pm_ops);

	return 0;
}
arch_initcall(loongson_pm_init);
