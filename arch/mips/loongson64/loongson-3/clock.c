/*
 * Copyright (C) 2008 - 2014 Lemote Inc.
 * Author: Yan Hua, yanh@lemote.com
 *         Chen Huacai, chenhc@lemote.com
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#include <linux/module.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/cpufreq.h>
#include <linux/platform_device.h>

#include <asm/clock.h>

#include <loongson.h>

static LIST_HEAD(clock_list);
static DEFINE_SPINLOCK(clock_lock);
static DEFINE_MUTEX(clock_list_sem);

/* Minimum CLK support */
enum {
	DC_ZERO, DC_25PT = 2, DC_37PT, DC_50PT, DC_62PT, DC_75PT,
	DC_87PT, DC_DISABLE, DC_RESV
};

struct cpufreq_frequency_table loongson3_clockmod_table[] = {
	{0, DC_RESV, CPUFREQ_ENTRY_INVALID},
	{0, DC_ZERO, CPUFREQ_ENTRY_INVALID},
	{0, DC_25PT, 0},
	{0, DC_37PT, 0},
	{0, DC_50PT, 0},
	{0, DC_62PT, 0},
	{0, DC_75PT, 0},
	{0, DC_87PT, 0},
	{0, DC_DISABLE, 0},
	{0, DC_RESV, CPUFREQ_TABLE_END},
};
EXPORT_SYMBOL_GPL(loongson3_clockmod_table);

static struct clk cpu_clks[NR_CPUS];
static char clk_names[NR_CPUS][10];

struct clk *cpu_clk_get(int cpu)
{
	return &cpu_clks[cpu];
}

struct clk *clk_get(struct device *dev, const char *id)
{
	int i;
	struct clk *clk;

	if (!id)
		return NULL;

	for_each_possible_cpu(i) {
		clk = &cpu_clks[i];
		if (strcmp(clk->name, id) == 0)
			return clk;
	}

	return NULL;
}
EXPORT_SYMBOL(clk_get);

static void propagate_rate(struct clk *clk)
{
	struct clk *clkp;

	list_for_each_entry(clkp, &clock_list, node) {
		if (likely(clkp->parent != clk))
			continue;
		if (likely(clkp->ops && clkp->ops->recalc))
			clkp->ops->recalc(clkp);
		if (unlikely(clkp->flags & CLK_RATE_PROPAGATES))
			propagate_rate(clkp);
	}
}

int clk_enable(struct clk *clk)
{
	return 0;
}
EXPORT_SYMBOL(clk_enable);

void clk_disable(struct clk *clk)
{
}
EXPORT_SYMBOL(clk_disable);

unsigned long clk_get_rate(struct clk *clk)
{
	if (!clk)
		return 0;

	return (unsigned long)clk->rate;
}
EXPORT_SYMBOL(clk_get_rate);

void clk_put(struct clk *clk)
{
}
EXPORT_SYMBOL(clk_put);

int clk_set_rate(struct clk *clk, unsigned long rate)
{
	int regval, ret = 0;
	struct cpufreq_frequency_table *pos;
	int cpu = clk - cpu_clks;
	uint64_t core_id = cpu_core(&cpu_data[cpu]);
	uint64_t package_id = cpu_data[cpu].package;

	if (likely(clk->ops && clk->ops->set_rate)) {
		unsigned long flags;

		spin_lock_irqsave(&clock_lock, flags);
		ret = clk->ops->set_rate(clk, rate, 0);
		spin_unlock_irqrestore(&clock_lock, flags);
	}

	if (unlikely(clk->flags & CLK_RATE_PROPAGATES))
		propagate_rate(clk);

	cpufreq_for_each_valid_entry(pos, loongson3_clockmod_table)
		if (rate == pos->frequency)
			break;
	if (rate != pos->frequency)
		return -ENOTSUPP;

	clk->rate = rate;

	if ((read_c0_prid() & 0xf) == PRID_REV_LOONGSON3A_R1) {
		regval = LOONGSON_CHIPCFG(package_id);
		regval = (regval & ~0x7) | (pos->driver_data - 1);
		LOONGSON_CHIPCFG(package_id) = regval;
	} else {
		regval = LOONGSON_FREQCTRL(package_id);
		regval = (regval & ~(0x7 << (core_id*4))) |
			((pos->driver_data - 1) << (core_id*4));
		LOONGSON_FREQCTRL(package_id) = regval;
	}

	return ret;
}
EXPORT_SYMBOL_GPL(clk_set_rate);

long clk_round_rate(struct clk *clk, unsigned long rate)
{
	if (likely(clk->ops && clk->ops->round_rate)) {
		unsigned long flags, rounded;

		spin_lock_irqsave(&clock_lock, flags);
		rounded = clk->ops->round_rate(clk, rate);
		spin_unlock_irqrestore(&clock_lock, flags);

		return rounded;
	}

	return rate;
}
EXPORT_SYMBOL_GPL(clk_round_rate);

static int loongson3_clock_init(void)
{
	int i;

	for_each_possible_cpu(i) {
		sprintf(clk_names[i], "cpu%d_clk", i);
		cpu_clks[i].name = clk_names[i];
		cpu_clks[i].flags = CLK_ALWAYS_ENABLED | CLK_RATE_PROPAGATES;
		cpu_clks[i].rate = cpu_clock_freq / 1000;
	}

	/* clock table init */
	for (i = 2;
	     (loongson3_clockmod_table[i].frequency != CPUFREQ_TABLE_END);
	     i++)
		loongson3_clockmod_table[i].frequency = ((cpu_clock_freq / 1000) * i) / 8;

	return 0;
}
arch_initcall(loongson3_clock_init);

MODULE_AUTHOR("Huacai Chen <chenhc@lemote.com>");
MODULE_DESCRIPTION("CPUFreq driver for Loongson 3A/3B");
MODULE_LICENSE("GPL");
