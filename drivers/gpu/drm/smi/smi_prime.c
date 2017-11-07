/*
 * Copyright 2016 SiliconMotion Inc.
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License version 2. See the file COPYING in the main
 * directory of this archive for more details.
 *
 */

#include <drm/drmP.h>
#include <linux/dma-buf.h>

#include "smi_drv.h"


struct sg_table *smi_gem_prime_get_sg_table(struct drm_gem_object *obj)
{
	struct smi_bo *bo = gem_to_smi_bo(obj);
	int npages = bo->bo.num_pages;

	return drm_prime_pages_to_sg(bo->bo.ttm->pages, npages);
}

void *smi_gem_prime_vmap(struct drm_gem_object *obj)
{
	struct smi_bo *bo = gem_to_smi_bo(obj);
	int ret;

	ret = ttm_bo_kmap(&bo->bo, 0, bo->bo.num_pages,
			  &bo->dma_buf_vmap);
	if (ret)
		return ERR_PTR(ret);

	return bo->dma_buf_vmap.virtual;
}

void smi_gem_prime_vunmap(struct drm_gem_object *obj, void *vaddr)
{
	struct smi_bo *bo = gem_to_smi_bo(obj);

	ttm_bo_kunmap(&bo->dma_buf_vmap);
}

struct drm_gem_object *smi_gem_prime_import_sg_table(struct drm_device *dev,
							struct dma_buf_attachment *attach,
							struct sg_table *sg)
{
	struct smi_device *sdev = dev->dev_private;
	struct smi_bo *bo;
	int ret;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,17,0)
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,4,0)
	struct reservation_object *resv = attach->dmabuf->resv;
#else
	struct dma_resv *resv = attach->dmabuf->resv;
#endif
	ww_mutex_lock(&resv->lock, NULL);
	ret = smi_bo_create(dev, attach->dmabuf->size, PAGE_SIZE, 0, sg, resv, &bo);
	ww_mutex_unlock(&resv->lock);
#else
	ret = smi_bo_create(dev, attach->dmabuf->size, PAGE_SIZE, 0, sg, &bo);
#endif
	if (ret)
		return ERR_PTR(ret);

	return &bo->gem;
}

int smi_gem_prime_pin(struct drm_gem_object *obj)
{
	ENTER();

	struct smi_bo *bo  = gem_to_smi_bo(obj);
	int ret = 0;

	ret = smi_bo_reserve(bo, false);
	if (unlikely(ret != 0))
		LEAVE(ret);

	/* pin buffer into GTT */
	ret = smi_bo_pin(bo, TTM_PL_FLAG_SYSTEM, NULL);
	smi_bo_unreserve(bo);
	LEAVE(ret);
}


void smi_gem_prime_unpin(struct drm_gem_object *obj)
{
	struct smi_bo *bo = gem_to_smi_bo(obj);
	int ret = 0;

	ret = smi_bo_reserve(bo, false);
	if (unlikely(ret != 0))
		return;

	smi_bo_unpin(bo);
	smi_bo_unreserve(bo);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,17,0) && LINUX_VERSION_CODE < KERNEL_VERSION(5,4,0)
struct reservation_object *smi_gem_prime_res_obj(struct drm_gem_object *obj)
{
	struct smi_bo *bo = gem_to_smi_bo(obj);

	return bo->bo.resv;
}
#endif

