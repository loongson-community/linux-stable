EXTRA_CFLAGS += -DLINUX -DDRIVER

ifeq ($(ENUM_WORKAROUND), 1)
EXTRA_CFLAGS += -DENUM_WORKAROUND=1
else
EXTRA_CFLAGS += -DENUM_WORKAROUND=0
endif

ifeq ($(FLAREON),1)
EXTRA_CFLAGS += -DFLAREON
endif

ifeq ($(CONFIG_VIV_DEBUG), 1)
EXTRA_CFLAGS += -DDBG=1 -DDEBUG -D_DEBUG
else
EXTRA_CFLAGS += -DDBG=0
endif

ifeq ($(CONFIG_VIV_NO_DMA_COHERENT), 1)
EXTRA_CFLAGS += -DNO_DMA_COHERENT
endif

ifeq ($(CONFIG_DOVE_GPU), 1)
EXTRA_CFLAGS += -DCONFIG_DOVE_GPU=1
endif

ifeq ($(CONFIG_USE_POWER_MANAGEMENT), 1)
EXTRA_CFLAGS += -DgcdPOWER_MANAGEMENT=1
else
EXTRA_CFLAGS += -DgcdPOWER_MANAGEMENT=0
endif

ifneq ($(CONFIG_USE_PLATFORM_DRIVER), 0)
EXTRA_CFLAGS += -DUSE_PLATFORM_DRIVER=1
else
EXTRA_CFLAGS += -DUSE_PLATFORM_DRIVER=0
endif

ifeq ($(CONFIG_VIV_USE_PROFILER), 1)
EXTRA_CFLAGS += -DVIVANTE_PROFILER=1
else
EXTRA_CFLAGS += -DVIVANTE_PROFILER=0
endif

ifeq ($(ANDROID), 1)
EXTRA_CFLAGS += -DANDROID=1
endif

ifeq ($(CONFIG_GPU_CLOCK), 1)
EXTRA_CFLAGS += -DENABLE_GPU_CLOCK_BY_DRIVER=1
else
EXTRA_CFLAGS += -DENABLE_GPU_CLOCK_BY_DRIVER=0
endif

ifeq ($(USE_NEW_LINUX_SIGNAL), 1)
EXTRA_CFLAGS += -DUSE_NEW_LINUX_SIGNAL=1
else
EXTRA_CFLAGS += -DUSE_NEW_LINUX_SIGNAL=0
endif

ifeq ($(CONFIG_NO_D_A_F_K), 1)
EXTRA_CFLAGS += -DNO_USER_DIRECT_ACCESS_FROM_KERNEL=1
else
EXTRA_CFLAGS += -DNO_USER_DIRECT_ACCESS_FROM_KERNEL=0
endif

ifeq ($(CONFIG_AL_VDMEMY_CACHED), 1)
EXTRA_CFLAGS += -DgcdPAGED_MEMORY_CACHEABLE=1
else
EXTRA_CFLAGS += -DgcdPAGED_MEMORY_CACHEABLE=0
endif

ifeq ($(CONFIG_NPGD_MEM_CACHEABLE), 1)
EXTRA_CFLAGS += -DgcdNONPAGED_MEMORY_CACHEABLE=1
else
EXTRA_CFLAGS += -DgcdNONPAGED_MEMORY_CACHEABLE=0
endif

ifeq ($(CONFIG_NPGD_MEM_BUFFABLE), 1)
EXTRA_CFLAGS += -DgcdNONPAGED_MEMORY_BUFFERABLE=1
else
EXTRA_CFLAGS += -DgcdNONPAGED_MEMORY_BUFFERABLE=0
endif

ifeq ($(CONFIG_CACHE_FUNC_UNIMPLMT), 1)
EXTRA_CFLAGS += -DgcdCACHE_FUNCTION_UNIMPLEMENTED=1
else
EXTRA_CFLAGS += -DgcdCACHE_FUNCTION_UNIMPLEMENTED=0
endif

ifeq ($(CONFIG_SUPPORT_SWAP_RECTANGLE), 1)
EXTRA_CFLAGS += -DgcdSUPPORT_SWAP_RECTANGLE=1
else
EXTRA_CFLAGS += -DgcdSUPPORT_SWAP_RECTANGLE=0
endif

ifeq ($(CONFIG_VIVANTE_ENABLE_VG), 1)
EXTRA_CFLAGS += -DgcdENABLE_VG=1
else
EXTRA_CFLAGS += -DgcdENABLE_VG=0
endif

ifeq ($(CONFIG_SMP), y)
EXTRA_CFLAGS += -DgcdSMP=1
else
EXTRA_CFLAGS += -DgcdSMP=0
endif

ifeq ($(CONFIG_VIVANTE_NO_3D),1)
EXTRA_CFLAGS += -DVIVANTE_NO_3D
endif

ifeq ($(CONFIG_ENABLE_OUTER_CACHE_PATCH), 1)
EXTRA_CFLAGS += -DgcdENABLE_OUTER_CACHE_PATCH=1
else
EXTRA_CFLAGS += -DgcdENABLE_OUTER_CACHE_PATCH=0
endif

ifeq ($(CONFIG_USE_BANK_ALIGNMENT), 1)
    EXTRA_CFLAGS += -DgcdENABLE_BANK_ALIGNMENT=1
    ifneq ($(CONFIG_BANK_BIT_START), 0)
	        ifneq ($(CONFIG_BANK_BIT_END), 0)
	            EXTRA_CFLAGS += -DgcdBANK_BIT_START=$(CONFIG_BANK_BIT_START)
	            EXTRA_CFLAGS += -DgcdBANK_BIT_END=$(CONFIG_BANK_BIT_END)
	        endif
    endif

    ifneq ($(CONFIG_BANK_CHANNEL_BIT), 0)
        EXTRA_CFLAGS += -DgcdBANK_CHANNEL_BIT=$(CONFIG_BANK_CHANNEL_BIT)
    endif
endif

obj-$(CONFIG_VIVANTE)	+= galcore.o
galcore-y	:= loongson-gpu.o gc_hal_kernel_device.o gc_hal_kernel_driver.o gc_hal_kernel_linux.o gc_hal_kernel_math.o \
	gc_hal_kernel_os.o gc_hal_kernel.o gc_hal_kernel_command.o  gc_hal_kernel_db.o gc_hal_kernel_debug.o \
	gc_hal_kernel_event.o gc_hal_kernel_heap.o gc_hal_kernel_mmu.o gc_hal_kernel_video_memory.o \
	gc_hal_kernel_context.o gc_hal_kernel_hardware.o gc_hal_kernel_debugfs.o \
	gc_hal_kernel_vg.o gc_hal_kernel_command_vg.o \
	gc_hal_kernel_interrupt_vg.o gc_hal_kernel_mmu_vg.o gc_hal_kernel_hardware_command_vg.o  \
	gc_hal_kernel_hardware_vg.o 

galcore-objs := $(galcore-y)

