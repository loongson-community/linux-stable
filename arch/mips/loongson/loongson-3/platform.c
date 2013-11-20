/*
 * Copyright (C) 2009 Lemote Inc.
 * Author: Wu Zhangjin, wuzhangjin@gmail.com
 *         Xiang Yu, xiangy@lemote.com
 *         Chen Huacai, chenhc@lemote.com
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <loongson_hwmon.h>
#include <asm/bootinfo.h>
#include <linux/i2c.h>

#define GPIO5	5
#define GPIO7	7

#define A1205_LCD_CNTL	GPIO5
#define A1205_BACKLIGHIT_CNTL	GPIO7

/* Loongson laptops use wpce775l as Embeded Controller */
static struct platform_device wpce775l_chip = {
	.name = "wpce775l",
	.id = -1,
};

/* wpce-fan1 in A1004 */
static struct platform_device wpce_fan1 = {
	.name = "wpce-fan1",
	.id = 0,
};

/* tmp75 sensor in A1214 */
static struct platform_device tmp75_device = {
	.name = "tmp75",
	.id = 0,
};

/* sbx00-fan1 in A1101, A1205 */
static struct platform_device sbx00_fan1 = {
	.name = "sbx00-fan1",
	.id = 0,
};

/* sbx00-fan2 in A1101 */
static struct platform_device sbx00_fan2 = {
	.name = "sbx00-fan2",
	.id = 0,
};

/* sbx00-fan3 in A1214 */
static struct platform_device sbx00_fan3 = {
	.name = "sbx00-fan3",
	.id = 0,
};

/*
 * Kernel helper policy
 *
 * Fan is controlled by EC in A1004 pruducts, but EC can not get the current
 * cpu temperature which used for adjusting the current fan speed.
 *
 * So, kernel read the CPU temperature and notify it to EC per second,
 * that's all!
 */
struct loongson_fan_policy kernel_helper_policy = {
	.type = KERNEL_HELPER_POLICY,
	.adjust_period = 1,
	.depend_temp = loongson3_cpu_temp,
	.depend_data = 1, /* CPU0 */
};

/*
 * Constant speed policy
 *
 */
struct loongson_fan_policy constant_speed_50p_policy = {
	.type = CONSTANT_SPEED_POLICY,
	.percent = 50,
};

struct loongson_fan_policy constant_speed_70p_policy = {
	.type = CONSTANT_SPEED_POLICY,
	.percent = 70,
};

/*
 * Policy at step mode
 *
 * up_step array    |   down_step array
 *                  |
 * [min, 50),  50%  |   (min, 45),  50%
 * [50,  60),  60%  |   [45,  55),  60%
 * [60,  70),  70%  |   [55,  65),  70%
 * [70,  80),  80%  |   [65,  75),  80%
 * [80,  max), 100% |   [75,  max), 100%
 *
 */
struct loongson_fan_policy step_speed_policy = {
	.type = STEP_SPEED_POLICY,
	.adjust_period = 1,
	.depend_temp = loongson3_cpu_temp,
	.depend_data = 1, /* CPU0 */
	.up_step_num = 5,
	.down_step_num = 5,
	.up_step = {
			{MIN_TEMP, 50,    50},
			{   50,    60,    60},
			{   60,    70,    70},
			{   70,    80,    80},
			{   80, MAX_TEMP, 100},
		   },
	.down_step = {
			{MIN_TEMP, 45,    50},
			{   45,    55,    60},
			{   55,    65,    70},
			{   65,    75,    80},
			{   75, MAX_TEMP, 100},
		     },
};

