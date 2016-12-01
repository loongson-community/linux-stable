/*
 *	include/asm-mips/mach-loongson64/ioremap.h
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */
#ifndef __ASM_MACH_LOONGSON_IOREMAP_H
#define __ASM_MACH_LOONGSON_IOREMAP_H

#include <linux/types.h>

/*
 * Allow physical addresses to be fixed up to help peripherals located
 * outside the low 32-bit range -- generic pass-through version.
 */
static inline phys_addr_t fixup_bigphys_addr(phys_addr_t phys_addr, phys_addr_t size)
{
	return phys_addr;
}

/* Add support for uncached accelerate */
static inline void __iomem *plat_ioremap(phys_addr_t offset, unsigned long size,
	unsigned long flags)
{
#ifdef CONFIG_64BIT
#define LOONGSON_UNCACHED_ACCEL_BASE _AC(0xb800000000000000, UL)
	u64 base = LOONGSON_UNCACHED_ACCEL_BASE;

	if (flags == _CACHE_UNCACHED_ACCELERATED &&
	    offset + size < XKSEG - base) {
		return (void __iomem *)(unsigned long) (base + offset);
	}
#endif
	return NULL;
}

static inline int plat_iounmap(const volatile void __iomem *addr)
{
	return 0;
}

#endif /* __ASM_MACH_LOONGSON_IOREMAP_H */
