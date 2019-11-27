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
#include <loongson_regs.h>
#include <loongson_smc.h>

static int new_method = 0;
static int boost_supported = 0;
static struct mutex cpufreq_mutex[MAX_PACKAGES];
static spinlock_t cpufreq_reg_lock[MAX_PACKAGES];

enum freq {
	FREQ_LEV0, /* Reserved */
	FREQ_LEV1, FREQ_LEV2, FREQ_LEV3, FREQ_LEV4,
	FREQ_LEV5, FREQ_LEV6, FREQ_LEV7, FREQ_LEV8,
	FREQ_LEV9, FREQ_LEV10, FREQ_LEV11, FREQ_LEV12,
	FREQ_LEV13, FREQ_LEV14, FREQ_LEV15, FREQ_LEV16,
	FREQ_RESV
};

/* For Loongson-3A4000+, support boost */
static struct cpufreq_frequency_table loongson3_cpufreq_table[] = {
	{0, FREQ_LEV0, CPUFREQ_ENTRY_INVALID},
	{0, FREQ_LEV1, CPUFREQ_ENTRY_INVALID},
	{0, FREQ_LEV2, CPUFREQ_ENTRY_INVALID},
	{0, FREQ_LEV3, CPUFREQ_ENTRY_INVALID},
	{0, FREQ_LEV4, CPUFREQ_ENTRY_INVALID},
	{0, FREQ_LEV5, CPUFREQ_ENTRY_INVALID},
	{0, FREQ_LEV6, CPUFREQ_ENTRY_INVALID},
	{0, FREQ_LEV7, CPUFREQ_ENTRY_INVALID},
	{0, FREQ_LEV8, CPUFREQ_ENTRY_INVALID},
	{0, FREQ_LEV9, CPUFREQ_ENTRY_INVALID},
	{0, FREQ_LEV10, CPUFREQ_ENTRY_INVALID},
	{0, FREQ_LEV11, CPUFREQ_ENTRY_INVALID},
	{0, FREQ_LEV12, CPUFREQ_ENTRY_INVALID},
	{0, FREQ_LEV13, CPUFREQ_ENTRY_INVALID},
	{0, FREQ_LEV14, CPUFREQ_ENTRY_INVALID},
	{0, FREQ_LEV15, CPUFREQ_ENTRY_INVALID},
	{0, FREQ_LEV16, CPUFREQ_ENTRY_INVALID},
	{0, FREQ_RESV, CPUFREQ_TABLE_END},
};
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

	if (cpu_has_constant_timer)
		return 0;

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

	if (cpu_has_constant_timer)
		return 0;

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

static int loongson3_cpufreq_set(struct cpufreq_policy *policy, int freq_level)
{
	uint32_t core_id = cpu_core(&cpu_data[policy->cpu]);
	struct freq_level_setting_args args;

	args.level = freq_level;
	args.cpumask = 1 << core_id;
	do_service_request(CMD_SET_CPU_LEVEL, &args);

	return 0;
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
	if (new_method) {
		mutex_lock(&cpufreq_mutex[package]);
		loongson3_cpufreq_set(policy, index);
		mutex_unlock(&cpufreq_mutex[package]);
	} else {
		spin_lock(&cpufreq_reg_lock[package]);
		clk_set_rate(policy->clk, freq);
		spin_unlock(&cpufreq_reg_lock[package]);
	}

	return 0;
}

static int loongson3_cpufreq_cpu_init(struct cpufreq_policy *policy)
{
	if (!cpu_online(policy->cpu))
		return -ENODEV;

	policy->clk = cpu_clk_get(policy->cpu);
	policy->cur = loongson3_cpufreq_get(policy->cpu);

	if (new_method) {
		/* Loongson-3A R4: new method */
		policy->cpuinfo.transition_latency = 2560;
		policy->freq_table = loongson3_cpufreq_table;
	} else {
		policy->cpuinfo.transition_latency = 1000;
		policy->freq_table = loongson3_clockmod_table;
		/* Loongson-3A R1: all cores in a package share one clock */
		if ((read_c0_prid() & PRID_REV_MASK) == PRID_REV_LOONGSON3A_R1)
			cpumask_copy(policy->cpus, topology_core_cpumask(policy->cpu));
	}

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

static int configure_cpufreq_info(void)
{
	int i, r, max_level;
	struct feature_args args1;
	struct freq_level_args args2;
	struct freq_info_args args3;

	if (!cpu_has_csr())
		return -EPERM;

	args1.index = FEATURE_INDEX_GENERAL;
	r = do_service_request(CMD_GET_FEATURES, &args1);
	if (r < 0)
		return -EPERM;

	if (args1.flags & FEATURE_FREQ_SCALE)
		new_method = 1;

	if (args1.flags & FEATURE_BOOST)
		boost_supported = 1;

	r = do_service_request(CMD_SET_ENABLED_FEATURES, &args1);
	if (r < 0) {
		new_method = 0;
		boost_supported = 0;
		return -EPERM;
	}

	r = do_service_request(CMD_GET_FREQ_LEVELS, &args2);
	if (r < 0) {
		new_method = 0;
		boost_supported = 0;
		return -EPERM;
	}

	if (boost_supported)
		max_level = args2.max_boost_level;
	else
		max_level = args2.max_normal_level;

	for (i = args2.min_level; i <= max_level; i++) {
		args3.info = i;
		args3.index = FREQ_INFO_INDEX_FREQ;
		do_service_request(CMD_GET_FREQ_INFO, &args3);
		loongson3_cpufreq_table[i].frequency = args3.info * 1000;
		if (i > args2.max_normal_level)
			loongson3_cpufreq_table[i].flags = CPUFREQ_BOOST_FREQ;
	}

	return 0;
}

static int __init cpufreq_init(void)
{
	int i, ret;

	/* Register platform stuff */
	ret = platform_driver_register(&platform_driver);
	if (ret)
		return ret;

	pr_info("cpufreq: Loongson-3 CPU frequency driver.\n");

	configure_cpufreq_info();

	for (i = 0; i < MAX_PACKAGES; i++) {
		mutex_init(&cpufreq_mutex[i]);
		spin_lock_init(&cpufreq_reg_lock[i]);
	}

	cpufreq_register_notifier(&loongson3_cpufreq_notifier_block,
				  CPUFREQ_TRANSITION_NOTIFIER);

	ret = cpufreq_register_driver(&loongson3_cpufreq_driver);

	if (boost_supported)
		cpufreq_enable_boost_support();

	return ret;
}

static void __exit cpufreq_exit(void)
{
	cpufreq_unregister_driver(&loongson3_cpufreq_driver);
	cpufreq_unregister_notifier(&loongson3_cpufreq_notifier_block,
				    CPUFREQ_TRANSITION_NOTIFIER);

	platform_driver_unregister(&platform_driver);
}

module_init(cpufreq_init);
module_exit(cpufreq_exit);

MODULE_AUTHOR("Huacai Chen <chenhc@lemote.com>");
MODULE_DESCRIPTION("CPUFreq driver for Loongson-3A/3B");
MODULE_LICENSE("GPL");
