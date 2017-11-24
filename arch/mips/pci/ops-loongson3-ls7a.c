#include <linux/types.h>
#include <linux/pci.h>
#include <linux/kernel.h>

#include <asm/mips-boards/bonito64.h>

#include <loongson.h>

#define PCI_ACCESS_READ  0
#define PCI_ACCESS_WRITE 1

#define HT1LO_PCICFG_BASE      0x1a000000
#define HT1LO_PCICFG_BASE_TP1  0x1b000000

static int ls7a_pci_config_access(unsigned char access_type,
		struct pci_bus *bus, unsigned int devfn,
		int where, u32 *data)
{
	unsigned char busnum = bus->number;
	u_int64_t addr, type;
	void *addrp;
	int device = PCI_SLOT(devfn);
	int function = PCI_FUNC(devfn);
	int reg = where & ~3;

	if (busnum == 0 && device == 6 && function == 0 && reg == 0x20)
		return 0;

	if (busnum == 0) {

		/** Filter out non-supported devices.
		 *  device 2:misc  device 21:confbus
		 */
		if (device > 23)
			return PCIBIOS_DEVICE_NOT_FOUND;
		addr = (device << 11) | (function << 8) | reg;
		addrp = (void *)(TO_UNCAC(HT1LO_PCICFG_BASE) | (addr & 0xffff));
		type = 0;

	} else {
		addr = (busnum << 16) | (device << 11) | (function << 8) | reg;
		addrp = (void *)(TO_UNCAC(HT1LO_PCICFG_BASE_TP1) | (addr));
		type = 0x10000;
	}

	if (access_type == PCI_ACCESS_WRITE)
		*(volatile unsigned int *)addrp = cpu_to_le32(*data);
	else {
		*data = le32_to_cpu(*(volatile unsigned int *)addrp);
		if (busnum == 0 && reg == PCI_CLASS_REVISION && *data == 0x06000001)
			*data = (PCI_CLASS_BRIDGE_PCI << 16) | (*data & 0xffff);
		if (busnum == 0 && reg == 0x3c &&  (*data &0xff00) == 0)
			*data |= 0x100;

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
