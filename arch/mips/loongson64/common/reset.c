/*
 * This program is free software; you can redistribute	it and/or modify it
 * under  the terms of	the GNU General	 Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * Copyright (C) 2007 Lemote, Inc. & Institute of Computing Technology
 * Author: Fuxin Zhang, zhangfx@lemote.com
 * Copyright (C) 2009 Lemote, Inc.
 * Author: Zhangjin Wu, wuzhangjin@gmail.com
 */
#include <linux/cpu.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/kexec.h>
#include <linux/pm.h>
#include <linux/slab.h>

#include <asm/bootinfo.h>
#include <asm/idle.h>
#include <asm/reboot.h>

#include <loongson.h>
#include <boot_param.h>

static inline void loongson_reboot(void)
{
#ifndef CONFIG_CPU_JUMP_WORKAROUNDS
	((void (*)(void))ioremap_nocache(LOONGSON_BOOT_BASE, 4)) ();
#else
	void (*func)(void);

	func = (void *)ioremap_nocache(LOONGSON_BOOT_BASE, 4);

	__asm__ __volatile__(
	"	.set	noat						\n"
	"	jr	%[func]						\n"
	"	.set	at						\n"
	: /* No outputs */
	: [func] "r" (func));
#endif
}

static void loongson_restart(char *command)
{
#ifndef CONFIG_LEFI_FIRMWARE_INTERFACE
	/* do preparation for reboot */
	mach_prepare_reboot();

	/* reboot via jumping to boot base address */
	loongson_reboot();
#else
	void (*fw_restart)(void) = (void *)loongson_sysconf.restart_addr;

	fw_restart();
	while (1) {
		if (cpu_wait)
			cpu_wait();
	}
#endif
}

static void loongson_poweroff(void)
{
#ifndef CONFIG_LEFI_FIRMWARE_INTERFACE
	mach_prepare_shutdown();
	unreachable();
#else
	void (*fw_poweroff)(void) = (void *)loongson_sysconf.poweroff_addr;

	fw_poweroff();
	while (1) {
		if (cpu_wait)
			cpu_wait();
	}
#endif
}

static void loongson_halt(void)
{
	pr_notice("\n\n** You can safely turn off the power now **\n\n");
	while (1) {
		if (cpu_wait)
			cpu_wait();
	}
}

#ifdef CONFIG_KEXEC

/* 0X80000000~0X80200000 is safe */
#define MAX_ARGS	64
#define KEXEC_CTRL_CODE	0xFFFFFFFF80100000UL
#define KEXEC_ARGV_ADDR	0xFFFFFFFF80108000UL
#define KEXEC_ARGV_SIZE	3060
#define KEXEC_ENVP_SIZE	4500

void *kexec_argv;
void *kexec_envp;

static int loongson_kexec_prepare(struct kimage *image)
{
	int i, argc = 0;
	unsigned int *argv;
	char *str, *ptr, *bootloader = "kexec";

	/* argv at offset 0, argv[] at offset KEXEC_ARGV_SIZE/2 */
	argv = (unsigned int *)kexec_argv;
	argv[argc++] = (unsigned int)(KEXEC_ARGV_ADDR + KEXEC_ARGV_SIZE/2);

	for (i = 0; i < image->nr_segments; i++) {
		if (!strncmp(bootloader, (char *)image->segment[i].buf,
				strlen(bootloader))) {
			/*
			 * convert command line string to array
			 * of parameters (as bootloader does).
			 */
			int offt;
			memcpy(kexec_argv + KEXEC_ARGV_SIZE/2, image->segment[i].buf, KEXEC_ARGV_SIZE/2);
			str = (char *)kexec_argv + KEXEC_ARGV_SIZE/2;
			ptr = strchr(str, ' ');

			while (ptr && (argc < MAX_ARGS)) {
				*ptr = '\0';
				if (ptr[1] != ' ') {
					offt = (int)(ptr - str + 1);
					argv[argc] = KEXEC_ARGV_ADDR + KEXEC_ARGV_SIZE/2 + offt;
					argc++;
				}
				ptr = strchr(ptr + 1, ' ');
			}
			break;
		}
	}

	kexec_args[0] = argc;
	kexec_args[1] = fw_arg1;
	kexec_args[2] = fw_arg2;
	image->control_code_page = virt_to_page((void *)KEXEC_CTRL_CODE);

	return 0;
}

#ifdef CONFIG_SMP
static void kexec_smp_down(void *ignored)
{
	int cpu = smp_processor_id();

	local_irq_disable();
	set_cpu_online(cpu, false);
	while (!atomic_read(&kexec_ready_to_reboot))
		cpu_relax();

	asm volatile (
	"	sync					\n"
	"	synci	($0)				\n");

	relocated_kexec_smp_wait(NULL);
}
#endif

static void loongson_kexec_shutdown(void)
{
#ifdef CONFIG_SMP
	int cpu;

	for_each_possible_cpu(cpu)
		if (!cpu_online(cpu))
			cpu_up(cpu); /* Everyone should go to reboot_code_buffer */

	smp_call_function(kexec_smp_down, NULL, 0);
	smp_wmb();
	while (num_online_cpus() > 1) {
		mdelay(1);
		cpu_relax();
	}
#endif
	memcpy((void *)fw_arg1, kexec_argv, KEXEC_ARGV_SIZE);
	memcpy((void *)fw_arg2, kexec_envp, KEXEC_ENVP_SIZE);
}

static void loongson_crash_shutdown(struct pt_regs *regs)
{
	default_machine_crash_shutdown(regs);
	memcpy((void *)fw_arg1, kexec_argv, KEXEC_ARGV_SIZE);
	memcpy((void *)fw_arg2, kexec_envp, KEXEC_ENVP_SIZE);
}

#endif

static int __init mips_reboot_setup(void)
{
	_machine_restart = loongson_restart;
	_machine_halt = loongson_halt;
	pm_power_off = loongson_poweroff;

#ifdef CONFIG_KEXEC
	kexec_argv = kmalloc(KEXEC_ARGV_SIZE, GFP_KERNEL);
	kexec_envp = kmalloc(KEXEC_ENVP_SIZE, GFP_KERNEL);
	fw_arg1 = KEXEC_ARGV_ADDR;
	memcpy(kexec_envp, (void *)fw_arg2, KEXEC_ENVP_SIZE);

	_machine_kexec_prepare = loongson_kexec_prepare;
	_machine_kexec_shutdown = loongson_kexec_shutdown;
	_machine_crash_shutdown = loongson_crash_shutdown;
#endif

	return 0;
}

arch_initcall(mips_reboot_setup);
