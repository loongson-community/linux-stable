#
# Makefile for Loongson-3 family machines
#
obj-y			+= irq.o cop2-ex.o ec_wpce775l.o platform.o cpu_temperature.o loongson_hwmon.o

obj-$(CONFIG_I2C_PIIX4) += emc1412.o

obj-$(CONFIG_SMP)	+= smp.o

obj-$(CONFIG_NUMA)	+= numa.o

obj-$(CONFIG_SUSPEND)	+= pm.o sleep.o
