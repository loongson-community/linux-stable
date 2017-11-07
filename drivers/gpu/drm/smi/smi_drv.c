/*
 * Copyright 2016 SiliconMotion Inc.
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License version 2. See the file COPYING in the main
 * directory of this archive for more details.
 *
 */
#include <linux/version.h>
#include <linux/module.h>
#include <linux/console.h>
#include <drm/drmP.h>
#include <drm/drm_crtc_helper.h>

#include "smi_drv.h"
#include "hw750.h"
#include "hw768.h"

int smi_modeset = -1;
int smi_indent = 0;
int smi_bpp = 32;
int force_connect = 0;
int g_specId;
int smi_pat = 0xff;
int lvds_channel = 0;

extern void hw750_suspend(struct smi_750_register * pSave);
extern void hw750_resume(struct smi_750_register * pSave);

module_param(smi_pat, int, S_IWUSR | S_IRUSR);

MODULE_PARM_DESC(modeset, "Enable/Disable modesetting");
module_param_named(modeset, smi_modeset, int, 0400);
MODULE_PARM_DESC(bpp, "Max bits-per-pixel (default:32)");
module_param_named(bpp, smi_bpp, int, 0400);
MODULE_PARM_DESC(nopnp, "Force conncet to the monitor without monitor EDID (default:0)");
module_param_named(nopnp, force_connect, int, 0400);
MODULE_PARM_DESC(lvds, "LVDS Channel, 0=disable 1=single_channel, 2=dual_channel (default:0)");
module_param_named(lvds, lvds_channel, int, 0400);

/*
 * This is the generic driver code. This binds the driver to the drm core,
 * which then performs further device association and calls our graphics init
 * functions
 */
#define PCI_VENDOR_ID_SMI 	0x126f
#define PCI_DEVID_SM750	0x0750
#define PCI_DEVID_SM768	0x0768

static struct drm_driver driver;

/* only bind to the smi chip in qemu */
static const struct pci_device_id pciidlist[] = {
	{PCI_VENDOR_ID_SMI,PCI_DEVID_SM750,PCI_ANY_ID,PCI_ANY_ID,0,0,0},
	{PCI_VENDOR_ID_SMI,PCI_DEVID_SM768,PCI_ANY_ID,PCI_ANY_ID,0,0,0},
	{0,}
};


static int smi_kick_out_firmware_fb(struct pci_dev *pdev)
{
	struct apertures_struct *ap;
	bool primary = false;

	ap = alloc_apertures(1);
	if (!ap)
		return -ENOMEM;

	ap->ranges[0].base = pci_resource_start(pdev, 0);
	ap->ranges[0].size = pci_resource_len(pdev, 0);

#ifdef CONFIG_X86
	primary = pdev->resource[PCI_ROM_RESOURCE].flags & IORESOURCE_ROM_SHADOW;
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0)
	drm_fb_helper_remove_conflicting_framebuffers(ap, "smidrmfb", primary);
#else
	remove_conflicting_framebuffers(ap, "smidrmfb", primary);
#endif


	kfree(ap);

	return 0;
}

static void claim(void)
{
	printk("+-------------SMI Driver Information------------+\n");
	printk("Release type: " RELEASE_TYPE "\n");
	printk("Driver version: v" _version_ "\n");
	printk("Support products: " SUPPORT_CHIP "\n");
	printk("Support OS: " SUPPORT_XVERSION "\n");
	printk("Support ARCH: " SUPPORT_ARCH "\n");
	printk("+-----------------------------------------------+\n");
}


static int smi_pci_probe(struct pci_dev *pdev,
			    const struct pci_device_id *ent)
{
	int ret;

	ret = smi_kick_out_firmware_fb(pdev);
	if (ret)
		return ret;

	claim();

	switch (ent->device){
		case PCI_DEVID_LYNX_EXP:
			g_specId = SPC_SM750;
			break;
		case PCI_DEVID_SM768:
			g_specId = SPC_SM768;
			break;
		default:
			break;
	}
	dbg_msg("ent->device:0x%x\n", ent->device);
	dbg_msg("g_specId:0x%x\n", g_specId);
	
	return drm_get_pci_dev(pdev, ent, &driver);
}

static void smi_pci_remove(struct pci_dev *pdev)
{
	struct drm_device *dev = pci_get_drvdata(pdev);

	drm_put_dev(dev);
}



