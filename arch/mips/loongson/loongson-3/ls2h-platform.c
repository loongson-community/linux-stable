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
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/resource.h>
#include <linux/serial_8250.h>
#include <linux/i2c.h>
#include <linux/phy.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <linux/delay.h>
#include <linux/stmmac.h>
#include <linux/usb/ehci_pdriver.h>
#include <linux/usb/ohci_pdriver.h>
#include <asm/io.h>

#include <irq.h>
#include <pci.h>
#include <boot_param.h>
#include <loongson-pch.h>

extern struct pci_ops ls2h_pcie_ops_port0;
extern struct pci_ops ls2h_pcie_ops_port1;
extern struct pci_ops ls2h_pcie_ops_port2;
extern struct pci_ops ls2h_pcie_ops_port3;

extern void ls2h_init_irq(void);
extern void ls2h_irq_dispatch(void);
extern int ls2h_platform_init(void);
extern int ls2h_pcibios_map_irq(struct pci_dev *dev, u8 slot, u8 pin);

extern u32 loongson_dma_mask_bits;
static u64 platform_dma_mask = DMA_BIT_MASK(32);

/*
 * UART
 */
struct plat_serial8250_port ls2h_uart8250_data[] = {
	[0] = {
		.mapbase = LS2H_UART0_REG_BASE,
		.uartclk = 125000000,
		.membase = (void *)CKSEG1ADDR(LS2H_UART0_REG_BASE),
		.irq = LS2H_PCH_UART0_IRQ,
		.flags = UPF_BOOT_AUTOCONF | UPF_SKIP_TEST,
		.iotype = UPIO_MEM,
		.regshift = 0,
	},
	[1] = {
		.mapbase = LS2H_UART1_REG_BASE,
		.uartclk = 125000000,
		.membase = (void *)CKSEG1ADDR(LS2H_UART1_REG_BASE),
		.irq = LS2H_PCH_UART1_IRQ,
		.flags = UPF_BOOT_AUTOCONF | UPF_SKIP_TEST,
		.iotype = UPIO_MEM,
		.regshift = 0,
	},
	[2] = {
		.mapbase = LS2H_UART2_REG_BASE,
		.uartclk = 125000000,
		.membase = (void *)CKSEG1ADDR(LS2H_UART2_REG_BASE),
		.irq = LS2H_PCH_UART2_IRQ,
		.flags = UPF_BOOT_AUTOCONF | UPF_SKIP_TEST,
		.iotype = UPIO_MEM,
		.regshift = 0,
	},
	[3] = {
		.mapbase = LS2H_UART3_REG_BASE,
		.uartclk = 125000000,
		.membase = (void *)CKSEG1ADDR(LS2H_UART3_REG_BASE),
		.irq = LS2H_PCH_UART3_IRQ,
		.flags = UPF_BOOT_AUTOCONF | UPF_SKIP_TEST,
		.iotype = UPIO_MEM,
		.regshift = 0,
	},
	{}
};

static struct platform_device uart8250_device = {
	.name = "serial8250",
	.id = PLAT8250_DEV_PLATFORM1,
	.dev = {
		.platform_data = ls2h_uart8250_data,
	}
};

/*
 * NAND
 */
struct ls2h_nand_platform_data{
	int enable_arbiter;
	struct mtd_partition *parts;
	unsigned int nr_parts;
};

static struct mtd_partition ls2h_nand_partitions[]={
	[0] = {
		.name   = "kernel",
		.offset = 0,
		.size   = 0x01400000,
	},
	[1] = {
		.name   = "os",
		.offset = 0x01400000,
		.size   = 0x0,

	},
};

static struct ls2h_nand_platform_data ls2h_nand_parts = {
        .enable_arbiter = 1,
        .parts          = ls2h_nand_partitions,
        .nr_parts       = ARRAY_SIZE(ls2h_nand_partitions),

};

