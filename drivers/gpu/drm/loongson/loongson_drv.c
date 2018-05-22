/*
 * Copyright (c) 2018 Loongson Technology Co., Ltd.
 * Authors:
 *	Chen Zhu <zhuchen@loongson.cn>
 *	Yaling Fang <fangyaling@loongson.cn>
 *	Dandan Zhang <zhangdandan@loongson.cn>
 *	Huacai Chen <chenhc@lemote.com>
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <drm/drmP.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_crtc_helper.h>
#include "loongson_drv.h"
#include <loongson-pch.h>

#include <asm/addrspace.h>
#include <linux/dma-contiguous.h>
#include <linux/dma-mapping.h>
#include <linux/vmalloc.h>
#include <linux/console.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/vgaarb.h>
#include <linux/vga_switcheroo.h>
#include <drm/drm_gem_cma_helper.h>
#include <drm/drm_gem_framebuffer_helper.h>
#include "loongson_drv.h"

#define DEVICE_NAME	"loongson-drm"
#define DRIVER_NAME	"loongson-drm"
#define DRIVER_DESC	"Loongson DRM Driver"
#define DRIVER_DATE	"20190831"
#define DRIVER_MAJOR	1
#define DRIVER_MINOR	0
#define DRIVER_PATCHLEVEL	0

static bool poll_connector = false;
module_param_named(poll, poll_connector, bool, 0600);

DEFINE_SPINLOCK(loongson_reglock);

/**
 * loongson_mode_funcs---basic driver provided mode setting functions
 *
 * Some global (i.e. not per-CRTC, connector, etc) mode setting functions that
 * involve drivers.
 */
static const struct drm_mode_config_funcs loongson_mode_funcs = {
	.fb_create = drm_gem_fb_create,
	.atomic_check = drm_atomic_helper_check,
	.atomic_commit = drm_atomic_helper_commit,
	.output_poll_changed = drm_fb_helper_output_poll_changed
};

/**
 *  loongson_drm_device_init  ----init drm device
 *
 * @dev   pointer to drm_device structure
 * @flags start up flag
 *
 * RETURN
 *   drm device init result
 */
static int loongson_drm_device_init(struct drm_device *dev, uint32_t flags)
{
	struct loongson_drm_device *ldev = dev->dev_private;

	loongson_vbios_init(ldev);
	ldev->num_crtc = ldev->vbios->crtc_num;

	if (ldev->dev->pdev) {
		/*BAR 0 contains registers */
		ldev->rmmio_base = pci_resource_start(ldev->dev->pdev, 0);
		ldev->rmmio_size = pci_resource_len(ldev->dev->pdev, 0);

		ldev->rmmio = pcim_iomap(dev->pdev, 0, 0);
		if (ldev->rmmio == NULL)
			return -ENOMEM;
	} else {
		struct resource	*res;

		res = platform_get_resource(to_platform_device(dev->dev), IORESOURCE_MEM, 0);

		ldev->rmmio_base = res->start;
		ldev->rmmio_size = resource_size(res);

		ldev->rmmio = ioremap(ldev->rmmio_base, ldev->rmmio_size);
		if (ldev->rmmio == NULL)
			return -ENOMEM;
	}

	DRM_INFO("ldev->rmmio_base = 0x%llx, ldev->rmmio_size = 0x%llx\n",
			ldev->rmmio_base, ldev->rmmio_size);

	if (!devm_request_mem_region(ldev->dev->dev, ldev->rmmio_base, ldev->rmmio_size,
			"loongson_drmfb_mmio")) {
		DRM_ERROR("Can't reserve mmio registers\n");
		return -ENOMEM;
	}

	return 0;
}

/**
 * loongson_modeset_init --- init kernel mode setting
 *
 * @ldev: pointer to loongson_drm_device structure
 *
 * RETURN
 *  return init result
 */
