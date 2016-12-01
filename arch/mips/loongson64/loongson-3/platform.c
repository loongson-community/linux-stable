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
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <asm/bootinfo.h>
#include <boot_param.h>
#include <loongson_hwmon.h>
#include <workarounds.h>

/*
 * Kernel helper policy
 *
 * Fan is controlled by EC in laptop pruducts, but EC can not get the current
 * cpu temperature which used for adjusting the current fan speed.
 *
 * So, kernel read the CPU temperature and notify it to EC per second,
 * that's all!
 */
struct loongson_fan_policy kernel_helper_policy = {
	.type = KERNEL_HELPER_POLICY,
	.adjust_period = 1,
	.depend_temp = loongson3_cpu_temp,
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

/*
 * Constant speed policy
 *
 */
struct loongson_fan_policy constant_speed_policy = {
	.type = CONSTANT_SPEED_POLICY,
};

#define GPIO_LCD_CNTL		5
#define GPIO_BACKLIGHIT_CNTL	7

static int __init loongson3_platform_init(void)
{
	int i;
	struct platform_device *pdev;

	if (loongson_sysconf.ecname[0] != '\0')
		platform_device_register_simple(loongson_sysconf.ecname, -1, NULL, 0);

	for (i = 0; i < loongson_sysconf.nr_sensors; i++) {
		if (loongson_sysconf.sensors[i].type > SENSOR_FAN)
			continue;

		pdev = kzalloc(sizeof(struct platform_device), GFP_KERNEL);
		pdev->name = loongson_sysconf.sensors[i].name;
		pdev->id = loongson_sysconf.sensors[i].id;
		pdev->dev.platform_data = &loongson_sysconf.sensors[i];
		platform_device_register(pdev);
	}

	if (loongson_sysconf.workarounds & WORKAROUND_LVDS_GPIO) {
		gpio_request(GPIO_LCD_CNTL,  "gpio_lcd_cntl");
		gpio_request(GPIO_BACKLIGHIT_CNTL, "gpio_bl_cntl");
	}

	return 0;
}

arch_initcall(loongson3_platform_init);