static struct resource ls2h_nand_resources[] = {
	[0] = {
		.start      = 0,
		.end        = 0,
		.flags      = IORESOURCE_DMA,
	},
	[1] = {
		.start      = LS2H_NAND_REG_BASE,
		.end        = LS2H_NAND_REG_BASE + 0x20,
		.flags      = IORESOURCE_MEM,
	},
	[2] = {
		.start      = LS2H_DMA_ORDER_REG,
		.end        = LS2H_DMA_ORDER_REG,
		.flags      = IORESOURCE_MEM,
	},
	[3] = {
		.start      = LS2H_PCH_DMA0_IRQ,
		.end        = LS2H_PCH_DMA0_IRQ,
		.flags      = IORESOURCE_IRQ,
	},
};

struct platform_device ls2h_nand_device = {
	.name       = "ls2h-nand",
	.id         = 0,
	.dev        = {
		.platform_data = &ls2h_nand_parts,
	},
	.num_resources  = ARRAY_SIZE(ls2h_nand_resources),
	.resource       = ls2h_nand_resources,
};

/*
 * OHCI
 */
static struct resource ls2h_ohci_resources[] = {
	[0] = {
		.start = LS2H_OHCI_REG_BASE,
		.end   = (LS2H_OHCI_REG_BASE + 0x1000 - 1),
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = LS2H_PCH_OHCI_IRQ,
		.end   = LS2H_PCH_OHCI_IRQ,
		.flags = IORESOURCE_IRQ,
	},
};

static struct usb_ohci_pdata ls2h_ohci_platform_data = {
	.num_ports = 6,
};

static struct platform_device ls2h_ohci_device = {
	.name           = "ohci-platform",
	.id             = 0,
	.dev = {
		.platform_data	= &ls2h_ohci_platform_data,
	},
	.num_resources  = ARRAY_SIZE(ls2h_ohci_resources),
	.resource       = ls2h_ohci_resources,
};

/*
 * EHCI
 */
static struct resource ls2h_ehci_resources[] = {
	[0] = {
		.start = LS2H_EHCI_REG_BASE,
		.end   = (LS2H_EHCI_REG_BASE + 0x100 - 1),
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = LS2H_PCH_EHCI_IRQ,
		.end   = LS2H_PCH_EHCI_IRQ,
		.flags = IORESOURCE_IRQ,
	},
};

static struct usb_ehci_pdata ls2h_ehci_platform_data = {
};

static struct platform_device ls2h_ehci_device = {
	.name	= "ehci-platform",
	.id	= 0,
	.dev	= {
		.platform_data	= &ls2h_ehci_platform_data,
	},
	.num_resources  = ARRAY_SIZE(ls2h_ehci_resources),
	.resource       = ls2h_ehci_resources,
};

/*
 * PHY0
 */
static struct stmmac_mdio_bus_data phy0_plat_data = {
	.phy_mask	= 0,
};

/*
 * GMAC0
 */
static struct plat_stmmacenet_data gmac0_plat_dat = {
	.bus_id		= 0,
	.phy_addr	= 1,
	.has_gmac	= 1,
	.enh_desc	= 1,
	.mdio_bus_data = &phy0_plat_data,
};

