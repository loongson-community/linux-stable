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

#include <ttm/ttm_page_alloc.h>
#include "loongson_drv.h"

#define DRM_FILE_PAGE_OFFSET (0x100000000ULL >> PAGE_SHIFT)

/**
 * loongson_bdev
 *
 * @bd: The ttm_bo_device.
 *
 * Return the virtual address of loongson_drm_device
 */
static inline struct loongson_drm_device *
loongson_bdev(struct ttm_bo_device *bd)
{
	return container_of(bd, struct loongson_drm_device, ttm.bdev);
}

/**
 * loongson_ttm_global_init
 *
 * @ldev: Pointer to a struct loongson_drm_object
 *
 * This function is initial loongson memory object, alloc memory
 */
static int
loongson_ttm_mem_global_init(struct drm_global_reference *ref)
{
	return ttm_mem_global_init(ref->object);
}

/**
 * loongson_ttm_mem_global_release
 *
 * @ref: Pointer to a struct drm_global_reference
 *
 * This function is release memory resource
 */
static void
loongson_ttm_mem_global_release(struct drm_global_reference *ref)
{
	ttm_mem_global_release(ref->object);
}

/**
 * loongson_ttm_global_init
 *
 * @ldev: Pointer to a struct loongson_drm_object
 *
 * This function is initial loongson memory object, alloc memory
 */
static int loongson_ttm_global_init(struct loongson_drm_device *ldev)
{
	struct drm_global_reference *global_ref;
	int r;

	global_ref = &ldev->ttm.mem_global_ref;
	global_ref->global_type = DRM_GLOBAL_TTM_MEM;
	global_ref->size = sizeof(struct ttm_mem_global);
	global_ref->init = &loongson_ttm_mem_global_init;
	global_ref->release = &loongson_ttm_mem_global_release;
	r = drm_global_item_ref(global_ref);
	if (r != 0) {
		DRM_ERROR("Failed setting up TTM memory accounting "
			  "subsystem.\n");
		return r;
	}

	ldev->ttm.bo_global_ref.mem_glob =
		ldev->ttm.mem_global_ref.object;
	global_ref = &ldev->ttm.bo_global_ref.ref;
	global_ref->global_type = DRM_GLOBAL_TTM_BO;
	global_ref->size = sizeof(struct ttm_bo_global);
	global_ref->init = &ttm_bo_global_init;
	global_ref->release = &ttm_bo_global_release;
	r = drm_global_item_ref(global_ref);
	if (r != 0) {
		DRM_ERROR("Failed setting up TTM BO subsystem.\n");
		drm_global_item_unref(&ldev->ttm.mem_global_ref);
		return r;
	}
	return 0;
}

/**
 * loongson_bo_ttm_destroy
 *
 * @tbo: Pointer to a struct ttm_buffer_object
 *
 * This function is release a fake mmap offset for an object
 */
static void loongson_bo_ttm_destroy(struct ttm_buffer_object *tbo)
{
	struct loongson_bo *bo;

	bo = container_of(tbo, struct loongson_bo, bo);

	drm_gem_object_release(&bo->gem);
	kfree(bo);
}

/**
 * loongson_ttm_bo_is_loongson_bo
 *
 * @bo: Pointer to a struct ttm_buffer_object
 *
 * This function is check if loongson buffer object
 * ture:  yes
 * false: no
 */
static bool loongson_ttm_bo_is_loongson_bo(struct ttm_buffer_object *bo)
{
	if (bo->destroy == &loongson_bo_ttm_destroy)
		return true;
	return false;
}

/**
 * loongson_bo_init_mem_type
 *
 * @bdev: Pointer to a struct ttm_bo_device
 * @type: Memory regions type for data placement
 * @man:used to identify and manage memory types for a device
 *
 * This function is used to identify and manage memory types for a device.
 */
