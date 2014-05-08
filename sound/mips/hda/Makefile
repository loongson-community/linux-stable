# common and codec driver
snd-hda-codec-loongson-y := hda_codec.o \
	hda_jack.o hda_auto_parser.o patch_realtek.o
snd-hda-codec-loongson-$(CONFIG_PROC_FS) += hda_proc.o

obj-$(CONFIG_SND_HDA_LS2H) := snd-hda-codec-loongson.o

snd-hda-loongson-objs := hda_loongson.o

# this must be the last entry after codec drivers;
obj-$(CONFIG_SND_HDA_LS2H) += snd-hda-loongson.o
