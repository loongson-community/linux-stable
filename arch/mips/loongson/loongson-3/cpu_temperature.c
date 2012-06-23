#include <linux/types.h>

#ifdef CONFIG_64BIT
#define TEMPRATURE_SENSOR_REG 0xffffffffbfe0019c
#else
#define TEMPRATURE_SENSOR_REG 0xbfe0019c
#endif

/*
 * Loongson-3 series cpu has two sensors inside,
 * each of them from 0 to 255,
 * if more than 127, that is dangerous.
 * here only provide sensor1 data, because it always hot than sensor0
 */
int loongson3_cpu_temp(void)
{
	u32 reg;

	reg = *(volatile u32 *)TEMPRATURE_SENSOR_REG;
	reg = (reg >> 8) & 0xff;

	return (int)reg * 1000;
}
