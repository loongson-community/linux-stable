/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 */
#include <linux/init.h>
#include <linux/oprofile.h>
#include <linux/interrupt.h>
#include <linux/smp.h>
#include <linux/spinlock.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <irq.h>
#include "op_impl.h"
extern void prom_printf(char * fmt, ...);

#define PERF_IRQ (MIPS_CPU_IRQ_BASE + 7 )
#define LOONGSON_COUNTER1_EVENT(event)	((event&0x3f) << 5)
#define LOONGSON_COUNTER1_SUPERVISOR	(1UL << 2)
#define LOONGSON_COUNTER1_KERNEL	(1UL << 1)
#define LOONGSON_COUNTER1_USER		(1UL << 3)
#define LOONGSON_COUNTER1_ENABLE	(1UL << 4)
#define LOONGSON_COUNTER1_OVERFLOW	(1ULL << 63)
#define LOONGSON_COUNTER1_W		(1UL << 30)
#define LOONGSON_COUNTER1_M		(1UL << 31)
#define LOONGSON_COUNTER1_EXL		(1UL << 0)

#define LOONGSON_COUNTER2_EVENT(event)	((event&0x0f) << 5)
#define LOONGSON_COUNTER2_SUPERVISOR	LOONGSON_COUNTER1_SUPERVISOR
#define LOONGSON_COUNTER2_KERNEL	LOONGSON_COUNTER1_KERNEL
#define LOONGSON_COUNTER2_USER		LOONGSON_COUNTER1_USER
#define LOONGSON_COUNTER2_ENABLE	LOONGSON_COUNTER1_ENABLE
#define LOONGSON_COUNTER2_OVERFLOW	(1ULL << 63)
#define LOONGSON_COUNTER2_W		(1UL  << 30)
#define LOONGSON_COUNTER2_M		(1UL  << 31)
#define LOONGSON_COUNTER2_EXL		(1UL  << 0 )

/* Loongson3 PerfCount performance counter1 register */
#define read_c0_perflo1() __read_64bit_c0_register($25, 0)
#define write_c0_perflo1(val) __write_64bit_c0_register($25, 0, val)
#define read_c0_perfhi1() __read_64bit_c0_register($25, 1)
#define write_c0_perfhi1(val) __write_64bit_c0_register($25, 1, val)

/* Loongson3 PerfCount performance counter2 register */
#define read_c0_perflo2() __read_64bit_c0_register($25, 2)
#define write_c0_perflo2(val) __write_64bit_c0_register($25, 2, val)
#define read_c0_perfhi2() __read_64bit_c0_register($25, 3)
#define write_c0_perfhi2(val) __write_64bit_c0_register($25, 3, val)

static struct loongson3_register_config {
	unsigned int control1;
	unsigned int control2;
	unsigned long long reset_counter1;
	unsigned long long reset_counter2;
	int ctr1_enable, ctr2_enable;
} reg;

DEFINE_SPINLOCK(sample_lock);

static char *oprofid = "LoongsonPerf";
static irqreturn_t loongson3_perfcount_handler(int irq, void * dev_id);

/* Compute all of the registers in preparation for enabling profiling.  */
static void loongson3_reg_setup(struct op_counter_config *ctr)
{
	unsigned int control1 = 0;
	unsigned int control2 = 0;

	reg.reset_counter1 = 0;
	reg.reset_counter2 = 0;
	/* Compute the performance counter control word. */
	/* For now count kernel and user mode */
	if (ctr[0].enabled){
		control1 |= LOONGSON_COUNTER1_EVENT(ctr[0].event) |
					LOONGSON_COUNTER1_ENABLE;
		if(ctr[0].kernel)
			control1 |= LOONGSON_COUNTER1_KERNEL;
		if(ctr[0].user)
			control1 |= LOONGSON_COUNTER1_USER;
		reg.reset_counter1 = 0x8000000000000000ULL - ctr[0].count;
	}

	if (ctr[1].enabled){
		control2 |= LOONGSON_COUNTER2_EVENT(ctr[1].event) |
		           LOONGSON_COUNTER2_ENABLE;
		if(ctr[1].kernel)
			control2 |= LOONGSON_COUNTER2_KERNEL;
		if(ctr[1].user)
			control2 |= LOONGSON_COUNTER2_USER;
		reg.reset_counter2 = (0x8000000000000000ULL- ctr[1].count) ;
	}

	if(ctr[0].enabled){
		control1 |= LOONGSON_COUNTER1_EXL;
	}
	if(ctr[1].enabled){
		control2 |= LOONGSON_COUNTER2_EXL;
	}

	reg.control1 = control1;
	reg.control2 = control2;

	reg.ctr1_enable = ctr[0].enabled;
	reg.ctr2_enable = ctr[1].enabled;

}

