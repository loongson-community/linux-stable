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
#include <loongson-pch.h>
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

void rs780_msi_irq_dispatch(void)
{
	struct irq_data *irqd;
	struct cpumask affinity;
	unsigned int i, irq, irqs;

	/* Handle normal IRQs */
	for (i = 0; i < 4; i++) {
		irqs = LOONGSON_HT1_INT_VECTOR(i);
		LOONGSON_HT1_INT_VECTOR(i) = irqs; /* Acknowledge the IRQs */

		while (irqs) {
			irq = ffs(irqs) - 1;
			irqs &= ~(1 << irq);
			irq = (i * 32) + irq;

			if (irq >= MIPS_CPU_IRQ_BASE && irq < (MIPS_CPU_IRQ_BASE + 8)) {
				pr_err("spurious interrupt: IRQ%d.\n", irq);
				spurious_interrupt();
				continue;
			}

			/* handled by local core */
			if (loongson_ipi_irq2pos[irq] == -1) {
				do_IRQ(irq);
				continue;
			}

			irqd = irq_get_irq_data(irq);
			cpumask_and(&affinity, irqd->common->affinity, cpu_active_mask);
			if (cpumask_empty(&affinity)) {
				do_IRQ(irq);
				continue;
			}

			irq_cpu[irq] = cpumask_next(irq_cpu[irq], &affinity);
			if (irq_cpu[irq] >= nr_cpu_ids)
				irq_cpu[irq] = cpumask_first(&affinity);

			if (irq_cpu[irq] == 0) {
				do_IRQ(irq);
				continue;
			}

			/* balanced by other cores */
			loongson3_send_irq_by_ipi(irq_cpu[irq], (0x1 << (loongson_ipi_irq2pos[irq])));
		}
	}
}

#define MSI_IRQ_BASE 16
#define MSI_LAST_IRQ 128
#define MSI_TARGET_ADDRESS_HI	0x0
#define MSI_TARGET_ADDRESS_LO	0xFEE00000

static DEFINE_SPINLOCK(bitmap_lock);

int rs780_setup_msi_irq(struct pci_dev *pdev, struct msi_desc *desc)
{
	int irq = irq_alloc_desc_from(MSI_IRQ_BASE, 0);
	struct msi_msg msg;

	if (irq < 0)
		return irq;

	if (irq >= MSI_LAST_IRQ) {
		irq_free_desc(irq);
		return -ENOSPC;
	}

	spin_lock(&bitmap_lock);
	create_ipi_dirq(irq);
	spin_unlock(&bitmap_lock);
	irq_set_msi_desc(irq, desc);

	msg.data = irq;
	msg.address_hi = MSI_TARGET_ADDRESS_HI;
	msg.address_lo = MSI_TARGET_ADDRESS_LO;

	write_msi_msg(irq, &msg);
	irq_set_chip_and_handler(irq, &loongson_msi_irq_chip, handle_edge_irq);

	return 0;
}

void rs780_teardown_msi_irq(unsigned int irq)
{
	irq_free_desc(irq);
	spin_lock(&bitmap_lock);
	destroy_ipi_dirq(irq);
	spin_unlock(&bitmap_lock);
}

void rs780_init_irq(void)
{
	int i;
	struct irq_chip *chip;

	/* Route UART int to cpu Core0 INT0 */
	LOONGSON_INT_ROUTER_UART0 = LOONGSON_INT_COREx_INTy(loongson_sysconf.boot_cpu_id, 0);
	LOONGSON_INT_ROUTER_UART1 = LOONGSON_INT_COREx_INTy(loongson_sysconf.boot_cpu_id, 0);
	/* Route HT1 int0 ~ int3 to cpu Core0 INT1 */
	for (i = 0; i < 4; i++) {
		LOONGSON_INT_ROUTER_HT1(i) = LOONGSON_INT_COREx_INTy(loongson_sysconf.boot_cpu_id, 1);
		/* Enable HT1 interrupts */
		LOONGSON_HT1_INTN_EN(i) = 0xffffffff;
	}
	/* Enable router interrupt intenset */
	LOONGSON_INT_ROUTER_INTENSET = LOONGSON_INT_ROUTER_INTEN |
				       (0xffff << 16) | (0x1 << 15) | (0x1 << 10);

	/* Clear HT1 original interrupts */
	LOONGSON_HT1_INT_VECTOR(1) = 0xffffffff;
	LOONGSON_HT1_INT_VECTOR(2) = 0xffffffff;
	LOONGSON_HT1_INT_VECTOR(3) = 0xffffffff;

	chip = irq_get_chip(I8259A_IRQ_BASE);
	chip->irq_set_affinity = plat_set_irq_affinity;

	if (pci_msi_enabled())
		loongson_pch->irq_dispatch = rs780_msi_irq_dispatch;
	else {
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
}
