/*
 *  Copyright (C) 2013, Loongson Technology Corporation Limited, Inc.
 *  Copyright (C) 2014-2017, Lemote, Inc.
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
#include <pci.h>
#include <boot_param.h>
#include <loongson-pch.h>

static void rs780_early_config(void)
{
}

static struct resource pci_mem_resource = {
	.name	= "pci memory space",
	.flags	= IORESOURCE_MEM,
};

static struct resource pci_io_resource = {
	.name	= "pci io space",
	.start	= LOONGSON_PCI_IO_START,
	.end	= IO_SPACE_LIMIT,
	.flags	= IORESOURCE_IO,
};

static struct pci_controller rs780_pci_controller = {
	.pci_ops	= &rs780_pci_ops,
	.io_resource	= &pci_io_resource,
	.mem_resource	= &pci_mem_resource,
	.mem_offset	= 0x00000000UL,
	.io_offset	= 0x00000000UL,
};

static void __init rs780_arch_initcall(void)
{
	pci_mem_resource.start = loongson_sysconf.pci_mem_start_addr;
	pci_mem_resource.end   = loongson_sysconf.pci_mem_end_addr;
	rs780_pci_controller.io_map_base = mips_io_port_base;
	register_pci_controller(&rs780_pci_controller);
}

static void __init rs780_device_initcall(void)
{
}

struct platform_controller_hub rs780_pch = {
	.type			= RS780E,
	.pcidev_max_funcs 	= 7,
	.early_config		= rs780_early_config,
	.init_irq		= rs780_init_irq,
	.irq_dispatch		= rs780_irq_dispatch,
	.pch_arch_initcall	= rs780_arch_initcall,
	.pch_device_initcall	= rs780_device_initcall,
};
