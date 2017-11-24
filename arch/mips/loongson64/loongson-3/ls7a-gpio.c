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
#define LS7A_GPIO_NR	64
#define LS7A_GPIO_MAX	(LS7A_GPIO_BASE + LS7A_GPIO_NR)

#define LS7A_DC_CNT_REG_BASE	(LS7A_PCIE_BAR_BASE(0x0, 0x6, 0x1) & 0xfffffff0)
#define LS7A_DC_GPIO_IO_REG	(void *)TO_UNCAC(LS7A_DC_CNT_REG_BASE + 0x1650)
#define LS7A_DC_GPIO_EN_REG	(void *)TO_UNCAC(LS7A_DC_CNT_REG_BASE + 0x1660)

static inline int ls7a_gpio_get_value(struct gpio_chip *chip, unsigned gpio)
{
	return readl(LS7A_DC_GPIO_IO_REG) & (1 << gpio);
}

static inline void ls7a_gpio_set_value(struct gpio_chip *chip, unsigned gpio, int value)
{
	unsigned long tmp;

	tmp = readl(LS7A_DC_GPIO_IO_REG) & ~(1 << gpio);
	if (value)
		tmp |= 1ULL << gpio;
	writel(tmp, LS7A_DC_GPIO_IO_REG);
}

static inline int ls7a_gpio_direction_input(struct gpio_chip *chip, unsigned gpio)
{
	if (gpio >= LS7A_GPIO_MAX)
		return -EINVAL;

	writel(readl(LS7A_DC_GPIO_EN_REG) | (1ULL << gpio), LS7A_DC_GPIO_EN_REG);

	return 0;
}

static inline int ls7a_gpio_direction_output(struct gpio_chip *chip, unsigned gpio, int value)
{
	if (gpio >= LS7A_GPIO_MAX)
		return -EINVAL;

	ls7a_gpio_set_value(chip, gpio, value);
	writel(readl(LS7A_DC_GPIO_EN_REG) & ~(1ULL << gpio), LS7A_DC_GPIO_EN_REG);

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

static int __init ls7a_i2c_gpio_setup(void)
{
	struct device_node *np;

	for_each_matching_node(np, ls7a_gpio_ids) {
		ls7a_gpio_chip.of_node = np;
		gpiochip_add(&ls7a_gpio_chip);
	}

	return 0;
}
arch_initcall(ls7a_i2c_gpio_setup);