static int smi_drm_freeze(struct drm_device *dev)
{
	ENTER();

	struct smi_device *sdev = dev->dev_private;
	
	drm_kms_helper_poll_disable(dev);

	pci_save_state(dev->pdev);

	if (sdev->mode_info.gfbdev) {
		console_lock();
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,3,0)			
		fb_set_suspend(sdev->mode_info.gfbdev->helper.fbdev, 1);
#else
		drm_fb_helper_set_suspend(&sdev->mode_info.gfbdev->helper, 1);
#endif
		console_unlock();
	}

	if(g_specId == SPC_SM750)
         hw750_suspend(sdev->regsave);
    else if(g_specId == SPC_SM768)
         hw768_suspend(sdev->regsave_768);

	LEAVE(0);

}

static int smi_drm_thaw(struct drm_device *dev)
{	
	ENTER();
	struct smi_device *sdev = dev->dev_private;
	

	drm_mode_config_reset(dev);
	drm_helper_resume_force_mode(dev);

	if (sdev->mode_info.gfbdev) {
		console_lock();
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,3,0)		
		fb_set_suspend(sdev->mode_info.gfbdev->helper.fbdev, 0);
#else
		drm_fb_helper_set_suspend(&sdev->mode_info.gfbdev->helper, 0);
#endif
		smi_fb_zfill(dev, sdev->mode_info.gfbdev);

		console_unlock();
	}

	
	if(g_specId == SPC_SM750)
			hw750_resume(sdev->regsave);
	else if(g_specId == SPC_SM768)
			hw768_resume(sdev->regsave_768);

	LEAVE(0);
}


static int smi_drm_resume(struct drm_device *dev)
{	
	ENTER();
	
	struct smi_device *sdev = dev->dev_private;
	int ret;

	if (pci_enable_device(dev->pdev))
		return -EIO;

	ret = smi_drm_thaw(dev);
	if (ret)
		return ret;

	drm_kms_helper_poll_enable(dev);

	LEAVE(0);
}


static int smi_pm_suspend(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct drm_device *ddev = pci_get_drvdata(pdev);
	int error;

	error = smi_drm_freeze(ddev);
	if (error)
		return error;

	pci_disable_device(pdev);
	pci_set_power_state(pdev, PCI_D3hot);
	return 0;

}


static int smi_pm_resume(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct drm_device *ddev = pci_get_drvdata(pdev);
	return smi_drm_resume(ddev);
}




static int smi_pm_freeze(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct drm_device *ddev = pci_get_drvdata(pdev);

	if (!ddev || !ddev->dev_private)
		return -ENODEV;
	return smi_drm_freeze(ddev);

}


static int smi_pm_thaw(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct drm_device *ddev = pci_get_drvdata(pdev);
	return smi_drm_thaw(ddev);

}


static int smi_pm_poweroff(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct drm_device *ddev = pci_get_drvdata(pdev);

	return smi_drm_freeze(ddev);


}

static int smi_enable_vblank(struct drm_device *dev, unsigned int pipe)
{
	if(g_specId == SPC_SM750)
	{
		hw750_en_dis_interrupt(1, pipe);
	}else if(g_specId == SPC_SM768)
	{
		hw768_en_dis_interrupt(1, pipe);
	}
	return 0;
}


static void smi_disable_vblank(struct drm_device *dev, unsigned int pipe)
{
	if(g_specId == SPC_SM750)
	{
		hw750_en_dis_interrupt(0, pipe);
	}else if(g_specId == SPC_SM768)
	{
		hw768_en_dis_interrupt(0, pipe);
	}
}


static void smi_irq_preinstall(struct drm_device *dev)
{
	//To Do....
	/* Disable *all* interrupts */

	/* Clear bits if they're already high */

}

static int smi_irq_postinstall(struct drm_device *dev)
{
	return 0;
}

static void smi_irq_uninstall(struct drm_device *dev)
{

	/* Disable *all* interrupts */
	if(g_specId == SPC_SM750)
	{
		ddk750_disable_IntMask();
	}else if(g_specId == SPC_SM768)
	{
		ddk768_disable_IntMask();
	}

}


irqreturn_t smi_drm_interrupt(int irq, void *arg)
{
	struct drm_device *dev = (struct drm_device *) arg;
	
	int handled = 0;
	
	if(g_specId == SPC_SM750)
	{
	    if (hw750_check_vsync_interrupt(0))
	    {
	        /* Clear the panel VSync Interrupt */	
			drm_handle_vblank(dev, 0);		
			handled = 1;
			hw750_clear_vsync_interrupt(0);
	    }   
		if (hw750_check_vsync_interrupt(1)) {			
			drm_handle_vblank(dev, 1);
			handled = 1;
			hw750_clear_vsync_interrupt(1);
		}
	}else if(g_specId == SPC_SM768)
	{
		if (hw768_check_vsync_interrupt(0))
		{
			/* Clear the panel VSync Interrupt */
			drm_handle_vblank(dev, 0);
			handled = 1;
			hw768_clear_vsync_interrupt(0);
		}	
		if (hw768_check_vsync_interrupt(1)) {		
			drm_handle_vblank(dev, 1);
			handled = 1;
			hw768_clear_vsync_interrupt(1);
		}
	}
	
	if (handled)
		return IRQ_HANDLED;
	return IRQ_NONE;
}



