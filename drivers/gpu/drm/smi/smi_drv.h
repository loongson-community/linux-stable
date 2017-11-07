/*
 * Copyright 2016 SiliconMotion Inc.
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License version 2. See the file COPYING in the main
 * directory of this archive for more details.
 *
 */

#ifndef __SMI_DRV_H__
#define __SMI_DRV_H__

#include <video/vga.h>
#include <linux/version.h>
#include <drm/drm_fb_helper.h>
#include <drm/drm_edid.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0)
#include <drm/drm_encoder.h>
#endif

#include <drm/ttm/ttm_bo_api.h>
#include <drm/ttm/ttm_bo_driver.h>
#include <drm/ttm/ttm_placement.h>
#include <drm/ttm/ttm_memory.h>
#include <drm/ttm/ttm_module.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,17,0)
#include <drm/drm_gem.h>
#endif

#define DRIVER_AUTHOR		"SiliconMotion"

#define DRIVER_NAME		"smifb"
#define DRIVER_DESC		"SiliconMotion GPU DRM Driver"
#define DRIVER_DATE		"20180723"

#define DRIVER_MAJOR		1
#define DRIVER_MINOR		2
#define DRIVER_PATCHLEVEL	5

#define SMIFB_CONN_LIMIT 3

#define RELEASE_TYPE "Linux DRM Display Driver Release"
#define SUPPORT_ARCH "x64, x86, mips"
#define SUPPORT_CHIP "SM750, SM768"
#define SUPPORT_XVERSION "All Linux distribution"

#define _version_	"1.2.5.0"

#ifdef CONFIG_CPU_LOONGSON3
#define NO_WC
#endif

#define HW_I2C 1

extern int g_specId;

extern int smi_pat;

#define DEBUG 0
#define PFX "[smi] "
extern int smi_indent;

#if (DEBUG == 0)
#define ENTER()
#define LEAVE(...) return __VA_ARGS__;
#define dbg_msg(fmt,args...)
#define err_msg(fmt,args...) 
#define war_msg(fmt,args...) 
#define inf_msg(fmt,args...) 

#elif(DEBUG == 1)
/* debug level == 1 */
#warning "debug=1 build"
#define ENTER()	printk(KERN_DEBUG PFX "%*c %s\n",smi_indent++,'>',__func__)
#define LEAVE(...) printk(KERN_DEBUG PFX "%*c %s\n",--smi_indent,'<',__func__);return __VA_ARGS__;
#define dbg_msg(fmt,args...)	printk(KERN_DEBUG "[%s]:" fmt,__func__,## args)
#define err_msg(fmt,args...) printk(KERN_ERR  fmt, ## args)
#define war_msg(fmt,args...) printk(KERN_WARNING fmt, ## args)
#define inf_msg(fmt,args...) printk(KERN_INFO fmt, ## args)

#elif(DEBUG == 2)
/* debug level == 2 */
#warning "debug=2 build"
#define dbg_msg(fmt,args...) printk(KERN_DEBUG PFX fmt, ## args)
#define ENTER()	printk(KERN_DEBUG PFX "%*c %s\n",smi_indent++,'>',__func__)
#define LEAVE()	\//LEAVE(...)
	do{				\
	printk(KERN_DEBUG PFX "%*c %s\n",--smi_indent,'<',__func__); \
	return __VA_ARGS__; \
	}while(0)
#define dbg_msg(fmt,args...) printk(KERN_DEBUG "[%s]:" fmt,__func__,## args)
#define err_msg(fmt,args...) printk(KERN_ERR  fmt, ## args)
#define war_msg(fmt,args...) printk(KERN_WARNING fmt, ## args)
#define inf_msg(fmt,args...) printk(KERN_INFO fmt, ## args)
	
#endif

#define SMI_MAX_FB_HEIGHT 8192
#define SMI_MAX_FB_WIDTH 8192

#define smi_DPMS_CLEARED (-1)

extern int smi_bpp;
extern int force_connect;
extern int lvds_channel;

struct smi_fbdev;

struct smi_crtc {
	struct drm_crtc			base;
	u8				lut_r[256], lut_g[256], lut_b[256];
	int				last_dpms;
	bool				enabled;
	int crtc_index;
	int CursorOffset;
};

#define to_smi_crtc(x) container_of(x, struct smi_crtc, base)
#define to_smi_encoder(x) container_of(x, struct smi_encoder, base)
#define to_smi_framebuffer(x) container_of(x, struct smi_framebuffer, base)


#define MAX_CRTC	2	
#define MAX_ENCODER 3


#define smi_LUT_SIZE 256
#define PALETTE_INDEX 0x8
#define PALETTE_DATA 0x9


#define USE_DVI 1
#define USE_VGA (1<<1)
#define USE_HDMI (1<<2)
#define USE_DVI_VGA (USE_DVI|USE_VGA)
#define USE_DVI_HDMI (USE_DVI | USE_HDMI)
#define USE_VGA_HDMI (USE_VGA | USE_HDMI)
#define USE_ALL (USE_DVI |USE_VGA | USE_HDMI)


