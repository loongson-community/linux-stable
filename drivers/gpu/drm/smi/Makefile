obj-$(CONFIG_DRM_SMI) := smifb.o
smifb-objs :=smi_drv.o smi_fbdev.o smi_main.o smi_mode.o smi_plane.o smi_ttm.o smi_prime.o hw750.o hw768.o
smifb-objs += ddk750/ddk750_help.o  ddk750/ddk750_chip.o  ddk750/ddk750_clock.o  ddk750/ddk750_mode.o ddk750/ddk750_power.o ddk750/ddk750_helper.o ddk750/ddk750_display.o ddk750/ddk750_2d.o ddk750/ddk750_edid.o ddk750/ddk750_swi2c.o ddk750/ddk750_hwi2c.o ddk750/ddk750_cursor.o
smifb-objs += ddk768/ddk768_help.o  ddk768/ddk768_chip.o  ddk768/ddk768_clock.o  ddk768/ddk768_mode.o ddk768/ddk768_power.o ddk768/ddk768_helper.o ddk768/ddk768_display.o ddk768/ddk768_2d.o \
ddk768/ddk768_edid.o ddk768/ddk768_swi2c.o ddk768/ddk768_hwi2c.o ddk768/ddk768_cursor.o ddk768/ddk768_video.o ddk768/ddk768_hdmi.o  ddk768/ddk768_timer.o ddk768/ddk768_intr.o

ifeq ($(CONFIG_DRM_SMI_HDMI),y)
EXTRA_CFLAGS += -DUSE_HDMICHIP
smifb-y += ddk750/ddk750_sii9022.o
smifb-y += ddk750/siHdmiTx_902x_TPI.o
endif

ifeq ($(CONFIG_DRM_SMI_AUDIO),y)
EXTRA_CFLAGS += -DAUDIO_EN
smifb-y += smi_snd.o
smifb-y += ddk768/ddk768_iis.o
smifb-y += ddk768/uda1345.o
smifb-y += ddk768/l3.o
endif

ifeq ($(CONFIG_DRM_SMI_PRIME),y)
EXTRA_CFLAGS += -DPRIME
endif

ccflags-y :=-O2 -Iinclude/drm -fno-tree-scev-cprop -D_D_SMI -D_D_SMI_D -D__cdecl
