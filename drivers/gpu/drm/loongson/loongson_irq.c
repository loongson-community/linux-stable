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

irqreturn_t loongson_irq_handler(int irq, void *arg)
{
	unsigned int val;
	struct drm_device *dev = (struct drm_device *) arg;
	struct loongson_drm_device *ldev = dev->dev_private;
	volatile void __iomem *base = ldev->rmmio;
 
	val = readl(base + FB_INT_REG);
	spin_lock(&loongson_reglock);
	writel(0x0, base + FB_INT_REG);
	spin_unlock(&loongson_reglock);
 
	if (val & BIT(INT_DVO0_FB_END)){
		drm_crtc_handle_vblank(&ldev->lcrtc[0].base);
	}

	if (val & BIT(INT_DVO1_FB_END)){
		drm_crtc_handle_vblank(&ldev->lcrtc[1].base);
	}

	spin_lock(&loongson_reglock);
	writel(ldev->int_reg, base + FB_INT_REG);
	spin_unlock(&loongson_reglock);

	return IRQ_HANDLED;
}

void loongson_irq_preinstall(struct drm_device *dev)
{
	unsigned long flags;
	struct loongson_drm_device *ldev = dev->dev_private;
	volatile void __iomem *base = ldev->rmmio;

	/* disable interupt */
	spin_lock_irqsave(&loongson_reglock, flags);
	writel(0x0000 << 16, base + FB_INT_REG);
	spin_unlock_irqrestore(&loongson_reglock, flags);
}

int loongson_irq_postinstall(struct drm_device *dev)
{
	return 0;
}

void loongson_irq_uninstall(struct drm_device *dev)
{
	unsigned long flags;
	struct loongson_drm_device *ldev = dev->dev_private;
	volatile void __iomem *base = ldev->rmmio;

	/* disable interupt */
	spin_lock_irqsave(&loongson_reglock, flags);
	writel(0x0000 << 16, base + FB_INT_REG);
	spin_unlock_irqrestore(&loongson_reglock, flags);
}
