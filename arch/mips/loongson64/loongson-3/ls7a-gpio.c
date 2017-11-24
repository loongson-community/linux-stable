#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/of.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <asm/types.h>

#include <loongson.h>
#include <loongson-pch.h>

#define LS7A_GPIO_BASE  16
#define LS7A_GPIO_DC	57
#define LS7A_GPIO_NR	64

#define LS7A_GPIO_BASE_REG (LS7A_MISC_REG_BASE + 0x00060000)

#define LS7A_GPIO_EN_REG (void *)TO_UNCAC(LS7A_GPIO_BASE_REG + 0x800)
#define LS7A_GPIO_IN_REG (void *)TO_UNCAC(LS7A_GPIO_BASE_REG + 0xa00)
#define LS7A_GPIO_OUT_REG (void *)TO_UNCAC(LS7A_GPIO_BASE_REG + 0x900)

#define PCICFG_BASE		0x1a000000
#define LS7A_DC_BAR0_BASE	\
	(readl((void *)TO_UNCAC(PCICFG_BASE | (6 << 11) | (1 << 8) | 0x10)) & 0xfffffff0)

#define LS7A_DC_GPIO_IO_REG	(void *)TO_UNCAC(LS7A_DC_BAR0_BASE + 0x1650)
#define LS7A_DC_GPIO_EN_REG	(void *)TO_UNCAC(LS7A_DC_BAR0_BASE + 0x1660)

static inline int ls7a_gpio_get_value(struct gpio_chip *chip, unsigned gpio)
{
	if (gpio < LS7A_GPIO_DC) {
		if (readb(LS7A_GPIO_EN_REG + gpio))
			return readb(LS7A_GPIO_IN_REG + gpio) & BIT(0);
		else
			return readq(LS7A_GPIO_OUT_REG + gpio) & BIT(0);
	} else {
		gpio -= LS7A_GPIO_DC;
		return readl(LS7A_DC_GPIO_IO_REG) & (1 << gpio);
	}
}

static inline void ls7a_gpio_set_value(struct gpio_chip *chip, unsigned gpio, int value)
{
	unsigned long tmp;

	if (gpio < LS7A_GPIO_DC) {
		tmp = readb(LS7A_GPIO_OUT_REG + gpio) & ~BIT(0);
		if (value)
			tmp |= BIT(0);
		writeb(tmp, LS7A_GPIO_OUT_REG + gpio);
	} else {
		gpio -= LS7A_GPIO_DC;
		tmp = readl(LS7A_DC_GPIO_IO_REG) & ~(1 << gpio);
		if (value)
			tmp |= 1ULL << gpio;
		writel(tmp, LS7A_DC_GPIO_IO_REG);
	}
}

static inline int ls7a_gpio_direction_input(struct gpio_chip *chip, unsigned gpio)
{
	if (gpio >= LS7A_GPIO_NR)
		return -EINVAL;

	if (gpio < LS7A_GPIO_DC)
		writeb(readb(LS7A_GPIO_EN_REG + gpio) | BIT(0), LS7A_GPIO_EN_REG + gpio);
	else {
		gpio -= LS7A_GPIO_DC;
		writel(readl(LS7A_DC_GPIO_EN_REG) | (1ULL << gpio), LS7A_DC_GPIO_EN_REG);
	}

	return 0;
}

static inline int ls7a_gpio_direction_output(struct gpio_chip *chip, unsigned gpio, int value)
{
	if (gpio >= LS7A_GPIO_NR)
		return -EINVAL;

	ls7a_gpio_set_value(chip, gpio, value);
	if (gpio < LS7A_GPIO_DC)
		writeb(readq(LS7A_GPIO_EN_REG + gpio) & ~BIT(0), LS7A_GPIO_EN_REG + gpio);
	else {
		gpio -= LS7A_GPIO_DC;
		writel(readl(LS7A_DC_GPIO_EN_REG) & ~(1ULL << gpio), LS7A_DC_GPIO_EN_REG);
	}

	return 0;
}

static struct gpio_chip ls7a_gpio_chip = {
	.label			= "ls7a-gpio-chip",
	.direction_input	= ls7a_gpio_direction_input,
	.get			= ls7a_gpio_get_value,
	.direction_output	= ls7a_gpio_direction_output,
	.set			= ls7a_gpio_set_value,
	.base			= LS7A_GPIO_BASE,
	.ngpio			= LS7A_GPIO_NR,
};

static struct of_device_id ls7a_gpio_ids[] __initdata = {
	{ .compatible = "loongson,ls7a-gpio", },
	{},
};

static int __init ls7a_gpio_setup(void)
{
	struct device_node *np;

	for_each_matching_node(np, ls7a_gpio_ids) {
		ls7a_gpio_chip.of_node = np;
		gpiochip_add(&ls7a_gpio_chip);
	}

	return 0;
}
arch_initcall(ls7a_gpio_setup);
