// SPDX-License-Identifier: GPL-2.0
/*
 * Virtio pci, based on GPEX in QEMU
 *
 * Copyright (C) Lemote, Inc.
 * Author: Huacai Chen <chenhc@lemote.com>
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/pci.h>

#include <uapi/asm/bitfield.h>
#include <asm/byteorder.h>
#include <asm/io.h>

#define ECAM_BASE 0x1a000000
#define ECAM_BUS_SHIFT   20
#define ECAM_DEVFN_SHIFT 12

void __iomem *virtio_ecam_map_bus(struct pci_bus *bus, unsigned int devfn,
			       int where)
{
	int busn = bus->number;
	void __iomem *base;

	if (busn >= 32)
		return NULL;

	base = (void __iomem *)TO_UNCAC(ECAM_BASE) + (busn << ECAM_BUS_SHIFT);

	return base + (devfn << ECAM_DEVFN_SHIFT) + where;
}

static int virtio_pci_pcibios_read(struct pci_bus *bus, unsigned int devfn,
					int where, int size, u32 *val)
{
	void __iomem *addrp;

	addrp = virtio_ecam_map_bus(bus, devfn, where);
	if (!addrp) {
		*val = -1;
		return PCIBIOS_DEVICE_NOT_FOUND;
	}

	switch (size) {
	case 1:
		*val = readb(addrp);
	case 2:
		*val = readw(addrp);
	case 4:
	default:
		*val = readl(addrp);
	}

	return PCIBIOS_SUCCESSFUL;
}

static int virtio_pci_pcibios_write(struct pci_bus *bus,
		unsigned int devfn, int where, int size, u32 val)
{
	void __iomem *addrp;

	addrp = virtio_ecam_map_bus(bus, devfn, where);
	if (!addrp)
		return PCIBIOS_DEVICE_NOT_FOUND;

	switch (size) {
	case 1:
		writeb(val, addrp);
	case 2:
		writew(val, addrp);
	case 4:
	default:
		writel(val, addrp);
	}

	return PCIBIOS_SUCCESSFUL;
}

struct pci_ops virtio_pci_ops = {
	.read  = virtio_pci_pcibios_read,
	.write = virtio_pci_pcibios_write,
};
