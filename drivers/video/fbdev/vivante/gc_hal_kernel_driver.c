/****************************************************************************
*
*    Copyright (C) 2005 - 2013 by Vivante Corp.
*
*    This program is free software; you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation; either version 2 of the license, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not write to the Free Software
*    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*****************************************************************************/


#include <linux/device.h>
#include <linux/slab.h>

#if USE_PLATFORM_DRIVER
#   include <linux/platform_device.h>
#endif

#include "loongson-pch.h"
#include "platform_driver.h"
#include "gc_hal_kernel_linux.h"
#include "gc_hal_driver.h"

#ifdef CONFIG_PXA_DVFM
#   include <mach/dvfm.h>
#   include <mach/pxa3xx_dvfm.h>
#endif

#define ALL_IN_2H

/* Zone used for header/footer. */
#define _GC_OBJ_ZONE    gcvZONE_DRIVER


MODULE_DESCRIPTION("Vivante Graphics Driver");
MODULE_LICENSE("GPL");

static struct class* gpuClass;

static gckGALDEVICE galDevice;

static uint major = 199;
module_param(major, uint, 0644);

static int irqLine = -1;
module_param(irqLine, int, 0644);

static ulong registerMemBase = 0x80000000;
module_param(registerMemBase, ulong, 0644);

static ulong registerMemSize = 2 << 10;
module_param(registerMemSize, ulong, 0644);

static int irqLine2D = -1;
module_param(irqLine2D, int, 0644);

static ulong registerMemBase2D = 0x00000000;
module_param(registerMemBase2D, ulong, 0644);

static ulong registerMemSize2D = 2 << 10;
module_param(registerMemSize2D, ulong, 0644);

static int irqLineVG = -1;
module_param(irqLineVG, int, 0644);

static ulong registerMemBaseVG = 0x00000000;
module_param(registerMemBaseVG, ulong, 0644);

static ulong registerMemSizeVG = 2 << 10;
module_param(registerMemSizeVG, ulong, 0644);

static ulong contiguousSize = 4 << 20;
module_param(contiguousSize, ulong, 0644);

static ulong contiguousBase = 0;
module_param(contiguousBase, ulong, 0644);

static ulong bankSize = 0;
module_param(bankSize, ulong, 0644);

static int fastClear = -1;
module_param(fastClear, int, 0644);

static int compression = -1;
module_param(compression, int, 0644);

static int powerManagement = 1;
module_param(powerManagement, int, 0644);

static int signal = 48;
module_param(signal, int, 0644);

static ulong baseAddress = 0;
module_param(baseAddress, ulong, 0644);

static ulong physSize = 0;
module_param(physSize, ulong, 0644);

static uint logFileSize=0;
module_param(logFileSize,uint, 0644);

static int showArgs = 1;
module_param(showArgs, int, 0644);

#if ENABLE_GPU_CLOCK_BY_DRIVER
    unsigned long coreClock = 156000000;
    module_param(coreClock, ulong, 0644);
#endif

struct dma_coherent_mem {
    void            *virt_base;
    dma_addr_t      device_base;
    unsigned long   pfn_base;
    int             size;
    int             flags;
    unsigned long   *bitmap;
    spinlock_t      spinlock;
};

static struct file_operations driver_fops =
{
    .owner      = THIS_MODULE,
    .open       = loongson_drv_open,
    .release    = loongson_drv_release,
    .unlocked_ioctl = loongson_drv_ioctl,
#ifdef HAVE_COMPAT_IOCTL
    .compat_ioctl = loongson_drv_ioctl,
#endif
    .mmap       = loongson_drv_mmap,
};

