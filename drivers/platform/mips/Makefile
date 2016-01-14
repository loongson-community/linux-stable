obj-y += acpi_init.o
obj-y += wpce_fan.o sbx00_fan.o
obj-$(CONFIG_LEMOTE3A_LAPTOP) += lemote3a-laptop.o
obj-m += ec_rom.o at24c04.o

ifdef CONFIG_MTD
obj-m += pmon_flash.o
endif
