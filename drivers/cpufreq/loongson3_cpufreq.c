/*
 * CPUFreq driver for the loongson-3 processors
 *
 * All revisions of Loongson-3 processor support this feature.
 *
 * Copyright (C) 2008 - 2014 Lemote Inc.
 * Author: Yan Hua, yanh@lemote.com
 *         Chen Huacai, chenhc@lemote.com
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */
#include <linux/module.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/cpufreq.h>
#include <linux/platform_device.h>
#include <asm/idle.h>
#include <asm/clock.h>
#include <asm/cevt-r4k.h>

#include <loongson.h>

static uint nowait = 1;
static spinlock_t cpufreq_reg_lock[MAX_PACKAGES];

static void (*saved_cpu_wait)(void);
extern struct clk *cpu_clk_get(int cpu);

static int loongson3_cpu_freq_notifier(struct notifier_block *nb,
					unsigned long val, void *data);

static struct notifier_block loongson3_cpufreq_notifier_block = {
	.notifier_call = loongson3_cpu_freq_notifier
};

#ifdef CONFIG_SMP
static int loongson3_cpu_freq_notifier(struct notifier_block *nb,
					unsigned long val, void *data)
{
	struct cpufreq_freqs *freqs = (struct cpufreq_freqs *)data;
	unsigned long cpu = freqs->cpu;
	struct clock_event_device *cd = &per_cpu(mips_clockevent_device, cpu);

	if (val == CPUFREQ_POSTCHANGE) {
		if (cpu == smp_processor_id())
			clockevents_update_freq(cd, freqs->new * 1000 / 2);
		else {
			clockevents_calc_mult_shift(cd, freqs->new * 1000 / 2, 4);
			cd->min_delta_ns = clockevent_delta2ns(cd->min_delta_ticks, cd);
			cd->max_delta_ns = clockevent_delta2ns(cd->max_delta_ticks, cd);
		}
		cpu_data[cpu].udelay_val =
			cpufreq_scale(loops_per_jiffy, cpu_clock_freq / 1000, freqs->new);
	}

	return 0;
}
#else
static int loongson3_cpu_freq_notifier(struct notifier_block *nb,
					unsigned long val, void *data)
{
	struct cpufreq_freqs *freqs = (struct cpufreq_freqs *)data;
	struct clock_event_device *cd = &per_cpu(mips_clockevent_device, 0);

	if (val == CPUFREQ_POSTCHANGE) {
		clockevents_update_freq(cd, freqs->new * 1000 / 2);
		current_cpu_data.udelay_val = loops_per_jiffy;
	}

	return 0;
}
#endif

static unsigned int loongson3_cpufreq_get(unsigned int cpu)
{
	return clk_get_rate(cpu_clk_get(cpu));
}

/*
 * Here we notify other drivers of the proposed change and the final change.
 */
static int loongson3_cpufreq_target(struct cpufreq_policy *policy,
				     unsigned int index)
{
	unsigned int freq;
	unsigned int cpu = policy->cpu;
	unsigned int package = cpu_data[cpu].package;

	if (!cpu_online(cpu))
		return -ENODEV;

	freq =
	    ((cpu_clock_freq / 1000) *
	     loongson3_clockmod_table[index].driver_data) / 8;

	/* setting the cpu frequency */
	spin_lock(&cpufreq_reg_lock[package]);
	clk_set_rate(policy->clk, freq);
	spin_unlock(&cpufreq_reg_lock[package]);

	return 0;
}

static int loongson3_cpufreq_cpu_init(struct cpufreq_policy *policy)
{
	if (!cpu_online(policy->cpu))
		return -ENODEV;

	policy->clk = cpu_clk_get(policy->cpu);
	policy->cur = loongson3_cpufreq_get(policy->cpu);

	policy->cpuinfo.transition_latency = 1000;
	policy->freq_table = loongson3_clockmod_table;

	/* Loongson-3A R1: all cores in a package share one clock */
	if ((read_c0_prid() & 0xf) == PRID_REV_LOONGSON3A_R1)
		cpumask_copy(policy->cpus, topology_core_cpumask(policy->cpu));

	return 0;
}

