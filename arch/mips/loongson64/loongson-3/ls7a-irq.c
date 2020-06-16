/*
 *  Copyright (C) 2017, Loongson Technology Corporation Limited, Inc.
 *
 *  This program is free software; you can distribute it and/or modify it
 *  under the terms of the GNU General Public License (Version 2) as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 */
#include <linux/interrupt.h>
#include <linux/irqchip.h>
#include <linux/module.h>

#include <asm/irq_cpu.h>
#include <asm/i8259.h>
#include <asm/mipsregs.h>
#include <irq.h>
#include <loongson.h>
#include <loongson-pch.h>

static unsigned int irq_cpu[NR_IRQS] = {[0 ... NR_IRQS-1] = -1};

static DEFINE_RAW_SPINLOCK(pch_irq_lock);

static void mask_pch_irq(struct irq_data *d)
{
	int irq_nr;
	unsigned long flags;

	if (d->irq < 16) {
		local_irq_save(flags);
		writel(readl(LS7A_LPC_INT_ENA) & ~(0x1 << (d->irq)), LS7A_LPC_INT_ENA);
		local_irq_restore(flags);
	} else {
		raw_spin_lock_irqsave(&pch_irq_lock, flags);
		irq_nr = d->irq - LS7A_PCH_IRQ_BASE;
		writeq(readq(LS7A_INT_MASK_REG) | (1ULL << irq_nr), LS7A_INT_MASK_REG);
		raw_spin_unlock_irqrestore(&pch_irq_lock, flags);
	}
}

static void unmask_pch_irq(struct irq_data *d)
{
	int irq_nr;
	unsigned long flags;

	if (d->irq < 16) {
		local_irq_save(flags);
		writel(readl(LS7A_LPC_INT_ENA) | (0x1 << (d->irq)), LS7A_LPC_INT_ENA);
		local_irq_restore(flags);
	} else {
		raw_spin_lock_irqsave(&pch_irq_lock, flags);
		irq_nr = d->irq - LS7A_PCH_IRQ_BASE;
		writeq(readq(LS7A_INT_MASK_REG) & ~(1ULL << irq_nr), LS7A_INT_MASK_REG);
		raw_spin_unlock_irqrestore(&pch_irq_lock, flags);
	}
}

static struct irq_chip pch_irq_chip = {
	.name			= "Loongson",
	.irq_mask		= mask_pch_irq,
	.irq_unmask		= unmask_pch_irq,
	.irq_set_affinity	= plat_set_irq_affinity,
	.flags			= IRQCHIP_MASK_ON_SUSPEND,
};

#define LPC_OFFSET 19

/* Handle LPC IRQs */
static irqreturn_t lpc_irq_dispatch(int irq, void *data)
{
	int irqs;

	irqs = readl(LS7A_LPC_INT_ENA) & readl(LS7A_LPC_INT_STS);
	if (!irqs)
		return IRQ_NONE;

	while ((irq = ffs(irqs))) {
		do_IRQ(irq - 1);
		irqs &= ~(1 << (irq - 1));
	}

	return IRQ_HANDLED;
}

static struct irqaction lpc_irqaction = {
	.name = "lpc",
	.flags = IRQF_NO_THREAD,
	.handler = lpc_irq_dispatch,
};

