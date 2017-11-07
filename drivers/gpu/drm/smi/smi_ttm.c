/*
 * Copyright 2016 SiliconMotion Inc.
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License version 2. See the file COPYING in the main
 * directory of this archive for more details.
 *
 */
 
#include <drm/drmP.h>
#include <ttm/ttm_page_alloc.h>
#include "smi_drv.h"

static inline struct smi_device *
smi_bdev(struct ttm_bo_device *bd)
{
	return container_of(bd, struct smi_device, ttm.bdev);
}

static int
smi_ttm_mem_global_init(struct drm_global_reference *ref)
{
	return ttm_mem_global_init(ref->object);
}

static void
smi_ttm_mem_global_release(struct drm_global_reference *ref)
{
	ttm_mem_global_release(ref->object);
}

static int smi_ttm_global_init(struct smi_device *smi)
{
	struct drm_global_reference *global_ref;
	int r;

	global_ref = &smi->ttm.mem_global_ref;
	global_ref->global_type = DRM_GLOBAL_TTM_MEM;
	global_ref->size = sizeof(struct ttm_mem_global);
	global_ref->init = &smi_ttm_mem_global_init;
	global_ref->release = &smi_ttm_mem_global_release;
	r = drm_global_item_ref(global_ref);
	if (r != 0) {
		DRM_ERROR("Failed setting up TTM memory accounting "
			  "subsystem.\n");
		return r;
	}

	smi->ttm.bo_global_ref.mem_glob =
		smi->ttm.mem_global_ref.object;
	global_ref = &smi->ttm.bo_global_ref.ref;
	global_ref->global_type = DRM_GLOBAL_TTM_BO;
	global_ref->size = sizeof(struct ttm_bo_global);
	global_ref->init = &ttm_bo_global_init;
	global_ref->release = &ttm_bo_global_release;
	r = drm_global_item_ref(global_ref);
	if (r != 0) {
		DRM_ERROR("Failed setting up TTM BO subsystem.\n");
		drm_global_item_unref(&smi->ttm.mem_global_ref);
		return r;
	}
	return 0;
}

void
smi_ttm_global_release(struct smi_device *smi)
{
	if (smi->ttm.mem_global_ref.release == NULL)
		return;

	drm_global_item_unref(&smi->ttm.bo_global_ref.ref);
	drm_global_item_unref(&smi->ttm.mem_global_ref);
	smi->ttm.mem_global_ref.release = NULL;
}


void smi_bo_ttm_destroy(struct ttm_buffer_object *tbo)
{
	struct smi_bo *bo;

	bo = container_of(tbo, struct smi_bo, bo);

	drm_gem_object_release(&bo->gem);
	kfree(bo);
}

bool smi_ttm_bo_is_smi_bo(struct ttm_buffer_object *bo)
{
	if (bo->destroy == &smi_bo_ttm_destroy)
		return true;
	return false;
}

static int
smi_bo_init_mem_type(struct ttm_bo_device *bdev, uint32_t type,
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
#ifdef NO_WC
		man->available_caching = TTM_PL_FLAG_UNCACHED;
		man->default_caching = TTM_PL_FLAG_UNCACHED;
#else
		man->available_caching = TTM_PL_FLAG_UNCACHED |
			TTM_PL_FLAG_WC;
		man->default_caching = TTM_PL_FLAG_WC;
#endif
		break;
	default:
		DRM_ERROR("Unsupported memory type %u\n", (unsigned)type);
		return -EINVAL;
	}
	return 0;
}

static void
smi_bo_evict_flags(struct ttm_buffer_object *bo, struct ttm_placement *pl)
{
	struct smi_bo *smibo = smi_bo(bo);

	if (!smi_ttm_bo_is_smi_bo(bo))
		return;

	smi_ttm_placement(smibo, TTM_PL_FLAG_SYSTEM);
	*pl = smibo->placement;
}

static int smi_bo_verify_access(struct ttm_buffer_object *bo, struct file *filp)
{
	struct smi_bo *smibo = smi_bo(bo);
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,12,0)
	return 0;
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0)
	return drm_vma_node_verify_access(&smibo->gem.vma_node, filp);
#else
	return drm_vma_node_verify_access(&smibo->gem.vma_node, filp->private_data);
