/****************************************************************************
*
*    Copyright (C) 2005 - 2013 by Vivante Corp.
*
*    This program is free software; you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation; either version 2 of the license, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not write to the Free Software
*    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*****************************************************************************/

#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <asm/dma-coherence.h>
#include "platform_driver.h"
#include <loongson-pch.h>

#define DEVICE_NAME "galcore"

#ifdef CONFIG_OF
static struct of_device_id gpu_id_table[] = {
	{ .compatible = "loongson,galcore", },
};
#endif

static struct platform_driver gpu_plat_driver = {
	.probe		= loongson_gpu_plat_probe,
	.remove		= loongson_gpu_plat_remove,
	.suspend	= loongson_gpu_plat_suspend,
	.resume		= loongson_gpu_plat_resume,
	.driver		= {
		.name = DEVICE_NAME,
#ifdef CONFIG_OF
		.of_match_table = of_match_ptr(gpu_id_table),
#endif
	}
};

static struct pci_device_id loongson_gpu_devices[] = {
	{PCI_DEVICE(PCI_VENDOR_ID_LOONGSON, PCI_DEVICE_ID_LOONGSON_GPU)},
	{0, 0, 0, 0, 0, 0, 0}
};

static struct pci_driver gpu_pci_driver = {
	.name		= "loongson-gpu",
	.probe		= loongson_gpu_pci_probe,
	.remove		= loongson_gpu_pci_remove,
	.id_table	= loongson_gpu_devices,
#ifdef CONFIG_SUSPEND
	.suspend = loongson_gpu_pci_suspend,
	.resume	 = loongson_gpu_pci_resume,
#endif
};

static int __init gpu_init(void)
{
	int ret;
	struct pci_dev *pdev = NULL;

	/* Prefer to use External Graphics Card */
	while ((pdev = pci_get_class(PCI_CLASS_DISPLAY_VGA << 8, pdev))) {
		if (pdev->vendor == PCI_VENDOR_ID_ATI)
			return 0;
		if (pdev->vendor == 0x1a03) /* ASpeed */
			return 0;
	}

	ret = pci_register_driver(&gpu_pci_driver);
	if(ret)
		return ret;

	ret = platform_driver_register(&gpu_plat_driver);
	return ret;
}

static void __exit gpu_exit(void)
{
	pci_unregister_driver(&gpu_pci_driver);
	platform_driver_unregister(&gpu_plat_driver);
}

module_init(gpu_init);
module_exit(gpu_exit);
MODULE_DESCRIPTION("Loongson-2H Graphics Driver");
MODULE_LICENSE("GPL");