int loongson_modeset_init(struct loongson_drm_device *ldev)
{
	int i;
	struct drm_encoder *encoder;
	struct drm_connector *connector;

	ldev->mode_info[0].mode_config_initialized = true;
	ldev->mode_info[1].mode_config_initialized = true;

	ldev->dev->mode_config.max_width = LOONGSON_MAX_FB_WIDTH;
	ldev->dev->mode_config.max_height = LOONGSON_MAX_FB_HEIGHT;

	ldev->dev->mode_config.cursor_width = 32;
	ldev->dev->mode_config.cursor_height = 32;

	ldev->dev->mode_config.allow_fb_modifiers = true;

	loongson_crtc_init(ldev);
	ldev->num_crtc = ldev->vbios->crtc_num;

	for (i=0; i<ldev->num_crtc; i++) {
		DRM_DEBUG("loongson drm encoder init\n");
		ldev->mode_info[i].crtc = &ldev->lcrtc[i];
		encoder = loongson_encoder_init(ldev->dev, i);
		if (!encoder) {
			DRM_ERROR("loongson_encoder_init failed\n");
			return -1;
		}

		DRM_DEBUG("loongson drm i2c init\n");
		connector = loongson_vga_init(ldev->dev, i);
		if (!connector) {
			DRM_ERROR("loongson_vga_init failed\n");
			return -1;
		}

		ldev->mode_info[i].connector = to_loongson_connector(connector);
		drm_connector_attach_encoder(connector, encoder);
		if (poll_connector)
			connector->polled = DRM_CONNECTOR_POLL_CONNECT | DRM_CONNECTOR_POLL_DISCONNECT;
	}

	return 0;
}

/**
 * loongson_modeset_fini --- deinit kernel mode setting
 *
 * @ldev: pointer to loongson_drm_device structure
 *
 * RETURN
 */
void loongson_modeset_fini(struct loongson_drm_device *ldev)
{
}

/**
 * loongson_vga_load - setup chip and create an initial config
 * @dev: DRM device
 * @flags: startup flags
 *
 * The driver load routine has to do several things:
 *   - initialize the memory manager
 *   - allocate initial config memory
 *   - setup the DRM framebuffer with the allocated memory
 */
static int loongson_drm_load(struct drm_device *dev, unsigned long flags)
{
	int r, ret, irq;
	struct loongson_drm_device *ldev;

	dma_set_mask_and_coherent(dev->dev, DMA_BIT_MASK(32));

	ldev = devm_kzalloc(dev->dev, sizeof(struct loongson_drm_device), GFP_KERNEL);
	if (ldev == NULL)
		return -ENOMEM;
	dev->dev_private = (void *)ldev;
	ldev->dev = dev;

	ret = loongson_drm_device_init(dev, flags);
	DRM_DEBUG("end loongson drm device init.\n");

	drm_mode_config_init(dev);
	dev->mode_config.funcs = (void *)&loongson_mode_funcs;
	dev->mode_config.preferred_depth = 24;
	dev->mode_config.prefer_shadow = 1;

	if (dev->pdev) {
		irq = dev->pdev->irq;
		pci_set_drvdata(dev->pdev, dev);
		vga_set_default_device(dev->pdev);
	} else {
		irq = platform_get_irq(to_platform_device(dev->dev), 0);
		platform_set_drvdata(to_platform_device(dev->dev), dev);
	}
	dev_set_drvdata(dev->dev, dev);

	r = drm_irq_install(dev, irq);
	if (r)
		dev_err(dev->dev, "Fatal error during irq install: %d\n", r);

	r = loongson_modeset_init(ldev);
	if (r)
		dev_err(dev->dev, "Fatal error during modeset init: %d\n", r);

	ldev->inited = true;
	drm_mode_config_reset(dev);

	r = drm_vblank_init(dev, ldev->num_crtc);
	if (r)
		dev_err(dev->dev, "Fatal error during vblank init: %d\n", r);

	/* Make small buffers to store a hardware cursor (double buffered icon updates) */
	ldev->cursor = drm_gem_cma_create(dev, roundup(32*32*4, PAGE_SIZE));

	drm_kms_helper_poll_init(dev);

	return 0;
}

/**
 * loongson_drm_unload--release drm resource
 *
 * @dev: pointer to drm_device
 *
 */
static void loongson_drm_unload(struct drm_device *dev)
{
        struct loongson_drm_device *ldev = dev->dev_private;

	if (ldev == NULL)
		return;

	loongson_modeset_fini(ldev);
	drm_mode_config_cleanup(dev);
	dev->dev_private = NULL;
	dev_set_drvdata(dev->dev, NULL);
	ldev->inited = false;

	return;
}

/**
 * loongson_drm_open -Driver callback when a new struct drm_file is opened.
 * Useful for setting up driver-private data structures like buffer allocators,
 *  execution contexts or similar things.
 *
 * @dev DRM device
 * @file DRM file private date
 *
 * RETURN
 * 0 on success, a negative error code on failure, which will be promoted to
 *  userspace as the result of the open() system call.
 */