struct smi_mode_info {
	bool				mode_config_initialized;
	struct smi_crtc		*crtc;
	/* pointer to fbdev info structure */
	struct smi_fbdev		*gfbdev;
};

struct smi_encoder {
	struct drm_encoder		base;
	int				last_dpms;
};

struct smi_connector {
	struct drm_connector		base;
};

struct smi_framebuffer {
	struct drm_framebuffer		base;
	struct drm_gem_object *obj;
	void * vmapping;
};

struct smi_mc {
	resource_size_t			vram_size;
	resource_size_t			vram_base;
};

struct smi_750_register;
struct smi_768_register;

struct smi_device {
	struct drm_device		*dev;
	unsigned long			flags;	

	resource_size_t			rmmio_base;
	resource_size_t			rmmio_size;
	void __iomem			*rmmio;

	struct smi_mc			mc;
	struct smi_mode_info		mode_info;

	int				num_crtc;
	int fb_mtrr;	
	bool				need_dma32;
	struct {
		struct drm_global_reference mem_global_ref;
		struct ttm_bo_global_ref bo_global_ref;
		struct ttm_bo_device bdev;
	} ttm;
	bool mm_inited;
	struct smi_750_register *regsave;
	struct smi_768_register *regsave_768;
	struct edid edid[3];
	bool is_hdmi;
};


struct smi_fbdev {
	struct drm_fb_helper helper;
	struct smi_framebuffer gfb;
	struct list_head fbdev_list;
	int size;
	int x1, y1, x2, y2; /* dirty rect */
	spinlock_t dirty_lock;
};

struct smi_bo {
	struct ttm_buffer_object bo;
	struct ttm_placement placement;
	struct ttm_bo_kmap_obj kmap;
	struct drm_gem_object gem;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0) // Loongson backport from 3.18
	struct ttm_place placements[3];
#else
	u32 placements[3];
#endif	
	int pin_count;
	struct ttm_bo_kmap_obj dma_buf_vmap;
};
#define gem_to_smi_bo(gobj) container_of((gobj), struct smi_bo, gem)

static inline struct smi_bo *
smi_bo(struct ttm_buffer_object *bo)
{
	return container_of(bo, struct smi_bo, bo);
}


#define to_smi_obj(x) container_of(x, struct smi_gem_object, base)
#define DRM_FILE_PAGE_OFFSET (0x100000000ULL >> PAGE_SHIFT)

				/* smi_mode.c */
void smi_crtc_fb_gamma_set(struct drm_crtc *crtc, u16 red, u16 green,
			     u16 blue, int regno);
void smi_crtc_fb_gamma_get(struct drm_crtc *crtc, u16 *red, u16 *green,
			     u16 *blue, int regno);

int smi_calc_hdmi_ctrl(int m_connector);


				/* smi_main.c */
int smi_device_init(struct smi_device *cdev,
		      struct drm_device *ddev,
		      struct pci_dev *pdev,
		      uint32_t flags);
void smi_device_fini(struct smi_device *cdev);
int smi_gem_init_object(struct drm_gem_object *obj);
void smi_gem_free_object(struct drm_gem_object *obj);
int smi_dumb_mmap_offset(struct drm_file *file,
			    struct drm_device *dev,
			    uint32_t handle,
			    uint64_t *offset);
int smi_gem_create(struct drm_device *dev,
		   u32 size, bool iskernel,
		   struct drm_gem_object **obj);
int smi_dumb_create(struct drm_file *file,
		    struct drm_device *dev,
		    struct drm_mode_create_dumb *args);
int smi_dumb_destroy(struct drm_file *file,
		     struct drm_device *dev,
		     uint32_t handle);

int smi_framebuffer_init(struct drm_device *dev,
			   struct smi_framebuffer *gfb,
			    const struct drm_mode_fb_cmd2 *mode_cmd,
			    struct drm_gem_object *obj);

				/* smi_display.c */
int smi_modeset_init(struct smi_device *cdev);
void smi_modeset_fini(struct smi_device *cdev);

				/* smi_fbdev.c */
int smi_fbdev_init(struct smi_device *cdev);
void smi_fbdev_fini(struct smi_device *cdev);
void smi_fb_zfill(struct drm_device *dev, struct smi_fbdev *gfbdev);


				/* smi_irq.c */
void smi_driver_irq_preinstall(struct drm_device *dev);
int smi_driver_irq_postinstall(struct drm_device *dev);
void smi_driver_irq_uninstall(struct drm_device *dev);
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,14,0)		
irqreturn_t smi_driver_irq_handler(int irq, void *arg);
#else
irqreturn_t smi_driver_irq_handler(DRM_IRQ_ARGS);
#endif
				/* smi_kms.c */
