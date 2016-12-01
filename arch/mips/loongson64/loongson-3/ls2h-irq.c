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

static unsigned int irq_cpu[224] = {[0 ... 223] = -1};
extern void loongson3_send_irq_by_ipi(int cpu, int irqs);

unsigned int ls2h_ipi_irq2pos[224] = {
	[0 ... 223] = -1,
	[LS2H_PCH_SATA_IRQ] = 0,
	[LS2H_PCH_GMAC0_IRQ] = 1,
	[LS2H_PCH_GMAC1_IRQ] = 2,
	[LS2H_PCH_PCIE_PORT0_IRQ] = 3,
	[LS2H_PCH_PCIE_PORT1_IRQ] = 4,
	[LS2H_PCH_PCIE_PORT2_IRQ] = 5,
	[LS2H_PCH_PCIE_PORT3_IRQ] = 6,
	[LS2H_PCH_OTG_IRQ] = 7,
	[LS2H_PCH_EHCI_IRQ] = 8,
	[LS2H_PCH_OHCI_IRQ] = 9,
};

unsigned int ls2h_ipi_pos2irq[] = {
	LS2H_PCH_SATA_IRQ,
	LS2H_PCH_GMAC0_IRQ,
	LS2H_PCH_GMAC1_IRQ,
	LS2H_PCH_PCIE_PORT0_IRQ,
	LS2H_PCH_PCIE_PORT1_IRQ,
	LS2H_PCH_PCIE_PORT2_IRQ,
	LS2H_PCH_PCIE_PORT3_IRQ,
	LS2H_PCH_OTG_IRQ,
	LS2H_PCH_EHCI_IRQ,
	LS2H_PCH_OHCI_IRQ,
};

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

static void ack_pch_irq(struct irq_data *d)
{
	int irq_nr;
	unsigned long flags;

	if (d->irq < 16) {
		local_irq_save(flags);
		writel(0x1 << (d->irq), LS2H_LPC_INT_CLR);
		local_irq_restore(flags);
	} else {
		raw_spin_lock_irqsave(&pch_irq_lock, flags);
		irq_nr = d->irq - LS2H_PCH_IRQ_BASE;
		(int_ctrl_regs + (irq_nr >> 5))->int_clr = (1 << (irq_nr & 0x1f));
		raw_spin_unlock_irqrestore(&pch_irq_lock, flags);
	}
}

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

static void mask_ack_pch_irq(struct irq_data *d)
{
	int irq_nr;
	unsigned long flags;

	if (d->irq < 16) {
		local_irq_save(flags);
		writel(readl(LS2H_LPC_INT_ENA) & ~(0x1 << (d->irq)), LS2H_LPC_INT_ENA);
		writel(0x1 << (d->irq), LS2H_LPC_INT_CLR);
		local_irq_restore(flags);
	} else {
		raw_spin_lock_irqsave(&pch_irq_lock, flags);
		irq_nr = d->irq - LS2H_PCH_IRQ_BASE;
		(int_ctrl_regs + (irq_nr >> 5))->int_en &= ~(1 << (irq_nr & 0x1f));
		(int_ctrl_regs + (irq_nr >> 5))->int_clr = (1 << (irq_nr & 0x1f));
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

#define eoi_pch_irq unmask_pch_irq

static struct irq_chip pch_irq_chip = {
	.name			= "Loongson",
	.irq_ack		= ack_pch_irq,
	.irq_mask		= mask_pch_irq,
	.irq_mask_ack		= mask_ack_pch_irq,
	.irq_unmask		= unmask_pch_irq,
	.irq_eoi		= eoi_pch_irq,
	.irq_set_affinity	= plat_set_irq_affinity,
};

#define LPC_OFFSET 13

void ls2h_irq_dispatch(void)
{
	unsigned long flags;
	struct irq_data *irqd;
	struct cpumask affinity;
	unsigned int i, irq, inten, intstatus, lints = 0;

	for (i = 0; i < 3; i++) {
		raw_spin_lock_irqsave(&pch_irq_lock, flags);
		inten = (int_ctrl_regs + i)->int_en;
		intstatus = (int_ctrl_regs + i)->int_isr;
		(int_ctrl_regs + i)->int_en = (inten & ~intstatus) | (1 << LPC_OFFSET);
		raw_spin_unlock_irqrestore(&pch_irq_lock, flags);

		if (intstatus == 0)
			continue;

		/* Get LPC IRQs */
		if ((i == 0) && (intstatus & (1 << LPC_OFFSET))) {
			intstatus &= ~(1 << LPC_OFFSET);
			lints = readl(LS2H_LPC_INT_ENA) & readl(LS2H_LPC_INT_STS);
		}

		/* Handle normal IRQs */
		while (intstatus) {
			irq = ffs(intstatus);
			intstatus &= ~(1 << (irq-1));
			irq = LS2H_PCH_IRQ_BASE + i * 32 + irq - 1;

			/* handled by local core */
			if (ls2h_ipi_irq2pos[irq] == -1) {
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
			loongson3_send_irq_by_ipi(irq_cpu[irq], (0x1 << (ls2h_ipi_irq2pos[irq])));
		}
	}

	/* Handle LPC IRQs */
	if (lints) {
		while ((irq = ffs(lints))) {
			do_IRQ(irq - 1);
			lints &= ~(1 << (irq-1));
		}
		int_ctrl_regs->int_clr = (1 << LPC_OFFSET);
	}
}

void ls2h_init_irq(void)
{
	/* Route INTn0 to Core0 INT1 */
	LOONGSON_INT_ROUTER_ENTRY(0) = LOONGSON_INT_COREx_INTy(loongson_sysconf.boot_cpu_id, 1);

	/* Route the LPC interrupt to Core0 INT0 */
	LOONGSON_INT_ROUTER_LPC = LOONGSON_INT_COREx_INTy(loongson_sysconf.boot_cpu_id, 0);

	/* Enable UART and INT0 interrupts */
	LOONGSON_INT_ROUTER_INTENSET = (0x1 << 10) | (1 << 0);

	/* uart, keyboard, and mouse are active high */
	(int_ctrl_regs + 0)->int_edge	= 0x00000000;
	(int_ctrl_regs + 0)->int_pol	= 0xff7fffff;
	(int_ctrl_regs + 0)->int_en	= 0x00ffffff;
	(int_ctrl_regs + 0)->int_clr	= 0x00000000;

	(int_ctrl_regs + 1)->int_edge	= 0x00000000;
	(int_ctrl_regs + 1)->int_pol	= 0xfeffffff;
	(int_ctrl_regs + 1)->int_en	= 0x03ffffff;
	(int_ctrl_regs + 1)->int_clr	= 0x00000000;

	(int_ctrl_regs + 2)->int_edge	= 0x00000000;
	(int_ctrl_regs + 2)->int_pol	= 0xfffffffe;
	(int_ctrl_regs + 2)->int_en	= 0x00000001;
	(int_ctrl_regs + 2)->int_clr	= 0x00000000;

	/* Enable the LPC interrupt */
	writel(0x80000000, LS2H_LPC_INT_CTL);

	/* set the 18-bit interrpt enable bit for keyboard and mouse */
	writel(0x1 << 1 | 0x1 << 4 | 0x1 << 12, LS2H_LPC_INT_ENA);

	/* clear all 18-bit interrpt bit */
	writel(0x3ffff, LS2H_LPC_INT_CLR);
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

	return 0;
}
IRQCHIP_DECLARE(plat_intc, "loongson,ls2h-interrupt-controller", ls2h_irq_of_init);
