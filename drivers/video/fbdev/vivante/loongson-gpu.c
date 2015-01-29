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
#include "platform_driver.h"

#define DEVICE_NAME "galcore"

#ifdef CONFIG_OF
static struct of_device_id ls2h_gpu_id_table[] = {
	{ .compatible = "loongson,galcore", },
};
#endif

static struct platform_driver gpu_driver = {
	.probe		= loongson_gpu_probe,
	.remove		= loongson_gpu_remove,
	.suspend	= loongson_gpu_suspend,
	.resume		= loongson_gpu_resume,
	.driver		= {
		.name = DEVICE_NAME,
#ifdef CONFIG_OF
		.of_match_table = of_match_ptr(ls2h_gpu_id_table),
#endif
	}
};

static int __init gpu_init(void)
{
	struct pci_dev *pdev = NULL;

	/* Prefer to use PCI VGA Card */
	pdev = pci_get_class(PCI_CLASS_DISPLAY_VGA << 8, NULL);
	if (pdev)
		return -ENXIO;

	return platform_driver_register(&gpu_driver);
}

static void __exit gpu_exit(void)
{
	platform_driver_unregister(&gpu_driver);
}

module_init(gpu_init);
module_exit(gpu_exit);
MODULE_DESCRIPTION("Loongson-2H Graphics Driver");
MODULE_LICENSE("GPL");