int smi_driver_load(struct drm_device *dev, unsigned long flags);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0)
void smi_driver_unload(struct drm_device *dev);
#else
int smi_driver_unload(struct drm_device *dev);
#endif
extern struct drm_ioctl_desc smi_ioctls[];
extern int smi_max_ioctl;

int smi_mm_init(struct smi_device *smi);
void smi_mm_fini(struct smi_device *smi);
void smi_ttm_placement(struct smi_bo *bo, int domain);
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0)
int smi_bo_create(struct drm_device *dev, int size, int align,
		  uint32_t flags, struct sg_table *sg, struct smi_bo **psmibo);
#else
int smi_bo_create(struct drm_device *dev, int size, int align,
		  uint32_t flags, struct sg_table *sg, struct reservation_object *resv, struct smi_bo **psmibo);
#endif

void smi_bo_ttm_destroy(struct ttm_buffer_object *tbo);

int smi_mmap(struct file *filp, struct vm_area_struct *vma);

void hw750_suspend(struct smi_750_register * pSave);
void hw750_resume(struct smi_750_register * pSave);

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,15,0)
struct drm_plane *smi_plane_init(struct smi_device *cdev, unsigned int possible_crtcs);
#else
struct drm_plane *smi_plane_init(struct smi_device *cdev, unsigned int possible_crtcs, enum drm_plane_type type);
#endif

static inline int smi_bo_reserve(struct smi_bo *bo, bool no_wait)
{
	int ret;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,7,0)
	ret = ttm_bo_reserve(&bo->bo, true, no_wait, false, NULL);
#else
	ret = ttm_bo_reserve(&bo->bo, true, no_wait, NULL);
#endif
	if (ret) {
		if (ret != -ERESTARTSYS && ret != -EBUSY)
			DRM_ERROR("reserve failed %p\n", bo);
		return ret;
	}
	return 0;
}

static inline void smi_bo_unreserve(struct smi_bo *bo)
{
	ttm_bo_unreserve(&bo->bo);
}


void smi_fb_output_poll_changed(struct smi_device *sdev);



int smi_bo_pin(struct smi_bo *bo, u32 pl_flag, u64 *gpu_addr);
int smi_bo_unpin(struct smi_bo *bo);



struct sg_table *smi_gem_prime_get_sg_table(struct drm_gem_object *obj);
void *smi_gem_prime_vmap(struct drm_gem_object *obj);
void smi_gem_prime_vunmap(struct drm_gem_object *obj, void *vaddr);


struct drm_gem_object *smi_gem_prime_import_sg_table(struct drm_device *dev,
							struct dma_buf_attachment *attach,
							struct sg_table *sg);

int smi_gem_prime_pin(struct drm_gem_object *obj);
void smi_gem_prime_unpin(struct drm_gem_object *obj);

struct reservation_object *smi_gem_prime_res_obj(struct drm_gem_object *obj);

#if KERNEL_VERSION(4, 12, 0) > LINUX_VERSION_CODE
int smi_crtc_page_flip(struct drm_crtc *crtc,struct drm_framebuffer *fb,
	struct drm_pending_vblank_event *event, uint32_t page_flip_flags);
#else
int smi_crtc_page_flip(struct drm_crtc *crtc,struct drm_framebuffer *fb,
	struct drm_pending_vblank_event *event, uint32_t page_flip_flags, struct drm_modeset_acquire_ctx *ctx);
#endif

int smi_audio_init(struct drm_device *dev);
void smi_audio_remove(struct drm_device *dev);

/* please use revision id to distinguish sm750le and sm750*/
#define SPC_SM750 	0
#define SPC_SM712 	1
#define SPC_SM502   2
#define SPC_SM768   3
//#define SPC_SM750LE 8

#define PCI_VENDOR_ID_SMI 	0x126f
#define PCI_DEVID_LYNX_EXP	0x0750
#define PCI_DEVID_SM768		0x0768

extern int g_specId;

#define BPP32_RED    0x00ff0000
#define BPP32_GREEN  0x0000ff00
#define BPP32_BLUE   0x000000ff
#define BPP32_WHITE  0x00ffffff
#define BPP32_GRAY   0x00808080
#define BPP32_YELLOW 0x00ffff00
#define BPP32_CYAN   0x0000ffff
#define BPP32_PINK   0x00ff00ff
#define BPP32_BLACK  0x00000000


#define BPP16_RED    0x0000f800
#define BPP16_GREEN  0x000007e0
#define BPP16_BLUE   0x0000001f
#define BPP16_WHITE  0x0000ffff
#define BPP16_GRAY   0x00008410
#define BPP16_YELLOW 0x0000ffe0
#define BPP16_CYAN   0x000007ff
#define BPP16_PINK   0x0000f81f
#define BPP16_BLACK  0x00000000

#endif				/* __SMI_DRV_H__ */
