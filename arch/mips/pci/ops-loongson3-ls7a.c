// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Lemote, Inc.
 * Author: Huacai Chen <chenhc@lemote.com>
 */
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/kernel.h>

#include <asm/mips-boards/bonito64.h>

#include <loongson.h>

#define PCI_ACCESS_READ  0
#define PCI_ACCESS_WRITE 1

#define HT1LO_PCICFG_BASE      0x1a000000
#define HT1LO_PCICFG_BASE_TP1  0x1b000000

#define HT1LO_PCICFG_BASE_EXTEND 	0xefe00000000
#define HT1LO_PCICFG_BASE_TP1_EXTEND 	0xefe10000000

static int ls7a_pci_config_access(unsigned char access_type,
		struct pci_bus *bus, unsigned int devfn,
		int where, u32 *data)
{
	u_int64_t addr;
	void *addrp;
	unsigned char busnum = bus->number;
	int device = PCI_SLOT(devfn);
	int function = PCI_FUNC(devfn);
	int reg = where & ~3;

	/** Filter out non-supported devices.
	 *  device 2: misc, device 21: confbus
	 */
	if (where < PCI_CFG_SPACE_SIZE) { /* standard config */
		addr = (busnum << 16) | (device << 11) | (function << 8) | reg;
		if (busnum == 0) {
			if (device > 23)
				return PCIBIOS_DEVICE_NOT_FOUND;
			addrp = (void *)TO_UNCAC(HT1LO_PCICFG_BASE | addr);
		} else {
			addrp = (void *)TO_UNCAC(HT1LO_PCICFG_BASE_TP1 | addr);
		}
	} else if (where < PCI_CFG_SPACE_EXP_SIZE) {  /* extended config */
		reg = (reg & 0xff) | ((reg & 0xf00) << 16);
		addr = (busnum << 16) | (device << 11) | (function << 8) | reg;
		if (busnum == 0) {
			if (device > 23)
				return PCIBIOS_DEVICE_NOT_FOUND;
			addrp = (void *)TO_UNCAC(HT1LO_PCICFG_BASE_EXTEND | addr);
		} else {
			addrp = (void *)TO_UNCAC(HT1LO_PCICFG_BASE_TP1_EXTEND | addr);
		}
	} else {
		return PCIBIOS_DEVICE_NOT_FOUND;
	}

	if (access_type == PCI_ACCESS_WRITE)
		writel(*data, addrp);
	else {
		*data = readl(addrp);
		if (busnum == 0 && reg == PCI_CLASS_REVISION && *data == 0x06000001)
			*data = (PCI_CLASS_BRIDGE_PCI << 16) | (*data & 0xffff);

		if (*data == 0xffffffff) {
			*data = -1;
			return PCIBIOS_DEVICE_NOT_FOUND;
		}
	}
	return PCIBIOS_SUCCESSFUL;
}

static int ls7a_pci_pcibios_read(struct pci_bus *bus, unsigned int devfn,
				 int where, int size, u32 * val)
{
	u32 data = 0;
	int ret = ls7a_pci_config_access(PCI_ACCESS_READ,
			bus, devfn, where, &data);

	if (ret != PCIBIOS_SUCCESSFUL)
		return ret;

	if (size == 1)
		*val = (data >> ((where & 3) << 3)) & 0xff;
	else if (size == 2)
		*val = (data >> ((where & 3) << 3)) & 0xffff;
	else
		*val = data;

	return PCIBIOS_SUCCESSFUL;
}

static int ls7a_pci_pcibios_write(struct pci_bus *bus, unsigned int devfn,
				  int where, int size, u32 val)
{
	u32 data = 0;
	int ret;

	if (size == 4)
		data = val;
	else {
		ret = ls7a_pci_config_access(PCI_ACCESS_READ,
				bus, devfn, where, &data);
		if (ret != PCIBIOS_SUCCESSFUL)
			return ret;

		if (size == 1)
			data = (data & ~(0xff << ((where & 3) << 3))) |
			    (val << ((where & 3) << 3));
		else if (size == 2)
			data = (data & ~(0xffff << ((where & 3) << 3))) |
			    (val << ((where & 3) << 3));
	}

	ret = ls7a_pci_config_access(PCI_ACCESS_WRITE,
			bus, devfn, where, &data);
	if (ret != PCIBIOS_SUCCESSFUL)
		return ret;

	return PCIBIOS_SUCCESSFUL;
}

struct pci_ops ls7a_pci_ops = {
	.read = ls7a_pci_pcibios_read,
	.write = ls7a_pci_pcibios_write
};
