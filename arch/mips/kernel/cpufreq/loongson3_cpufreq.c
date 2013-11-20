/*
 * CPUFreq driver for the loongson-3 processors
 *
 * All revisions of Loongson-3 processor support this feature.
 *
 * Copyright (C) 2006 - 2013 Lemote Inc. & Insititute of Computing Technology
 * Author: Yanhua, yanh@lemote.com
 *         Chen Huacai, chenhc@lemote.com
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */
#include <linux/cpufreq.h>
#include <linux/module.h>
#include <linux/sched.h>	/* set_cpus_allowed() */
#include <linux/delay.h>
#include <linux/clockchips.h>
#include <linux/platform_device.h>

#include <asm/clock.h>
#include <asm/cevt-r4k.h>

#include <loongson.h>

int cpufreq_enabled = 0;
EXPORT_SYMBOL(cpufreq_enabled);
extern struct clk *cpu_clk_get(int cpu);

static uint nowait;

static void (*saved_cpu_wait) (void);

void maybe_enable_cpufreq(void)
{
	if ((num_online_cpus() == 1) && (system_state != SYSTEM_BOOTING))
		cpufreq_enabled = 1;
}

void maybe_disable_cpufreq(void)
{
	if ((num_online_cpus() == 1) && (system_state != SYSTEM_BOOTING)) {
		struct cpufreq_freqs freqs;
		struct clk *cpuclk = cpu_clk_get(0);

		freqs.cpu   = 0;
		freqs.old   = cpuclk->rate;
		freqs.new   = cpu_clock_freq / 1000;
		freqs.flags = 0;

		cpufreq_enabled = 0;
		cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);
		cpuclk->rate = cpu_clock_freq / 1000;
		LOONGSON_CHIPCFG0 |= 0x7;	/* Set to highest frequency */
		cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);
	}
}

static int loongson3_cpu_freq_notifier(struct notifier_block *nb,
					unsigned long val, void *data);

static struct notifier_block loongson3_cpufreq_notifier_block = {
	.notifier_call = loongson3_cpu_freq_notifier
};

static int loongson3_cpu_freq_notifier(struct notifier_block *nb,
					unsigned long val, void *data)
{
	struct cpufreq_freqs *freqs = (struct cpufreq_freqs *)data;
	int cpu = freqs->cpu;
	struct clock_event_device *cd = &per_cpu(mips_clockevent_device, cpu);

	if (val == CPUFREQ_POSTCHANGE) {
		cpu_data[cpu].udelay_val =
			cpufreq_scale(cpu_data[cpu].udelay_val, freqs->old, freqs->new);
		clockevents_update_freq(cd, freqs->new * 1000 / 2);
	}

	return 0;
}

static unsigned int loongson3_cpufreq_get(unsigned int cpu)
{
	return clk_get_rate(cpu_clk_get(cpu));
}

/*
 * Here we notify other drivers of the proposed change and the final change.
 */
static int loongson3_cpufreq_target(struct cpufreq_policy *policy,
				     unsigned int target_freq,
				     unsigned int relation)
{
	unsigned int cpu = policy->cpu;
	unsigned int newstate = 0;
	struct cpufreq_freqs freqs;
	unsigned int freq;

	if (!cpu_online(cpu))
		return -ENODEV;

	if (!cpufreq_enabled)
		return 0;

	if (cpufreq_frequency_table_target
	    (policy, &loongson3_clockmod_table[0], target_freq, relation,
	     &newstate))
		return -EINVAL;

	freq =
	    ((cpu_clock_freq / 1000) *
	     loongson3_clockmod_table[newstate].index) / 8;
	if (freq < policy->min || freq > policy->max)
		return -EINVAL;

	pr_debug("cpufreq: requested frequency %u Hz\n", target_freq * 1000);

	freqs.cpu = cpu;
	freqs.old = loongson3_cpufreq_get(cpu);
	freqs.new = freq;
	freqs.flags = 0;

	if (freqs.new == freqs.old)
		return 0;

	/* notifiers */
	cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);

	/* setting the cpu frequency */
	clk_set_rate(cpu_clk_get(cpu), freq);

	/* notifiers */
	cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);

	pr_debug("cpufreq: set frequency %u kHz\n", freq);

	return 0;
}

static int loongson3_cpufreq_cpu_init(struct cpufreq_policy *policy)
{
	if (!cpu_online(policy->cpu))
		return -ENODEV;

	policy->cur = loongson3_cpufreq_get(policy->cpu);

	cpufreq_frequency_table_get_attr(&loongson3_clockmod_table[0],
					 policy->cpu);

	return cpufreq_frequency_table_cpuinfo(policy,
					    &loongson3_clockmod_table[0]);
}

static int loongson3_cpufreq_verify(struct cpufreq_policy *policy)
{
	return cpufreq_frequency_table_verify(policy,
					      &loongson3_clockmod_table[0]);
}

static int loongson3_cpufreq_exit(struct cpufreq_policy *policy)
{
	return 0;
}

static struct freq_attr *loongson3_table_attr[] = {
	&cpufreq_freq_attr_scaling_available_freqs,
	NULL,
};

static struct cpufreq_driver loongson3_cpufreq_driver = {
	.owner = THIS_MODULE,
	.name = "loongson3",
	.init = loongson3_cpufreq_cpu_init,
	.verify = loongson3_cpufreq_verify,
	.target = loongson3_cpufreq_target,
	.get = loongson3_cpufreq_get,
	.exit = loongson3_cpufreq_exit,
	.attr = loongson3_table_attr,
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
	u32 cpu_freq;
	unsigned long flags;

	if (!cpufreq_enabled || system_state != SYSTEM_RUNNING)
		return;

	local_irq_save(flags);
	if (cputype == Loongson_3A) {
		cpu_freq = LOONGSON_CHIPCFG0;
		LOONGSON_CHIPCFG0 &= ~0x7;	/* Put CPU into wait mode */
		LOONGSON_CHIPCFG0 = cpu_freq;	/* Restore CPU state */
	}
	else if (cputype == Loongson_3B) {
		int cpu = smp_processor_id();
		cpu_freq = LOONGSON_FREQCTRL;
		LOONGSON_FREQCTRL &= ~(0x7 << (cpu*4)); /* Put CPU into wait mode */
		LOONGSON_FREQCTRL = cpu_freq;           /* Restore CPU state */
	}
	local_irq_restore(flags);
}
EXPORT_SYMBOL_GPL(loongson3_cpu_wait);

static int __init cpufreq_init(void)
{
	int ret;

	if (num_online_cpus() == 1) /* smp_init() has finished at this time */
		cpufreq_enabled = 1;

	if (!cpufreq_workaround)
		cpufreq_enabled = 1;

	/* Register platform stuff */
	ret = platform_driver_register(&platform_driver);
	if (ret)
		return ret;

	pr_info("cpufreq: Loongson-3 CPU frequency driver.\n");

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
MODULE_DESCRIPTION("CPUFreq driver for Loongson3A/3B");
MODULE_LICENSE("GPL");
