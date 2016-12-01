#include <linux/delay.h>
#include <linux/gpio.h>
#include <asm/bootinfo.h>
#include <ec_wpce775l.h>
#include <workarounds.h>

#define GPIO_LCD_CNTL		5
#define GPIO_BACKLIGHIT_CNTL	7

void gpio_lvds_off(void)
{
	gpio_direction_output(GPIO_BACKLIGHIT_CNTL, 0);
	gpio_direction_output(GPIO_LCD_CNTL, 0);
}

static void gpio_lvds_on(void)
{
	gpio_direction_output(GPIO_LCD_CNTL, 1);
	msleep(250);
	gpio_direction_output(GPIO_BACKLIGHIT_CNTL, 1);
}

void turn_off_lvds(void)
{
	if (loongson_sysconf.workarounds & WORKAROUND_LVDS_EC)
		ec_write(INDEX_BACKLIGHT_STSCTRL, BACKLIGHT_OFF);
	if (loongson_sysconf.workarounds & WORKAROUND_LVDS_GPIO)
		gpio_lvds_off();
}

void turn_on_lvds(void)
{
	if (loongson_sysconf.workarounds & WORKAROUND_LVDS_EC)
		ec_write(INDEX_BACKLIGHT_STSCTRL, BACKLIGHT_ON);
	if (loongson_sysconf.workarounds & WORKAROUND_LVDS_GPIO)
		gpio_lvds_on();
}
