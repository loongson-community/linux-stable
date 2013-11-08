/*
 * Based on Ocelot Linux port, which is
 * Copyright 2001 MontaVista Software Inc.
 * Author: jsun@mvista.com or jsun@junsun.net
 *
 * Copyright 2003 ICT CAS
 * Author: Michael Guo <guoyi@ict.ac.cn>
 *
 * Copyright (C) 2007 Lemote Inc. & Insititute of Computing Technology
 * Author: Fuxin Zhang, zhangfx@lemote.com
 *
 * Copyright (C) 2009 Lemote Inc.
 * Author: Wu Zhangjin, wuzhangjin@gmail.com
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */
#include <linux/module.h>
#include <asm/bootinfo.h>
#include <loongson.h>
#include <boot_param.h>

struct boot_params *boot_p;
struct loongson_params *loongson_p;

struct efi_cpuinfo_loongson *ecpu;
struct efi_memory_map_loongson *emap;
struct system_loongson *esys;
struct irq_source_routing_table *eirq_source;

u64 ht_control_base;
u64 pci_mem_start_addr, pci_mem_end_addr;
u64 loongson_pciio_base;
u64 vgabios_addr;
u64 poweroff_addr, restart_addr;
unsigned long long smp_group[4];

enum loongson_cpu_type cputype;
u32 nr_cpus_loongson = NR_CPUS;
u32 nr_nodes_loongson = MAX_NUMNODES;
int cores_per_node;
int cores_per_package;
int cpufreq_workaround = 0;
int cpuhotplug_workaround = 0;

u32 cpu_clock_freq;
EXPORT_SYMBOL(cpu_clock_freq);

#define parse_even_earlier(res, option, p)				\
do {									\
	unsigned int tmp __maybe_unused;				\
									\
	if (strncmp(option, (char *)p, strlen(option)) == 0)		\
		tmp = kstrtou32((char *)p + strlen(option"="), 10, &res); \
} while (0)

void __init prom_init_env(void)
{
	/* pmon passes arguments in 32bit pointers */
	unsigned int processor_id;

#ifndef CONFIG_UEFI_FIRMWARE_INTERFACE
	int *_prom_envp;
	long l;
	extern u32 memsize, highmemsize;

	/* firmware arguments are initialized in head.S */
	_prom_envp = (int *)fw_arg2;

	l = (long)*_prom_envp;
	while (l != 0) {
		parse_even_earlier(cpu_clock_freq, "cpuclock", l);
		parse_even_earlier(memsize, "memsize", l);
		parse_even_earlier(highmemsize, "highmemsize", l);
		_prom_envp++;
		l = (long)*_prom_envp;
	}
	if (memsize == 0)
		memsize = 256;
#else
	/* firmware arguments are initialized in head.S */
	boot_p = (struct boot_params *)fw_arg2;
	loongson_p = &(boot_p->efi.smbios.lp);

	ecpu	= (struct efi_cpuinfo_loongson *)((u64)loongson_p + loongson_p->cpu_offset);
	emap 	= (struct efi_memory_map_loongson *)((u64)loongson_p + loongson_p->memory_offset);
	eirq_source = (struct irq_source_routing_table *)((u64)loongson_p + loongson_p->irq_offset);

	cputype = ecpu->cputype;
	if (cputype == Loongson_3A) {
		cores_per_node = 4;
		cores_per_package = 4;
		smp_group[0] = 0x900000003ff01000;
		smp_group[1] = 0x900010003ff01000;
		smp_group[2] = 0x900020003ff01000;
		smp_group[3] = 0x900030003ff01000;
		ht_control_base = 0x90000EFDFB000000;
		cpufreq_workaround = 1;
	}
	else if (cputype == Loongson_3B) {
		cores_per_node = 4; /* Loongson 3B has two node in one package */
		cores_per_package = 8;
		smp_group[0] = 0x900000003ff01000;
		smp_group[1] = 0x900010003ff05000;
		smp_group[2] = 0x900020003ff09000;
		smp_group[3] = 0x900030003ff0d000;
		ht_control_base = 0x90001EFDFB000000;
		cpuhotplug_workaround = 1;
	}
	else {
		cores_per_node = 1;
		cores_per_package = 1;
	}

	nr_cpus_loongson = ecpu->nr_cpus;
	cpu_clock_freq = ecpu->cpu_clock_freq;
	if (nr_cpus_loongson > NR_CPUS || nr_cpus_loongson == 0)
		nr_cpus_loongson = NR_CPUS;
	nr_nodes_loongson = (nr_cpus_loongson + cores_per_node - 1) / cores_per_node;

	pci_mem_start_addr = eirq_source->pci_mem_start_addr;
	pci_mem_end_addr = eirq_source->pci_mem_end_addr;
	loongson_pciio_base = eirq_source->pci_io_start_addr;

	poweroff_addr = boot_p->reset_system.Shutdown;
	restart_addr = boot_p->reset_system.ResetWarm;
	pr_info("Shutdown Addr: %llx Reset Addr: %llx\n", poweroff_addr, restart_addr);

	vgabios_addr = boot_p->efi.smbios.vga_bios;
#endif
	if (cpu_clock_freq == 0) {
		processor_id = (&current_cpu_data)->processor_id;
		switch (processor_id & PRID_REV_MASK) {
		case PRID_REV_LOONGSON2E:
			cpu_clock_freq = 533080000;
			break;
		case PRID_REV_LOONGSON2F:
			cpu_clock_freq = 797000000;
			break;
		case PRID_REV_LOONGSON3A:
			cpu_clock_freq = 900000000;
			break;
		case PRID_REV_LOONGSON3B_R1:
		case PRID_REV_LOONGSON3B_R2:
			cpu_clock_freq = 1000000000;
			break;
		default:
			cpu_clock_freq = 100000000;
			break;
		}
	}
	pr_info("CpuClock = %u\n", cpu_clock_freq);
}