static int
loongson_bo_init_mem_type(struct ttm_bo_device *bdev, uint32_t type,
		     struct ttm_mem_type_manager *man)
{
	switch (type) {
	case TTM_PL_SYSTEM:
		man->flags = TTM_MEMTYPE_FLAG_MAPPABLE;
		man->available_caching = TTM_PL_MASK_CACHING;
		man->default_caching = TTM_PL_FLAG_CACHED;
		break;
	case TTM_PL_VRAM:
		man->func = &ttm_bo_manager_func;
		man->flags = TTM_MEMTYPE_FLAG_FIXED |
			TTM_MEMTYPE_FLAG_MAPPABLE;
		man->available_caching = TTM_PL_FLAG_UNCACHED |
			TTM_PL_FLAG_WC;
		man->default_caching = TTM_PL_FLAG_WC;
		break;
	default:
		DRM_ERROR("Unsupported memory type %u\n", (unsigned)type);
		return -EINVAL;
	}
	return 0;
}

/**
 * loongson_bo_evict_flags:
 *
 * @bo: the buffer object to be evicted
 *
 * Return the bo flags for a buffer which is not mapped to the hardware.
 * These will be placed in proposed_flags so that when the move is
 * finished, they'll end up in bo->mem.flags
 */
static void
loongson_bo_evict_flags(struct ttm_buffer_object *bo, struct ttm_placement *pl)
{
	struct loongson_bo *loongsonbo = loongson_bo(bo);

	if (!loongson_ttm_bo_is_loongson_bo(bo))
		return;

	loongson_ttm_placement(loongsonbo, TTM_PL_FLAG_SYSTEM);
	*pl = loongsonbo->placement;
}

/**
 * loongson_bo_verify_access - Access verification helper for TTM
 * @node: Offset node
 * @filp: Open-file
 *
 * This checks whether @filp is granted access to @node. It is the same as
 * drm_vma_node_is_allowed() but suitable as drop-in helper for TTM
 * verify_access() callbacks.
 *
 * RETURNS:
 * 0 if access is granted, -EACCES otherwise.
 */
static int loongson_bo_verify_access(struct ttm_buffer_object *bo, struct file *filp)
{
	struct loongson_bo *loongsonbo = loongson_bo(bo);

	return drm_vma_node_verify_access(&loongsonbo->gem.vma_node, filp->private_data);
}

/**
 * loongson_ttm_io_mem_reserve
 *
 * @bdev: Pointer to a struct ttm_bo_device
 * @mem: A valid struct ttm_mem_reg
 *
 * Driver callback on when mapping io memory
 */
static int loongson_ttm_io_mem_reserve(struct ttm_bo_device *bdev,
				  struct ttm_mem_reg *mem)
{
	struct ttm_mem_type_manager *man = &bdev->man[mem->mem_type];
	struct loongson_drm_device *ldev = loongson_bdev(bdev);

	mem->bus.addr = NULL;
	mem->bus.offset = 0;
	mem->bus.size = mem->num_pages << PAGE_SHIFT;
	mem->bus.base = 0;
	mem->bus.is_iomem = false;
	if (!(man->flags & TTM_MEMTYPE_FLAG_MAPPABLE))
		return -EINVAL;
	switch (mem->mem_type) {
	case TTM_PL_SYSTEM:
		/* system memory */
		return 0;
	case TTM_PL_VRAM:
		mem->bus.base = ldev->mc.vram_base;
		mem->bus.offset = mem->start << PAGE_SHIFT;
		mem->bus.is_iomem = true;
		break;
	default:
		return -EINVAL;
		break;
	}
	return 0;
}

/**
 * loongson_ttm_io_mem_free
 *
 * @bdev: Pointer to a struct ttm_bo_device
 * @mem: A valid struct ttm_mem_reg
 *
 * Driver callback, free io memory
 */
static void loongson_ttm_io_mem_free(struct ttm_bo_device *bdev, struct ttm_mem_reg *mem)
{
}

/**
 * loongson_ttm_backend_destroy
 *
 * @bdev: Pointer to a struct ttm_bo_device
 * @mem: A valid struct ttm_mem_reg
 *
 * Driver callback on when mapping io memory
 */
