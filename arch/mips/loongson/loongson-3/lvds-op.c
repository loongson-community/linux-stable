#include <linux/delay.h>
#include <linux/gpio.h>
#include <asm/bootinfo.h>
#include <ec_wpce775l.h>

#define GPIO5	5
#define GPIO7	7

#define A1205_LCD_CNTL	GPIO5
#define A1205_BACKLIGHIT_CNTL	GPIO7

void A1205_lvds_off(void)
{
	gpio_direction_output(A1205_BACKLIGHIT_CNTL, 0);
	gpio_direction_output(A1205_LCD_CNTL, 0);
}

static void A1205_lvds_on(void)
{
	gpio_direction_output(A1205_LCD_CNTL, 1);
	msleep(250);
	gpio_direction_output(A1205_BACKLIGHIT_CNTL, 1);
}

void turn_off_lvds(void)
{
	switch (mips_machtype) {
	case MACH_LEMOTE_A1201:
		ec_write(INDEX_BACKLIGHT_STSCTRL, BACKLIGHT_OFF);
		break;
	case MACH_LEMOTE_A1205:
		A1205_lvds_on();
		break;
	default:
		break;
	};
}

void turn_on_lvds(void)
{
	switch (mips_machtype) {
	case MACH_LEMOTE_A1201:
		ec_write(INDEX_BACKLIGHT_STSCTRL, BACKLIGHT_ON);
		break;
	case MACH_LEMOTE_A1205:
		A1205_lvds_on();
		break;
	default:
		break;
	};
}
