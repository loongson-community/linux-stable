/*
 * Copyright 2016 SiliconMotion Inc.
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License version 2. See the file COPYING in the main
 * directory of this archive for more details.
 *
 */

#include <drm/drmP.h>
#include <drm/drm_crtc_helper.h>
#include <linux/dma-buf.h>

#include "smi_drv.h"

#include "hw750.h"
#include "hw768.h"

#define MB(x) (x<<20) /* Macro for Mega Bytes */

#ifdef PRIME

int smi_handle_damage(struct smi_framebuffer *fb, int x, int y,
						int width, int height)
{
	bool kmap = false;
	int i, ret = 0;
	unsigned long offset = 0;
	void *dst = NULL;
	struct smi_bo *dst_bo = NULL;
	
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0)
	unsigned bytesPerPixel = fb->base.format->cpp[0];
#else
	unsigned bytesPerPixel = (fb->base.bits_per_pixel >> 3);
#endif

	
	if (!fb->obj->import_attach) {
		return -EINVAL;
	}
	
	if (!fb->vmapping) {
		fb->vmapping = dma_buf_vmap(fb->obj->import_attach->dmabuf);
		if (!fb->vmapping)
			return 0;
	}

	dst_bo = gem_to_smi_bo(fb->obj);
	ret = smi_bo_reserve(dst_bo, true);
	if (ret){
		dbg_msg("smi_bo_reserve failed\n");
		smi_bo_unreserve(dst_bo);
		goto error;
	}

	if (!dst_bo->dma_buf_vmap.virtual) {
		ret = ttm_bo_kmap(&dst_bo->bo, 0, dst_bo->bo.num_pages, &dst_bo->dma_buf_vmap);
		if (ret) {
			DRM_ERROR("failed to kmap fbcon\n");
			goto error;
		}
		kmap = true;
	}
	dst = dst_bo->dma_buf_vmap.virtual;

	dbg_msg("src: %p, dst: %p, x=%d, y=%d, fbwidth=%d, fbheight=%d, width=%d, height=%d, bpp = %u, pitch=%d\n",
		fb->vmapping, dst, x, y, fb->base.width, fb->base.height, width,height, (bytesPerPixel << 3), fb->base.pitches[0]);

#if 1
	for (i = y; i < y + height; i++) {
		offset = i * fb->base.pitches[0] + (x * bytesPerPixel);
		memcpy_toio(dst + offset, fb->vmapping + offset, width * bytesPerPixel);
	}
#else
	//copy whole screen
	memcpy_toio(dst, fb->vmapping, fb->base.pitches[0] * fb->base.height);
#endif
	if (kmap)
		ttm_bo_kunmap(&dst_bo->dma_buf_vmap);
	smi_bo_unreserve(dst_bo);

error:
	return 0;
}
#endif

static void smi_user_framebuffer_destroy(struct drm_framebuffer *fb)
{
	struct smi_framebuffer *smi_fb = to_smi_framebuffer(fb);

	if (smi_fb->obj) {
		if (smi_fb->obj->import_attach) {
			if(smi_fb->vmapping)
				dma_buf_vunmap(smi_fb->obj->import_attach->dmabuf, smi_fb->vmapping);
		}
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,12,0)
		drm_gem_object_put_unlocked(smi_fb->obj);
#else
		drm_gem_object_unreference_unlocked(smi_fb->obj);
#endif
	}
	drm_framebuffer_cleanup(fb);
	kfree(fb);
}

#ifdef PRIME

static int smi_user_framebuffer_dirty(struct drm_framebuffer *fb,
                                        struct drm_file *file,
                                        unsigned flags, unsigned color,
                                        struct drm_clip_rect *clips,
                                        unsigned num_clips)
{
	struct smi_framebuffer *smi_fb = to_smi_framebuffer(fb);
	int i, ret = 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0)
	drm_modeset_lock_all(fb->dev);
#else
	mutex_lock(&fb->dev->mode_config.mutex);
#endif

	if (smi_fb->obj->import_attach) {
		ret = dma_buf_begin_cpu_access(smi_fb->obj->import_attach->dmabuf,
#if KERNEL_VERSION(4, 6, 0) > LINUX_VERSION_CODE
						0, smi_fb->obj->size,
#endif
						DMA_FROM_DEVICE);
		if (ret)
			goto unlock;
	}	

	for (i = 0; i < num_clips; i++) {
		ret = smi_handle_damage(smi_fb, clips[i].x1, clips[i].y1,
                            clips[i].x2 - clips[i].x1, clips[i].y2 - clips[i].y1);
		if (ret)
			break;
	}
	
	if (smi_fb->obj->import_attach) {
		dma_buf_end_cpu_access(smi_fb->obj->import_attach->dmabuf,
#if KERNEL_VERSION(4, 6, 0) > LINUX_VERSION_CODE
					0, smi_fb->obj->size,
#endif
					DMA_FROM_DEVICE);
	}
	
