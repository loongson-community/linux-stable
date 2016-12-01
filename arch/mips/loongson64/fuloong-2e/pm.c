/*
 *  Lemote loongson2e family machines' specific suspend support
 *
 *  Copyright (C) 2009 Lemote Inc.
 *  Author: Wu Zhangjin <wuzhangjin@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/suspend.h>
#include <linux/interrupt.h>
#include <linux/pm.h>
#include <linux/export.h>

#include <asm/mipsregs.h>
#include <asm/bootinfo.h>

#include <loongson.h>

/*
 * Stop all perf counters
 *
 * $24 is the control register of Loongson perf counter
 */
static inline void stop_perf_counters(void)
{
	__write_64bit_c0_register($24, 0, 0);
}

void loongson_suspend_enter(void)
{
	static unsigned int cached_cpu_freq;

	stop_perf_counters();

	cached_cpu_freq = LOONGSON_CHIPCFG(0);

	/* Put CPU into wait mode */
	LOONGSON_CHIPCFG(0) &= ~0x7;

	/* wait for events to wakeup cpu from wait mode */
	LOONGSON_CHIPCFG(0) = cached_cpu_freq;
	mmiowb();
}

void mach_suspend(void)
{
}

void mach_resume(void)
{
}
