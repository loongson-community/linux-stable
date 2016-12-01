// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright (C) 2013, Loongson Technology Corporation Limited, Inc.
 *  Copyright (C) 2014-2017, Lemote, Inc.
 */
#include <linux/init.h>
#include <linux/cpumask.h>
#include <linux/irqchip.h>
#include <linux/interrupt.h>

#include <asm/io.h>
#include <asm/irq_cpu.h>
#include <asm/mipsregs.h>
#include <asm/smp.h>
#include <asm/delay.h>
#include <irq.h>
#include <loongson.h>
#include <boot_param.h>
#include <loongson-pch.h>

static unsigned int irq_cpu[NR_IRQS] = {[0 ... NR_IRQS-1] = -1};

struct intctl_regs {
	volatile u32 int_isr;
	volatile u32 int_en;
	volatile u32 int_set;
	volatile u32 int_clr;
	volatile u32 int_pol;
	volatile u32 int_edge;
};
static struct intctl_regs volatile *int_ctrl_regs =
	(struct intctl_regs volatile *)(CKSEG1ADDR(LS2H_INT_REG_BASE));

static DEFINE_RAW_SPINLOCK(pch_irq_lock);

static void mask_pch_irq(struct irq_data *d)
{
	int irq_nr;
	unsigned long flags;

	if (d->irq < 16) {
		local_irq_save(flags);
		writel(readl(LS2H_LPC_INT_ENA) & ~(0x1 << (d->irq)), LS2H_LPC_INT_ENA);
		local_irq_restore(flags);
	} else {
		raw_spin_lock_irqsave(&pch_irq_lock, flags);
		irq_nr = d->irq - LS2H_PCH_IRQ_BASE;
		(int_ctrl_regs + (irq_nr >> 5))->int_en &= ~(1 << (irq_nr & 0x1f));
		raw_spin_unlock_irqrestore(&pch_irq_lock, flags);
	}
}

static void unmask_pch_irq(struct irq_data *d)
{
	int irq_nr;
	unsigned long flags;

	if (d->irq < 16) {
		local_irq_save(flags);
		writel(readl(LS2H_LPC_INT_ENA) | (0x1 << (d->irq)), LS2H_LPC_INT_ENA);
		local_irq_restore(flags);
	} else {
		raw_spin_lock_irqsave(&pch_irq_lock, flags);
		irq_nr = d->irq - LS2H_PCH_IRQ_BASE;
		(int_ctrl_regs + (irq_nr >> 5))->int_en |= (1 << (irq_nr & 0x1f));
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

#define LPC_OFFSET 13

/* Handle LPC IRQs */
static irqreturn_t lpc_irq_dispatch(int irq, void *data)
{
	int irqs;

	irqs = readl(LS2H_LPC_INT_ENA) & readl(LS2H_LPC_INT_STS);
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

void ls2h_irq_dispatch(void)
{
	unsigned long flags;
	struct irq_data *irqd;
	struct cpumask affinity;
	unsigned int i, irq, inten, intstatus;

	for (i = 0; i < 3; i++) {
		raw_spin_lock_irqsave(&pch_irq_lock, flags);
		inten = (int_ctrl_regs + i)->int_en;
		intstatus = (int_ctrl_regs + i)->int_isr;
		(int_ctrl_regs + i)->int_en = (inten & ~intstatus);
		raw_spin_unlock_irqrestore(&pch_irq_lock, flags);

		if (intstatus == 0)
			continue;

		/* Handle normal IRQs */
		while (intstatus) {
			irq = ffs(intstatus);
			intstatus &= ~(1 << (irq-1));
			irq = LS2H_PCH_IRQ_BASE + i * 32 + irq - 1;

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

void ls2h_init_irq(void)
{
	int i;

	/* Route INTn0 to Core0 INT1 */
	LOONGSON_INT_ROUTER_ENTRY(0) = LOONGSON_INT_COREx_INTy(loongson_sysconf.boot_cpu_id, 1);

	/* Route the LPC interrupt to Core0 INT0 */
	LOONGSON_INT_ROUTER_LPC = LOONGSON_INT_COREx_INTy(loongson_sysconf.boot_cpu_id, 0);

	/* Enable UART and INT0 interrupts */
	LOONGSON_INT_ROUTER_INTENSET = (0x1 << 10) | (1 << 0);

	/* uart, keyboard, and mouse are active high */
	(int_ctrl_regs + 0)->int_edge	= 0x00000000;
	(int_ctrl_regs + 0)->int_pol	= 0xff7fffff;
	(int_ctrl_regs + 0)->int_en	= 0x00002000;
	(int_ctrl_regs + 0)->int_clr	= 0xffffffff;

	(int_ctrl_regs + 1)->int_edge	= 0x00000000;
	(int_ctrl_regs + 1)->int_pol	= 0xfeffffff;
	(int_ctrl_regs + 1)->int_en	= 0x00000000;
	(int_ctrl_regs + 1)->int_clr	= 0xffffffff;

	(int_ctrl_regs + 2)->int_edge	= 0x00000000;
	(int_ctrl_regs + 2)->int_pol	= 0xfffffffe;
	(int_ctrl_regs + 2)->int_en	= 0x00000000;
	(int_ctrl_regs + 2)->int_clr	= 0xffffffff;

	/* Enable the LPC interrupt */
	writel(0x80000000, LS2H_LPC_INT_CTL);
	/* Clear all 18-bit interrupt bits */
	writel(0x3ffff, LS2H_LPC_INT_CLR);

	for (i = 0; i < NR_IRQS; i++)
		loongson_ipi_irq2pos[i] = -1;
	for (i = 0; i < NR_DIRQS; i++)
		loongson_ipi_pos2irq[i] = -1;
	create_ipi_dirq(LS2H_PCH_SATA_IRQ);
	create_ipi_dirq(LS2H_PCH_GMAC0_IRQ);
	create_ipi_dirq(LS2H_PCH_GMAC1_IRQ);
	create_ipi_dirq(LS2H_PCH_PCIE_PORT0_IRQ);
	create_ipi_dirq(LS2H_PCH_PCIE_PORT1_IRQ);
	create_ipi_dirq(LS2H_PCH_PCIE_PORT2_IRQ);
	create_ipi_dirq(LS2H_PCH_PCIE_PORT3_IRQ);
	create_ipi_dirq(LS2H_PCH_OTG_IRQ);
	create_ipi_dirq(LS2H_PCH_EHCI_IRQ);
	create_ipi_dirq(LS2H_PCH_OHCI_IRQ);
}

int __init ls2h_irq_of_init(struct device_node *node, struct device_node *parent)
{
	u32 i;

	irq_domain_add_legacy(node, 160, LS2H_PCH_IRQ_BASE,
			LS2H_PCH_IRQ_BASE, &irq_domain_simple_ops, NULL);

	irq_set_chip_and_handler(1, &pch_irq_chip, handle_level_irq);
	irq_set_chip_and_handler(4, &pch_irq_chip, handle_level_irq);
	irq_set_chip_and_handler(12, &pch_irq_chip, handle_level_irq);

	for (i = LS2H_PCH_IRQ_BASE; i < LS2H_PCH_LAST_IRQ; i++)
		irq_set_chip_and_handler(i, &pch_irq_chip, handle_level_irq);
	setup_irq(LS2H_PCH_IRQ_BASE + LPC_OFFSET, &lpc_irqaction);

	return 0;
}
IRQCHIP_DECLARE(plat_intc, "loongson,ls2h-interrupt-controller", ls2h_irq_of_init);