unlock:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0)
	drm_modeset_unlock_all(fb->dev);
#else
	mutex_unlock(&fb->dev->mode_config.mutex);
#endif

	return ret;
}

#endif


static int smi_user_framebuffer_create_handle(struct drm_framebuffer *fb,
						  struct drm_file *file_priv,
						  unsigned int *handle)
{
	struct smi_framebuffer *smi_fb = to_smi_framebuffer(fb);
	return drm_gem_handle_create(file_priv, smi_fb->obj, handle);
}


static const struct drm_framebuffer_funcs smi_fb_funcs = {
	.create_handle = smi_user_framebuffer_create_handle,
	.destroy = smi_user_framebuffer_destroy,
#ifdef PRIME
	.dirty = smi_user_framebuffer_dirty,
#endif
};

int smi_framebuffer_init(struct drm_device *dev,
			    struct smi_framebuffer *gfb,
			    const struct drm_mode_fb_cmd2 *mode_cmd,
			    struct drm_gem_object *obj)
{
	int ret;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0)
		drm_helper_mode_fill_fb_struct(dev, &gfb->base, mode_cmd);
#else
		drm_helper_mode_fill_fb_struct(&gfb->base, mode_cmd);
#endif
	gfb->obj = obj;
	ret = drm_framebuffer_init(dev, &gfb->base, &smi_fb_funcs);
	if (ret) {
		DRM_ERROR("drm_framebuffer_init failed: %d\n", ret);
		return ret;
	}
	return 0;
}

static struct drm_framebuffer *
smi_user_framebuffer_create(struct drm_device *dev,
			       struct drm_file *filp,
			       const struct drm_mode_fb_cmd2 *mode_cmd)
{
	struct drm_gem_object *obj;
	struct smi_framebuffer *smi_fb;
	int ret;
	

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,7,0)
	obj = drm_gem_object_lookup(dev, filp, mode_cmd->handles[0]);
#else
	obj = drm_gem_object_lookup(filp, mode_cmd->handles[0]);
#endif
	if (obj == NULL)
		return ERR_PTR(-ENOENT);

	smi_fb = kzalloc(sizeof(*smi_fb), GFP_KERNEL);
	if (!smi_fb) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,12,0)
		drm_gem_object_put_unlocked(obj);
#else
		drm_gem_object_unreference_unlocked(obj);
#endif
		return ERR_PTR(-ENOMEM);
	}

	ret = smi_framebuffer_init(dev, smi_fb, mode_cmd, obj);
	if (ret) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,12,0)
		drm_gem_object_put_unlocked(obj);
#else
		drm_gem_object_unreference_unlocked(obj);
#endif
		kfree(smi_fb);
		return ERR_PTR(ret);
	}
	return &smi_fb->base;
}


static void smi_output_poll_changed(struct drm_device *dev)
{
	struct smi_device *sdev = dev->dev_private;
	smi_fb_output_poll_changed(sdev);
}


static const struct drm_mode_config_funcs smi_mode_funcs = {
	.fb_create = smi_user_framebuffer_create,
	.output_poll_changed = smi_output_poll_changed,
};

/* Unmap the framebuffer from the core and release the memory */
static void smi_vram_fini(struct smi_device *cdev)
{
	iounmap(cdev->rmmio);
	cdev->rmmio = NULL;
	if (cdev->mc.vram_base)
		release_mem_region(cdev->mc.vram_base, cdev->mc.vram_size);
}

/* Map the framebuffer from the card and configure the core */
static int smi_vram_init(struct smi_device *cdev)
{
	/* BAR 0 is VRAM */
	cdev->mc.vram_base = pci_resource_start(cdev->dev->pdev, 0);

	/* VRAM Size */
	if(g_specId == SPC_SM750)
		cdev->mc.vram_size = ddk750_getFrameBufSize();
	else
		cdev->mc.vram_size = ddk768_getFrameBufSize();

	if (!request_mem_region(cdev->mc.vram_base, cdev->mc.vram_size,
				"smidrmfb_vram")) {
		DRM_ERROR("can't reserve VRAM\n");
		return -ENXIO;
	}

	return 0;
}

/*
 * SMI Graphics has two sets of memory. One is video RAM and can
 * simply be used as a linear framebuffer - the other provides mmio access
 * to the display registers. The latter can also be accessed via IO port
 * access, but we map the range and use mmio to program them instead
 */

