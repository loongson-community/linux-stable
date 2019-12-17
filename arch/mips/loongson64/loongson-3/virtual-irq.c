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

void virtio_irq_dispatch(void)
{
	int irq;

	irq = i8259_irq();
	if (irq >= 0)
		do_IRQ(irq);
	else
		spurious_interrupt();
}

void virtio_init_irq(void)
{
	struct irq_chip *chip;

	outb(0xff, 0x4d0);
	outb(0xff, 0x4d1);
	chip = irq_get_chip(I8259A_IRQ_BASE);
	chip->irq_set_affinity = plat_set_irq_affinity;
}
