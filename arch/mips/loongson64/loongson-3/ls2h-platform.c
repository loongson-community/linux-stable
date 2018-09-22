// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright (C) 2013, Loongson Technology Corporation Limited, Inc.
 *  Copyright (C) 2014-2017, Lemote, Inc.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <asm/io.h>

#include <irq.h>
#include <pci.h>
#include <boot_param.h>
#include <loongson-pch.h>

/*
 * PCI Controller
 */
static int nr_pci_ports;

static struct resource pci_mem_resource[4] = {
	{
		.name	= "pci memory space",
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "pci memory space",
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "pci memory space",
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "pci memory space",
		.flags	= IORESOURCE_MEM,
	}
};

static struct resource pci_io_resource[4] = {
	{
		.name	= "pci io space",
		.flags	= IORESOURCE_IO,
	},
	{
		.name	= "pci io space",
		.flags	= IORESOURCE_IO,
	},
	{
		.name	= "pci io space",
		.flags	= IORESOURCE_IO,
	},
	{
		.name	= "pci io space",
		.flags	= IORESOURCE_IO,
	}
};

static struct pci_controller ls2h_pci_controller[4] = {
	{
		.pci_ops	= &ls2h_pci_ops[0],
		.mem_resource	= &pci_mem_resource[0],
		.io_resource	= &pci_io_resource[0],
		.mem_offset	= 0x00000000UL,
		.io_offset	= 0x00000000UL,
		.io_map_base	= 0x90000e0018000000UL,
	},
	{
		.pci_ops	= &ls2h_pci_ops[1],
		.mem_resource	= &pci_mem_resource[1],
		.io_resource	= &pci_io_resource[1],
		.mem_offset	= 0x00000000UL,
		.io_offset	= 0x00000000UL,
		.io_map_base	= 0x90000e0018000000UL,
	},
	{
		.pci_ops	= &ls2h_pci_ops[2],
		.mem_resource	= &pci_mem_resource[2],
		.io_resource	= &pci_io_resource[2],
		.mem_offset	= 0x00000000UL,
		.io_offset	= 0x00000000UL,
		.io_map_base	= 0x90000e0018000000UL,
	},
	{
		.pci_ops	= &ls2h_pci_ops[3],
		.mem_resource	= &pci_mem_resource[3],
		.io_resource	= &pci_io_resource[3],
		.mem_offset	= 0x00000000UL,
		.io_offset	= 0x00000000UL,
		.io_map_base	= 0x90000e0018000000UL,
	}
};

static void en_ref_clock(void)
{
	unsigned int data;

	data = readl(LS2H_CLK_CTRL3_REG);
	data |= (LS2H_CLK_CTRL3_BIT_PEREF_EN(0)
		 | LS2H_CLK_CTRL3_BIT_PEREF_EN(1)
		 | LS2H_CLK_CTRL3_BIT_PEREF_EN(2)
		 | LS2H_CLK_CTRL3_BIT_PEREF_EN(3));
	writel(data, LS2H_CLK_CTRL3_REG);
}

static int pcie_is_x4_mode(void)
{
	u32 data = readl((u32 *)(LS2H_PCIE_PORT_REG_BASE(0) | LS2H_PCIE_PORT_REG_CTR_STAT));

	return data & LS2H_PCIE_REG_CTR_STAT_BIT_ISX4;
}

static void pcie_port_init(int port)
{
	unsigned int *reg, data;

	reg = (void *)(LS2H_PCIE_PORT_REG_BASE(port) | LS2H_PCIE_PORT_REG_CTR0);
	writel(0x00ff204c, reg);

	reg = (void *)(LS2H_PCIE_PORT_HEAD_BASE(port) | PCI_CLASS_REVISION);
	data = (readl(reg) & 0xffff) | (PCI_CLASS_BRIDGE_PCI << 16);
	writel(data, reg);

	reg = (void *)(LS2H_PCIE_PORT_HEAD_BASE(port) | LS2H_PCI_EXP_LNKCAP);
	data = (readl(reg) & (~0xf)) | 0x1;
	writel(data, reg);
}

void pci_no_msi(void);

static void ls2h_early_config(void)
{
	u32 i, val;

	/*
	 * Loongson-2H chip_config0: 0x1fd00200
	 * bit[5]: 	Loongson-2H bridge mode,0: disable      1: enable
	 * bit[4]:	ac97/hda select,	0: ac97		1: hda
	 * bit[14]:	host/otg select,	0: host         1: otg
	 * bit[26]:	usb reset,		0: enable       1: disable
	 */
	val = readl(LS2H_CHIP_CFG0_REG);
	writel(val | (1 << 5) | (1 << 4) | (1 << 14) | (1 << 26), LS2H_CHIP_CFG0_REG);

	val = readl(LS2H_GPIO_OE_REG);
	writel(val | (1 << 0), LS2H_GPIO_OE_REG);

	en_ref_clock();
	pcie_bus_config = PCIE_BUS_PERFORMANCE;

	if (!cpu_support_msi())
		pci_no_msi();

	val = readl((void *)(LS2H_PCIE_PORT_REG_BASE(0) | LS2H_PCIE_PORT_REG_CTR_STAT));
	val |= LS2H_PCIE_REG_CTR_STAT_BIT_ISRC;  /* Enable RC mode */
	writel(val, (void *)(LS2H_PCIE_PORT_REG_BASE(0) | LS2H_PCIE_PORT_REG_CTR_STAT));

	if (pcie_is_x4_mode())
		nr_pci_ports = 1;
	else
		nr_pci_ports = 4;

	for (i = 0; i < nr_pci_ports; i++)
		pcie_port_init(i);
}

static void __init ls2h_arch_initcall(void)
{
	u64 i, pci_mem_size;

	if (!loongson_sysconf.pci_mem_start_addr)
		loongson_sysconf.pci_mem_start_addr = LOONGSON_PCI_MEM_START;
	if (!loongson_sysconf.pci_mem_end_addr)
		loongson_sysconf.pci_mem_end_addr = LOONGSON_PCI_MEM_START + 0x40000000UL - 1;
	pci_mem_size = loongson_sysconf.pci_mem_end_addr - loongson_sysconf.pci_mem_start_addr + 1;

	ioport_resource.end = 0xffffffff;
	for (i = 0; i < nr_pci_ports; i++) {
		pci_io_resource[i].start = 0x400000*i + 0x100000;
		pci_io_resource[i].end   = 0x400000*i + 0x10ffff;
		pci_mem_resource[i].start =
			loongson_sysconf.pci_mem_start_addr + pci_mem_size*i/nr_pci_ports;
		pci_mem_resource[i].end   =
			loongson_sysconf.pci_mem_start_addr + pci_mem_size*(i+1)/nr_pci_ports - 1;
		register_pci_controller(&ls2h_pci_controller[i]);
	}
}

static void __init ls2h_device_initcall(void)
{
}

struct platform_controller_hub ls2h_pch = {
	.type			= LS2H,
	.pcidev_max_funcs 	= 1,
	.early_config		= ls2h_early_config,
	.init_irq		= ls2h_init_irq,
	.irq_dispatch		= ls2h_irq_dispatch,
	.pch_arch_initcall	= ls2h_arch_initcall,
	.pch_device_initcall	= ls2h_device_initcall,
#ifdef CONFIG_PCI_MSI
	.setup_msi_irq		= ls2h_setup_msi_irq,
	.teardown_msi_irq	= ls2h_teardown_msi_irq,
#endif
};