/* Program all of the registers in preparation for enabling profiling.  */
static void loongson3_cpu_setup(void *args)
{
	uint64_t perfcount1, perfcount2;

	perfcount1 = reg.reset_counter1;
	perfcount2 = reg.reset_counter2;
	write_c0_perfhi1(perfcount1);
	write_c0_perfhi2(perfcount2);
}

static void loongson3_cpu_start(void *args)
{
	/* Start all counters on current CPU */
	reg.control1 |= (LOONGSON_COUNTER1_W|LOONGSON_COUNTER1_M);
	reg.control2 |= (LOONGSON_COUNTER2_W|LOONGSON_COUNTER2_M);

	if(reg.ctr1_enable) {
		write_c0_perflo1(reg.control1);
	}
	if(reg.ctr2_enable) {
		write_c0_perflo2(reg.control2);
	}
}

static void loongson3_cpu_stop(void *args)
{
	/* Stop all counters on current CPU */
	write_c0_perflo1(0xc0000000);
	write_c0_perflo2(0x40000000);
	memset(&reg, 0, sizeof(reg));
}

static irqreturn_t loongson3_perfcount_handler(int irq, void * dev_id)
{
	uint64_t counter1, counter2;
	struct pt_regs *regs = get_irq_regs();
	unsigned long flags;

       if(!(read_c0_cause() & (1<<26)))  return IRQ_NONE;

	counter1 = read_c0_perfhi1();
	counter2 = read_c0_perfhi2();

	spin_lock_irqsave(&sample_lock, flags);

	if (counter1 & LOONGSON_COUNTER1_OVERFLOW) {
		if(reg.ctr1_enable)
			oprofile_add_sample(regs, 0);
		counter1 = reg.reset_counter1;
	}
	if (counter2 & LOONGSON_COUNTER2_OVERFLOW) {
		if(reg.ctr2_enable)
			oprofile_add_sample(regs, 1);
		counter2 = reg.reset_counter2;
	}

	spin_unlock_irqrestore(&sample_lock, flags);

	write_c0_perfhi1(counter1);
	write_c0_perfhi2(counter2);

	return IRQ_HANDLED;
}

static int __init loongson3_init(void)
{
#ifdef CONFIG_SMP
	return  request_irq(PERF_IRQ, loongson3_perfcount_handler,
	                  IRQF_SHARED|IRQF_PERCPU|IRQF_IRQPOLL, "Perfcounter", oprofid);
#else
	return  request_irq(PERF_IRQ, loongson3_perfcount_handler,
	                   IRQF_SHARED, "Perfcounter", oprofid);
#endif
}

static void loongson3_exit(void)
{
	free_irq(PERF_IRQ, oprofid);
}

struct op_mips_model op_model_loongson3_ops = {
	.reg_setup	= loongson3_reg_setup,
	.cpu_setup	= loongson3_cpu_setup,
	.init		= loongson3_init,
	.exit		= loongson3_exit,
	.cpu_start	= loongson3_cpu_start,
	.cpu_stop	= loongson3_cpu_stop,
	.cpu_type	= "mips/loongson3",
	.num_counters	= 2
};
