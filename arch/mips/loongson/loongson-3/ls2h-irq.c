/*
 *  Copyright (C) 2013, Loongson Technology Corporation Limited, Inc.
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
#include <linux/init.h>
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
#include <linux/cpumask.h>

static struct ls2h_intctl_regs volatile *int_ctrl_regs
	= (struct ls2h_intctl_regs volatile *)(CKSEG1ADDR(LS2H_INT_REG_BASE));

static DEFINE_SPINLOCK(pch_irq_lock);

static void ack_pch_irq(struct irq_data *d)
{
	int irq_nr;
	unsigned long flags;

	spin_lock_irqsave(&pch_irq_lock, flags);

	irq_nr = d->irq - LS2H_PCH_IRQ_BASE;
	(int_ctrl_regs + (irq_nr >> 5))->int_clr = (1 << (irq_nr & 0x1f));

	spin_unlock_irqrestore(&pch_irq_lock, flags);
}

static void mask_pch_irq(struct irq_data *d)
{
	int irq_nr;
	unsigned long flags;

	spin_lock_irqsave(&pch_irq_lock, flags);

	irq_nr = d->irq - LS2H_PCH_IRQ_BASE;
	(int_ctrl_regs + (irq_nr >> 5))->int_en &= ~(1 << (irq_nr & 0x1f));

	spin_unlock_irqrestore(&pch_irq_lock, flags);
}

static void mask_ack_pch_irq(struct irq_data *d)
{
	int irq_nr;
	unsigned long flags;

	spin_lock_irqsave(&pch_irq_lock, flags);

	irq_nr = d->irq - LS2H_PCH_IRQ_BASE;
	(int_ctrl_regs + (irq_nr >> 5))->int_clr = (1 << (irq_nr & 0x1f));
	(int_ctrl_regs + (irq_nr >> 5))->int_en &= ~(1 << (irq_nr & 0x1f));

	spin_unlock_irqrestore(&pch_irq_lock, flags);
}

static void unmask_pch_irq(struct irq_data *d)
{
	int irq_nr;
	unsigned long flags;

	spin_lock_irqsave(&pch_irq_lock, flags);

	irq_nr = d->irq - LS2H_PCH_IRQ_BASE;
	(int_ctrl_regs + (irq_nr >> 5))->int_en |= (1 << (irq_nr & 0x1f));

	spin_unlock_irqrestore(&pch_irq_lock, flags);
}

#define eoi_pch_irq unmask_pch_irq

static struct irq_chip pch_irq_chip = {
	.name		= "Loongson",
	.irq_ack	= ack_pch_irq,
	.irq_mask	= mask_pch_irq,
	.irq_mask_ack	= mask_ack_pch_irq,
	.irq_unmask	= unmask_pch_irq,
	.irq_eoi	= eoi_pch_irq,
};

extern void loongson3_ipi_interrupt(struct pt_regs *regs);

static DEFINE_SPINLOCK(lpc_irq_lock);

static void ack_lpc_irq(struct irq_data *d)
{
	unsigned long flags;

	spin_lock_irqsave(&lpc_irq_lock, flags);

	ls2h_writel(0x1 << (d->irq), LS2H_LPC_INT_CLR);

	spin_unlock_irqrestore(&lpc_irq_lock, flags);
}

static void mask_lpc_irq(struct irq_data *d)
{
	unsigned long flags;

	spin_lock_irqsave(&lpc_irq_lock, flags);

	ls2h_writel(ls2h_readl(LS2H_LPC_INT_ENA) & ~(0x1 << (d->irq)), LS2H_LPC_INT_ENA);

	spin_unlock_irqrestore(&lpc_irq_lock, flags);
}

static void mask_ack_lpc_irq(struct irq_data *d)
{
	unsigned long flags;

	spin_lock_irqsave(&lpc_irq_lock, flags);

	ls2h_writel(0x1 << (d->irq), LS2H_LPC_INT_CLR);
	ls2h_writel(ls2h_readl(LS2H_LPC_INT_ENA) & ~(0x1 << (d->irq)), LS2H_LPC_INT_ENA);

	spin_unlock_irqrestore(&lpc_irq_lock, flags);
}

static void unmask_lpc_irq(struct irq_data *d)
{
	unsigned long flags;

	spin_lock_irqsave(&lpc_irq_lock, flags);

	ls2h_writel(ls2h_readl(LS2H_LPC_INT_ENA) | (0x1 << (d->irq)), LS2H_LPC_INT_ENA);

	spin_unlock_irqrestore(&lpc_irq_lock, flags);
}

#define eoi_lpc_irq unmask_lpc_irq

static struct irq_chip lpc_irq_chip = {
	.name		= "Loongson",
	.irq_ack	= ack_lpc_irq,
	.irq_mask	= mask_lpc_irq,
	.irq_mask_ack	= mask_ack_lpc_irq,
	.irq_unmask	= unmask_lpc_irq,
	.irq_eoi	= eoi_lpc_irq,
};

void ls2h_irq_dispatch(void)
{
	int i, intstatus, irq, irqs, lpc_irq;

	for (i = 0; i < 5; i++) {
		if ((intstatus = (int_ctrl_regs + i)->int_isr) == 0)
			continue;

		while ((irq = ffs(intstatus))) {
			if (!irq) {
				pr_info("Unknow INT%d: intstatus %x\n", i, intstatus);
				spurious_interrupt();
			} else if ((i == 0) && (intstatus & (1 << 13))) {
				irqs = ls2h_readl(LS2H_LPC_INT_ENA) & ls2h_readl(LS2H_LPC_INT_STS) & 0xfeff;
				if (irqs)
					while ((lpc_irq = ffs(irqs))) {
						do_IRQ(lpc_irq - 1);
						irqs &= ~(1 << (lpc_irq-1));
					}
			} else
				do_IRQ(LS2H_PCH_IRQ_BASE + i * 32 + irq - 1);

			intstatus &= ~(1 << (irq-1));
		}
	}
}

void ls2h_irq_router_init(void)
{
	/* Route INTn0 to Core0 INT1 */
	LOONGSON_INT_ROUTER_ENTRY(0) = LOONGSON_INT_COREx_INTy(loongson_boot_cpu_id, 1);

	/* Route the LPC interrupt to Core0 INT0 */
	LOONGSON_INT_ROUTER_LPC = LOONGSON_INT_COREx_INTy(loongson_boot_cpu_id, 0);

	/* Enable UART and INT0 interrupts */
	LOONGSON_INT_ROUTER_INTENSET = (0x1 << 10) | (1 << 0);

	/* uart, keyboard, and mouse are active high */
	(int_ctrl_regs + 0)->int_edge	= 0x00000000;
	(int_ctrl_regs + 0)->int_pol	= 0xff7fffff;
	(int_ctrl_regs + 0)->int_clr	= 0x00000000;
	(int_ctrl_regs + 0)->int_en	= 0x00ffffff;

	(int_ctrl_regs + 1)->int_edge	= 0x00000000;
	(int_ctrl_regs + 1)->int_pol	= 0xfeffffff;
	(int_ctrl_regs + 1)->int_clr	= 0x00000000;
	(int_ctrl_regs + 1)->int_en	= 0x03ffffff;

	(int_ctrl_regs + 2)->int_edge	= 0x00000000;
	(int_ctrl_regs + 2)->int_pol	= 0xfffffffe;
	(int_ctrl_regs + 2)->int_clr	= 0x00000000;
	(int_ctrl_regs + 2)->int_en	= 0x00000001;

	/* Enable the LPC interrupt */
	ls2h_writel(0x80000000, LS2H_LPC_INT_CTL);

	/* set the 18-bit interrpt enable bit for keyboard and mouse */
	ls2h_writel(0x1 << 0x1 | 0x1 << 12, LS2H_LPC_INT_ENA);

	/* clear all 18-bit interrpt bit */
	ls2h_writel(0x3ffff, LS2H_LPC_INT_CLR);
}

void __init ls2h_init_irq(void)
{
	u32 i;

	local_irq_disable();
	ls2h_irq_router_init();

	for (i = LS2H_PCH_IRQ_BASE; i <= LS2H_PCH_LAST_IRQ; i++)
		irq_set_chip_and_handler(i, &pch_irq_chip,
					 handle_level_irq);

	/* added for KBC attached on LPC controler */
	irq_set_chip_and_handler(1, &lpc_irq_chip, handle_level_irq);
	irq_set_chip_and_handler(12, &lpc_irq_chip, handle_level_irq);
}
