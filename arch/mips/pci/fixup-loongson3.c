/*
 * fixup-loongson3.c
 *
 * Copyright (C) 2012 Lemote, Inc.
 * Author: Xiang Yu, xiangy@lemote.com
 *         Chen Huacai, chenhc@lemote.com
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 * WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 * NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 * USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * You should have received a copy of the  GNU General Public License along
 * with this program; if not, write  to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <linux/pci.h>
#include <irq.h>
#include <boot_param.h>
#include <workarounds.h>

static void print_fixup_info(const struct pci_dev * pdev)
{
	dev_info(&pdev->dev, "Device %x:%x, irq %d\n",
			pdev->vendor, pdev->device, pdev->irq);
}

int __init pcibios_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	print_fixup_info(dev);
	return dev->irq;
}

static void pci_fixup_radeon(struct pci_dev *pdev)
{
	if (pdev->resource[PCI_ROM_RESOURCE].start)
		return;

	if (!vgabios_addr)
		return;

	pdev->resource[PCI_ROM_RESOURCE].start  = vgabios_addr;
	pdev->resource[PCI_ROM_RESOURCE].end    = vgabios_addr + 256*1024 - 1;
	pdev->resource[PCI_ROM_RESOURCE].flags |= IORESOURCE_ROM_COPY;

	dev_info(&pdev->dev, "BAR %d: assigned %pR for Radeon ROM\n",
			PCI_ROM_RESOURCE, &pdev->resource[PCI_ROM_RESOURCE]);
}

DECLARE_PCI_FIXUP_CLASS_FINAL(PCI_VENDOR_ID_ATI, 0x9615,
				PCI_CLASS_DISPLAY_VGA, 8, pci_fixup_radeon);

/* Do platform specific device initialization at pci_enable_device() time */
int pcibios_plat_dev_init(struct pci_dev *dev)
{
	init_dma_attrs(&dev->dev.archdata.dma_attrs);
	if (loongson_workarounds & WORKAROUND_PCIE_DMA)
		dma_set_attr(DMA_ATTR_FORCE_SWIOTLB, &dev->dev.archdata.dma_attrs);

	return 0;
}