#endif
}

static int smi_ttm_io_mem_reserve(struct ttm_bo_device *bdev,
				  struct ttm_mem_reg *mem)
{
	struct ttm_mem_type_manager *man = &bdev->man[mem->mem_type];
	struct smi_device *smi = smi_bdev(bdev);

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
		mem->bus.offset = mem->start << PAGE_SHIFT;
		mem->bus.base = pci_resource_start(smi->dev->pdev, 0);
		mem->bus.is_iomem = true;
		break;
	default:
		return -EINVAL;
		break;
	}
	return 0;
}

static void smi_ttm_io_mem_free(struct ttm_bo_device *bdev, struct ttm_mem_reg *mem)
{
}
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,8,0)
static int smi_bo_move(struct ttm_buffer_object *bo,
		       bool evict, bool interruptible,
		       bool no_wait_gpu,
		       struct ttm_mem_reg *new_mem)
{
	int r;
	r = ttm_bo_move_memcpy(bo, evict, no_wait_gpu, new_mem);
	return r;
}
#endif

static void smi_ttm_backend_destroy(struct ttm_tt *tt)
{
	ttm_tt_fini(tt);
	kfree(tt);
}

static struct ttm_backend_func smi_tt_backend_func = {
	.destroy = &smi_ttm_backend_destroy,
};


#if LINUX_VERSION_CODE > KERNEL_VERSION(4,16,0)
struct ttm_tt *smi_ttm_tt_create(struct ttm_buffer_object *bo, uint32_t page_flags)
#else
struct ttm_tt *smi_ttm_tt_create(struct ttm_bo_device *bdev, unsigned long size,
				 uint32_t page_flags, struct page *dummy_read_page)
#endif
{
	struct ttm_tt *tt;

	tt = kzalloc(sizeof(struct ttm_tt), GFP_KERNEL);
	if (tt == NULL)
		return NULL;
	tt->func = &smi_tt_backend_func;
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,16,0)
	if (ttm_tt_init(tt, bo, page_flags)) {
#else
	if (ttm_tt_init(tt, bdev, size, page_flags, dummy_read_page)) {
#endif
		kfree(tt);
		return NULL;
	}
	return tt;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0)
static int smi_ttm_tt_populate(struct ttm_tt *ttm)
#else
static int smi_ttm_tt_populate(struct ttm_tt *ttm, struct ttm_operation_ctx *ctx)
#endif
{
	bool slave = !!(ttm->page_flags & TTM_PAGE_FLAG_SG);

	if (ttm->state != tt_unpopulated)
 		return 0;

	if (slave && ttm->sg) {
		drm_prime_sg_to_page_addr_arrays(ttm->sg, ttm->pages,
						 NULL, ttm->num_pages);
		ttm->state = tt_unbound;
		return 0;
	}
	
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0)
	return ttm_pool_populate(ttm);
#else
	return ttm_pool_populate(ttm, ctx);
#endif
}

static void smi_ttm_tt_unpopulate(struct ttm_tt *ttm)
{
	bool slave = !!(ttm->page_flags & TTM_PAGE_FLAG_SG);

	if (slave)
		return;

	ttm_pool_unpopulate(ttm);
}

struct ttm_bo_driver smi_bo_driver = {
	.ttm_tt_create = smi_ttm_tt_create,
	.ttm_tt_populate = smi_ttm_tt_populate,
	.ttm_tt_unpopulate = smi_ttm_tt_unpopulate,
	.init_mem_type = smi_bo_init_mem_type,
	.evict_flags = smi_bo_evict_flags,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
	.eviction_valuable = ttm_bo_eviction_valuable,
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,8,0)
	.move = smi_bo_move,
#else
	.move = NULL,
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,7,0) && LINUX_VERSION_CODE < KERNEL_VERSION(4,11,0))
	.lru_tail = &ttm_bo_default_lru_tail,
	.swap_lru_tail = &ttm_bo_default_swap_lru_tail,
#endif
	.verify_access = smi_bo_verify_access,
	.io_mem_reserve = &smi_ttm_io_mem_reserve,
	.io_mem_free = &smi_ttm_io_mem_free,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0) && LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0))
	.io_mem_pfn = ttm_bo_default_io_mem_pfn,