int loongson_drv_open(
    struct inode* inode,
    struct file* filp
    )
{
    gceSTATUS status;
    gctBOOL attached = gcvFALSE;
    gcsHAL_PRIVATE_DATA_PTR data = gcvNULL;
    gctINT i;

    gcmkHEADER_ARG("inode=0x%08X filp=0x%08X", inode, filp);

    if (filp == gcvNULL)
    {
        gcmkTRACE_ZONE(
            gcvLEVEL_ERROR, gcvZONE_DRIVER,
            "%s(%d): filp is NULL\n",
            __FUNCTION__, __LINE__
            );

        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    data = kmalloc(sizeof(gcsHAL_PRIVATE_DATA), GFP_KERNEL | __GFP_NOWARN);

    if (data == gcvNULL)
    {
        gcmkTRACE_ZONE(
            gcvLEVEL_ERROR, gcvZONE_DRIVER,
            "%s(%d): private_data is NULL\n",
            __FUNCTION__, __LINE__
            );

        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    data->device             = galDevice;
    data->mappedMemory       = gcvNULL;
    data->contiguousLogical  = gcvNULL;
    gcmkONERROR(gckOS_GetProcessID(&data->pidOpen));

    /* Attached the process. */
    for (i = 0; i < gcdMAX_GPU_COUNT; i++)
    {
        if (galDevice->kernels[i] != gcvNULL)
        {
            gcmkONERROR(gckKERNEL_AttachProcess(galDevice->kernels[i], gcvTRUE));
        }
    }
    attached = gcvTRUE;

    if (!galDevice->contiguousMapped)
    {
        gcmkONERROR(gckOS_MapMemory(
            galDevice->os,
            galDevice->contiguousPhysical,
            galDevice->contiguousSize,
            &data->contiguousLogical
            ));
    }

    filp->private_data = data;

    /* Success. */
    gcmkFOOTER_NO();
    return 0;

OnError:
    if (data != gcvNULL)
    {
        if (data->contiguousLogical != gcvNULL)
        {
            gcmkVERIFY_OK(gckOS_UnmapMemory(
                galDevice->os,
                galDevice->contiguousPhysical,
                galDevice->contiguousSize,
                data->contiguousLogical
                ));
        }

        kfree(data);
    }

    if (attached)
    {
        for (i = 0; i < gcdMAX_GPU_COUNT; i++)
        {
            if (galDevice->kernels[i] != gcvNULL)
            {
                gcmkVERIFY_OK(gckKERNEL_AttachProcess(galDevice->kernels[i], gcvFALSE));
            }
        }
    }

    gcmkFOOTER();
    return -ENOTTY;
}

int loongson_drv_release(
    struct inode* inode,
    struct file* filp
    )
{
    gceSTATUS status;
    gcsHAL_PRIVATE_DATA_PTR data;
    gckGALDEVICE device;
    gctINT i;

    gcmkHEADER_ARG("inode=0x%08X filp=0x%08X", inode, filp);

    if (filp == gcvNULL)
    {
        gcmkTRACE_ZONE(
            gcvLEVEL_ERROR, gcvZONE_DRIVER,
            "%s(%d): filp is NULL\n",
            __FUNCTION__, __LINE__
            );

        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    data = filp->private_data;

    if (data == gcvNULL)
    {
        gcmkTRACE_ZONE(
            gcvLEVEL_ERROR, gcvZONE_DRIVER,
            "%s(%d): private_data is NULL\n",
            __FUNCTION__, __LINE__
            );

        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    device = data->device;

    if (device == gcvNULL)
    {
        gcmkTRACE_ZONE(
            gcvLEVEL_ERROR, gcvZONE_DRIVER,
            "%s(%d): device is NULL\n",
            __FUNCTION__, __LINE__
            );

        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (!device->contiguousMapped)
    {
        if (data->contiguousLogical != gcvNULL)
        {
            gcmkONERROR(gckOS_UnmapMemoryEx(
                galDevice->os,
                galDevice->contiguousPhysical,
                galDevice->contiguousSize,
                data->contiguousLogical,
                data->pidOpen
                ));

            data->contiguousLogical = gcvNULL;
        }
    }

    /* A process gets detached. */
    for (i = 0; i < gcdMAX_GPU_COUNT; i++)
    {
        if (galDevice->kernels[i] != gcvNULL)
        {
            gcmkONERROR(gckKERNEL_AttachProcessEx(galDevice->kernels[i], gcvFALSE, data->pidOpen));
        }
    }

    kfree(data);
    filp->private_data = NULL;

    /* Success. */
    gcmkFOOTER_NO();
    return 0;

OnError:
    gcmkFOOTER();
    return -ENOTTY;
}

long loongson_drv_ioctl(
    struct file* filp,
    unsigned int ioctlCode,
    unsigned long arg
    )
{
    gceSTATUS status;
    gcsHAL_INTERFACE iface;
    gctUINT32 copyLen;
    DRIVER_ARGS drvArgs;
    gckGALDEVICE device;
    gcsHAL_PRIVATE_DATA_PTR data;
    gctINT32 i, count;

    gcmkHEADER_ARG(
        "filp=0x%08X ioctlCode=0x%08X arg=0x%08X",
        filp, ioctlCode, arg
        );

    if (filp == gcvNULL)
    {
        gcmkTRACE_ZONE(
            gcvLEVEL_ERROR, gcvZONE_DRIVER,
            "%s(%d): filp is NULL\n",
            __FUNCTION__, __LINE__
            );

        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    data = filp->private_data;

    if (data == gcvNULL)
    {
        gcmkTRACE_ZONE(
            gcvLEVEL_ERROR, gcvZONE_DRIVER,
            "%s(%d): private_data is NULL\n",
            __FUNCTION__, __LINE__
            );

        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    device = data->device;

    if (device == gcvNULL)
    {
        gcmkTRACE_ZONE(
            gcvLEVEL_ERROR, gcvZONE_DRIVER,
            "%s(%d): device is NULL\n",
            __FUNCTION__, __LINE__
            );

        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if ((ioctlCode != IOCTL_GCHAL_INTERFACE)
    &&  (ioctlCode != IOCTL_GCHAL_KERNEL_INTERFACE)
    )
    {
        gcmkTRACE_ZONE(
            gcvLEVEL_ERROR, gcvZONE_DRIVER,
            "%s(%d): unknown command %d\n",
            __FUNCTION__, __LINE__,
            ioctlCode
            );

        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Get the drvArgs. */
    copyLen = copy_from_user(
        &drvArgs, (void *) arg, sizeof(DRIVER_ARGS)
        );

    if (copyLen != 0)
    {
        gcmkTRACE_ZONE(
            gcvLEVEL_ERROR, gcvZONE_DRIVER,
            "%s(%d): error copying of the input arguments.\n",
            __FUNCTION__, __LINE__
            );

        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Now bring in the gcsHAL_INTERFACE structure. */
    if ((drvArgs.InputBufferSize  != sizeof(gcsHAL_INTERFACE))
    ||  (drvArgs.OutputBufferSize != sizeof(gcsHAL_INTERFACE))
    )
    {
        gcmkTRACE_ZONE(
            gcvLEVEL_ERROR, gcvZONE_DRIVER,
            "%s(%d): input or/and output structures are invalid.\n",
            __FUNCTION__, __LINE__
            );

        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    copyLen = copy_from_user(
        &iface, gcmUINT64_TO_PTR(drvArgs.InputBuffer), sizeof(gcsHAL_INTERFACE)
        );

    if (copyLen != 0)
    {
        gcmkTRACE_ZONE(
            gcvLEVEL_ERROR, gcvZONE_DRIVER,
            "%s(%d): error copying of input HAL interface.\n",
            __FUNCTION__, __LINE__
            );

        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }


    if (iface.command == gcvHAL_OPEN_BURST_REG)
    {
	gctUINT32 data;

	printk("set outstanding register \n");

        for (i = 0; i < gcdMAX_GPU_COUNT; i++)
        {
            if (device->kernels[i] != gcvNULL)
            {
		gcmkONERROR(
	                gckOS_ReadRegisterEx(device->kernels[i]->hardware->os,
	                                     device->kernels[i]->hardware->core,
	                                     0x00414,
	                                     &data));
		data &= 0xffffff80;
	
	        gcmkONERROR(
	                gckOS_WriteRegisterEx(device->kernels[i]->hardware->os,
	                                      device->kernels[i]->hardware->core,
	                                      0x00414,
	                                      data));

		printk("0x414 in ioctl.c is %x: \n", data);
		break;
            }
        }
    }

    if (iface.command == gcvHAL_CHIP_INFO)
    {
        count = 0;
        for (i = 0; i < gcdMAX_GPU_COUNT; i++)
        {
            if (device->kernels[i] != gcvNULL)
            {
#if gcdENABLE_VG
                if (i == gcvCORE_VG)
                {
                    iface.u.ChipInfo.types[count] = gcvHARDWARE_VG;
                }
                else
#endif
                {
                    gcmkVERIFY_OK(gckHARDWARE_GetType(device->kernels[i]->hardware,
                                                      &iface.u.ChipInfo.types[count]));
                }
                count++;
            }
        }

        iface.u.ChipInfo.count = count;
        iface.status = status = gcvSTATUS_OK;
    }
    else
    {
        if (iface.hardwareType < 0 || iface.hardwareType > 7)
        {
            gcmkTRACE_ZONE(
                gcvLEVEL_ERROR, gcvZONE_DRIVER,
                "%s(%d): unknown hardwareType %d\n",
                __FUNCTION__, __LINE__,
                iface.hardwareType
                );

            gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

#if gcdENABLE_VG
        if (device->coreMapping[iface.hardwareType] == gcvCORE_VG)
        {
            status = gckVGKERNEL_Dispatch(device->kernels[gcvCORE_VG],
                                        (ioctlCode == IOCTL_GCHAL_INTERFACE),
                                        &iface);
        }
        else
#endif
        {
            status = gckKERNEL_Dispatch(device->kernels[device->coreMapping[iface.hardwareType]],
                                        (ioctlCode == IOCTL_GCHAL_INTERFACE),
                                        &iface);
        }
    }

    /* Redo system call after pending signal is handled. */
    if (status == gcvSTATUS_INTERRUPTED)
    {
        gcmkFOOTER();
        return -ERESTARTSYS;
    }

    if (gcmIS_SUCCESS(status) && (iface.command == gcvHAL_LOCK_VIDEO_MEMORY))
    {
        gcuVIDMEM_NODE_PTR node = gcmUINT64_TO_PTR(iface.u.LockVideoMemory.node);
        /* Special case for mapped memory. */
        if ((data->mappedMemory != gcvNULL)
        &&  (node->VidMem.memory->object.type == gcvOBJ_VIDMEM)
        )
        {
            /* Compute offset into mapped memory. */
            gctUINT32 offset
                = (gctUINT8 *) gcmUINT64_TO_PTR(iface.u.LockVideoMemory.memory)
                - (gctUINT8 *) device->contiguousBase;

            /* Compute offset into user-mapped region. */
            iface.u.LockVideoMemory.memory =
                gcmPTR_TO_UINT64((gctUINT8 *) data->mappedMemory + offset);
        }
    }

    /* Copy data back to the user. */
    copyLen = copy_to_user(
        gcmUINT64_TO_PTR(drvArgs.OutputBuffer), &iface, sizeof(gcsHAL_INTERFACE)
        );

    if (copyLen != 0)
    {
        gcmkTRACE_ZONE(
            gcvLEVEL_ERROR, gcvZONE_DRIVER,
            "%s(%d): error copying of output HAL interface.\n",
            __FUNCTION__, __LINE__
            );

        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Success. */
    gcmkFOOTER_NO();
    return 0;

OnError:
    gcmkFOOTER();
    return -ENOTTY;
}

int loongson_drv_mmap(
    struct file* filp,
    struct vm_area_struct* vma
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsHAL_PRIVATE_DATA_PTR data;
    gckGALDEVICE device;

    gcmkHEADER_ARG("filp=0x%08X vma=0x%08X", filp, vma);

    if (filp == gcvNULL)
    {
        gcmkTRACE_ZONE(
            gcvLEVEL_ERROR, gcvZONE_DRIVER,
            "%s(%d): filp is NULL\n",
            __FUNCTION__, __LINE__
            );

        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    data = filp->private_data;

    if (data == gcvNULL)
    {
        gcmkTRACE_ZONE(
            gcvLEVEL_ERROR, gcvZONE_DRIVER,
            "%s(%d): private_data is NULL\n",
            __FUNCTION__, __LINE__
            );

        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    device = data->device;

    if (device == gcvNULL)
    {
        gcmkTRACE_ZONE(
            gcvLEVEL_ERROR, gcvZONE_DRIVER,
            "%s(%d): device is NULL\n",
            __FUNCTION__, __LINE__
            );

        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

#ifndef HARDWARE_CACHE_COHERENT

#if !gcdPAGED_MEMORY_CACHEABLE
    vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
    vma->vm_flags    |= gcdVM_FLAGS;
#endif

#endif
    vma->vm_pgoff     = 0;

    if (device->contiguousMapped)
    {
        unsigned long size = vma->vm_end - vma->vm_start;
        int ret = 0;

        if (size > device->contiguousSize)
        {
            gcmkTRACE_ZONE(
                gcvLEVEL_ERROR, gcvZONE_DRIVER,
                "%s(%d): Invalid mapping size.\n",
                __FUNCTION__, __LINE__
                );

            gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        ret = io_remap_pfn_range(
            vma,
            vma->vm_start,
#ifdef ALL_IN_2H
            (((unsigned long) device->contiguousPhysical) | 0x40000000) >> PAGE_SHIFT,
#else
            device->requestedContiguousBase >> PAGE_SHIFT,
#endif
            size,
            vma->vm_page_prot
            );

        if (ret != 0)
        {
            gcmkTRACE_ZONE(
                gcvLEVEL_ERROR, gcvZONE_DRIVER,
                "%s(%d): io_remap_pfn_range failed %d\n",
                __FUNCTION__, __LINE__,
                ret
                );

            data->mappedMemory = gcvNULL;

            gcmkONERROR(gcvSTATUS_OUT_OF_RESOURCES);
        }

        data->mappedMemory = (gctPOINTER) vma->vm_start;

        /* Success. */
        gcmkFOOTER_NO();
        return 0;
    }


OnError:
    gcmkFOOTER();
    return -ENOTTY;
}


#if !USE_PLATFORM_DRIVER
static int __init drv_init(void)
#else
static int drv_init(void)
#endif
{
    int ret;
    int result = -EINVAL;
    gceSTATUS status;
    gckGALDEVICE device = gcvNULL;
    struct class* device_class = gcvNULL;

    gcmkHEADER();

#if ENABLE_GPU_CLOCK_BY_DRIVER && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28))
    {
        struct clk * clk;

        clk = clk_get(NULL, "GCCLK");

        if (IS_ERR(clk))
        {
            gcmkTRACE_ZONE(
                gcvLEVEL_ERROR, gcvZONE_DRIVER,
                "%s(%d): clk get error: %d\n",
                __FUNCTION__, __LINE__,
                PTR_ERR(clk)
                );

            result = -ENODEV;
            gcmkONERROR(gcvSTATUS_GENERIC_IO);
        }

        /*
         * APMU_GC_156M, APMU_GC_312M, APMU_GC_PLL2, APMU_GC_PLL2_DIV2 currently.
         * Use the 2X clock.
         */
        if (clk_set_rate(clk, coreClock * 2))
        {
            gcmkTRACE_ZONE(
                gcvLEVEL_ERROR, gcvZONE_DRIVER,
                "%s(%d): Failed to set core clock.\n",
                __FUNCTION__, __LINE__
                );

            result = -EAGAIN;
            gcmkONERROR(gcvSTATUS_GENERIC_IO);
        }

        clk_enable(clk);

#if defined(CONFIG_PXA_DVFM) && (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,29))
        gc_pwr(1);
#   endif
    }
#endif

    printk(KERN_INFO "Galcore Version %d.%d.%d.%d\n",
        gcvVERSION_MAJOR, gcvVERSION_MINOR, gcvVERSION_PATCH, gcvVERSION_BUILD);

    if (showArgs)
    {
        printk("Galcore Options:\n");
        printk("  irqLine           = %d\n",      irqLine);
        printk("  registerMemBase   = 0x%08lX\n", registerMemBase);
        printk("  registerMemSize   = 0x%08lX\n", registerMemSize);

        if (irqLine2D != -1)
        {
            printk("  irqLine2D         = %d\n",      irqLine2D);
            printk("  registerMemBase2D = 0x%08lX\n", registerMemBase2D);
            printk("  registerMemSize2D = 0x%08lX\n", registerMemSize2D);
        }

        if (irqLineVG != -1)
        {
            printk("  irqLineVG         = %d\n",      irqLineVG);
            printk("  registerMemBaseVG = 0x%08lX\n", registerMemBaseVG);
            printk("  registerMemSizeVG = 0x%08lX\n", registerMemSizeVG);
        }

        printk("  contiguousSize    = %ld\n",     contiguousSize);
        printk("  contiguousBase    = 0x%08lX\n", contiguousBase);
        printk("  bankSize          = 0x%08lX\n", bankSize);
        printk("  fastClear         = %d\n",      fastClear);
        printk("  compression       = %d\n",      compression);
        printk("  signal            = %d\n",      signal);
        printk("  baseAddress       = 0x%08lX\n", baseAddress);
        printk("  physSize          = 0x%08lX\n", physSize);
        printk("  logFileSize       = %d KB \n",  logFileSize);
        printk("  powerManagement   = %d\n",      powerManagement);
#if ENABLE_GPU_CLOCK_BY_DRIVER
        printk("  coreClock       = %lu\n",     coreClock);
#endif
    }

    if(logFileSize != 0)
    {
    	gckDebugFileSystemInitialize();
    }

    /* Create the GAL device. */
    gcmkONERROR(gckGALDEVICE_Construct(
        irqLine,
        registerMemBase, registerMemSize,
        irqLine2D,
        registerMemBase2D, registerMemSize2D,
        irqLineVG,
        registerMemBaseVG, registerMemSizeVG,
        contiguousBase, contiguousSize,
        bankSize, fastClear, compression, baseAddress, physSize, signal,
        logFileSize,
        powerManagement,
        &device
        ));

    /* Start the GAL device. */
    gcmkONERROR(gckGALDEVICE_Start(device));

    if ((physSize != 0)
       && (device->kernels[gcvCORE_MAJOR] != gcvNULL)
       && (device->kernels[gcvCORE_MAJOR]->hardware->mmuVersion != 0))
    {
        status = gckMMU_Enable(device->kernels[gcvCORE_MAJOR]->mmu, baseAddress, physSize);
        gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DRIVER,
            "Enable new MMU: status=%d\n", status);

        if ((device->kernels[gcvCORE_2D] != gcvNULL)
            && (device->kernels[gcvCORE_2D]->hardware->mmuVersion != 0))
        {
            status = gckMMU_Enable(device->kernels[gcvCORE_2D]->mmu, baseAddress, physSize);
            gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DRIVER,
                "Enable new MMU for 2D: status=%d\n", status);
        }

        /* Reset the base address */
        device->baseAddress = 0;
    }

    /* Register the character device. */
    ret = register_chrdev(major, DRV_NAME, &driver_fops);

    if (ret < 0)
    {
        gcmkTRACE_ZONE(
            gcvLEVEL_ERROR, gcvZONE_DRIVER,
            "%s(%d): Could not allocate major number for mmap.\n",
            __FUNCTION__, __LINE__
            );

        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    if (major == 0)
    {
        major = ret;
    }

    /* Create the device class. */
    device_class = class_create(THIS_MODULE, "graphics_class");

    if (IS_ERR(device_class))
    {
        gcmkTRACE_ZONE(
            gcvLEVEL_ERROR, gcvZONE_DRIVER,
            "%s(%d): Failed to create the class.\n",
            __FUNCTION__, __LINE__
            );

        gcmkONERROR(gcvSTATUS_OUT_OF_RESOURCES);
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
    device_create(device_class, NULL, MKDEV(major, 0), NULL, "galcore");
#else
    device_create(device_class, NULL, MKDEV(major, 0), "galcore");
#endif

    galDevice = device;
    gpuClass  = device_class;

    gcmkTRACE_ZONE(
        gcvLEVEL_INFO, gcvZONE_DRIVER,
        "%s(%d): irqLine=%d, contiguousSize=%lu, memBase=0x%lX\n",
        __FUNCTION__, __LINE__,
        irqLine, contiguousSize, registerMemBase
        );

    /* Success. */
    gcmkFOOTER_NO();
    return 0;

OnError:
    /* Roll back. */
    if (device_class != gcvNULL)
    {
        device_destroy(device_class, MKDEV(major, 0));
        class_destroy(device_class);
    }

    if (device != gcvNULL)
    {
        gcmkVERIFY_OK(gckGALDEVICE_Stop(device));
        gcmkVERIFY_OK(gckGALDEVICE_Destroy(device));
    }

    gcmkFOOTER();
    return result;
}

#if !USE_PLATFORM_DRIVER
static void __exit drv_exit(void)
#else
static void drv_exit(void)
#endif
{
    gcmkHEADER();

    gcmkASSERT(gpuClass != gcvNULL);
    device_destroy(gpuClass, MKDEV(major, 0));
    class_destroy(gpuClass);

    unregister_chrdev(major, DRV_NAME);

    gcmkVERIFY_OK(gckGALDEVICE_Stop(galDevice));
    gcmkVERIFY_OK(gckGALDEVICE_Destroy(galDevice));

   if(gckDebugFileSystemIsEnabled())
   {
   	 gckDebugFileSystemTerminate();
   }

#if ENABLE_GPU_CLOCK_BY_DRIVER && LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28)
    {
        struct clk * clk = NULL;

#if defined(CONFIG_PXA_DVFM) && (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,29))
        gc_pwr(0);
#endif
        clk = clk_get(NULL, "GCCLK");
        clk_disable(clk);
    }
#endif

    gcmkFOOTER_NO();
}

#if !USE_PLATFORM_DRIVER
    module_init(drv_init);
    module_exit(drv_exit);
#else

#ifdef ALL_IN_2H
struct device *GPU_DEV;
size_t all_reserved_size;
dma_addr_t bus_addr;
dma_addr_t device_addr = 0;
#endif

int gpu_burst_type;
unsigned long vram_addr_offset;

int loongson_gpu_plat_probe(struct platform_device *pdev)
{
    int ret = -ENODEV;
    struct resource* res;

    gcmkHEADER();

#ifdef ALL_IN_2H
    gpu_burst_type = 0;
    GPU_DEV = &pdev->dev;
    physSize = 0x80000000;
#endif

    res = platform_get_resource_byname(pdev, IORESOURCE_IRQ, "gpu_irq");

    if (!res)
    {
        printk(KERN_ERR "%s: No irq line supplied.\n",__FUNCTION__);
        goto gpu_probe_fail;
    }

    irqLine = res->start;

    res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "gpu_base");
    if (!res)
    {
        printk(KERN_ERR "%s: No register base supplied.\n",__FUNCTION__);
        goto gpu_probe_fail;
    }

    registerMemBase = res->start;
    registerMemSize = res->end - res->start + 1;

    res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "gpu_mem");
    if (!res)
    {
        printk(KERN_ERR "%s: No memory base supplied.\n",__FUNCTION__);
        goto gpu_probe_fail;
    }

    contiguousBase = 0;
    /* contiguousSize = all_reserved_size - 16MB */
    contiguousSize = res->end - res->start + 1 - 0x01000000;

    printk("res->start is 0x%llx, res->end is 0x%llx\n", res->start, res->end);
    printk("contiguousSize is %lx\n", contiguousSize);

    bus_addr = res->start;
    all_reserved_size  = res->end - res->start + 1;

    printk("all reserved_size is %lx\n", all_reserved_size);

    if(loongson_sysconf.vram_type == VRAM_TYPE_UMA)
    	device_addr = bus_addr | 0x40000000;
    if(loongson_sysconf.vram_type == VRAM_TYPE_SP)
    	device_addr = bus_addr & 0xffffffff;

#ifdef ALL_IN_2H
    if (dma_declare_coherent_memory(GPU_DEV, bus_addr, device_addr, all_reserved_size, DMA_MEMORY_EXCLUSIVE) < 0)
    {
        dev_err(GPU_DEV, "cannot declare coherent memory\n");
    }
#endif

    ret = drv_init();

    if (!ret)
    {
        platform_set_drvdata(pdev, galDevice);

        gcmkFOOTER_NO();
        return ret;
    }

gpu_probe_fail:
    gcmkFOOTER_ARG(KERN_INFO "Failed to register gpu driver: %d\n", ret);
    return ret;
}

int loongson_gpu_plat_remove(struct platform_device *pdev)
{
    gcmkHEADER();
    drv_exit();
    gcmkFOOTER_NO();
    return 0;
}

void *real_vram;
void *shadow_vram = NULL;

int loongson_gpu_plat_suspend(struct platform_device *dev, pm_message_t state)
{
    gceSTATUS status;
    gckGALDEVICE device;
    gctINT i;

    device = platform_get_drvdata(dev);

    for (i = 0; i < gcdMAX_GPU_COUNT; i++)
    {
        if (device->kernels[i] != gcvNULL)
        {
            /* Store states. */
#if gcdENABLE_VG
            if (i == gcvCORE_VG)
            {
                status = gckVGHARDWARE_QueryPowerManagementState(device->kernels[i]->vg->hardware, &device->statesStored[i]);
            }
            else
#endif
            {
                status = gckHARDWARE_QueryPowerManagementState(device->kernels[i]->hardware, &device->statesStored[i]);
            }

            if (gcmIS_ERROR(status))
            {
                return -1;
            }

#if gcdENABLE_VG
            if (i == gcvCORE_VG)
            {
                status = gckVGHARDWARE_SetPowerManagementState(device->kernels[i]->vg->hardware, gcvPOWER_OFF);
            }
            else
#endif
            {
                status = gckHARDWARE_SetPowerManagementState(device->kernels[i]->hardware, gcvPOWER_OFF);
            }

            if (gcmIS_ERROR(status))
            {
                return -1;
            }

        }
    }

    if (loongson_sysconf.vram_type == VRAM_TYPE_SP) {
        shadow_vram = vmalloc(all_reserved_size);
        if (shadow_vram) {
            real_vram = GPU_DEV->dma_mem->virt_base;
            memcpy_fromio(shadow_vram, real_vram, all_reserved_size);
        }
    }

    return 0;
}

int loongson_gpu_plat_resume(struct platform_device *dev)
{
    gceSTATUS status;
    gckGALDEVICE device;
    gctINT i;
    gceCHIPPOWERSTATE   statesStored;

    device = platform_get_drvdata(dev);

    if (loongson_sysconf.vram_type == VRAM_TYPE_SP) {
        if (shadow_vram) {
            real_vram = GPU_DEV->dma_mem->virt_base;
            memcpy_toio(real_vram, shadow_vram, all_reserved_size);
            vfree(shadow_vram);
        }
    }

    for (i = 0; i < gcdMAX_GPU_COUNT; i++)
    {
        if (device->kernels[i] != gcvNULL)
        {
#if gcdENABLE_VG
            if (i == gcvCORE_VG)
            {
                status = gckVGHARDWARE_SetPowerManagementState(device->kernels[i]->vg->hardware, gcvPOWER_ON);
            }
            else
#endif
            {
                status = gckHARDWARE_SetPowerManagementState(device->kernels[i]->hardware, gcvPOWER_ON);
            }

            if (gcmIS_ERROR(status))
            {
                return -1;
            }

            /* Convert global state to crossponding internal state. */
            switch(device->statesStored[i])
            {
            case gcvPOWER_OFF:
                statesStored = gcvPOWER_OFF_BROADCAST;
                break;
            case gcvPOWER_IDLE:
                statesStored = gcvPOWER_IDLE_BROADCAST;
                break;
            case gcvPOWER_SUSPEND:
                statesStored = gcvPOWER_SUSPEND_BROADCAST;
                break;
            case gcvPOWER_ON:
                statesStored = gcvPOWER_ON_AUTO;
                break;
            default:
                statesStored = device->statesStored[i];
                break;
            }

            /* Restore states. */
#if gcdENABLE_VG
            if (i == gcvCORE_VG)
            {
                status = gckVGHARDWARE_SetPowerManagementState(device->kernels[i]->vg->hardware, statesStored);
            }
            else
#endif
            {
                status = gckHARDWARE_SetPowerManagementState(device->kernels[i]->hardware, statesStored);
            }

            if (gcmIS_ERROR(status))
            {
                return -1;
            }
        }
    }

    return 0;
}

int loongson_gpu_pci_probe(struct pci_dev *pdev, const struct pci_device_id *ent)

{
	int ret = -ENODEV;

	gcmkHEADER();

	pr_debug("loongson_gpu_pci_register BEGIN\n");

	/* Enable device in PCI config */
	ret = pci_enable_device(pdev);
	if (ret < 0) {
		printk(KERN_ERR "loongson gpu (%s): Cannot enable PCI device\n",
		       pci_name(pdev));
		goto gpu_probe_fail;
	}

#ifdef ALL_IN_2H
	gpu_burst_type = 1;
	GPU_DEV = &pdev->dev;
	physSize = 0x80000000;
#endif

	irqLine = pdev->irq;

	registerMemBase = pci_resource_start(pdev, 0);
	registerMemSize = pci_resource_end(pdev, 0) - pci_resource_start(pdev, 0) + 1;

	if (!loongson_sysconf.uma_vram_addr || !loongson_sysconf.uma_vram_size)
		goto gpu_probe_fail;

	if (!loongson_sysconf.vuma_vram_addr)
		loongson_sysconf.vuma_vram_addr = loongson_sysconf.uma_vram_addr;
	if (!loongson_sysconf.vuma_vram_size)
		loongson_sysconf.vuma_vram_size = loongson_sysconf.uma_vram_size;

	contiguousBase = 0;
	/* contiguousSize = all_reserved_size - 16MB */
	contiguousSize = loongson_sysconf.uma_vram_size - 0x01000000;

	printk("res->start is 0x%llx, res->end is 0x%llx\n", pci_resource_start(pdev, 0), pci_resource_end(pdev, 0));
	printk("contiguousSize is %lx\n", contiguousSize);

	bus_addr = loongson_sysconf.uma_vram_addr;
	device_addr = loongson_sysconf.vuma_vram_addr;
	vram_addr_offset = bus_addr - device_addr;
	all_reserved_size = loongson_sysconf.uma_vram_size;

	printk("all reserved_size is %lx\n", all_reserved_size);

#ifdef ALL_IN_2H
	if (dma_declare_coherent_memory(GPU_DEV, bus_addr, device_addr, all_reserved_size, DMA_MEMORY_EXCLUSIVE) < 0)
	{
		dev_err(GPU_DEV, "cannot declare coherent memory\n");
	}
#endif

	ret = drv_init();

	if (!ret)
	{
		pci_set_drvdata(pdev, galDevice);

		gcmkFOOTER_NO();
		return ret;
	}

gpu_probe_fail:
	gcmkFOOTER_ARG(KERN_INFO "Failed to register gpu driver: %d\n", ret);
	return ret;
}

void loongson_gpu_pci_remove(struct pci_dev *pdev)
{
	gcmkHEADER();
	drv_exit();
	gcmkFOOTER_NO();
	pci_release_region(pdev, 0);
}

int loongson_gpu_pci_suspend(struct pci_dev *pdev, pm_message_t mesg)
{
	gctINT i;
	gceSTATUS status;
	gckGALDEVICE device;

	device = pci_get_drvdata(pdev);

	for (i = 0; i < gcdMAX_GPU_COUNT; i++)
	{
		if (device->kernels[i] != gcvNULL)
		{
			/* Store states. */
#if gcdENABLE_VG
			if (i == gcvCORE_VG)
			{
				status = gckVGHARDWARE_QueryPowerManagementState(device->kernels[i]->vg->hardware, &device->statesStored[i]);
			}
			else
#endif
			{
				status = gckHARDWARE_QueryPowerManagementState(device->kernels[i]->hardware, &device->statesStored[i]);
			}

			if (gcmIS_ERROR(status))
			{
				return -1;
			}

#if gcdENABLE_VG
			if (i == gcvCORE_VG)
			{
				status = gckVGHARDWARE_SetPowerManagementState(device->kernels[i]->vg->hardware, gcvPOWER_OFF);
			}
			else
#endif
			{
				status = gckHARDWARE_SetPowerManagementState(device->kernels[i]->hardware, gcvPOWER_OFF);
			}

			if (gcmIS_ERROR(status))
			{
				return -1;
			}
		}
        }

	pci_save_state(pdev);
	return 0;
}

int loongson_gpu_pci_resume(struct pci_dev *pdev)
{
	gctINT i;
	gceSTATUS status;
	gckGALDEVICE device;
	gceCHIPPOWERSTATE   statesStored;

	pci_restore_state(pdev);
	device = pci_get_drvdata(pdev);

	for (i = 0; i < gcdMAX_GPU_COUNT; i++)
	{
		if (device->kernels[i] != gcvNULL)
		{
#if gcdENABLE_VG
			if (i == gcvCORE_VG)
			{
				status = gckVGHARDWARE_SetPowerManagementState(device->kernels[i]->vg->hardware, gcvPOWER_ON);
			}
			else
#endif
			{
				status = gckHARDWARE_SetPowerManagementState(device->kernels[i]->hardware, gcvPOWER_ON);
			}

			if (gcmIS_ERROR(status))
			{
				return -1;
			}

			/* Convert global state to crossponding internal state. */
			switch(device->statesStored[i])
			{
			case gcvPOWER_OFF:
				statesStored = gcvPOWER_OFF_BROADCAST;
				break;
			case gcvPOWER_IDLE:
				statesStored = gcvPOWER_IDLE_BROADCAST;
				break;
			case gcvPOWER_SUSPEND:
				statesStored = gcvPOWER_SUSPEND_BROADCAST;
				break;
			case gcvPOWER_ON:
				statesStored = gcvPOWER_ON_AUTO;
				break;
			default:
                statesStored = device->statesStored[i];
                break;
            }

            /* Restore states. */
#if gcdENABLE_VG
            if (i == gcvCORE_VG)
            {
                status = gckVGHARDWARE_SetPowerManagementState(device->kernels[i]->vg->hardware, statesStored);
            }
            else
#endif
            {
                status = gckHARDWARE_SetPowerManagementState(device->kernels[i]->hardware, statesStored);
            }

            if (gcmIS_ERROR(status))
            {
                return -1;
            }
        }
    }

	return 0;
}

#endif
