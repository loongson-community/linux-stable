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


#include "loongson_drv.h"

static DEFINE_SPINLOCK(drmfb_lock);

/**
 * enable_vblank - enable vblank interrupt events
 * @dev: DRM device
 * @crtc: which irq to enable
 *
 * Enable vblank interrupts for @crtc.  If the device doesn't have
 * a hardware vblank counter, this routine should be a no-op, since
 * interrupts will have to stay on to keep the count accurate.
 *
 * RETURNS
 * Zero on success, appropriate errno if the given @crtc's vblank
 * interrupt cannot be enabled.
 */
int loongson_irq_enable_vblank(struct drm_device *dev,unsigned int crtc_id)
{
	return 0;
}

/**
 * disable_vblank - disable vblank interrupt events
 * @dev: DRM device
 * @crtc: which irq to enable
 *
 * Disable vblank interrupts for @crtc.  If the device doesn't have
 * a hardware vblank counter, this routine should be a no-op, since
 * interrupts will have to stay on to keep the count accurate.
 */
void loongson_irq_disable_vblank(struct drm_device *dev,unsigned int crtc_id)
{
}

irqreturn_t loongson_irq_handler(int irq, void *arg)
{
	unsigned int val, cfg;
	struct drm_device *dev = (struct drm_device *) arg;
	struct loongson_drm_device *ldev = dev->dev_private;
	volatile void *base = ldev->rmmio;

	spin_lock(&drmfb_lock);

	val = readl(base + LS_FB_INT_REG);
	writel(0x0000 << 16, base + LS_FB_INT_REG);

	/* if underflow, reset DVO0 */
	if (val & 0x500) {
		cfg = readl(base + LS_FB_CFG_DVO0_REG);
		writel(0, base + LS_FB_CFG_DVO0_REG);
		writel(cfg, base + LS_FB_CFG_DVO0_REG);
	}

	/* if underflow, reset DVO1 */
	if (val & 0x280) {
		cfg = readl(base + LS_FB_CFG_DVO1_REG);
		writel(0, base + LS_FB_CFG_DVO1_REG);
		writel(cfg, base + LS_FB_CFG_DVO1_REG);
	}

	writel(0x0780 << 16, base + LS_FB_INT_REG);

	spin_unlock(&drmfb_lock);

	return IRQ_HANDLED;
}

void loongson_irq_preinstall(struct drm_device *dev)
{
	struct loongson_drm_device *ldev = dev->dev_private;
	volatile void *base = ldev->rmmio;

	/* disable interupt */
	writel(0x0000 << 16, base + LS_FB_INT_REG);
}

int loongson_irq_postinstall(struct drm_device *dev)
{
	struct loongson_drm_device *ldev = dev->dev_private;
	volatile void *base = ldev->rmmio;

	/* enable interupt */
	writel(0x0780 << 16, base + LS_FB_INT_REG);
	return 0;
}

void loongson_irq_uninstall(struct drm_device *dev)
{
	struct loongson_drm_device *ldev = dev->dev_private;
	volatile void *base = ldev->rmmio;

	/* disable interupt */
	writel(0x0000 << 16, base + LS_FB_INT_REG);
}