static struct resource ls2h_gmac0_resources[] = {
	[0] = {
		.start = LS2H_GMAC0_REG_BASE,
		.end   = (LS2H_GMAC0_REG_BASE + 0x1000 - 1),
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = LS2H_PCH_GMAC0_IRQ,
		.end   = LS2H_PCH_GMAC0_IRQ,
		.name  = "macirq",
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device ls2h_gmac0_device = {
	.name	= "stmmaceth",
	.id	= 0,
	.dev	= {
		.platform_data	= &gmac0_plat_dat,
	},
	.num_resources  = ARRAY_SIZE(ls2h_gmac0_resources),
	.resource       = ls2h_gmac0_resources,
};

/*
 * PHY1
 */
static struct stmmac_mdio_bus_data phy1_plat_data = {
	.phy_mask	= 0,
};

/*
 * GMAC1
 */
static struct plat_stmmacenet_data gmac1_plat_dat = {
	.bus_id		= 1,
	.phy_addr	= 1,
	.has_gmac	= 1,
	.enh_desc	= 1,
	.mdio_bus_data = &phy1_plat_data,
};

static struct resource ls2h_gmac1_resources[] = {
	[0] = {
		.start = LS2H_GMAC1_REG_BASE,
		.end   = (LS2H_GMAC1_REG_BASE + 0x1000 - 1),
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = LS2H_PCH_GMAC1_IRQ,
		.end   = LS2H_PCH_GMAC1_IRQ,
		.name  = "macirq",
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device ls2h_gmac1_device = {
	.name           = "stmmaceth",
	.id             = 1,
	.dev = {
		.platform_data = &gmac1_plat_dat,
	},
	.num_resources  = ARRAY_SIZE(ls2h_gmac1_resources),
	.resource       = ls2h_gmac1_resources,
};

/*
 * AHCI
 */
static struct resource ls2h_ahci_resources[] = {
	[0] = {
		.start = LS2H_SATA_REG_BASE,
		.end   = LS2H_SATA_REG_BASE + 0x1ff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = LS2H_PCH_SATA_IRQ,
		.end   = LS2H_PCH_SATA_IRQ,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device ls2h_ahci_device = {
	.name           = "ahci",
	.id             = 0,
	.num_resources  = ARRAY_SIZE(ls2h_ahci_resources),
	.resource       = ls2h_ahci_resources,
};

/*
 * RTC
 */
static struct resource ls2h_rtc_resources[] = {
       [0] = {
               .start  = LS2H_RTC_REG_BASE,
               .end    = (LS2H_RTC_REG_BASE + 0xff),
               .flags  = IORESOURCE_MEM,
       },
       [1] = {
               .start  = LS2H_PCH_RTC_INT0_IRQ,
               .end    = LS2H_PCH_TOY_TICK_IRQ,
               .flags  = IORESOURCE_IRQ,
       },
};

static struct platform_device ls2h_rtc_device = {
       .name   = "ls2h-rtc",
       .id     = 0,
       .num_resources  = ARRAY_SIZE(ls2h_rtc_resources),
       .resource       = ls2h_rtc_resources,
};


/*
 * DC
 */
static struct resource ls2h_dc_resources[] = {
	[0] = {
		.start	= LS2H_DC_REG_BASE,
		.end	= LS2H_DC_REG_BASE + 0x2000,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= LS2H_PCH_DC_IRQ,
		.end	= LS2H_PCH_DC_IRQ,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device ls2h_dc_device = {
	.name           = "ls2h-fb",
	.id             = 0,
	.num_resources	= ARRAY_SIZE(ls2h_dc_resources),
	.resource	= ls2h_dc_resources,
};

/*
 * HD Audio
 */
static struct resource ls2h_audio_resources[] = {
	[0] = {
		.start = LS2H_HDA_REG_BASE,
		.end   = LS2H_HDA_REG_BASE + 0x17f,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = LS2H_PCH_HDA_IRQ,
		.end   = LS2H_PCH_HDA_IRQ,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device ls2h_audio_device = {
	.name           = "ls2h-audio",
	.id             = 0,
	.num_resources  = ARRAY_SIZE(ls2h_audio_resources),
	.resource       = ls2h_audio_resources,
};

/*
 * I2C
 */
static struct resource ls2h_i2c0_resources[] = {
	[0] = {
		.start = LS2H_I2C0_REG_BASE,
		.end   = LS2H_I2C0_REG_BASE + 0x8,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = LS2H_PCH_I2C0_IRQ,
		.end   = LS2H_PCH_I2C0_IRQ,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device ls2h_i2c0_device = {
	.name           = "ls2h-i2c",
	.id             = 0,
	.num_resources  = ARRAY_SIZE(ls2h_i2c0_resources),
	.resource       = ls2h_i2c0_resources,
};

static struct resource ls2h_i2c1_resources[] = {
	[0] = {
		.start = LS2H_I2C1_REG_BASE,
		.end   = LS2H_I2C1_REG_BASE + 0x8,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = LS2H_PCH_I2C1_IRQ,
		.end   = LS2H_PCH_I2C1_IRQ,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device ls2h_i2c1_device = {
	.name           = "ls2h-i2c",
	.id             = 1,
	.num_resources  = ARRAY_SIZE(ls2h_i2c1_resources),
	.resource       = ls2h_i2c1_resources,
};

/*
 * GPU
 */
static struct resource ls2h_gpu_resources[] = {
	[0] = {
		.name	= "gpu_base",
		.start	= LS2H_GPU_REG_BASE,
		.end	= LS2H_GPU_REG_BASE + 0x7ff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.name	= "gpu_irq",
		.start	= LS2H_PCH_GPU_IRQ,
		.end	= LS2H_PCH_GPU_IRQ,
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		.name	= "gpu_mem",
		.start	= 0xe0004000000,
		.end	= 0xe000bffffff,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device ls2h_gpu_device = {
	.name           = "galcore",
	.id             = 0,
	.num_resources  = ARRAY_SIZE(ls2h_gpu_resources),
	.resource       = ls2h_gpu_resources,
};

/*
 * OTG
 */
static struct resource ls2h_otg_resources[] = {
	[0] = {
		.start = LS2H_OTG_REG_BASE,
		.end   = (LS2H_OTG_REG_BASE + 0x3ffff),
		.flags = IORESOURCE_MEM,
        },
        [1] = {
		.start = LS2H_PCH_OTG_IRQ,
		.end   = LS2H_PCH_OTG_IRQ,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device ls2h_otg_device = {
	.name           = "dwc2",
	.id             = 0,
	.num_resources  = ARRAY_SIZE(ls2h_otg_resources),
	.resource       = ls2h_otg_resources,
};

static struct platform_device *ls2h_platform_devices[] = {
	&uart8250_device,
	&ls2h_i2c0_device,
	&ls2h_i2c1_device,
	&ls2h_nand_device,
	&ls2h_ohci_device,
	&ls2h_ehci_device,
	&ls2h_gmac0_device,
	&ls2h_gmac1_device,
	&ls2h_ahci_device,
	&ls2h_dc_device,
	&ls2h_audio_device,
	&ls2h_rtc_device,
	&ls2h_gpu_device,
	&ls2h_otg_device,
};

const struct i2c_board_info __initdata loongson_eep_info = {
	I2C_BOARD_INFO("eeprom-loongson", 0x50),
};

/* PCI-E controller0 X4 can't use whole memory space */
static struct resource ls2h_pcie_mem_resource0 = {
	.name	= "LS2H PCIE0 MEM",
	.flags	= IORESOURCE_MEM,
};

static struct resource ls2h_pcie_io_resource0 = {
	.name	= "LS2H PCIE0 IO",
	.start	= 0x100000,
	.end	= 0x10ffff,
	.flags	= IORESOURCE_IO,
};

static struct pci_controller ls2h_pcie_controller0 = {
	.pci_ops	= &ls2h_pcie_ops_port0,
	.io_resource	= &ls2h_pcie_io_resource0,
	.mem_resource	= &ls2h_pcie_mem_resource0,
	.mem_offset	= 0x00000000UL,
	.io_offset	= 0x00000000UL,
	.io_map_base	= 0x90000e0018000000UL,
};

/* PCI-E controller1 */
static struct resource ls2h_pcie_mem_resource1 = {
	.name	= "LS2H PCIE1 MEM",
	.flags	= IORESOURCE_MEM,
};

static struct resource ls2h_pcie_io_resource1 = {
	.name	= "LS2H PCIE1 IO",
	.start	= 0x500000,
	.end	= 0x50ffff,
	.flags	= IORESOURCE_IO,
};

static struct pci_controller ls2h_pcie_controller1 = {
	.pci_ops	= &ls2h_pcie_ops_port1,
	.io_resource	= &ls2h_pcie_io_resource1,
	.mem_resource	= &ls2h_pcie_mem_resource1,
	.mem_offset	= 0x00000000UL,
	.io_offset	= 0x00000000UL,
	.io_map_base	= 0x90000e0018000000UL,
};

/* PCI-E controller2 */
static struct resource ls2h_pcie_mem_resource2 = {
	.name	= "LS2H PCIE2 MEM",
	.flags	= IORESOURCE_MEM,
};

static struct resource ls2h_pcie_io_resource2 = {
	.name	= "LS2H PCIE2 IO",
	.start	= 0x900000,
	.end	= 0x90ffff,
	.flags	= IORESOURCE_IO,
};

static struct pci_controller ls2h_pcie_controller2 = {
	.pci_ops	= &ls2h_pcie_ops_port2,
	.io_resource	= &ls2h_pcie_io_resource2,
	.mem_resource	= &ls2h_pcie_mem_resource2,
	.mem_offset	= 0x00000000UL,
	.io_offset	= 0x00000000UL,
	.io_map_base	= 0x90000e0018000000UL,
};

/* PCI-E controller3 */
static struct resource ls2h_pcie_mem_resource3 = {
	.name	= "LS2H PCIE3 MEM",
	.flags	= IORESOURCE_MEM,
};

static struct resource ls2h_pcie_io_resource3 = {
	.name	= "LS2H PCIE3 IO",
	.start	= 0xd00000,
	.end	= 0xd0ffff,
	.flags	= IORESOURCE_IO,
};

static struct pci_controller ls2h_pcie_controller3 = {
	.pci_ops	= &ls2h_pcie_ops_port3,
	.io_resource	= &ls2h_pcie_io_resource3,
	.mem_resource	= &ls2h_pcie_mem_resource3,
	.mem_offset	= 0x00000000UL,
	.io_offset	= 0x00000000UL,
	.io_map_base	= 0x90000e0018000000UL,
};

static void en_ref_clock(void)
{
	unsigned int data;

	data = ls2h_readl(LS2H_CLK_CTRL3_REG);
	data |= (LS2H_CLK_CTRL3_BIT_PEREF_EN(0)
		 | LS2H_CLK_CTRL3_BIT_PEREF_EN(1)
		 | LS2H_CLK_CTRL3_BIT_PEREF_EN(2)
		 | LS2H_CLK_CTRL3_BIT_PEREF_EN(3));
	ls2h_writel(data, LS2H_CLK_CTRL3_REG);
}

static int is_rc_mode(void)
{
	unsigned data;

	data = ls2h_readl(LS2H_PCIE_PORT_REG_BASE(0)
			| LS2H_PCIE_PORT_REG_CTR_STAT);

	return data & LS2H_PCIE_REG_CTR_STAT_BIT_ISRC;
}

static int is_x4_mode(void)
{
	unsigned data;

	data = ls2h_readl(LS2H_PCIE_PORT_REG_BASE(0)
			| LS2H_PCIE_PORT_REG_CTR_STAT);

	return data & LS2H_PCIE_REG_CTR_STAT_BIT_ISX4;
}

void ls2h_pcie_port_init(int port)
{
	unsigned reg, data;

	reg = LS2H_PCIE_PORT_REG_BASE(port) | LS2H_PCIE_PORT_REG_CTR0;
	ls2h_writel(0x00ff204c, reg);

	reg = LS2H_PCIE_PORT_HEAD_BASE(port) | PCI_CLASS_REVISION;
	data = (ls2h_readl(reg) & 0xffff) | (PCI_CLASS_BRIDGE_PCI << 16);
	ls2h_writel(data, reg);

	reg = LS2H_PCIE_PORT_HEAD_BASE(port) | LS2H_PCI_EXP_LNKCAP;
	data = (ls2h_readl(reg) & (~0xf)) | 0x1;
	ls2h_writel(data, reg);
}

static void ls2h_early_config(void)
{
	u32 val;

	/*
	 * Loongson-2H chip_config0: 0x1fd00200
	 * bit[5]: 	Loongson-2H bridge mode,0: disable      1: enable
	 * bit[4]:	ac97/hda select,	0: ac97		1: hda
	 * bit[14]:	host/otg select,	0: host         1: otg
	 * bit[26]:	usb reset,		0: enable       1: disable
	 */
	val = ls2h_readl(LS2H_CHIP_CFG0_REG);
	ls2h_writel(val | (1 << 5) | (1 << 4) | (1 << 14) | (1 << 26), LS2H_CHIP_CFG0_REG);

	val = ls2h_readl(LS2H_GPIO_OE_REG);
	ls2h_writel(val | (1 << 0), LS2H_GPIO_OE_REG);

	en_ref_clock();
	pcie_bus_config = PCIE_BUS_PERFORMANCE;

	if (is_rc_mode()) {
		ls2h_pcie_port_init(0);
		if (!is_x4_mode()) {
			ls2h_pcie_port_init(1);
			ls2h_pcie_port_init(2);
			ls2h_pcie_port_init(3);
		}
	}
}

static void __init ls2h_arch_initcall(void)
{
	u64 pci_mem_size;

	ioport_resource.end = 0xffffffff;
	if (!pci_mem_start_addr)
		pci_mem_start_addr = LOONGSON_PCI_MEM_START;
	if (!pci_mem_end_addr)
		pci_mem_end_addr = LOONGSON_PCI_MEM_START + 0x40000000UL - 1;
	pci_mem_size = pci_mem_end_addr - pci_mem_start_addr + 1;

	if (!is_rc_mode())
		return;

	if (is_x4_mode()) {
		ls2h_pcie_mem_resource0.start = pci_mem_start_addr;
		ls2h_pcie_mem_resource0.end   = pci_mem_start_addr + pci_mem_size - 1;
		register_pci_controller(&ls2h_pcie_controller0);
	} else {
		ls2h_pcie_mem_resource0.start = pci_mem_start_addr;
		ls2h_pcie_mem_resource0.end   = pci_mem_start_addr + pci_mem_size*1/4 - 1;
		register_pci_controller(&ls2h_pcie_controller0);

		ls2h_pcie_mem_resource1.start = pci_mem_start_addr + pci_mem_size*1/4;;
		ls2h_pcie_mem_resource1.end   = pci_mem_start_addr + pci_mem_size*2/4 - 1;
		register_pci_controller(&ls2h_pcie_controller1);

		ls2h_pcie_mem_resource2.start = pci_mem_start_addr + pci_mem_size*2/4;;
		ls2h_pcie_mem_resource2.end   = pci_mem_start_addr + pci_mem_size*3/4 - 1;
		register_pci_controller(&ls2h_pcie_controller2);

		ls2h_pcie_mem_resource3.start = pci_mem_start_addr + pci_mem_size*3/4;;
		ls2h_pcie_mem_resource3.end   = pci_mem_start_addr + pci_mem_size*4/4 - 1;
		register_pci_controller(&ls2h_pcie_controller3);
	}
}

#define I2C_BUS_0 0
#define I2C_BUS_1 1

static void __init ls2h_device_initcall(void)
{
	int i;

	i2c_register_board_info(I2C_BUS_1, &loongson_eep_info, 1);

	platform_dma_mask = DMA_BIT_MASK(loongson_dma_mask_bits);
	for (i=0; i<ARRAY_SIZE(ls2h_platform_devices); i++) {
		ls2h_platform_devices[i]->dev.dma_mask = &platform_dma_mask;
		ls2h_platform_devices[i]->dev.coherent_dma_mask = platform_dma_mask;
	}

	platform_add_devices(ls2h_platform_devices,
			ARRAY_SIZE(ls2h_platform_devices));
}

const struct platform_controller_hub ls2h_pch = {
	.board_type		= LS2H,
	.pcidev_max_funcs 	= 1,
	.early_config		= ls2h_early_config,
	.init_irq		= ls2h_init_irq,
	.irq_dispatch		= ls2h_irq_dispatch,
	.pcibios_map_irq	= ls2h_pcibios_map_irq,
	.pch_arch_initcall	= ls2h_arch_initcall,
	.pch_device_initcall	= ls2h_device_initcall,
};
