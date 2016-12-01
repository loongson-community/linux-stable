/*
 * Copyright (C) 2007 Lemote Inc. & Institute of Computing Technology
 * Author: Fuxin Zhang, zhangfx@lemote.com
 *
 *  This program is free software; you can redistribute	 it and/or modify it
 *  under  the terms of	 the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the	License, or (at your
 *  option) any later version.
 */
#include <linux/module.h>
#include <linux/libfdt.h>
#include <linux/of_fdt.h>

#include <asm/prom.h>
#include <asm/wbflush.h>
#include <asm/bootinfo.h>

#include <loongson.h>

#ifdef CONFIG_VT
#include <linux/console.h>
#include <linux/screen_info.h>
#endif

static void wbflush_loongson(void)
{
	asm(".set\tpush\n\t"
	    ".set\tnoreorder\n\t"
	    ".set mips3\n\t"
	    "sync\n\t"
	    "nop\n\t"
	    ".set\tpop\n\t"
	    ".set mips0\n\t");
}

void (*__wbflush)(void) = wbflush_loongson;
EXPORT_SYMBOL(__wbflush);

void __init plat_mem_setup(void)
{
#ifdef CONFIG_VT
#if defined(CONFIG_VGA_CONSOLE)
	conswitchp = &vga_con;

	screen_info = (struct screen_info) {
		.orig_x			= 0,
		.orig_y			= 25,
		.orig_video_cols	= 80,
		.orig_video_lines	= 25,
		.orig_video_isVGA	= VIDEO_TYPE_VGAC,
		.orig_video_points	= 16,
	};
#elif defined(CONFIG_DUMMY_CONSOLE)
	conswitchp = &dummy_con;
#endif
#endif
	if (loongson_fdt_blob)
		__dt_setup_arch(loongson_fdt_blob);
}

#define NR_CELLS 6

void __init device_tree_init(void)
{
	int len, gpu_node;
	u32 mem_regs[NR_CELLS];
	const u32 *gpu_mem_regs;

	if (!initial_boot_params)
		return;

	gpu_node = fdt_node_offset_by_compatible(initial_boot_params,
						 -1, "loongson,galcore");
	if (gpu_node >= 0) {
		gpu_mem_regs = fdt_getprop(initial_boot_params,
					   gpu_node, "reg", &len);
		memcpy(mem_regs, gpu_mem_regs, sizeof(u32) * NR_CELLS);

		/* mem_regs[0,1,2]: MMIO, mem_regs[3,4,5]: VRAM */
		if (loongson_sysconf.vram_type == VRAM_TYPE_UMA) {
			mem_regs[3] = cpu_to_fdt32(loongson_sysconf.uma_vram_addr >> 32);
			mem_regs[4] = cpu_to_fdt32(loongson_sysconf.uma_vram_addr & 0xffffffff);
			mem_regs[5] = cpu_to_fdt32(loongson_sysconf.uma_vram_size);
		}
		else {
			mem_regs[3] = cpu_to_fdt32(0xe0004000000 >> 32);
			mem_regs[4] = cpu_to_fdt32(0xe0004000000 & 0xffffffff);
			mem_regs[5] = cpu_to_fdt32(0x00008000000);
		}
		fdt_setprop(initial_boot_params, gpu_node, "reg", mem_regs, len);
	}

	if (early_init_dt_verify(initial_boot_params))
		unflatten_and_copy_device_tree();
}