int smi_device_init(struct smi_device *cdev,
		       struct drm_device *ddev,
		       struct pci_dev *pdev, uint32_t flags)
{
	int ret,dma_bits;

	cdev->dev = ddev;
	cdev->flags = flags;

	/* Hardcode the number of CRTCs to 2 */
	cdev->num_crtc = 2;

	dma_bits = 40;
	cdev->need_dma32 = false;
	ret = pci_set_dma_mask(cdev->dev->pdev, DMA_BIT_MASK(dma_bits));
	if (ret) {
		cdev->need_dma32 = true;
		dma_bits = 32;
		printk(KERN_WARNING "smifb: No suitable DMA available.\n");
	}

#if 0
	ret = pci_set_consistent_dma_mask(cdev->dev->pdev, DMA_BIT_MASK(dma_bits));
	if (ret) {
		pci_set_consistent_dma_mask(cdev->dev->pdev, DMA_BIT_MASK(32));
		printk(KERN_WARNING "smifb: No coherent DMA available.\n");
	}
#endif

	/* BAR 0 is the framebuffer, BAR 1 contains registers */
	cdev->rmmio_base = pci_resource_start(cdev->dev->pdev, 1);
	cdev->rmmio_size = pci_resource_len(cdev->dev->pdev, 1);

	if (!request_mem_region(cdev->rmmio_base, cdev->rmmio_size,
				"smidrmfb_mmio")) {
		DRM_ERROR("can't reserve mmio registers\n");
		return -ENOMEM;
	}

	cdev->rmmio = ioremap(cdev->rmmio_base, cdev->rmmio_size);

	if (cdev->rmmio == NULL)
		return -ENOMEM;

	if(g_specId == SPC_SM750)
		ddk750_set_mmio(cdev->rmmio,pdev->device,pdev->revision);
	else
		ddk768_set_mmio(cdev->rmmio,pdev->device,pdev->revision);


	ret = smi_vram_init(cdev);
	if (ret) {
		release_mem_region(cdev->rmmio_base, cdev->rmmio_size);
		return ret;
	}

	return 0;
}

void smi_device_fini(struct smi_device *cdev)
{
	release_mem_region(cdev->rmmio_base, cdev->rmmio_size);
	smi_vram_fini(cdev);
}

/*
 * Functions here will be called by the core once it's bound the driver to
 * a PCI device
 */
void drm_kms_helper_poll_init(struct drm_device *dev);
int smi_driver_load(struct drm_device *dev, unsigned long flags)
{
	struct smi_device *cdev;
	int r;

	cdev = kzalloc(sizeof(struct smi_device), GFP_KERNEL);
	if (cdev == NULL)
		return -ENOMEM;
	dev->dev_private = (void *)cdev;

	pci_enable_device(dev->pdev);

	r = smi_device_init(cdev, dev, dev->pdev, flags);
	if (r) {
		dev_err(&dev->pdev->dev, "Fatal error during GPU init: %d\n", r);
		goto out;
	}

	if(g_specId == SPC_SM750)
	{
		ddk750_initChip();
		ddk750_deInit();
		
#ifdef USE_HDMICHIP
		if((r = sii9022xInitChip()) < 0)
		{	
			printk("Init HDMI-Tx chip failed!");
			r = 0;	
		}
#endif

	}
	else
	{
		ddk768_initChip();
		ddk768_deInit();
		hw768_init_hdmi();
#ifdef AUDIO_EN
		smi_audio_init(dev);
#endif
	}
	
	r = smi_mm_init(cdev);
	if (r){
		dev_err(&dev->pdev->dev, "fatal err on mm init\n");
		goto out;
	}

	drm_vblank_init(dev, dev->mode_config.num_crtc);

	int ret;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,16,0)	
		ret = drm_irq_install(dev, dev->pdev->irq);
#else
		ret = drm_irq_install(dev);
#endif
		if (ret)
			DRM_ERROR("install irq failed , ret = %d\n", ret);
	
	dev->mode_config.funcs = (void *)&smi_mode_funcs;
	r = smi_modeset_init(cdev);
	if (r){
		dev_err(&dev->pdev->dev, "Fatal error during modeset init: %d\n", r);
		goto out;
	}

	cdev->regsave = vmalloc(1024);
	if(!cdev->regsave)
	{
		printk("cannot allocate regsave\n");
		//return -ENOMEM;
	}

	drm_kms_helper_poll_init(dev);

	return 0;
out:
	if (r)
		smi_driver_unload(dev);
	return r;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0)
void smi_driver_unload(struct drm_device *dev)
#else
int smi_driver_unload(struct drm_device *dev)
#endif

