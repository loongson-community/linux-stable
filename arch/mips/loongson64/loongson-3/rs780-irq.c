// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright (C) 2013, Loongson Technology Corporation Limited, Inc.
 *  Copyright (C) 2014-2017, Lemote, Inc.
 */
#include <linux/module.h>
#include <linux/interrupt.h>

#include <asm/irq_cpu.h>
#include <asm/i8259.h>
#include <asm/mipsregs.h>

#include <irq.h>
#include <loongson.h>
#include "smp.h"

static unsigned int irq_cpu[NR_IRQS] = {[0 ... NR_IRQS-1] = -1};
static unsigned int ht_irq[] = {0, 1, 3, 4, 5, 6, 7, 8, 12, 14, 15};

void rs780_irq_dispatch(void)
{
	unsigned int i, irq;
	struct irq_data *irqd;
	struct cpumask affinity;

	irq = LOONGSON_HT1_INT_VECTOR(0);
	LOONGSON_HT1_INT_VECTOR(0) = irq; /* Acknowledge the IRQs */

	for (i = 0; i < ARRAY_SIZE(ht_irq); i++) {
		if (!(irq & (0x1 << ht_irq[i])))
			continue;

		/* handled by local core */
		if (loongson_ipi_irq2pos[ht_irq[i]] == -1) {
			do_IRQ(ht_irq[i]);
			continue;
		}

		irqd = irq_get_irq_data(ht_irq[i]);
		cpumask_and(&affinity, irqd->common->affinity, cpu_active_mask);
		if (cpumask_empty(&affinity)) {
			do_IRQ(ht_irq[i]);
			continue;
		}

		irq_cpu[ht_irq[i]] = cpumask_next(irq_cpu[ht_irq[i]], &affinity);
		if (irq_cpu[ht_irq[i]] >= nr_cpu_ids)
			irq_cpu[ht_irq[i]] = cpumask_first(&affinity);

		if (irq_cpu[ht_irq[i]] == 0) {
			do_IRQ(ht_irq[i]);
			continue;
		}

		/* balanced by other cores */
		loongson3_send_irq_by_ipi(irq_cpu[ht_irq[i]], (0x1 << (loongson_ipi_irq2pos[ht_irq[i]])));
	}
}

void rs780_init_irq(void)
{
	int i;
	struct irq_chip *chip;

	/* Route LPC int to cpu Core0 INT0 */
	LOONGSON_INT_ROUTER_LPC = LOONGSON_INT_COREx_INTy(loongson_sysconf.boot_cpu_id, 0);
	/* Route HT1 int0 ~ int3 to cpu Core0 INT1 */
	for (i = 0; i < 4; i++)
		LOONGSON_INT_ROUTER_HT1(i) = LOONGSON_INT_COREx_INTy(loongson_sysconf.boot_cpu_id, 1);
	/* Enable HT1 interrupts */
	LOONGSON_HT1_INTN_EN(0) = 0xffffffff;
	/* Enable router interrupt intenset */
	LOONGSON_INT_ROUTER_INTENSET =
		LOONGSON_INT_ROUTER_INTEN | (0xffff << 16) | 0x1 << 10;

	chip = irq_get_chip(I8259A_IRQ_BASE);
	chip->irq_set_affinity = plat_set_irq_affinity;

	for (i = 0; i < NR_IRQS; i++)
		loongson_ipi_irq2pos[i] = -1;
	for (i = 0; i < NR_DIRQS; i++)
		loongson_ipi_pos2irq[i] = -1;
	create_ipi_dirq(3);
	create_ipi_dirq(4);
	create_ipi_dirq(5);
	create_ipi_dirq(6);
	create_ipi_dirq(14);
	create_ipi_dirq(15);
}