void ls7a_irq_dispatch(void)
{
	struct irq_data *irqd;
	struct cpumask affinity;
	unsigned long irq, flags, intmask, intstatus;

	raw_spin_lock_irqsave(&pch_irq_lock, flags);
	intmask = readq(LS7A_INT_MASK_REG);
	intstatus = readq(LS7A_INT_STATUS_REG);
	writeq((intstatus | intmask), LS7A_INT_MASK_REG);
	raw_spin_unlock_irqrestore(&pch_irq_lock, flags);

	/* Handle normal IRQs */
	while (intstatus) {
		irq = __ffs(intstatus);
		intstatus &= ~(1ULL << irq);
		irq = LS7A_PCH_IRQ_BASE + irq;

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

void ls7a_init_irq(void)
{
	int i;

	/* Route LPC int to cpu Core0 INT0 */
	LOONGSON_INT_ROUTER_LPC = LOONGSON_INT_COREx_INTy(loongson_sysconf.boot_cpu_id, 0);
	LOONGSON_INT_ROUTER_INTENSET = LOONGSON_INT_ROUTER_INTEN | 0x1 << 10;

	/* Route INTn0 to Core0 INT1 (IP3) */
	LOONGSON_INT_ROUTER_ENTRY(0) = LOONGSON_INT_COREx_INTy(loongson_sysconf.boot_cpu_id, 1);
	LOONGSON_INT_ROUTER_INTENSET = LOONGSON_INT_ROUTER_INTEN | 0x1 << 0;

	/* Route IRQs to LS7A INT0 */
	for (i = LS7A_PCH_IRQ_BASE; i < LS7A_PCH_LAST_IRQ; i++)
		writeb(0x1, (LS7A_INT_ROUTE_ENTRY_REG + i - LS7A_PCH_IRQ_BASE));

	writeq(0x0ULL, LS7A_INT_EDGE_REG);
	writeq(0x0ULL, LS7A_INT_STATUS_REG);
	/* Mask all interrupts except LPC (bit 19) */
	writeq(0xfffffffffff7ffffULL, LS7A_INT_MASK_REG);
	writeq(0xffffffffffffffffULL, LS7A_INT_CLEAR_REG);

	/* Enable the LPC interrupt */
	writel(0x80000000, LS7A_LPC_INT_CTL);
	/* Clear all 18-bit interrupt bits */
	writel(0x3ffff, LS7A_LPC_INT_CLR);

	for (i = 0; i < NR_IRQS; i++)
		loongson_ipi_irq2pos[i] = -1;
	for (i = 0; i < NR_DIRQS; i++)
		loongson_ipi_pos2irq[i] = -1;
	create_ipi_dirq(LS7A_PCH_SATA0_IRQ);
	create_ipi_dirq(LS7A_PCH_SATA1_IRQ);
	create_ipi_dirq(LS7A_PCH_SATA2_IRQ);
	create_ipi_dirq(LS7A_PCH_GMAC0_SBD_IRQ);
	create_ipi_dirq(LS7A_PCH_GMAC1_SBD_IRQ);
	create_ipi_dirq(LS7A_PCH_PCIE_F0_PORT0_IRQ);
	create_ipi_dirq(LS7A_PCH_PCIE_F0_PORT1_IRQ);
	create_ipi_dirq(LS7A_PCH_PCIE_F0_PORT2_IRQ);
	create_ipi_dirq(LS7A_PCH_PCIE_F0_PORT3_IRQ);
	create_ipi_dirq(LS7A_PCH_PCIE_F1_PORT0_IRQ);
	create_ipi_dirq(LS7A_PCH_PCIE_F1_PORT1_IRQ);
	create_ipi_dirq(LS7A_PCH_PCIE_G0_LO_IRQ);
	create_ipi_dirq(LS7A_PCH_PCIE_G0_HI_IRQ);
	create_ipi_dirq(LS7A_PCH_PCIE_G1_LO_IRQ);
	create_ipi_dirq(LS7A_PCH_PCIE_G1_HI_IRQ);
	create_ipi_dirq(LS7A_PCH_PCIE_H_LO_IRQ);
	create_ipi_dirq(LS7A_PCH_PCIE_H_HI_IRQ);
	create_ipi_dirq(LS7A_PCH_EHCI0_IRQ);
	create_ipi_dirq(LS7A_PCH_EHCI1_IRQ);
	create_ipi_dirq(LS7A_PCH_OHCI0_IRQ);
	create_ipi_dirq(LS7A_PCH_OHCI1_IRQ);
}

int __init ls7a_irq_of_init(struct device_node *node, struct device_node *parent)
{
	u32 i;

	irq_domain_add_legacy(node, 64, LS7A_PCH_IRQ_BASE,
			LS7A_PCH_IRQ_BASE, &irq_domain_simple_ops, NULL);

	irq_set_chip_and_handler(1, &pch_irq_chip, handle_level_irq);
	irq_set_chip_and_handler(4, &pch_irq_chip, handle_level_irq);
	irq_set_chip_and_handler(12, &pch_irq_chip, handle_level_irq);

	for (i = LS7A_PCH_IRQ_BASE; i < LS7A_PCH_LAST_IRQ; i++)
		irq_set_chip_and_handler(i, &pch_irq_chip, handle_level_irq);
	setup_irq(LS7A_PCH_IRQ_BASE + LPC_OFFSET, &lpc_irqaction);

	return 0;
}
IRQCHIP_DECLARE(plat_intc, "loongson,ls7a-interrupt-controller", ls7a_irq_of_init);
