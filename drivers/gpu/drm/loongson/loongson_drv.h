#ifndef __LOONGSON_DRV_H__
#define __LOONGSON_DRV_H__

#include <video/vga.h>

#include <drm/ttm/ttm_bo_api.h>
#include <drm/ttm/ttm_bo_driver.h>
#include <drm/ttm/ttm_placement.h>
#include <drm/ttm/ttm_memory.h>
#include <drm/ttm/ttm_module.h>
#include <drm/drmP.h>

#include <drm/drm_encoder.h>
#include <drm/drm_gem.h>
#include <drm/drm_fb_helper.h>
#include <drm/loongson_drm.h>

#include <linux/i2c.h>
#include <linux/i2c-algo-bit.h>
#include <loongson-pch.h>

#include <linux/module.h>
#include <linux/types.h>
#include "loongson_vbios.h"

#define to_loongson_crtc(x) container_of(x, struct loongson_crtc, base)
#define to_loongson_encoder(x) container_of(x, struct loongson_encoder, base)
#define to_loongson_connector(x) container_of(x, struct loongson_connector, base)
#define to_loongson_framebuffer(x) container_of(x, struct loongson_framebuffer, base)

#define LOONGSON_MAX_FB_HEIGHT 4096
#define LOONGSON_MAX_FB_WIDTH 4096

#define CUR_WIDTH_SIZE		32
#define CUR_HEIGHT_SIZE		32

#define LO_OFF	0
#define HI_OFF	8

#define gem_to_loongson_bo(gobj) container_of((gobj), struct loongson_bo, gem)

#define LS7A_PCH_CFG_REG_BASE		(LS7A_PCIE_BAR_BASE(0x0, 0x15, 0x0) & 0xfffffff0)

#define LS2H_PIX0_PLL			(void *)TO_UNCAC(LS2H_CHIPCFG_REG_BASE + 0x0230)
#define LS2H_PIX1_PLL			(void *)TO_UNCAC(LS2H_CHIPCFG_REG_BASE + 0x0238)
#define LS7A_PIX0_PLL			(void *)TO_UNCAC(LS7A_PCH_CFG_REG_BASE + 0x04b0)
#define LS7A_PIX1_PLL			(void *)TO_UNCAC(LS7A_PCH_CFG_REG_BASE + 0x04c0)

#define CURIOSET_CORLOR		0x4607
#define CURIOSET_POSITION	0x4608
#define CURIOLOAD_ARGB		0x4609
#define CURIOLOAD_IMAGE		0x460A
#define CURIOHIDE_SHOW		0x460B
#define FBEDID_GET			0X860C

#define LS_FB_CFG_DVO0_REG			(0x1240)
#define LS_FB_CFG_DVO1_REG			(0x1250)
#define LS_FB_ADDR0_DVO0_REG		(0x1260)
#define LS_FB_ADDR0_DVO1_REG		(0x1270)
#define LS_FB_STRI_DVO0_REG			(0x1280)
#define LS_FB_STRI_DVO1_REG			(0x1290)

#define LS_FB_DITCFG_DVO0_REG		(0x1360)
#define LS_FB_DITCFG_DVO1_REG		(0x1370)
#define LS_FB_DITTAB_LO_DVO0_REG	(0x1380)
#define LS_FB_DITTAB_LO_DVO1_REG	(0x1390)
#define LS_FB_DITTAB_HI_DVO0_REG	(0x13a0)
#define LS_FB_DITTAB_HI_DVO1_REG	(0x13b0)
#define LS_FB_PANCFG_DVO0_REG		(0x13c0)
#define LS_FB_PANCFG_DVO1_REG		(0x13d0)
#define LS_FB_PANTIM_DVO0_REG		(0x13e0)
#define LS_FB_PANTIM_DVO1_REG		(0x13f0)