#endif
};

int smi_mm_init(struct smi_device *smi)
{
	int ret;
	struct drm_device *dev = smi->dev;
	struct ttm_bo_device *bdev = &smi->ttm.bdev;

	ret = smi_ttm_global_init(smi);
	if (ret)
		return ret;

	ret = ttm_bo_device_init(&smi->ttm.bdev,
				 smi->ttm.bo_global_ref.ref.object,
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,14,0)					 
				 &smi_bo_driver, dev->anon_inode->i_mapping,
				 DRM_FILE_PAGE_OFFSET,smi->need_dma32);
#else
				 &smi_bo_driver, DRM_FILE_PAGE_OFFSET,
				 smi->need_dma32);
#endif				 
	if (ret) {
		DRM_ERROR("Error initialising bo driver; %d\n", ret);
		return ret;
	}

	if(g_specId == SPC_SM750)  //SM750 has only 16MB vram. We have to report 64MB vram for the prime function.
		ret = ttm_bo_init_mm(bdev, TTM_PL_VRAM,
			     0x4000000 >> PAGE_SHIFT);
	else
		ret = ttm_bo_init_mm(bdev, TTM_PL_VRAM,
			     smi->mc.vram_size >> PAGE_SHIFT);

	if (ret) {
		DRM_ERROR("Failed ttm VRAM init: %d\n", ret);
		return ret;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0)
	arch_io_reserve_memtype_wc(pci_resource_start(dev->pdev, 0),
				   pci_resource_len(dev->pdev, 0));
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,11,0)
 	smi->fb_mtrr = arch_phys_wc_add(pci_resource_start(dev->pdev, 0),
					   pci_resource_len(dev->pdev, 0));
#endif
 	smi->mm_inited = true;
	return 0;
}

void smi_mm_fini(struct smi_device *smi)
{
	struct drm_device *dev = smi->dev;
	
	if (!smi->mm_inited)
		return;
 	ttm_bo_device_release(&smi->ttm.bdev);
 	smi_ttm_global_release(smi);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0)
	arch_io_free_memtype_wc(pci_resource_start(dev->pdev, 0),
				pci_resource_len(dev->pdev, 0));
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,11,0)
 	arch_phys_wc_del(smi->fb_mtrr);
#endif
	smi->fb_mtrr = 0;
}

void smi_ttm_placement(struct smi_bo *bo, int domain)
{
	u32 c = 0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0) // Loongson backport from 3.18
	unsigned i;
	bo->placement.placement = bo->placements;
	bo->placement.busy_placement = bo->placements;
	if (domain & TTM_PL_FLAG_VRAM)
#ifdef NO_WC
		bo->placements[c++].flags = TTM_PL_FLAG_UNCACHED | TTM_PL_FLAG_VRAM;
#else
		bo->placements[c++].flags = TTM_PL_FLAG_WC | TTM_PL_FLAG_UNCACHED | TTM_PL_FLAG_VRAM;
#endif
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
#else	
	bo->placement.fpfn = 0;
	bo->placement.lpfn = 0;
	bo->placement.placement = bo->placements;
	bo->placement.busy_placement = bo->placements;
	if (domain & TTM_PL_FLAG_VRAM)
#ifdef NO_WC
		bo->placements[c++] = TTM_PL_FLAG_UNCACHED | TTM_PL_FLAG_VRAM;
#else
		bo->placements[c++] = TTM_PL_FLAG_WC | TTM_PL_FLAG_UNCACHED | TTM_PL_FLAG_VRAM;
#endif
	if (domain & TTM_PL_FLAG_SYSTEM)
		bo->placements[c++] = TTM_PL_MASK_CACHING | TTM_PL_FLAG_SYSTEM;
	if (!c)
		bo->placements[c++] = TTM_PL_MASK_CACHING | TTM_PL_FLAG_SYSTEM;
	bo->placement.num_placement = c;
	bo->placement.num_busy_placement = c;
#endif
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0)
int smi_bo_create(struct drm_device *dev, int size, int align,
		  uint32_t flags, struct sg_table *sg, struct smi_bo **psmibo)