static void loongson_ttm_backend_destroy(struct ttm_tt *tt)
{
	ttm_tt_fini(tt);
	kfree(tt);
}

static struct ttm_backend_func loongson_tt_backend_func = {
	.destroy = &loongson_ttm_backend_destroy,
};

/**
 * loongson_ttm_tt_create
 *
 * @bdev: Pointer to a ttm_bo_device struct.
 * @size: Size of the data needed backing.
 * @page_flags: Page flags as identified by TTM_PAGE_FLAG_XX flags.
 * @dummy_read_page: See struct ttm_bo_device.
 *
 * Initialize a manager for a given memory type.
 * Note: if part of driver firstopen, it must be protected from a
 * potentially racing lastclose.
 * Returns:
 * NULL: out of memeory
 */
static struct ttm_tt *loongson_ttm_tt_create(struct ttm_buffer_object *bo, uint32_t page_flags)
{
	struct ttm_tt *tt;

	tt = kzalloc(sizeof(struct ttm_tt), GFP_KERNEL);
	if (tt == NULL)
		return NULL;
	tt->func = &loongson_tt_backend_func;

	/* Create a struct ttm_tt to back data with system memory pages.
	 * No pages are actually allocated. NULL out of memory */
	if (ttm_tt_init(tt, bo, page_flags)) {
		kfree(tt);
		return NULL;
	}
	return tt;
}

/**
 * loongson_ttm_pool_populate:
 *
 * @ttm: The struct ttm_tt to contain the backing pages.
 *
 * Add backing pages to all of @ttm
 */
static int loongson_ttm_tt_populate(struct ttm_tt *ttm, struct ttm_operation_ctx *ctx)
{
	return ttm_pool_populate(ttm, ctx);
}

/**
 * loongson_ttm_pool_unpopulate:
 *
 * @ttm: The struct ttm_tt which to free backing pages.
 *
 * Free all pages of @ttm
 */
static void loongson_ttm_tt_unpopulate(struct ttm_tt *ttm)
{
	ttm_pool_unpopulate(ttm);
}

struct ttm_bo_driver loongson_bo_driver = {
	.ttm_tt_create = loongson_ttm_tt_create,
	.ttm_tt_populate = loongson_ttm_tt_populate,
	.ttm_tt_unpopulate = loongson_ttm_tt_unpopulate,
	.init_mem_type = loongson_bo_init_mem_type,
	.evict_flags = loongson_bo_evict_flags,
	.eviction_valuable = ttm_bo_eviction_valuable,
	.verify_access = loongson_bo_verify_access,
	.io_mem_reserve = &loongson_ttm_io_mem_reserve,
	.io_mem_free = &loongson_ttm_io_mem_free,
};

/**
 * loongson_ttm_init:
 *
 * @ldev: The struct loongson_drm_device
 *
 * loongson ttm init
 */
int loongson_ttm_init(struct loongson_drm_device *ldev)
{
	int ret;
	struct drm_device *dev = ldev->dev;
	struct ttm_bo_device *bdev = &ldev->ttm.bdev;
	struct pci_dev *gpu_pdev;

	gpu_pdev= pci_get_device(PCI_VENDOR_ID_LOONGSON,PCI_DEVICE_ID_LOONGSON_GPU,NULL);
	ret = loongson_ttm_global_init(ldev);
	if (ret)
		return ret;

	ret = ttm_bo_device_init(&ldev->ttm.bdev,
				 ldev->ttm.bo_global_ref.ref.object,
				 &loongson_bo_driver,
				 dev->anon_inode->i_mapping,
				 DRM_FILE_PAGE_OFFSET,
				 true);
	if (ret) {
		DRM_ERROR("Error initialising bo driver; %d\n", ret);
		return ret;
	}

	ret = ttm_bo_init_mm(bdev, TTM_PL_VRAM, ldev->mc.vram_size >> PAGE_SHIFT);
	if (ret) {
		DRM_ERROR("Failed ttm VRAM init: %d\n", ret);
		return ret;
	}

	return 0;
}

