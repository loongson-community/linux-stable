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
#include <pci.h>
#include <boot_param.h>
#include <loongson-pch.h>

#include <linux/serial_8250.h>
#include <linux/platform_device.h>

#include <linux/i2c.h>
#include <linux/platform_data/i2c-gpio.h>

u32 node_id_offset;

#define LS7A_DMA_CFG	(void *)TO_UNCAC(LS7A_CHIPCFG_REG_BASE + 0x041c)

static void ls7a_early_config(void)
{
	pcie_bus_config = PCIE_BUS_PERFORMANCE;
	node_id_offset = ((readl(LS7A_DMA_CFG) & 0x1f00) >> 8) + 36;
}

static struct resource pci_mem_resource = {
	.name	= "pci memory space",
	.flags	= IORESOURCE_MEM,
};

static struct resource pci_io_resource = {
	.name	= "pci io space",
	.flags	= IORESOURCE_IO,
};

static struct pci_controller ls7a_pci_controller = {
	.pci_ops	= &ls7a_pci_ops,
	.io_resource	= &pci_io_resource,
	.mem_resource	= &pci_mem_resource,
	.mem_offset	= 0x00000000UL,
	.io_offset	= 0x00000000UL,
};

static void __init ls7a_arch_initcall(void)
{
	pci_mem_resource.start = loongson_sysconf.pci_mem_start_addr;
	pci_mem_resource.end   = loongson_sysconf.pci_mem_end_addr;
	pci_io_resource.start  = 0x20000;
	pci_io_resource.end    = 0x3ffff;
	ioport_resource.end    = 0xfffff;
	ls7a_pci_controller.io_map_base = mips_io_port_base;
	register_pci_controller(&ls7a_pci_controller);
}

static void __init ls7a_device_initcall(void)
{
}

struct platform_controller_hub ls7a_pch = {
	.type			= LS7A,
	.pcidev_max_funcs 	= 7,
	.early_config		= ls7a_early_config,
	.init_irq		= ls7a_init_irq,
	.irq_dispatch		= ls7a_irq_dispatch,
	.pch_arch_initcall	= ls7a_arch_initcall,
	.pch_device_initcall	= ls7a_device_initcall,
};
