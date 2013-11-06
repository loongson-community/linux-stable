#include <asm/bootinfo.h>
#include <ec_wpce775l.h>

extern void A1205_lvds_on(void);
extern void A1205_lvds_off(void);

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
