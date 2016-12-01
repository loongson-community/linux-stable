/*
 *  Lemote Loongson-3A family machines' specific suspend support
 *
 *  Copyright (C) 2009 Lemote Inc.
 *  Author: Wu Zhangjin <wuzhangjin@gmail.com>
 *  Author: Chen Huacai <chenhuacai@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/suspend.h>
#include <linux/interrupt.h>
#include <linux/pm.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/delay.h>

#include <asm/mipsregs.h>
#include <asm/bootinfo.h>
#include <asm/tlbflush.h>

#include <loongson.h>
#include <mc146818rtc.h>

extern void irq_router_init(void);
extern void acpi_sleep_prepare(void);
extern void acpi_sleep_complete(void);
extern void acpi_registers_setup(void);

u32 loongson_nr_nodes;
u64 loongson_suspend_addr;
u32 loongson_pcache_ways;
u32 loongson_scache_ways;
u32 loongson_pcache_sets;
u32 loongson_scache_sets;
u32 loongson_pcache_linesz;
u32 loongson_scache_linesz;

struct loongson_registers {
	u32 config4;
	u32 config6;
	u64 pgd;
	u64 kpgd;
	u32 pwctl;
	u64 pwbase;
	u64 pwsize;
	u64 pwfield;
	u32 hwrena;
	u64 userlocal;
};
static struct loongson_registers loongson_regs;

uint64_t cmos_read64(unsigned long addr)
{
	unsigned char bytes[8];
	int i;

	for (i=0; i<8; i++)
		bytes[i] = CMOS_READ(addr + i);

	return *(uint64_t *)bytes;
}

void cmos_write64(uint64_t data, unsigned long addr)
{
	int i;
	unsigned char * bytes = (unsigned char *)&data;

	for (i=0; i<8; i++)
		CMOS_WRITE(bytes[i], addr + i);
}

void mach_suspend(void)
{
	acpi_sleep_prepare();

	if (cpu_has_ftlb) {
		loongson_regs.config4 = read_c0_config4();
		loongson_regs.config6 = read_c0_config6();
	}

	if (cpu_has_ldpte) {
		loongson_regs.pgd = read_c0_pgd();
		loongson_regs.kpgd = read_c0_kpgd();
		loongson_regs.pwctl = read_c0_pwctl();
		loongson_regs.pwbase = read_c0_pwbase();
		loongson_regs.pwsize = read_c0_pwsize();
		loongson_regs.pwfield = read_c0_pwfield();
	}

	if (cpu_has_userlocal) {
		loongson_regs.hwrena = read_c0_hwrena();
		loongson_regs.userlocal = read_c0_userlocal();
	}

	loongson_nr_nodes = loongson_sysconf.nr_nodes;
	loongson_suspend_addr = loongson_sysconf.suspend_addr;
	loongson_pcache_ways = cpu_data[0].dcache.ways;
	loongson_scache_ways = cpu_data[0].scache.ways;
	loongson_pcache_sets = cpu_data[0].dcache.sets;
	loongson_scache_sets = cpu_data[0].scache.sets*4;
	loongson_pcache_linesz = cpu_data[0].dcache.linesz;
	loongson_scache_linesz = cpu_data[0].scache.linesz;
}

void mach_resume(void)
{
	local_flush_tlb_all();
	cmos_write64(0x0, 0x40);  /* clear pc in cmos */
	cmos_write64(0x0, 0x48);  /* clear sp in cmos */

	if (cpu_has_ftlb) {
		write_c0_config4(loongson_regs.config4);
		write_c0_config6(loongson_regs.config6);
	}

	if (cpu_has_ldpte) {
		write_c0_pgd(loongson_regs.pgd);
		write_c0_kpgd(loongson_regs.kpgd);
		write_c0_pwctl(loongson_regs.pwctl);
		write_c0_pwbase(loongson_regs.pwbase);
		write_c0_pwsize(loongson_regs.pwsize);
		write_c0_pwfield(loongson_regs.pwfield);
	}

	if (cpu_has_userlocal) {
		write_c0_hwrena(loongson_regs.hwrena);
		write_c0_userlocal(loongson_regs.userlocal);
	}

	irq_router_init();
	acpi_registers_setup();
	acpi_sleep_complete();
}
