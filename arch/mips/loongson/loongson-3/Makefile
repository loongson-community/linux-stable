#
# Makefile for Loongson-3 family machines
#
obj-y			+= irq.o cop2-ex.o ec_wpce775l.o platform.o clock.o workarounds.o

obj-$(CONFIG_SMP)	+= smp.o

obj-$(CONFIG_NUMA)	+= numa.o

obj-$(CONFIG_SUSPEND)	+= pm.o sleep.o

obj-$(CONFIG_RS780_HPET) += hpet.o

obj-$(CONFIG_LOONGSON3_CPUAUTOPLUG) += loongson3_cpuautoplug.o
