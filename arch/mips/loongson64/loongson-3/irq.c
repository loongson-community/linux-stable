// SPDX-License-Identifier: GPL-2.0
#include <loongson.h>
#include <irq.h>
#include <linux/init.h>
#include <linux/irqchip.h>
#include <linux/interrupt.h>

#include <asm/irq_cpu.h>
#include <asm/i8259.h>
#include <asm/mipsregs.h>

#include <loongson-pch.h>
#include "smp.h"

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

	cpumask_copy(d->common->affinity, &new_affinity);

	return IRQ_SET_MASK_OK_NOCOPY;
}

static DECLARE_BITMAP(ipi_irq_in_use, NR_DIRQS);
unsigned int loongson_ipi_irq2pos[NR_IRQS] = { [0 ... NR_IRQS-1] = -1 };
unsigned int loongson_ipi_pos2irq[NR_DIRQS] = { [0 ... NR_DIRQS-1] = -1 };

void create_ipi_dirq(unsigned int irq)
{
	int pos;

	pos = find_first_zero_bit(ipi_irq_in_use, NR_DIRQS);

	if (pos == NR_DIRQS)
		return;

	loongson_ipi_pos2irq[pos] = irq;
	loongson_ipi_irq2pos[irq] = pos;
	set_bit(pos, ipi_irq_in_use);
}

void destroy_ipi_dirq(unsigned int irq)
{
	int pos;

	pos = loongson_ipi_irq2pos[irq];

	if (pos < 0)
		return;

	loongson_ipi_irq2pos[irq] = -1;
	loongson_ipi_pos2irq[pos] = -1;
	clear_bit(pos, ipi_irq_in_use);
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
		pr_err("%s : spurious interrupt\n", __func__);
		spurious_interrupt();
	}
}

static inline void mask_loongson_irq(struct irq_data *d) { }
static inline void unmask_loongson_irq(struct irq_data *d) { }

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
	int i;

	clear_c0_status(ST0_IM | ST0_BEV);

	irq_alloc_descs(-1, MIPS_CPU_IRQ_BASE, 8, 0);
	for (i = MIPS_CPU_IRQ_BASE; i < MIPS_CPU_IRQ_BASE + 8; i++)
		irq_set_noprobe(i);

	irqchip_init();
	loongson_pch->init_irq();

	irq_set_chip_and_handler(LOONGSON_UART_IRQ,
			&loongson_irq_chip, handle_percpu_irq);
	irq_set_chip_and_handler(LOONGSON_BRIDGE_IRQ,
			&loongson_irq_chip, handle_percpu_irq);

	set_c0_status(STATUSF_IP2 | STATUSF_IP3 | STATUSF_IP6);
}

#ifdef CONFIG_HOTPLUG_CPU

void fixup_irqs(void)
{
	irq_cpu_offline();
	clear_c0_status(ST0_IM);
}

#endif