#define LS_FB_HDISPLAY_DVO0_REG		(0x1400)
#define LS_FB_HDISPLAY_DVO1_REG		(0x1410)
#define LS_FB_HSYNC_DVO0_REG		(0x1420)
#define LS_FB_HSYNC_DVO1_REG		(0x1430)

#define LS_FB_VDISPLAY_DVO0_REG		(0x1480)
#define LS_FB_VDISPLAY_DVO1_REG		(0x1490)
#define LS_FB_VSYNC_DVO0_REG		(0x14a0)
#define LS_FB_VSYNC_DVO1_REG		(0x14b0)

#define LS_FB_GAMINDEX_DVO0_REG		(0x14e0)
#define LS_FB_GAMINDEX_DVO1_REG		(0x14f0)
#define LS_FB_GAMDATA_DVO0_REG		(0x1500)
#define LS_FB_GAMDATA_DVO1_REG		(0x1510)

#define LS_FB_CUR_CFG_REG			(0x1520)
#define LS_FB_CUR_ADDR_REG			(0x1530)
#define LS_FB_CUR_LOC_ADDR_REG		(0x1540)
#define LS_FB_CUR_BACK_REG			(0x1550)
#define LS_FB_CUR_FORE_REG			(0x1560)

#define LS_FB_INT_REG				(0x1570)

#define LS_FB_ADDR1_DVO0_REG		(0x1580)
#define LS_FB_ADDR1_DVO1_REG		(0x1590)

struct pix_pll {
	unsigned int l2_div;
	unsigned int l1_loopc;
	unsigned int l1_frefc;
};

struct loongson_mc {
	resource_size_t			vram_size;
	resource_size_t			vram_base;
	resource_size_t			vram_window;
};


struct loongson_bo {
	struct ttm_buffer_object bo;
	struct ttm_placement placement;
	struct ttm_bo_kmap_obj kmap;
	struct drm_gem_object gem;
	struct ttm_place placements[3];
	struct loongson_mc mc;
	int pin_count;
};

struct loongson_framebuffer {
	struct drm_framebuffer base;
	struct drm_gem_object *obj;
};

struct loongson_crtc {
	struct drm_crtc base;
	unsigned int crtc_id;
	int width;
	int height;
	int last_dpms;
	bool enabled;
};

struct loongson_mode_info {
	bool mode_config_initialized;
	struct loongson_crtc *crtc;
	struct loongson_connector *connector;
};

struct loongson_encoder {
	struct drm_encoder base;
	int last_dpms;
};

struct loongson_cursor {
	struct loongson_bo *pixels;
	u64 pixels_gpu_addr;
};

struct loongson_fbdev {
	struct drm_fb_helper helper;
	struct loongson_framebuffer lfb;
	void *sysram;
	int size;
	struct ttm_bo_kmap_obj mapping;
	int x1, y1, x2, y2; /* dirty rect */
	spinlock_t dirty_lock;
};

struct loongson_drm_device {
	struct drm_device		*dev;

	resource_size_t			rmmio_base;
	resource_size_t			rmmio_size;
	void __iomem			*rmmio;

	struct loongson_mc			mc;
	struct loongson_mode_info		mode_info[2];

	struct loongson_fbdev *lfbdev;
	struct loongson_cursor cursor;

	struct pci_dev *vram_pdev;		/**< PCI device structure */

	bool				suspended;
	int				num_crtc;
	int				cursor_crtc_id;
	int				has_sdram;
	struct drm_display_mode		mode;

	struct drm_property *rotation_prop;
	struct loongson_vbios *vbios;
	struct loongson_vbios_crtc *crtc_vbios[2];
	struct loongson_vbios_connector *connector_vbios[2];
	struct loongson_vbios_phy *phy_vbios[4];
	int fb_mtrr;

	struct {
		struct drm_global_reference mem_global_ref;
		struct ttm_bo_global_ref bo_global_ref;
		struct ttm_bo_device bdev;
	} ttm;
	struct {
		resource_size_t start;
		resource_size_t size;
	} vram;
	unsigned long fb_vram_base;
	bool	clone_mode;
	bool	cursor_showed;
	bool	inited;
};


