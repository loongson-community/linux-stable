#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/delay.h>

#define GPIO5	(1<<5)
#define GPIO7	(1<<7)

#define A1205_LCD_CNTL	GPIO5
#define A1205_BACKLIGHIT_CNTL	GPIO7

#ifdef CONFIG_64BIT
#define LOONGSON3A_GPIO_OUTPUT_DATA	0xffffffffbfe0011c
#define LOONGSON3A_GPIO_OUTPUT_ENABLE	0xffffffffbfe00120
#else
#define LOONGSON3A_GPIO_OUTPUT_DATA	0xbfe0011c
#define LOONGSON3A_GPIO_OUTPUT_ENABLE	0xbfe00120
#endif

static void loongson3a_gpio_out_low(u32 gpio)
{
	u32 reg;

	/* set output low level*/
	reg = *(u32 *)LOONGSON3A_GPIO_OUTPUT_DATA;
	reg &= ~(gpio);
	*(u32 *)LOONGSON3A_GPIO_OUTPUT_DATA = reg;

	/* enable output*/
	reg = *(u32 *)LOONGSON3A_GPIO_OUTPUT_ENABLE;
	reg &= ~(gpio);
	*(u32 *)LOONGSON3A_GPIO_OUTPUT_ENABLE = reg;
}

static void loongson3a_gpio_out_high(u32 gpio)
{
	u32 reg;

	/* set output high level*/
	reg = *(u32 *)LOONGSON3A_GPIO_OUTPUT_DATA;
	reg |= (gpio);
	*(u32 *)LOONGSON3A_GPIO_OUTPUT_DATA = reg;

	/* enable output*/
	reg = *(u32 *)LOONGSON3A_GPIO_OUTPUT_ENABLE;
	reg &= ~(gpio);
	*(u32 *)LOONGSON3A_GPIO_OUTPUT_ENABLE = reg;
}

void A1205_lvds_off(void)
{
	loongson3a_gpio_out_low(A1205_BACKLIGHIT_CNTL);
	loongson3a_gpio_out_low(A1205_LCD_CNTL);
}

void A1205_lvds_on(void)
{
	loongson3a_gpio_out_high(A1205_LCD_CNTL);
	msleep(250);
	loongson3a_gpio_out_high(A1205_BACKLIGHIT_CNTL);
}