static const struct file_operations smi_driver_fops = {
	.owner = THIS_MODULE,
	.open = drm_open,
	.release = drm_release,
	.unlocked_ioctl = drm_ioctl,
	.mmap = smi_mmap,
	.poll = drm_poll,
#ifdef CONFIG_COMPAT
	.compat_ioctl = drm_compat_ioctl,
#endif
	.read = drm_read,
	.llseek = no_llseek,
};
static struct drm_driver driver = {
#ifdef PRIME
	.driver_features = DRIVER_HAVE_IRQ | DRIVER_IRQ_SHARED |DRIVER_GEM |DRIVER_PRIME | DRIVER_MODESET,
#else
	.driver_features = DRIVER_HAVE_IRQ | DRIVER_IRQ_SHARED |DRIVER_GEM | DRIVER_MODESET,
#endif
	.load = smi_driver_load,
	.unload = smi_driver_unload,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,17,0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0))
	.set_busid = drm_pci_set_busid,
#endif	
	.fops = &smi_driver_fops,
	.name = DRIVER_NAME,
	.desc = DRIVER_DESC,
	.date = DRIVER_DATE,
	.major = DRIVER_MAJOR,
	.minor = DRIVER_MINOR,
	.patchlevel = DRIVER_PATCHLEVEL,
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,12,0)	
	.gem_init_object = smi_gem_init_object,
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,7,0)
	.gem_free_object = smi_gem_free_object,
#else
	.gem_free_object_unlocked = smi_gem_free_object,
#endif
	.dumb_create = smi_dumb_create,
	.dumb_map_offset = smi_dumb_mmap_offset,
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,12,0)
	.dumb_destroy = smi_dumb_destroy,
#else
	.dumb_destroy = drm_gem_dumb_destroy,
#endif
	
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(4,12,0))
	.get_vblank_counter	= drm_vblank_no_hw_counter,
#endif
	.enable_vblank		= smi_enable_vblank,
	.disable_vblank		= smi_disable_vblank,
	.irq_preinstall = smi_irq_preinstall,
	.irq_postinstall = smi_irq_postinstall,
	.irq_uninstall = smi_irq_uninstall,
	.irq_handler		= smi_drm_interrupt,
#ifdef PRIME
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,14,0)
	.prime_handle_to_fd	= drm_gem_prime_handle_to_fd,
	.prime_fd_to_handle	= drm_gem_prime_fd_to_handle,

	.gem_prime_import	= drm_gem_prime_import,
	.gem_prime_export	= drm_gem_prime_export,

	.gem_prime_get_sg_table	= smi_gem_prime_get_sg_table,
	.gem_prime_import_sg_table = smi_gem_prime_import_sg_table,
	.gem_prime_vmap		= smi_gem_prime_vmap,
	.gem_prime_vunmap	= smi_gem_prime_vunmap,
	.gem_prime_pin		= smi_gem_prime_pin,
	.gem_prime_unpin 	= smi_gem_prime_unpin,	
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,17,0)
	.gem_prime_res_obj = smi_gem_prime_res_obj,
#endif
#endif

};

static const struct dev_pm_ops smi_pm_ops = {
	.suspend = smi_pm_suspend,
	.resume = smi_pm_resume,
	.freeze = smi_pm_freeze,
	.thaw = smi_pm_thaw,
	.poweroff = smi_pm_poweroff,
	.restore = smi_pm_resume,
};

static struct pci_driver smi_pci_driver = {
	.name = DRIVER_NAME,
	.id_table = pciidlist,
	.probe = smi_pci_probe,
	.remove = smi_pci_remove,
	.driver.pm = &smi_pm_ops,
};

static int __init smi_init(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,7,0)
#ifdef CONFIG_VGA_CONSOLE
	if (vgacon_text_force() && smi_modeset == -1)
		return -EINVAL;
#endif
#else
	if (vgacon_text_force() && smi_modeset == -1)
		return -EINVAL;

#endif
	if (smi_modeset == 0)
		return -EINVAL;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,13,0)
	return drm_pci_init(&driver, &smi_pci_driver);
#else
	return pci_register_driver(&smi_pci_driver);
#endif
}

static void __exit smi_exit(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,13,0)
	drm_pci_exit(&driver, &smi_pci_driver);
#else
	pci_unregister_driver(&smi_pci_driver);
#endif
}

module_init(smi_init);
module_exit(smi_exit);

MODULE_DEVICE_TABLE(pci, pciidlist);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