static int loongson_drm_open(struct drm_device *dev, struct drm_file *file)
{
	file->driver_priv = NULL;

	DRM_DEBUG("open: dev=%p, file=%p", dev, file);

	return 0;
}

DEFINE_DRM_GEM_CMA_FOPS(fops);

/**
 * loongson_drm_driver - DRM device structure
 *
 * .load: driver callback to complete initialization steps after the driver is registered
 * .unload:Reverse the effects of the driver load callback
 * .open:Driver callback when a new struct drm_file is opened
 * .fops:File operations for the DRM device node.
 * .gem_free_object:deconstructor for drm_gem_objects
 * .dumb_create:This creates a new dumb buffer in the driver’s backing storage manager
 *  (GEM, TTM or something else entirely) and returns the resulting buffer handle.
 *  This handle can then be wrapped up into a framebuffer modeset object
 * .dumb_map_offset:Allocate an offset in the drm device node’s address space
 *  to be able to memory map a dumb buffer
 * .dump_destory:This destroys the userspace handle for the given dumb backing storage buffer
 */
static struct drm_driver loongson_drm_driver = {
	.driver_features = DRIVER_MODESET | DRIVER_GEM
			| DRIVER_PRIME | DRIVER_HAVE_IRQ |  DRIVER_IRQ_SHARED | DRIVER_ATOMIC,
	.open = loongson_drm_open,
	.fops = &fops,

	.dumb_create		= drm_gem_cma_dumb_create,
	.gem_free_object_unlocked = drm_gem_cma_free_object,
	.gem_vm_ops		= &drm_gem_cma_vm_ops,

	.prime_handle_to_fd	= drm_gem_prime_handle_to_fd,
	.prime_fd_to_handle	= drm_gem_prime_fd_to_handle,

	.gem_prime_import	= drm_gem_prime_import,
	.gem_prime_export	= drm_gem_prime_export,

	.gem_prime_get_sg_table	= drm_gem_cma_prime_get_sg_table,
	.gem_prime_import_sg_table = drm_gem_cma_prime_import_sg_table,
	.gem_prime_vmap		= drm_gem_cma_prime_vmap,
	.gem_prime_vunmap	= drm_gem_cma_prime_vunmap,
	.gem_prime_mmap		= drm_gem_cma_prime_mmap,

	.irq_handler = loongson_irq_handler,
	.irq_preinstall = loongson_irq_preinstall,
	.irq_postinstall = loongson_irq_postinstall,
	.irq_uninstall = loongson_irq_uninstall,
	.name = DRIVER_NAME,
	.desc = DRIVER_DESC,
	.date = DRIVER_DATE,
	.major = DRIVER_MAJOR,
	.minor = DRIVER_MINOR,
	.patchlevel = DRIVER_PATCHLEVEL,
};

/**
 * loongson_drm_pci_devices  -- pci device id info
 *
 * __u32 vendor, device           Vendor and device ID or PCI_ANY_ID
 * __u32 subvendor, subdevice     Subsystem ID's or PCI_ANY_ID
 * __u32 class, class_mask        (class,subclass,prog-if) triplet
 * kernel_ulong_t driver_data     Data private to the driver
 */
static struct pci_device_id loongson_drm_pci_devices[] = {
	{PCI_DEVICE(PCI_VENDOR_ID_LOONGSON, PCI_DEVICE_ID_LOONGSON_DC)},
	{0, 0, 0, 0, 0, 0, 0}
};

/**
 * loongson_drm_pci_register -- add pci device
 *
 * @pdev PCI device
 * @ent pci device id
 */
static int loongson_drm_pci_register(struct pci_dev *pdev,
				 const struct pci_device_id *ent)

{
	int ret;
	struct drm_device *dev;

	dev = drm_dev_alloc(&loongson_drm_driver, &pdev->dev);
	if (IS_ERR(dev))
		return PTR_ERR(dev);

	ret = pci_enable_device(pdev);
	if (ret)
		goto err_free;

	dev->pdev = pdev;

	loongson_drm_load(dev, 0x0);

	ret = drm_dev_register(dev, 0);
	if (ret)
		goto err_pdev;

	drm_fbdev_generic_setup(dev, 32);

	return 0;

err_pdev:
	pci_disable_device(pdev);
err_free:
	drm_dev_put(dev);
	return ret;
}

/**
 * loongson_drm_pci_unregister -- release drm device
 *
 * @pdev PCI device
 */
static void loongson_drm_pci_unregister(struct pci_dev *pdev)
{
	struct drm_device *dev = pci_get_drvdata(pdev);
	loongson_drm_unload(dev);
	drm_dev_put(dev);
}