#else
int smi_bo_create(struct drm_device *dev, int size, int align,
		  uint32_t flags, struct sg_table *sg, struct reservation_object *resv, struct smi_bo **psmibo)
#endif
{
	struct smi_device *smi = dev->dev_private;
	struct smi_bo *smibo;
	size_t acc_size;
	enum ttm_bo_type type;
	int ret;

	*psmibo = NULL;

	if (sg) {
		type = ttm_bo_type_sg;
	} else {
		type = ttm_bo_type_device;
	}


	smibo = kzalloc(sizeof(struct smi_bo), GFP_KERNEL);
	if (!smibo)
		return -ENOMEM;

	ret = drm_gem_object_init(dev, &smibo->gem, size);
	if (ret)
		goto error;
	

	smibo->bo.bdev = &smi->ttm.bdev;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,14,0)	
	smibo->bo.bdev->dev_mapping = dev->dev_mapping;
#endif
	smi_ttm_placement(smibo, TTM_PL_FLAG_VRAM | TTM_PL_FLAG_SYSTEM);

	acc_size = ttm_bo_dma_acc_size(&smi->ttm.bdev, size,
				       sizeof(struct smi_bo));

	ret = ttm_bo_init(&smi->ttm.bdev, &smibo->bo, size,
			  type, &smibo->placement,
			  align >> PAGE_SHIFT, false,
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4,16,0)
			  NULL,
#endif
			  acc_size, sg,
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,18,0)
			  resv,
#endif
			  smi_bo_ttm_destroy);
	if (ret)
		goto error;
	

	*psmibo = smibo;
	 return 0;

error:
	kfree(smibo);
	return ret;
}

static inline u64 smi_bo_gpu_offset(struct smi_bo *bo)
{
	return bo->bo.offset;
}

int smi_bo_pin(struct smi_bo *bo, u32 pl_flag, u64 *gpu_addr)
{
	int i, ret;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,15,0)
	struct ttm_operation_ctx ctx = { false, false };
#endif

	if (bo->pin_count) {
		bo->pin_count++;
		if (gpu_addr)
			*gpu_addr = smi_bo_gpu_offset(bo);
	}

	smi_ttm_placement(bo, pl_flag);
	for (i = 0; i < bo->placement.num_placement; i++)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0) // Loongson backport from 3.18
		bo->placements[i].flags |= TTM_PL_FLAG_NO_EVICT;
#else
		bo->placements[i] |= TTM_PL_FLAG_NO_EVICT;
#endif		
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,15,0)
	ret = ttm_bo_validate(&bo->bo, &bo->placement, &ctx);
#else
	ret = ttm_bo_validate(&bo->bo, &bo->placement, false, false);
#endif
	if (ret)
		return ret;

	bo->pin_count = 1;
	if (gpu_addr)
		*gpu_addr = smi_bo_gpu_offset(bo);
	return 0;
}

int smi_bo_unpin(struct smi_bo *bo)
{
	int i, ret;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,15,0)
	struct ttm_operation_ctx ctx = { false, false };
#endif
	if (!bo->pin_count) {
		dbg_msg("unpin bad %p\n", bo);
		return 0;
	}
	bo->pin_count--;
	if (bo->pin_count)
		return 0;

	for (i = 0; i < bo->placement.num_placement ; i++)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0) // Loongson backport from 3.18
		bo->placements[i].flags &= ~TTM_PL_FLAG_NO_EVICT;
#else
		bo->placements[i] &= ~TTM_PL_FLAG_NO_EVICT;
#endif		
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,15,0)
	ret = ttm_bo_validate(&bo->bo, &bo->placement, &ctx);
#else
	ret = ttm_bo_validate(&bo->bo, &bo->placement, false, false);
#endif
	if (ret)
		return ret;

	return 0;
}


int smi_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct drm_file *file_priv;
	struct smi_device *smi;

	if (unlikely(vma->vm_pgoff < DRM_FILE_PAGE_OFFSET))
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,14,0)	
		return -EINVAL;
#else
		return drm_mmap(filp, vma);
#endif		

	file_priv = filp->private_data;
	smi = file_priv->minor->dev->dev_private;
	return ttm_bo_mmap(filp, vma, &smi->ttm.bdev);
}

