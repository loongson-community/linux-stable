/*
 *  Lemote Loongson-3A family machines' specific suspend support
 *
 *  Copyright (C) 2009 Lemote Inc.
 *  Author: Wu Zhangjin <wuzhangjin@gmail.com>
 *  Author: Chen Huacai <chenhuacai@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/suspend.h>
#include <linux/interrupt.h>
#include <linux/pm.h>
#include <linux/i8042.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/delay.h>

#include <asm/i8259.h>
#include <asm/mipsregs.h>
#include <asm/bootinfo.h>
#include <asm/tlbflush.h>

#include <loongson.h>
#include <ec_wpce775l.h>

#define I8042_CTR_KBDINT	0x01
#define I8042_CTR_KBDDIS	0x10
#define I8042_KBD_IRQ		1

extern void irq_router_init(void);
extern void acpi_sleep_prepare(void);
extern void acpi_sleep_complete(void);
extern void acpi_registers_setup(void);

static unsigned char i8042_ctr;

static int i8042_enable_kbd_port(void)
{
	if (i8042_command(&i8042_ctr, I8042_CMD_CTL_RCTR)) {
		pr_err("i8042.c: Can't read CTR while enabling i8042 kbd port."
		       "\n");
		return -EIO;
	}

	i8042_ctr &= ~I8042_CTR_KBDDIS;
	i8042_ctr |= I8042_CTR_KBDINT;

	if (i8042_command(&i8042_ctr, I8042_CMD_CTL_WCTR)) {
		i8042_ctr &= ~I8042_CTR_KBDINT;
		i8042_ctr |= I8042_CTR_KBDDIS;
		pr_err("i8042.c: Failed to enable KBD port.\n");

		return -EIO;
	}

	return 0;
}

void setup_wakeup_events(void)
{
	switch (mips_machtype) {
	case MACH_LEMOTE_A1004:
		/* open the keyboard irq in i8259A */
		outb_p((0xff & ~(1 << I8042_KBD_IRQ)), PIC_MASTER_IMR);
		/* enable keyboard port */
		i8042_enable_kbd_port();
		break;

	default:
		break;
	}
}

int wakeup_loongson(void)
{
	int irq;

	/* query the interrupt number */
	irq = LOONGSON_HT1_INT_VECTOR(0);

	if (irq & (1<<I8042_KBD_IRQ)) {
		return 1;
	}

	return 0;
}

void mach_suspend(suspend_state_t state)
{
	if (state == PM_SUSPEND_MEM)
		acpi_sleep_prepare();

	/* Workaround: disable spurious IRQ1 via EC */
	if (state == PM_SUSPEND_STANDBY) {
		ec_write_noindex(CMD_RESET, STANDBY_ON);
		mdelay(950);
	}
}

void mach_resume(suspend_state_t state)
{
	if (state == PM_SUSPEND_MEM) {
		local_flush_tlb_all();
		irq_router_init();
		acpi_registers_setup();
		acpi_sleep_complete();
	}
}
