#
# Makefile for the drm device driver.  This driver provides support for the
# Direct Rendering Infrastructure (DRI)
#

ccflags-y := -Iinclude/drm
loongson-y := loongson_drv.o \
	loongson_irq.o \
	loongson_crtc.o \
	loongson_encoder.o \
	loongson_connector.o \
	loongson_cursor.o \
	loongson_vbios.o

obj-$(CONFIG_DRM_LOONGSON)	+= loongson.o