struct loongson_i2c_chan {
	struct i2c_adapter adapter;
	struct drm_device *dev;
	struct i2c_algo_bit_data bit;
	int data, clock;
};

struct loongson_connector {
	struct drm_connector base;
	struct loongson_i2c_chan *i2c;
};

static inline struct loongson_bo *
loongson_bo(struct ttm_buffer_object *bo)
{
	return container_of(bo, struct loongson_bo, bo);
}

static inline int loongson_bo_reserve(struct loongson_bo *bo, bool no_wait)
{
        int ret;

        ret = ttm_bo_reserve(&bo->bo, true, no_wait, NULL);
        if (ret) {
                if (ret != -ERESTARTSYS && ret != -EBUSY)
                        DRM_ERROR("reserve failed %p\n", bo);
                return ret;
        }
        return 0;
}

static inline void loongson_bo_unreserve(struct loongson_bo *bo)
{
        ttm_bo_unreserve(&bo->bo);
}

int loongson_irq_enable_vblank(struct drm_device *dev,unsigned int crtc_id);
void loongson_irq_disable_vblank(struct drm_device *dev,unsigned int crtc_id);
irqreturn_t loongson_irq_handler(int irq,void *arg);
void loongson_irq_preinstall(struct drm_device *dev);
int loongson_irq_postinstall(struct drm_device *dev);
void loongson_irq_uninstall(struct drm_device *dev);

int loongson_ttm_init(struct loongson_drm_device *ldev);
void loongson_ttm_fini(struct loongson_drm_device *ldev);

void loongson_ttm_placement(struct loongson_bo *bo, int domain);
int loongson_bo_create(struct drm_device *dev, int size, int align,uint32_t flags, struct loongson_bo **ploongsonbo);
int loongson_drm_mmap(struct file *filp, struct vm_area_struct *vma);
struct drm_encoder *loongson_encoder_init(struct drm_device *dev,unsigned int encoder_id);
void loongson_crtc_init(struct loongson_drm_device *ldev);
struct drm_connector *loongson_vga_init(struct drm_device *dev,unsigned int connector_id);
int loongson_bo_push_sysram(struct loongson_bo *bo);
int loongson_bo_pin(struct loongson_bo *bo, u32 pl_flag, u64 *gpu_addr);
int loongson_bo_unpin(struct loongson_bo *bo);
int loongson_gem_create(struct drm_device *dev, u32 size, bool iskernel,struct drm_gem_object **obj);
int loongson_framebuffer_init(struct drm_device *dev,struct loongson_framebuffer *lfb,const struct drm_mode_fb_cmd2 *mode_cmd,struct drm_gem_object *obj);

int loongson_fbdev_init(struct loongson_drm_device *ldev);
void loongson_fbdev_fini(struct loongson_drm_device *ldev);
bool loongson_fbdev_lobj_is_fb(struct loongson_drm_device *ldev, struct loongson_bo *lobj);
void loongson_fbdev_restore_mode(struct loongson_drm_device *ldev);
void loongson_fbdev_set_suspend(struct loongson_drm_device *ldev, int state);

int loongson_drm_drm_suspend(struct drm_device *dev, bool suspend,
                                   bool fbcon, bool freeze);
int loongson_drm_drm_resume(struct drm_device *dev, bool resume, bool fbcon);
			   /* loongson_cursor.c */
int loongson_crtc_cursor_set2(struct drm_crtc *crtc, struct drm_file *file_priv,
					uint32_t handle, uint32_t width, uint32_t height, int32_t hot_x, int32_t hot_y);
int loongson_crtc_cursor_move(struct drm_crtc *crtc, int x, int y);

int loongson_vbios_init(struct loongson_drm_device *ldev);
int loongson_vbios_information_display(struct loongson_drm_device *ldev);

#endif