/**
 * loongson_ttm_global_release --- release ttm global
 *
 * @ast: The struct loongson_drm_device
 *
 * release ttm global
 */
static void loongson_ttm_global_release(struct loongson_drm_device *ast)
{
        if (ast->ttm.mem_global_ref.release == NULL)
                return;

        drm_global_item_unref(&ast->ttm.bo_global_ref.ref);
        drm_global_item_unref(&ast->ttm.mem_global_ref);
        ast->ttm.mem_global_ref.release = NULL;
}

/**
 * loongson_ttm_fini --- deinit ttm
 *
 * @ast: The struct loongson_drm_device
 *
 * release ttm
 */
void loongson_ttm_fini(struct loongson_drm_device *ldev)
{
        ttm_bo_device_release(&ldev->ttm.bdev);
                   
        loongson_ttm_global_release(ldev);
}

/**
 * loongson_ttm_placement:
 *
 * @bo: The struct loongson_bo
 * @domain: ttm placements type
 *
 * loongson ttm placement config
 */
void loongson_ttm_placement(struct loongson_bo *bo, int domain)
{
	u32 c = 0;
	unsigned i;

	bo->placement.placement = bo->placements;
	bo->placement.busy_placement = bo->placements;
	if (domain & TTM_PL_FLAG_VRAM)
		bo->placements[c++].flags = TTM_PL_FLAG_WC | TTM_PL_FLAG_UNCACHED | TTM_PL_FLAG_VRAM;
	if (domain & TTM_PL_FLAG_SYSTEM)
		bo->placements[c++].flags = TTM_PL_MASK_CACHING | TTM_PL_FLAG_SYSTEM;
	if (!c)
		bo->placements[c++].flags = TTM_PL_MASK_CACHING | TTM_PL_FLAG_SYSTEM;
	bo->placement.num_placement = c;
	bo->placement.num_busy_placement = c;
	for (i = 0; i < c; ++i) {
		bo->placements[i].fpfn = 0;
		bo->placements[i].lpfn = 0;
	}
}

/**
 * loongson_bo_create ---create loongson ttm buffer object
 *
 * @dev:  The struct drm_device
 * @size: Requested size of buffer object.
 * @flags: Initial placement flags.
 * @ploongsonbo: Pointer to a ttm_buffer_object to be initialized
 *
 * RETURN
 *  ttm buffer object create result
 */
int loongson_bo_create(struct drm_device *dev, int size, int align,
		  uint32_t flags, struct loongson_bo **ploongsonbo)
{
	struct loongson_drm_device *ldev = dev->dev_private;
	struct loongson_bo *loongsonbo;
	size_t acc_size;
	int ret;

	loongsonbo = kzalloc(sizeof(struct loongson_bo), GFP_KERNEL);
	if (!loongsonbo)
		return -ENOMEM;

	ret = drm_gem_object_init(dev, &loongsonbo->gem, size);
	if (ret) {
		kfree(loongsonbo);
		return ret;
	}

	loongsonbo->bo.bdev = &ldev->ttm.bdev;
	loongsonbo->mc.vram_base = ldev->mc.vram_base;
	loongsonbo->mc.vram_size = ldev->mc.vram_size;
	loongsonbo->mc.vram_window = ldev->mc.vram_window;
	loongson_ttm_placement(loongsonbo, TTM_PL_FLAG_VRAM | TTM_PL_FLAG_SYSTEM);

	acc_size = ttm_bo_dma_acc_size(&ldev->ttm.bdev, size,
				       sizeof(struct loongson_bo));

	ret = ttm_bo_init(&ldev->ttm.bdev, &loongsonbo->bo, size,
			  ttm_bo_type_device, &loongsonbo->placement,
			  align >> PAGE_SHIFT, false, acc_size,
			  NULL, NULL, loongson_bo_ttm_destroy);
	if (ret)
		return ret;

	*ploongsonbo = loongsonbo;
	return 0;
}

