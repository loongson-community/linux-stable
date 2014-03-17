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
#include <asm/io.h>
#include <boot_param.h>
#include <loongson-pch.h>

extern void rs780_init_irq(void);
extern void rs780_irq_dispatch(void);
extern int ls2h_platform_init(void);

extern int rs780_pcibios_map_irq(struct pci_dev *dev, u8 slot, u8 pin);

static void rs780_early_config(void)
{
}

static void __init rs780_arch_initcall(void)
{
}

static void __init rs780_device_initcall(void)
{
}

const struct platform_controller_hub rs780_pch = {
	.board_type		= RS780E,
	.pcidev_max_funcs 	= 7,
	.early_config		= rs780_early_config,
	.init_irq		= rs780_init_irq,
	.irq_dispatch		= rs780_irq_dispatch,
	.pcibios_map_irq	= rs780_pcibios_map_irq,
	.pch_arch_initcall	= rs780_arch_initcall,
	.pch_device_initcall	= rs780_device_initcall,
};
