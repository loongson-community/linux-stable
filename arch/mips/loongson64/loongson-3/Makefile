#
# Makefile for Loongson-3 family machines
#
obj-y			+= irq.o cop2-ex.o platform.o acpi_init.o dma.o clock.o ec_wpce775l.o workarounds.o \
			   ls2h-irq.o ls2h-platform.o ls7a-irq.o ls7a-platform.o rs780-irq.o rs780-platform.o

obj-$(CONFIG_SMP)	+= smp.o

obj-$(CONFIG_NUMA)	+= numa.o

obj-$(CONFIG_SUSPEND)	+= pm.o sleep.o

obj-$(CONFIG_LOONGSON_HPET) += hpet.o

obj-$(CONFIG_LOONGSON3_CPUAUTOPLUG) += loongson3_cpuautoplug.o

obj-$(CONFIG_I2C_GPIO)	+= ls7a-gpio.o
