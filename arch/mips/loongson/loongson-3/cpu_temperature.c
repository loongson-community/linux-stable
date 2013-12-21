#include <linux/types.h>
#include <loongson_hwmon.h>

#ifdef CONFIG_64BIT
#define CPU0_TEMPRATURE_SENSOR_REG 0x900000001fe0019c
#define CPU1_TEMPRATURE_SENSOR_REG 0x900010001fe0019c
#else
#define CPU0_TEMPRATURE_SENSOR_REG 0xbfe0019c
#endif

/*
 * Loongson-3 series cpu has two sensors inside,
 * each of them from 0 to 255,
 * if more than 127, that is dangerous.
 * here only provide sensor1 data, because it always hot than sensor0
 */
int loongson3_cpu_temp(int id)
{
	u32 reg;
	void *addr;

	switch (id) {
	case 1:
		addr = (void *)CPU0_TEMPRATURE_SENSOR_REG;
		break;
	case 6:
		addr = (void *)CPU1_TEMPRATURE_SENSOR_REG;
		break;
	default:
		return NOT_VALID_TEMP;
	}

	reg = *(volatile u32 *)addr;
	reg = (reg >> 8) & 0xff;

	return (int)reg * 1000;
}
