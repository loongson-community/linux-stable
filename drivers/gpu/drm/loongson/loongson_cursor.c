/*
 * Copyright (c) 2018 Loongson Technology Co., Ltd.
 * Authors:
 *	Chen Zhu <zhuchen@loongson.cn>
 *	Yaling Fang <fangyaling@loongson.cn>
 *	Dandan Zhang <zhangdandan@loongson.cn>
 *	Huacai Chen <chenhc@lemote.com>
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <drm/drmP.h>
#include <drm/drm_gem_cma_helper.h>
#include "loongson_drv.h"

/*
  Hide the cursor off screen. We can't disable the cursor hardware because it
  takes too long to re-activate and causes momentary corruption
*/
static void loongson_hide_cursor(struct drm_crtc *crtc)
{
	unsigned long flags;
	volatile void __iomem *base;
	struct drm_device *dev = crtc->dev;
	struct loongson_drm_device *ldev = (struct loongson_drm_device *)dev->dev_private;
	struct loongson_crtc *loongson_crtc = to_loongson_crtc(crtc);
	unsigned int tmp, crtc_id = loongson_crtc->crtc_id;

	base = ldev->rmmio;
	tmp = readl(base + FB_CUR_CFG_REG);
	tmp &= ~0xff;
	if (clone_mode(ldev)) {
		spin_lock_irqsave(&loongson_reglock, flags);
		writel(tmp | 0x00, base + FB_CUR_CFG_REG);
		spin_unlock_irqrestore(&loongson_reglock, flags);
		ldev->cursor_showed = false;
	} else {
		if (ldev->cursor_crtc_id != crtc_id)
			return;

		spin_lock_irqsave(&loongson_reglock, flags);
		if (crtc_id) {
			writel(tmp | 0x10, base + FB_CUR_CFG_REG);
		} else {
			writel(tmp | 0x00, base + FB_CUR_CFG_REG);
		}
		spin_unlock_irqrestore(&loongson_reglock, flags);
		ldev->cursor_showed = false;
	}
}

static void loongson_show_cursor(struct drm_crtc *crtc)
{
	unsigned long flags;
	volatile void __iomem *base;
	struct drm_device *dev = crtc->dev;
	struct loongson_drm_device *ldev = (struct loongson_drm_device *)dev->dev_private;
	struct loongson_crtc *loongson_crtc = to_loongson_crtc(crtc);
	unsigned int crtc_id = loongson_crtc->crtc_id;

	base = ldev->rmmio;
	if (clone_mode(ldev)) {
		spin_lock_irqsave(&loongson_reglock, flags);
		writel(0x00050202, base + FB_CUR_CFG_REG);
		spin_unlock_irqrestore(&loongson_reglock, flags);
		ldev->cursor_crtc_id = 0;
		ldev->cursor_showed = true;
	} else {
		if (ldev->cursor_crtc_id == crtc_id) {
			spin_lock_irqsave(&loongson_reglock, flags);
			if(crtc_id == 0){
				writel(0x00050202, base + FB_CUR_CFG_REG);
		        }else{
				writel(0x00050212, base + FB_CUR_CFG_REG);
			}
			spin_unlock_irqrestore(&loongson_reglock, flags);

			ldev->cursor_showed = true;
			ldev->cursor_crtc_id = crtc_id;
		}
	}
}

int loongson_crtc_cursor_set2(struct drm_crtc *crtc,
			struct drm_file *file_priv,
			uint32_t handle,
			uint32_t width,
			uint32_t height,
			int32_t hot_x,
			int32_t hot_y)
{
	int ret = 0;
	u32 gpu_addr;
	unsigned long flags;
	unsigned int crtc_id;
	volatile void __iomem *base;
	struct drm_gem_object *obj;
	struct drm_device *dev = crtc->dev;
	struct loongson_crtc *loongson_crtc = to_loongson_crtc(crtc);
	struct loongson_drm_device *ldev = (struct loongson_drm_device *)dev->dev_private;
	struct drm_gem_cma_object *cma, *cursor = ldev->cursor;

	base = ldev->rmmio;
        crtc_id = loongson_crtc->crtc_id;

	if ((width != 32 || height != 32) && handle) {
		return -EINVAL;
	}

	if (!handle || !file_priv) {
		loongson_hide_cursor(crtc);
		return 0;
	}

	obj = drm_gem_object_lookup(file_priv, handle);
	if (!obj)
		return -ENOENT;

	cma = to_drm_gem_cma_obj(obj);

	memcpy(cursor->vaddr, cma->vaddr, 32*32*4);
	/* Program gpu address of cursor buffer */
	gpu_addr = ldev->cursor->paddr;
	spin_lock_irqsave(&loongson_reglock, flags);
	writel(gpu_addr, base + FB_CUR_ADDR_REG);
	writel(0x00eeeeee, base + FB_CUR_BACK_REG);
	writel(0x00aaaaaa, base + FB_CUR_FORE_REG);
	spin_unlock_irqrestore(&loongson_reglock, flags);

	loongson_show_cursor(crtc);
	ret = 0;

	if (ret)
		loongson_hide_cursor(crtc);

	drm_gem_object_unreference_unlocked(obj);

	return ret;
}

int loongson_crtc_cursor_move(struct drm_crtc *crtc, int x, int y)
{
	unsigned long flags;
	unsigned int tmp, crtc_id;
	int xorign = 0, yorign = 0;
	volatile void __iomem *base;
	struct loongson_crtc *loongson_crtc = to_loongson_crtc(crtc);
	struct loongson_drm_device *ldev = (struct loongson_drm_device *)crtc->dev->dev_private;

	base = ldev->rmmio;
        crtc_id = loongson_crtc->crtc_id;

	/* upper edge condition */
	yorign = y + crtc->y;
	if (yorign < 0)
		y = 0;

	/* left edge conditon */
	xorign = x + crtc->x;
	if (xorign < 0)
		x = 0;

	/* move from one crtc to another, check which crtc should he shown
	 * the x or y < 0, it means the cursor it out of current review,
	 * && xorign/ yorign > 0, it means the cursor is in the framebuffer
	 * but not in curren review */
	if ((x < 0 && xorign > 0) || (y < 0 && yorign > 0)) {
		if(ldev->cursor_crtc_id == crtc_id && !clone_mode(ldev))
		 /*the cursor is not show, so hide if the (x,y) is in active crtc*/
			loongson_hide_cursor(crtc);
		return 0;
	}

	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;

	tmp = x & 0xffff;
	tmp |= y << 16;
	spin_lock_irqsave(&loongson_reglock, flags);
	writel(tmp, base + FB_CUR_LOC_ADDR_REG);
	spin_unlock_irqrestore(&loongson_reglock, flags);
	if (ldev->cursor_crtc_id != crtc_id && !clone_mode(ldev)) {
		ldev->cursor_crtc_id = crtc_id;
		ldev->cursor_showed = false;
	}
	if (!ldev->cursor_showed)
		loongson_show_cursor(crtc);

	return 0;
}
