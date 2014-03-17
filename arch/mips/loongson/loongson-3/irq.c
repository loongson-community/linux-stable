#include <loongson.h>
#include <irq.h>
#include <linux/interrupt.h>
#include <linux/module.h>

#include <asm/irq_cpu.h>
#include <asm/i8259.h>
#include <asm/mipsregs.h>

#include <loongson-pch.h>

extern unsigned long long smp_group[4];
extern void loongson3_ipi_interrupt(struct pt_regs *regs);

int plat_set_irq_affinity(struct irq_data *d, const struct cpumask *affinity,
			  bool force)
{
	unsigned int cpu;
	struct cpumask new_affinity;

	/* I/O devices are connected on package-0 */
	cpumask_copy(&new_affinity, affinity);
	for_each_cpu(cpu, affinity)
		if (cpu_data[cpu].package > 0)
			cpumask_clear_cpu(cpu, &new_affinity);

	if (cpumask_empty(&new_affinity))
		return -EINVAL;

	cpumask_copy(d->affinity, &new_affinity);

	return IRQ_SET_MASK_OK_NOCOPY;
}

#define UNUSED_IPS (CAUSEF_IP5 | CAUSEF_IP4 | CAUSEF_IP1 | CAUSEF_IP0)

void mach_irq_dispatch(unsigned int pending)
{
	if (pending & CAUSEF_IP7)
		do_IRQ(LOONGSON_TIMER_IRQ);
#if defined(CONFIG_SMP)
	if (pending & CAUSEF_IP6)
		loongson3_ipi_interrupt(NULL);
#endif
	if (pending & CAUSEF_IP3)
		loongson_pch->irq_dispatch();
	if (pending & CAUSEF_IP2)
		do_IRQ(LOONGSON_UART_IRQ);
	if (pending & UNUSED_IPS) {
		printk(KERN_ERR "%s : spurious interrupt\n", __func__);
		spurious_interrupt();
	}
}

static struct irqaction cascade_irqaction = {
	.handler = no_action,
	.flags = IRQF_NO_SUSPEND,
	.name = "cascade",
};

static inline void mask_loongson_irq(struct irq_data *d)
{
	clear_c0_status(0x100 << (d->irq - MIPS_CPU_IRQ_BASE));
	irq_disable_hazard();

	/* Workaround: UART IRQ may deliver to any core */
	if (d->irq == LOONGSON_UART_IRQ) {
		int cpu = smp_processor_id();
		int node_id = cpu_logical_map(cpu) / cores_per_node;
		int core_id = cpu_logical_map(cpu) % cores_per_node;
		u64 intenclr_addr = smp_group[node_id] |
			(u64)(&LOONGSON_INT_ROUTER_INTENCLR);
		u64 introuter_lpc_addr = smp_group[node_id] |
			(u64)(&LOONGSON_INT_ROUTER_LPC);

		*(volatile u32 *)intenclr_addr = 1 << 10;
		*(volatile u8 *)introuter_lpc_addr = 0x10 + (1<<core_id);
	}
}

static inline void unmask_loongson_irq(struct irq_data *d)
{
	/* Workaround: UART IRQ may deliver to any core */
	if (d->irq == LOONGSON_UART_IRQ) {
		int cpu = smp_processor_id();
		int node_id = cpu_logical_map(cpu) / cores_per_node;
		int core_id = cpu_logical_map(cpu) % cores_per_node;
		u64 intenset_addr = smp_group[node_id] |
			(u64)(&LOONGSON_INT_ROUTER_INTENSET);
		u64 introuter_lpc_addr = smp_group[node_id] |
			(u64)(&LOONGSON_INT_ROUTER_LPC);

		*(volatile u32 *)intenset_addr = 1 << 10;
		*(volatile u8 *)introuter_lpc_addr = 0x10 + (1<<core_id);
	}

	set_c0_status(0x100 << (d->irq - MIPS_CPU_IRQ_BASE));
	irq_enable_hazard();
}

 /* For MIPS IRQs which shared by all cores */
static struct irq_chip loongson_irq_chip = {
	.name		= "Loongson",
	.irq_ack	= mask_loongson_irq,
	.irq_mask	= mask_loongson_irq,
	.irq_mask_ack	= mask_loongson_irq,
	.irq_unmask	= unmask_loongson_irq,
	.irq_eoi	= unmask_loongson_irq,
};

void __init mach_init_irq(void)
{
	clear_c0_status(ST0_IM | ST0_BEV);

	mips_cpu_irq_init();
	if (loongson_pch)
		loongson_pch->init_irq();

	/* setup CASCADE irq */
	setup_irq(LOONGSON_BRIDGE_IRQ, &cascade_irqaction);

	irq_set_chip_and_handler(LOONGSON_UART_IRQ,
			&loongson_irq_chip, handle_level_irq);

	set_c0_status(STATUSF_IP2 | STATUSF_IP6);
}

#ifdef CONFIG_HOTPLUG_CPU

void fixup_irqs(void)
{
	irq_cpu_offline();
	clear_c0_status(ST0_IM);
}

#endif
