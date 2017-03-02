/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2006  Ralf Baechle <ralf@linux-mips.org>
 *
 */
#ifndef __ASM_DMA_COHERENCE_H
#define __ASM_DMA_COHERENCE_H

#include <linux/dma-direction.h>

enum coherent_io_user_state {
	IO_COHERENCE_DEFAULT,
	IO_COHERENCE_ENABLED,
	IO_COHERENCE_DISABLED,
};

#if defined(CONFIG_DMA_PERDEV_COHERENT)
/* Don't provide (hw_)coherentio to avoid misuse */
#elif defined(CONFIG_DMA_MAYBE_COHERENT)
extern enum coherent_io_user_state coherentio;
extern int hw_coherentio;
#else
#ifdef CONFIG_DMA_NONCOHERENT
#define coherentio	IO_COHERENCE_DISABLED
#else
#define coherentio	IO_COHERENCE_ENABLED
#endif
#define hw_coherentio	0
#endif /* CONFIG_DMA_MAYBE_COHERENT */

#ifdef CONFIG_DMA_PERDEV_COHERENT
static inline int dev_is_coherent(struct device *dev)
{
	return dev->archdata.dma_coherent;
}
#else
static inline int dev_is_coherent(struct device *dev)
{
	switch (coherentio) {
	default:
	case IO_COHERENCE_DEFAULT:
		return hw_coherentio;
	case IO_COHERENCE_ENABLED:
		return 1;
	case IO_COHERENCE_DISABLED:
		return 0;
	}
}
#endif /* CONFIG_DMA_PERDEV_COHERENT */

static inline void dma_sync_virt(void *addr, size_t size,
		enum dma_data_direction dir)
{
	switch (dir) {
	case DMA_TO_DEVICE:
		dma_cache_wback((unsigned long)addr, size);
		break;

	case DMA_FROM_DEVICE:
		dma_cache_inv((unsigned long)addr, size);
		break;

	case DMA_BIDIRECTIONAL:
		dma_cache_wback_inv((unsigned long)addr, size);
		break;

	default:
		BUG();
	}
}

#endif
