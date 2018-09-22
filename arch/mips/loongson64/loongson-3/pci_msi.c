#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/msi.h>
#include <linux/pci.h>
#include <linux/spinlock.h>
#include <loongson-pch.h>

static bool msix_enable = 0;
core_param(msix, msix_enable, bool, 0664);

int arch_setup_msi_irqs(struct pci_dev *dev, int nvec, int type)
{
	struct msi_desc *entry;
	int ret;

	if (!pci_msi_enabled())
		return -ENOSPC;

	if (type == PCI_CAP_ID_MSIX && !msix_enable)
		return -ENOSPC;

	if (type == PCI_CAP_ID_MSI && nvec > 1)
		return 1;

	for_each_pci_msi_entry(entry, dev) {
		ret = loongson_pch->setup_msi_irq(dev, entry);
		if (ret < 0)
			return ret;
		if (ret > 0)
			return -ENOSPC;
	}

	return 0;
}

int arch_setup_msi_irq(struct pci_dev *pdev, struct msi_desc *desc)
{
	if (!pci_msi_enabled())
		return -ENOSPC;

	if (desc->msi_attrib.is_msix && !msix_enable)
		return -ENOSPC;

	return loongson_pch->setup_msi_irq(pdev, desc);
}

void arch_teardown_msi_irq(unsigned int irq)
{
	loongson_pch->teardown_msi_irq(irq);
}

static void msi_nop(struct irq_data *data) { }

struct irq_chip loongson_msi_irq_chip = {
	.name = "PCI-MSI",
	.irq_ack = msi_nop,
	.irq_enable = unmask_msi_irq,
	.irq_disable = mask_msi_irq,
	.irq_mask = mask_msi_irq,
	.irq_unmask = unmask_msi_irq,
	.irq_set_affinity = plat_set_irq_affinity,
};