static int loongson_drm_plat_register(struct platform_device *pdev)

{
	int ret;
	struct drm_device *dev;

	dev = drm_dev_alloc(&loongson_drm_driver, &pdev->dev);
	if (IS_ERR(dev))
		return PTR_ERR(dev);

	loongson_drm_load(dev, 0x0);

	ret = drm_dev_register(dev, 0);
	if (ret)
		goto err_free;

	drm_fbdev_generic_setup(dev, 32);

	return 0;

err_free:
	drm_dev_unref(dev);
	return ret;
}

static int loongson_drm_plat_unregister(struct platform_device *pdev)
{
	struct drm_device *dev = platform_get_drvdata(pdev);
	loongson_drm_unload(dev);
	drm_dev_put(dev);

	return 0;
}

/*
 * Suspend & resume.
 */
/*
 * loongson_drm_suspend - initiate device suspend
 *
 * @pdev: drm dev pointer
 * @state: suspend state
 *
 * Puts the hw in the suspend state (all asics).
 * Returns 0 for success or an error on failure.
 * Called at driver suspend.
 */
int loongson_drm_suspend(struct drm_device *dev, bool suspend)
{
        struct loongson_drm_device *ldev;

        if (dev == NULL || dev->dev_private == NULL)
                return -ENODEV;

        ldev = dev->dev_private;

        drm_kms_helper_poll_disable(dev);
	ldev->state = drm_atomic_helper_suspend(dev);

	if (dev->pdev) {
		pci_save_state(dev->pdev);
		if (suspend) {
			/* Shut down the device */
			pci_disable_device(dev->pdev);
			pci_set_power_state(dev->pdev, PCI_D3hot);
		}
	}

	console_lock();
	drm_fb_helper_set_suspend(ldev->dev->fb_helper, 1);
	console_unlock();

	return 0;
}

/*
 *  * loongson_drm_resume - initiate device suspend
 *
 * @pdev: drm dev pointer
 * @state: suspend state
 *
 * Puts the hw in the suspend state (all asics).
 * Returns 0 for success or an error on failure.
 * Called at driver suspend.
 */

int loongson_drm_resume(struct drm_device *dev, bool resume)
{
	struct loongson_drm_device *ldev = dev->dev_private;

	console_lock();

	if (resume && dev->pdev) {
		pci_set_power_state(dev->pdev, PCI_D0);
		pci_restore_state(dev->pdev);
		if (pci_enable_device(dev->pdev)) {
			console_unlock();
			return -1;
		}
	}

        /* blat the mode back in */
	drm_atomic_helper_resume(dev, ldev->state);

	drm_kms_helper_poll_enable(dev);

	drm_fb_helper_set_suspend(ldev->dev->fb_helper, 0);

	console_unlock();

	return 0;
}

/**
 * loongson_drm_pm_suspend
 *
 * @dev   pointer to the device
 *
 * Executed before putting the system into a sleep state in which the
 * contents of main memory are preserved.
 */
static int loongson_drm_pm_suspend(struct device *dev)
{
	struct drm_device *drm_dev = dev_get_drvdata(dev);

	return loongson_drm_suspend(drm_dev, true);
}

/**
 * loongson_drm_pm_resume
 *
 * @dev pointer to the device
 *
 * Executed after waking the system up from a sleep state in which the
 * contents of main memory were preserved.
 */
static int loongson_drm_pm_resume(struct device *dev)
{
	struct drm_device *drm_dev = dev_get_drvdata(dev);

	return loongson_drm_resume(drm_dev, true);
}

/**
 *  loongson_drm_pm_freeze
 *
 *  @dev pointer to device
 *
 *  Hibernation-specific, executed before creating a hibernation image.
 *  Analogous to @suspend(), but it should not enable the device to signal
 *  wakeup events or change its power state.
 */
static int loongson_drm_pm_freeze(struct device *dev)
{
	struct drm_device *drm_dev = dev_get_drvdata(dev);

	return loongson_drm_suspend(drm_dev, false);
}

/**
 * loongson_drm_pm_draw
 *
 * @dev pointer to device
 *
 * Hibernation-specific, executed after creating a hibernation image OR
 * if the creation of an image has failed.  Also executed after a failing
 * attempt to restore the contents of main memory from such an image.
 * Undo the changes made by the preceding @freeze(), so the device can be
 * operated in the same way as immediately before the call to @freeze().
 */
