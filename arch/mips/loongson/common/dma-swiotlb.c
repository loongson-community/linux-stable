#include <linux/mm.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/scatterlist.h>
#include <linux/swiotlb.h>
#include <linux/bootmem.h>

#include <asm/bootinfo.h>
#include <dma-coherence.h>

static void *loongson_dma_alloc_coherent(struct device *dev, size_t size,
				dma_addr_t *dma_handle, gfp_t gfp, struct dma_attrs *attrs)
{
	void *ret;

	if (dma_alloc_from_coherent(dev, size, dma_handle, &ret))
		return ret;

	/* ignore region specifiers */
	gfp &= ~(__GFP_DMA | __GFP_DMA32 | __GFP_HIGHMEM);

#ifdef CONFIG_ZONE_DMA
	if (dev == NULL)
		gfp |= __GFP_DMA;
	else if (dev->coherent_dma_mask <= DMA_BIT_MASK(24))
		gfp |= __GFP_DMA;
	else
#endif
#ifdef CONFIG_ZONE_DMA32
	if (dev == NULL)
		gfp |= __GFP_DMA32;
	else if (dev->coherent_dma_mask <= DMA_BIT_MASK(32))
		gfp |= __GFP_DMA32;
	else
#endif
	;
	gfp |= __GFP_NORETRY;

	ret = swiotlb_alloc_coherent(dev, size, dma_handle, gfp);
	mb();
	return ret;
}

static void loongson_dma_free_coherent(struct device *dev, size_t size,
				void *vaddr, dma_addr_t dma_handle, struct dma_attrs *attrs)
{
	int order = get_order(size);

	if (dma_release_from_coherent(dev, order, vaddr))
		return;

	swiotlb_free_coherent(dev, size, vaddr, dma_handle);
}

static dma_addr_t loongson_dma_map_page(struct device *dev, struct page *page,
				unsigned long offset, size_t size,
				enum dma_data_direction dir,
				struct dma_attrs *attrs)
{
	dma_addr_t daddr = swiotlb_map_page(dev, page, offset, size,
					dir, attrs);
	mb();
	return daddr;
}

static int loongson_dma_map_sg(struct device *dev, struct scatterlist *sg,
				int nents, enum dma_data_direction dir,
				struct dma_attrs *attrs)
{
	int r = swiotlb_map_sg_attrs(dev, sg, nents, dir, NULL);
	mb();

	return r;
}

static void loongson_dma_sync_single_for_device(struct device *dev,
				dma_addr_t dma_handle, size_t size,
				enum dma_data_direction dir)
{
	swiotlb_sync_single_for_device(dev, dma_handle, size, dir);
	mb();
}

static void loongson_dma_sync_sg_for_device(struct device *dev,
				struct scatterlist *sg, int nents,
				enum dma_data_direction dir)
{
	swiotlb_sync_sg_for_device(dev, sg, nents, dir);
	mb();
}

static dma_addr_t loongson_unity_phys_to_dma(struct device *dev, phys_addr_t paddr)
{
	return (paddr < 0x10000000) ?
			(paddr | 0x0000000080000000) : paddr;
}

static phys_addr_t loongson_unity_dma_to_phys(struct device *dev, dma_addr_t daddr)
{
	return (daddr < 0x90000000 && daddr >= 0x80000000) ?
			(daddr & 0x0fffffff) : daddr;
}

struct loongson_dma_map_ops {
	struct dma_map_ops dma_map_ops;
	dma_addr_t (*phys_to_dma)(struct device *dev, phys_addr_t paddr);
	phys_addr_t (*dma_to_phys)(struct device *dev, dma_addr_t daddr);
};

dma_addr_t phys_to_dma(struct device *dev, phys_addr_t paddr)
{
	struct loongson_dma_map_ops *ops = container_of(get_dma_ops(dev),
					struct loongson_dma_map_ops, dma_map_ops);

	return ops->phys_to_dma(dev, paddr);
}

phys_addr_t dma_to_phys(struct device *dev, dma_addr_t daddr)
{
	struct loongson_dma_map_ops *ops = container_of(get_dma_ops(dev),
					struct loongson_dma_map_ops, dma_map_ops);

	return ops->dma_to_phys(dev, daddr);
}

static int loongson_dma_set_mask(struct device *dev, u64 mask)
{
	/* Loongson doesn't support DMA above 32-bit */
	if (mask > DMA_BIT_MASK(32)) {
		*dev->dma_mask = DMA_BIT_MASK(32);
		return -EIO;
	}

	*dev->dma_mask = mask;

	return 0;
}

static struct loongson_dma_map_ops loongson_linear_dma_map_ops = {
	.dma_map_ops = {
		.alloc = loongson_dma_alloc_coherent,
		.free = loongson_dma_free_coherent,
		.map_page = loongson_dma_map_page,
		.unmap_page = swiotlb_unmap_page,
		.map_sg = loongson_dma_map_sg,
		.unmap_sg = swiotlb_unmap_sg_attrs,
		.sync_single_for_cpu = swiotlb_sync_single_for_cpu,
		.sync_single_for_device = loongson_dma_sync_single_for_device,
		.sync_sg_for_cpu = swiotlb_sync_sg_for_cpu,
		.sync_sg_for_device = loongson_dma_sync_sg_for_device,
		.mapping_error = swiotlb_dma_mapping_error,
		.dma_supported = swiotlb_dma_supported,
		.set_dma_mask = loongson_dma_set_mask
	},
	.phys_to_dma = loongson_unity_phys_to_dma,
	.dma_to_phys = loongson_unity_dma_to_phys
};

void __init plat_swiotlb_setup(void)
{
	swiotlb_init(1);
	mips_dma_map_ops = &loongson_linear_dma_map_ops.dma_map_ops;
}
