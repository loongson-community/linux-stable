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

#include <linux/i2c.h>
#include <linux/serial_8250.h>
#include <linux/platform_device.h>

void pci_no_msi(void);

static void virtio_early_config(void)
{
	pci_no_msi();
}

static struct resource pci_mem_resource = {
	.name	= "pci memory space",
	.flags	= IORESOURCE_MEM,
};

static struct resource pci_io_resource = {
	.name	= "pci io space",
	.flags	= IORESOURCE_IO,
};

static struct pci_controller virtio_pci_controller = {
	.pci_ops	= &virtio_pci_ops,
	.io_resource	= &pci_io_resource,
	.mem_resource	= &pci_mem_resource,
	.mem_offset	= 0x00000000UL,
	.io_offset	= 0x00000000UL,
};

static void __init virtio_arch_initcall(void)
{
	pci_mem_resource.start = loongson_sysconf.pci_mem_start_addr;
	pci_mem_resource.end   = loongson_sysconf.pci_mem_end_addr;
	pci_io_resource.start  = LOONGSON_PCI_IO_START;
	pci_io_resource.end    = 0x3ffff;
	ioport_resource.end    = 0xfffff;
	virtio_pci_controller.io_map_base = mips_io_port_base;
	register_pci_controller(&virtio_pci_controller);
}

static void __init virtio_device_initcall(void)
{
}

struct platform_controller_hub virtual_pch = {
	.type			= VIRTUAL,
	.pcidev_max_funcs 	= 7,
	.early_config		= virtio_early_config,
	.init_irq		= virtio_init_irq,
	.irq_dispatch		= virtio_irq_dispatch,
	.pch_arch_initcall	= virtio_arch_initcall,
	.pch_device_initcall	= virtio_device_initcall,
};