static int loongson_drm_pm_thaw(struct device *dev)
{
	struct drm_device *drm_dev = dev_get_drvdata(dev);
	
	return loongson_drm_resume(drm_dev, false);
}

#define loongson_drm_pm_poweroff	loongson_drm_pm_freeze
#define loongson_drm_pm_restore		loongson_drm_pm_resume

/*
 * * struct dev_pm_ops - device PM callbacks
 *
 *@suspend:  Executed before putting the system into a sleep state in which the
 *           contents of main memory are preserved.
 *@resume:   Executed after waking the system up from a sleep state in which the
 *           contents of main memory were preserved.
 *@freeze:   Hibernation-specific, executed before creating a hibernation image.
 *           Analogous to @suspend(), but it should not enable the device to signal
 *           wakeup events or change its power state.  The majority of subsystems
 *           (with the notable exception of the PCI bus type) expect the driver-level
 *           @freeze() to save the device settings in memory to be used by @restore()
 *           during the subsequent resume from hibernation.
 *@thaw:     Hibernation-specific, executed after creating a hibernation image OR
 *           if the creation of an image has failed.  Also executed after a failing
 *           attempt to restore the contents of main memory from such an image.
 *           Undo the changes made by the preceding @freeze(), so the device can be
 *           operated in the same way as immediately before the call to @freeze().
 *@poweroff: Hibernation-specific, executed after saving a hibernation image.
 *           Analogous to @suspend(), but it need not save the device's settings in
 *           memory.
 *@restore:  Hibernation-specific, executed after restoring the contents of main
 *           memory from a hibernation image, analogous to @resume().
 */
static const struct dev_pm_ops loongson_drm_pm_ops = {
	.suspend = loongson_drm_pm_suspend,
	.resume = loongson_drm_pm_resume,
	.freeze = loongson_drm_pm_freeze,
	.thaw = loongson_drm_pm_thaw,
	.poweroff = loongson_drm_pm_poweroff,
	.restore = loongson_drm_pm_restore,
};

/**
 * loongson_drm_pci_driver -- pci driver structure
 *
 * .id_table : must be non-NULL for probe to be called
 * .probe: New device inserted
 * .remove: Device removed
 * .resume: Device suspended
 * .suspend: Device woken up
 */
static struct pci_driver loongson_drm_pci_driver = {
	.name		= DRIVER_NAME,
	.id_table	= loongson_drm_pci_devices,
	.probe		= loongson_drm_pci_register,
	.remove		= loongson_drm_pci_unregister,
	.driver.pm	= &loongson_drm_pm_ops,
};

#ifdef CONFIG_OF
static struct of_device_id loongson_drm_ids[] = {
	{ .compatible = "loongson,ls2h-drmfb", },
};
#endif

static struct platform_driver loongson_drm_plat_driver = {
	.probe		= loongson_drm_plat_register,
	.remove		= loongson_drm_plat_unregister,
	.driver = {
		.name	= DRIVER_NAME,
		.pm	= &loongson_drm_pm_ops,
#ifdef CONFIG_OF
		.of_match_table = of_match_ptr(loongson_drm_ids),
#endif
	},
};

/**
 * loongson_drm_pci_init()  -- kernel module init function
 */
static int __init loongson_drm_init(void)
{
	int ret;
	struct pci_dev *pdev = NULL;

	/* If external graphics card exist, use it as default */
	while ((pdev = pci_get_class(PCI_CLASS_DISPLAY_VGA << 8, pdev))) {
		if (pdev->vendor == PCI_VENDOR_ID_ATI)
			return 0;
		if (pdev->vendor == 0x1a03) /* ASpeed */
			return 0;
	}

	ret = pci_register_driver(&loongson_drm_pci_driver);
	if (ret < 0)
		return ret;

	ret = platform_driver_register(&loongson_drm_plat_driver);

	return ret;
}

/**
 * loongson_drm_pci_exit()  -- kernel module exit function
 */
static void __exit loongson_drm_exit(void)
{
	pci_unregister_driver(&loongson_drm_pci_driver);
	platform_driver_unregister(&loongson_drm_plat_driver);
}

module_init(loongson_drm_init);
module_exit(loongson_drm_exit);

MODULE_AUTHOR("Chen Zhu <zhuchen@loongson.cn>");
MODULE_AUTHOR("Huacai Chen <chenhc@lemote.com>");
MODULE_DESCRIPTION("Loongson LS2H/LS7A DRM Driver");
MODULE_LICENSE("GPL");