/**
 * loongson_bo_gpu_offset
 *
 * @bo: The struct loongson_bo
 *
 * return GPU address space offset
 */
static inline u64 loongson_bo_gpu_offset(struct loongson_bo *bo)
{
	return (bo->bo.offset + bo->mc.vram_base);
}

/**
 * loongson_bo_pin
 *
 * @bo: The struct loongson_bo
 * @pl_flag: placement flag
 * @gpu_addr: gpu address space
 *
 * the function is intended to check pin_count usage,
 * return GPU address and set placement flags
 */
int loongson_bo_pin(struct loongson_bo *bo, u32 pl_flag, u64 *gpu_addr)
{
	int i, ret;
	struct ttm_operation_ctx ctx = { false, false };

	if (bo->pin_count) {
		bo->pin_count++;
		if (gpu_addr)
			*gpu_addr = loongson_bo_gpu_offset(bo);
		return 0;
	}

	loongson_ttm_placement(bo, pl_flag);
	for (i = 0; i < bo->placement.num_placement; i++)
		bo->placements[i].flags |= TTM_PL_FLAG_NO_EVICT;
	ret = ttm_bo_validate(&bo->bo, &bo->placement, &ctx);
	if (ret)
		return ret;

	bo->pin_count = 1;
	if (gpu_addr)
		*gpu_addr = loongson_bo_gpu_offset(bo);
	return 0;
}

/**
 * loongson_bo_unpin
 *
 * @bo: The struct loongson_bo
 *
 * the function is check pin_count usage and clear placement flag
 */
int loongson_bo_unpin(struct loongson_bo *bo)
{
	int i;
	struct ttm_operation_ctx ctx = { false, false };

	if (!bo->pin_count) {
		DRM_ERROR("unpin bad %p\n", bo);
		return 0;
	}
	bo->pin_count--;
	if (bo->pin_count)
		return 0;

	for (i = 0; i < bo->placement.num_placement ; i++)
		bo->placements[i].flags &= ~TTM_PL_FLAG_NO_EVICT;
	return ttm_bo_validate(&bo->bo, &bo->placement, &ctx);
}

/**
 * loongson_bo_push_sysram
 *
 * @bo pointer structure to loongson_bo
 *
 * This function is intended to be called by the device mmap method.
 */
int loongson_bo_push_sysram(struct loongson_bo *bo)
{
	int i, ret;
	struct ttm_operation_ctx ctx = { false, false };

	if (!bo->pin_count) {
		DRM_ERROR("unpin bad %p\n", bo);
		return 0;
	}
	bo->pin_count--;
	if (bo->pin_count)
		return 0;

	if (bo->kmap.virtual)
		ttm_bo_kunmap(&bo->kmap);

	loongson_ttm_placement(bo, TTM_PL_FLAG_SYSTEM);
	for (i = 0; i < bo->placement.num_placement ; i++)
		bo->placements[i].flags |= TTM_PL_FLAG_NO_EVICT;

	/** Changes placement and caching policy of the buffer object
      * according proposed placement. */
	ret = ttm_bo_validate(&bo->bo, &bo->placement, &ctx);
	if (ret) {
		DRM_ERROR("pushing to VRAM failed\n");
		return ret;
	}
	return 0;
}

/**
 * loongson_drm_mmap - mmap out of the ttm device address space.
 *
 * @filp:      filp as input from the mmap method.
 * @vma:       vma as input from the mmap method.
 *
 * This function is intended to be called by the device mmap method.
 * if the device address space is to be backed by the bo manager.
 */
int loongson_drm_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct drm_file *file_priv;
	struct loongson_drm_device *ldev;

	if (unlikely(vma->vm_pgoff < DRM_FILE_PAGE_OFFSET))
		return -EINVAL;

	file_priv = filp->private_data;
	ldev = file_priv->minor->dev->dev_private;
	return ttm_bo_mmap(filp, vma, &ldev->ttm.bdev);
}