static int __init loongson3a_platform_init(void)
{
	/* temprature info */
	switch (mips_machtype) {
	case MACH_LEMOTE_A1004:
	case MACH_LEMOTE_A1217:
		/* thermal sensor register and interface init */
		loongson_temp_info.get_cpu0_temp = loongson3_cpu_temp;
		loongson_temp_info.get_mb_temp = emc1412_internal_temp;
		loongson_temp_info.get_nb_temp = emc1412_external_temp;
		loongson_temp_info.get_sb_temp = NULL; // not implement now

		/* fan sensor register and choose fan control policy */
		platform_device_register(&wpce_fan1);
		loongson_fan1_ops.fan_policy = &kernel_helper_policy;

		/* for lemote-laptop */
		platform_device_register(&wpce775l_chip);
		break;
	case MACH_LEMOTE_A1101:
		/* thermal sensor register and interface init */
		loongson_temp_info.get_cpu0_temp = loongson3_cpu_temp;
		loongson_temp_info.get_mb_temp = emc1412_internal_temp;
		loongson_temp_info.get_nb_temp = emc1412_external_temp;
		loongson_temp_info.get_sb_temp = NULL; // not implement now

		/* fan sensor register and choose fan control policy */
		platform_device_register(&sbx00_fan1);
		platform_device_register(&sbx00_fan2);
		loongson_fan1_ops.fan_policy = &constant_speed_50p_policy;
		loongson_fan2_ops.fan_policy = &constant_speed_50p_policy;
		break;
	case MACH_LEMOTE_A1201:
		/* thermal sensor register and interface init */
		loongson_temp_info.get_cpu0_temp = loongson3_cpu_temp;
		loongson_temp_info.get_mb_temp = emc1412_internal_temp;
		loongson_temp_info.get_nb_temp = emc1412_external_temp;
		loongson_temp_info.get_sb_temp = NULL; // not implement now

		/* A1201 without fan */

		/* for lemote-laptop */
		platform_device_register(&wpce775l_chip);
		break;
	case MACH_LEMOTE_A1205:
		gpio_request(A1205_LCD_CNTL,  "a1205_lcd_cntl");
		gpio_request(A1205_BACKLIGHIT_CNTL, "a1205_bl_cntl");
		/* thermal sensor register and interface init */
		loongson_temp_info.get_cpu0_temp = loongson3_cpu_temp;
		loongson_temp_info.get_mb_temp = emc1412_internal_temp;
		loongson_temp_info.get_nb_temp = emc1412_external_temp;
		loongson_temp_info.get_sb_temp = NULL; // not implement now

		/* fan sensor register and choose fan control policy */
		platform_device_register(&sbx00_fan1);
		loongson_fan1_ops.fan_policy = &step_speed_policy;
		break;
	case MACH_LEMOTE_A1214:
		platform_device_register(&tmp75_device);

		/* thermal sensor register and interface init */
		loongson_temp_info.get_cpu0_temp = loongson3_cpu_temp;
		loongson_temp_info.get_mb_temp = emc1412_external_temp;
		loongson_temp_info.get_nb_temp = emc1412_external_temp;
		loongson_temp_info.get_sb_temp = emc1412_external_temp;
		loongson_temp_info.get_psy_temp = NULL;

		/* fan sensor register and choose fan control policy */
		platform_device_register(&sbx00_fan1);
		platform_device_register(&sbx00_fan2);
		platform_device_register(&sbx00_fan3);
		loongson_fan1_ops.fan_policy = &constant_speed_70p_policy;
		loongson_fan2_ops.fan_policy = &constant_speed_70p_policy;
		loongson_fan3_ops.fan_policy = &constant_speed_70p_policy;

		break;
	case MACH_LEMOTE_A1215:
		platform_device_register(&tmp75_device);

		/* thermal sensor register and interface init */
		loongson_temp_info.get_cpu0_temp = loongson3_cpu_temp;
		loongson_temp_info.get_cpu1_temp = loongson3_cpu_temp;
		loongson_temp_info.get_mb_temp = emc1412_external_temp;
		loongson_temp_info.get_nb_temp = emc1412_external_temp;
		loongson_temp_info.get_sb_temp = emc1412_external_temp;
		loongson_temp_info.get_psy_temp = emc1412_external_temp;

		/* fan sensor register and choose fan control policy */
		platform_device_register(&sbx00_fan1);
		platform_device_register(&sbx00_fan2);
		loongson_fan1_ops.fan_policy = &constant_speed_70p_policy;
		loongson_fan2_ops.fan_policy = &constant_speed_70p_policy;

		break;
	case MACH_LEMOTE_A1219:
		loongson_temp_info.get_cpu0_temp = loongson3_cpu_temp;
		loongson_temp_info.get_nb_temp = emc1412_external_temp;
		platform_device_register(&sbx00_fan1);
		platform_device_register(&sbx00_fan2);
		loongson_fan1_ops.fan_policy = &constant_speed_70p_policy;
		loongson_fan2_ops.fan_policy = &constant_speed_70p_policy;

		break;
	default:
		break;
	}

	return 0;
}

arch_initcall(loongson3a_platform_init);