{
	struct smi_device *cdev = dev->dev_private;

	if (dev->irq_enabled)
		drm_irq_uninstall(dev);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,17,0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)) 
		drm_vblank_cleanup(dev);
#endif

	if (cdev == NULL)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0)
		return;
#else
		return 0;
#endif

	smi_modeset_fini(cdev);
	smi_mm_fini(cdev);
	smi_device_fini(cdev);

#ifdef AUDIO_EN
	if(g_specId == SPC_SM768)
		smi_audio_remove(dev);
#endif
		
	vfree(cdev->regsave);
	kfree(cdev);
	dev->dev_private = NULL;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,11,0)
	return 0;
#endif

}

int smi_gem_create(struct drm_device *dev,
		   u32 size, bool iskernel,
		   struct drm_gem_object **obj)
{
	struct smi_bo *smibo;
	int ret;

	*obj = NULL;

	size = roundup(size, PAGE_SIZE);
	if (size == 0)
		return -EINVAL;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0)
	ret = smi_bo_create(dev, size, 0, 0, NULL, &smibo);
#else
	ret = smi_bo_create(dev, size, 0, 0, NULL, NULL, &smibo);
#endif
	if (ret) {
		if (ret != -ERESTARTSYS)
			DRM_ERROR("failed to allocate GEM object\n");
		return ret;
	}
	*obj = &smibo->gem;
	return 0;
}

int smi_dumb_create(struct drm_file *file,
		    struct drm_device *dev,
		    struct drm_mode_create_dumb *args)
{
	int ret;
	struct drm_gem_object *gobj;
	u32 handle;

	args->pitch = ((args->width) * (args->bpp) / 8 + 15) & ~15; 
	
	args->size = args->pitch * args->height;

	ret = smi_gem_create(dev, args->size, false,
			     &gobj);
	if (ret)
		return ret;

	ret = drm_gem_handle_create(file, gobj, &handle);
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,12,0)
	drm_gem_object_put_unlocked(gobj);
#else
	drm_gem_object_unreference_unlocked(gobj);
#endif
	if (ret)
		return ret;

	args->handle = handle;
	return 0;
}

int smi_gem_init_object(struct drm_gem_object *obj)
{
	BUG();
	return 0;
}

void smi_bo_unref(struct smi_bo **bo)
{
	struct ttm_buffer_object *tbo;

	if ((*bo) == NULL)
		return;

	tbo = &((*bo)->bo);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,1,0)
	ttm_bo_put(tbo);
#else
	ttm_bo_unref(&tbo);
#endif
	if (tbo == NULL)
		*bo = NULL;

}

void smi_gem_free_object(struct drm_gem_object *obj)
{
	struct smi_bo *smi_bo = gem_to_smi_bo(obj);

	if (smi_bo){
		if (smi_bo->gem.import_attach)
			drm_prime_gem_destroy(&smi_bo->gem, smi_bo->bo.sg);
		smi_bo_unref(&smi_bo);
	}
}


static inline u64 smi_bo_mmap_offset(struct smi_bo *bo)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,12,0)
	return bo->bo.addr_space_offset;
#elif LINUX_VERSION_CODE < KERNEL_VERSION(5,4,0)
	return drm_vma_node_offset_addr(&bo->bo.vma_node);
#else
	return drm_vma_node_offset_addr(&bo->bo.base.vma_node);
#endif
}

int
smi_dumb_mmap_offset(struct drm_file *file,
		     struct drm_device *dev,
		     uint32_t handle,
		     uint64_t *offset)
{
	struct drm_gem_object *obj;
	struct smi_bo *bo;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,7,0)
	obj = drm_gem_object_lookup(file, handle);
	if (obj == NULL) 
		return -ENOENT;

	bo = gem_to_smi_bo(obj);
	*offset = smi_bo_mmap_offset(bo);

#if LINUX_VERSION_CODE > KERNEL_VERSION(4,12,0)
	drm_gem_object_put_unlocked(obj);
#else
	drm_gem_object_unreference_unlocked(obj);
#endif
	return 0;
#else
	int ret;

	mutex_lock(&dev->struct_mutex);
	obj = drm_gem_object_lookup(dev, file, handle);
	if (obj == NULL) {
		ret = -ENOENT;
		goto out_unlock;
	}
	bo = gem_to_smi_bo(obj);
	*offset = smi_bo_mmap_offset(bo);
	drm_gem_object_unreference(obj);
	ret = 0;
out_unlock:
	mutex_unlock(&dev->struct_mutex);
	return ret;
#endif

}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,12,0)
int smi_dumb_destroy(struct drm_file *file,
		     struct drm_device *dev,
		     uint32_t handle)
{
       return drm_gem_handle_delete(file, handle);
}
#endif
