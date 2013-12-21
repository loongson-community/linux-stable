/*
 *  Loongson2F/3A GPIO Support
 *
 *  Copyright (c) 2008 Richard Liu,  STMicroelectronics	 <richard.liu@st.com>
 *  Copyright (c) 2008-2010 Arnaud Patard <apatard@mandriva.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/err.h>
#include <asm/types.h>
#include <loongson.h>
#include <linux/gpio.h>

#define STLS2F_N_GPIO		4
#define STLS3A_N_GPIO		16

#ifdef CONFIG_CPU_LOONGSON3
#define LOONGSON_GPIO_NR	STLS3A_N_GPIO
#else
#define LOONGSON_GPIO_NR	STLS2F_N_GPIO
#endif

#define LOONGSON_GPIO_IN_OFFSET	16

static DEFINE_SPINLOCK(gpio_lock);

int gpio_get_value(unsigned gpio)
{
	u32 val;
	u32 mask;

	if (gpio >= LOONGSON_GPIO_NR)
		return __gpio_get_value(gpio);

	mask = 1 << (gpio + LOONGSON_GPIO_IN_OFFSET);
	spin_lock(&gpio_lock);
	val = LOONGSON_GPIODATA;
	spin_unlock(&gpio_lock);

	return ((val & mask) != 0);
}
EXPORT_SYMBOL(gpio_get_value);

void gpio_set_value(unsigned gpio, int state)
{
	u32 val;
	u32 mask;

	if (gpio >= LOONGSON_GPIO_NR) {
		__gpio_set_value(gpio, state);
		return ;
	}

	mask = 1 << gpio;

	spin_lock(&gpio_lock);
	val = LOONGSON_GPIODATA;
	if (state)
		val |= mask;
	else
		val &= (~mask);
	LOONGSON_GPIODATA = val;
	spin_unlock(&gpio_lock);
}
EXPORT_SYMBOL(gpio_set_value);

int gpio_cansleep(unsigned gpio)
{
	if (gpio < LOONGSON_GPIO_NR)
		return 0;
	else
		return __gpio_cansleep(gpio);
}
EXPORT_SYMBOL(gpio_cansleep);

static int loongson_gpio_direction_input(struct gpio_chip *chip, unsigned gpio)
{
	u32 temp;
	u32 mask;

	if (gpio >= LOONGSON_GPIO_NR)
		return -EINVAL;

	spin_lock(&gpio_lock);
	mask = 1 << gpio;
	temp = LOONGSON_GPIOIE;
	temp |= mask;
	LOONGSON_GPIOIE = temp;
	spin_unlock(&gpio_lock);

	return 0;
}

static int loongson_gpio_direction_output(struct gpio_chip *chip,
		unsigned gpio, int level)
{
	u32 temp;
	u32 mask;

	if (gpio >= LOONGSON_GPIO_NR)
		return -EINVAL;

	gpio_set_value(gpio, level);
	spin_lock(&gpio_lock);
	mask = 1 << gpio;
	temp = LOONGSON_GPIOIE;
	temp &= (~mask);
	LOONGSON_GPIOIE = temp;
	spin_unlock(&gpio_lock);

	return 0;
}

static int loongson_gpio_get_value(struct gpio_chip *chip, unsigned gpio)
{
	return gpio_get_value(gpio);
}

static void loongson_gpio_set_value(struct gpio_chip *chip,
		unsigned gpio, int value)
{
	gpio_set_value(gpio, value);
}

static struct gpio_chip loongson_chip = {
	.label                  = "Loongson-gpio-chip",
	.direction_input        = loongson_gpio_direction_input,
	.get                    = loongson_gpio_get_value,
	.direction_output       = loongson_gpio_direction_output,
	.set                    = loongson_gpio_set_value,
	.base			= 0,
	.ngpio                  = LOONGSON_GPIO_NR,
};

static int __init loongson_gpio_setup(void)
{
	return gpiochip_add(&loongson_chip);
}
arch_initcall(loongson_gpio_setup);
