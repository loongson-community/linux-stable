#include <linux/init.h>
#include <linux/clockchips.h>
#include <linux/sched_clock.h>
#include <asm/time.h>
#include <asm/mipsregs.h>
#include <asm/cevt-r4k.h>
#include <loongson.h>
#include <loongson_regs.h>

int constant_timer_irq_installed;

static DEFINE_SPINLOCK(state_lock);
DEFINE_PER_CPU(struct clock_event_device, constant_clockevent_device);

void constant_event_handler(struct clock_event_device *dev)
{
}

irqreturn_t constant_timer_interrupt(int irq, void *data)
{
	int cpu = smp_processor_id();
	const int r2 = cpu_has_mips_r2_r6;
	struct clock_event_device *cd;

	if ((cp0_perfcount_irq < 0) && perf_irq() == IRQ_HANDLED && !r2)
		return IRQ_HANDLED;

	/*
	 * The same applies to performance counter interrupts.	But with the
	 * above we now know that the reason we got here must be a timer
	 * interrupt.  Being the paranoiacs we are we check anyway.
	 */
	if (!r2 || (read_c0_cause() & CAUSEF_TI)) {
		/* Clear Count/Compare Interrupt */
		write_c0_compare(read_c0_compare());
		cd = &per_cpu(constant_clockevent_device, cpu);
		cd->event_handler(cd);

		return IRQ_HANDLED;
	}

	return IRQ_NONE;
}

static struct irqaction constant_timer_irqaction = {
	.handler = constant_timer_interrupt,
	.flags = IRQF_PERCPU | IRQF_TIMER | IRQF_SHARED,
	.name = "timer",
};

static int constant_set_state_oneshot(struct clock_event_device *evt)
{
	unsigned int raw_cpuid;
	unsigned long cfg, addr;

	spin_lock(&state_lock);

	raw_cpuid = cpu_logical_map(smp_processor_id());
	addr = CSR_TO_RAW_ADDR(raw_cpuid, LOONGSON_CSR_TIMER_CFG);

	if (!cpu_has_csr())
		cfg = readq((void *)addr);
	else
		cfg = csr_readq(LOONGSON_CSR_TIMER_CFG);

	/* set timer type
	 * 1 : periodic interrupt
	 * 0 : non-periodic(oneshot) interrupt
	 */
	cfg |= CONSTANT_TIMER_CFG_EN;
	cfg &= ~CONSTANT_TIMER_CFG_PERIODIC;

	if (!cpu_has_csr())
		writeq(cfg, (void *)addr);
	else
		csr_writeq(cfg, LOONGSON_CSR_TIMER_CFG);

	spin_unlock(&state_lock);

	return 0;
}

static int constant_set_state_oneshot_stopped(struct clock_event_device *evt)
{
	return 0;
}

static int constant_set_state_periodic(struct clock_event_device *evt)
{
	unsigned int period;
	unsigned int raw_cpuid;
	unsigned long cfg, addr;

	spin_lock(&state_lock);

	raw_cpuid = cpu_logical_map(smp_processor_id());
	addr = CSR_TO_RAW_ADDR(raw_cpuid, LOONGSON_CSR_TIMER_CFG);

	if (!cpu_has_csr())
		cfg = readq((void *)addr);
	else
		cfg = csr_readq(LOONGSON_CSR_TIMER_CFG);

	cfg &= CONSTANT_TIMER_INITVAL_RESET;
	cfg |= (CONSTANT_TIMER_CFG_PERIODIC | CONSTANT_TIMER_CFG_EN);

	period = calc_const_freq();
	if (!period)
		period = cpu_clock_freq;

	period = period / HZ;

	if (!cpu_has_csr())
		writeq(cfg | period, (void *)addr);
	else
		csr_writeq(cfg | period, LOONGSON_CSR_TIMER_CFG);

	spin_unlock(&state_lock);

	return 0;
}

static int constant_set_state_shutdown(struct clock_event_device *evt)
{
	return 0;
}

static int constant_next_event(unsigned long delta,
				struct clock_event_device *evt)
{
	unsigned long addr;
	unsigned int raw_cpuid;

	raw_cpuid = cpu_logical_map(smp_processor_id());
	addr = CSR_TO_RAW_ADDR(raw_cpuid, LOONGSON_CSR_TIMER_CFG);

	writeq(delta | CONSTANT_TIMER_CFG_EN, (void *)addr);

	return 0;
}

static int csr_constant_next_event(unsigned long delta,
				struct clock_event_device *evt)
{
	csr_writeq(delta | CONSTANT_TIMER_CFG_EN, LOONGSON_CSR_TIMER_CFG);
	return 0;
}

int constant_clockevent_init(void)
{
	unsigned int irq;
	unsigned int config;
	unsigned int const_freq;
	unsigned long min_delta = 0x600;
	unsigned long max_delta = (1UL << 48) - 1;
	unsigned int cpu = smp_processor_id();
	struct clock_event_device *cd;

	config = read_c0_config6();
	config |= MIPS_CONF6_EXTIMER;
	write_c0_config6(config);

	const_freq = calc_const_freq();
	if (!const_freq)
		const_freq = cpu_clock_freq;

	irq = get_c0_compare_int();

	cd = &per_cpu(constant_clockevent_device, cpu);

	cd->name = "Constant";
	cd->features = CLOCK_EVT_FEAT_ONESHOT | CLOCK_EVT_FEAT_PERIODIC |
		       CLOCK_EVT_FEAT_PERCPU;

	cd->rating = 320;
	cd->irq = irq;
	cd->cpumask = cpumask_of(cpu);
	cd->set_state_oneshot = constant_set_state_oneshot;
	cd->set_state_oneshot_stopped = constant_set_state_oneshot_stopped;
	cd->set_state_periodic = constant_set_state_periodic;
	cd->set_state_shutdown = constant_set_state_shutdown;
	if (!cpu_has_csr())
		cd->set_next_event = constant_next_event;
	else
		cd->set_next_event = csr_constant_next_event;

	cd->event_handler = constant_event_handler;

	clockevents_config_and_register(cd, const_freq, min_delta, max_delta);

	if (constant_timer_irq_installed)
		return 0;

	constant_timer_irq_installed = 1;
	setup_irq(irq, &constant_timer_irqaction);

	return 0;
}

static u64 read_const_counter(struct clocksource *clk)
{
	u64 count;

	__asm__ __volatile__(
		" .set push\n"
		" .set mips32r2\n"
		" rdhwr   %0, $30\n"
		" .set pop\n"
		: "=r" (count));

	return count;
}

static struct clocksource clocksource_const = {
	.name = "Constant",
	.rating = 400,
	.read = read_const_counter,
	.mask = CLOCKSOURCE_MASK(64),
	.flags = CLOCK_SOURCE_IS_CONTINUOUS,
	.mult = 0,
	.shift = 10,
};

static u64 read_sched_clock(void)
{
	u64 count;

	__asm__ __volatile__(
		" .set push\n"
		" .set mips32r2\n"
		" rdhwr   %0, $30\n"
		" .set pop\n"
		: "=r" (count));

	return count;
}

int __init init_constant_clocksource(void)
{
	int res;
	unsigned long freq;

	freq = calc_const_freq();
	if (freq)
		freq = cpu_clock_freq;

	clocksource_const.mult =
		clocksource_hz2mult(freq, clocksource_const.shift);

	res = clocksource_register_hz(&clocksource_const, freq);

	sched_clock_register(read_sched_clock, 64, freq);
	clocksource_const.archdata.vdso_clock_mode = VDSO_CLOCK_CONST;

	return res;
}