static int loongson3_cpufreq_exit(struct cpufreq_policy *policy)
{
	return 0;
}

static struct cpufreq_driver loongson3_cpufreq_driver = {
	.name = "loongson3",
	.init = loongson3_cpufreq_cpu_init,
	.verify = cpufreq_generic_frequency_table_verify,
	.target_index = loongson3_cpufreq_target,
	.get = loongson3_cpufreq_get,
	.exit = loongson3_cpufreq_exit,
	.attr = cpufreq_generic_attr,
};

static struct platform_device_id platform_device_ids[] = {
	{
		.name = "loongson3_cpufreq",
	},
	{}
};

MODULE_DEVICE_TABLE(platform, platform_device_ids);

static struct platform_driver platform_driver = {
	.driver = {
		.name = "loongson3_cpufreq",
		.owner = THIS_MODULE,
	},
	.id_table = platform_device_ids,
};

/*
 * This is the simple version of Loongson-3 wait, Maybe we need do this in
 * interrupt disabled content
 */

void loongson3_cpu_wait(void)
{
	u32 cpu_freq, shared_cpus = 0;
	int i, cpu = smp_processor_id();
	uint64_t core_id = cpu_core(&cpu_data[cpu]);
	uint64_t package_id = cpu_data[cpu].package;

	for_each_online_cpu(i)
		if (cpu_data[i].package == package_id)
			shared_cpus++;

	if (shared_cpus > 1)
		goto out;

	if ((read_c0_prid() & 0xf) == PRID_REV_LOONGSON3A_R1) {
		cpu_freq = LOONGSON_CHIPCFG(package_id);
		LOONGSON_CHIPCFG(package_id) &= ~0x7;    /* Put CPU into wait mode */
		LOONGSON_CHIPCFG(package_id) = cpu_freq; /* Restore CPU state */
	} else {
		cpu_freq = LOONGSON_FREQCTRL(package_id);
		LOONGSON_FREQCTRL(package_id) &= ~(0x7 << (core_id*4)); /* Put CPU into wait mode */
		LOONGSON_FREQCTRL(package_id) = cpu_freq;               /* Restore CPU state */
	}

out:
	local_irq_enable();
}
EXPORT_SYMBOL_GPL(loongson3_cpu_wait);

static int __init cpufreq_init(void)
{
	int i, ret;

	/* Register platform stuff */
	ret = platform_driver_register(&platform_driver);
	if (ret)
		return ret;

	pr_info("cpufreq: Loongson-3 CPU frequency driver.\n");

	for (i = 0; i < MAX_PACKAGES; i++)
		spin_lock_init(&cpufreq_reg_lock[i]);

	cpufreq_register_notifier(&loongson3_cpufreq_notifier_block,
				  CPUFREQ_TRANSITION_NOTIFIER);

	ret = cpufreq_register_driver(&loongson3_cpufreq_driver);

	if (!ret && !nowait) {
		saved_cpu_wait = cpu_wait;
		cpu_wait = loongson3_cpu_wait;
	}

	return ret;
}

static void __exit cpufreq_exit(void)
{
	if (!nowait && saved_cpu_wait)
		cpu_wait = saved_cpu_wait;
	cpufreq_unregister_driver(&loongson3_cpufreq_driver);
	cpufreq_unregister_notifier(&loongson3_cpufreq_notifier_block,
				    CPUFREQ_TRANSITION_NOTIFIER);

	platform_driver_unregister(&platform_driver);
}

module_init(cpufreq_init);
module_exit(cpufreq_exit);

module_param(nowait, uint, 0644);
MODULE_PARM_DESC(nowait, "Disable Loongson-3A/3B specific wait");

MODULE_AUTHOR("Huacai Chen <chenhc@lemote.com>");
MODULE_DESCRIPTION("CPUFreq driver for Loongson-3A/3B");
MODULE_LICENSE("GPL");
