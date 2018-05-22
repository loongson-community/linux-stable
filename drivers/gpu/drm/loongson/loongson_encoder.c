// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018 Loongson Technology Co., Ltd.
 * Copyright (C) 2019 Lemote Inc.
 * Authors:
 *	Chen Zhu <zhuchen@loongson.cn>
 *	Yaling Fang <fangyaling@loongson.cn>
 *	Dandan Zhang <zhangdandan@loongson.cn>
 *	Huacai Chen <chenhc@lemote.com>
 *	Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#include <drm/drmP.h>
#include <drm/drm_crtc_helper.h>
#include "loongson_drv.h"

/**
 * loongson_encoder_destroy
 *
 * @encoder: encoder object
 *
 * Clean up encoder resources
 */
static void loongson_encoder_destroy(struct drm_encoder *encoder)
{
	struct loongson_encoder *loongson_encoder = to_loongson_encoder(encoder);
	drm_encoder_cleanup(encoder);
	kfree(loongson_encoder);
}

static int loongson_encoder_atomic_check(struct drm_encoder *encoder,
				    struct drm_crtc_state *crtc_state,
				    struct drm_connector_state *conn_state)
{
	return 0;
}

static void loongson_encoder_atomic_mode_set(struct drm_encoder *encoder,
				struct drm_crtc_state *crtc_state,
				struct drm_connector_state *conn_state)
{
	unsigned long flags;
	struct loongson_encoder *lenc = to_loongson_encoder(encoder);
	struct loongson_crtc *lcrtc_origin = lenc->lcrtc;
	struct loongson_crtc *lcrtc_current = to_loongson_crtc(crtc_state->crtc);

	if (lcrtc_origin->crtc_id != lcrtc_current->crtc_id)
		lcrtc_origin->cfg_reg |= CFG_PANELSWITCH;
	else
		lcrtc_origin->cfg_reg &= ~CFG_PANELSWITCH;

	spin_lock_irqsave(&loongson_reglock, flags);
	crtc_write(lcrtc_origin, FB_CFG_DVO_REG, lcrtc_origin->cfg_reg);
	spin_unlock_irqrestore(&loongson_reglock, flags);
}

/**
 * These provide the minimum set of functions required to handle a encoder
 *
 * Helper operations for encoders
 */
static const struct drm_encoder_helper_funcs loongson_encoder_helper_funcs = {
	.atomic_check = loongson_encoder_atomic_check,
	.atomic_mode_set = loongson_encoder_atomic_mode_set,
};

/**
 * These provide the minimum set of functions required to handle a encoder
 *
 * Encoder controls,encoder sit between CRTCs and connectors
 */
static const struct drm_encoder_funcs loongson_encoder_encoder_funcs = {
	.destroy = loongson_encoder_destroy,
};


/**
 * loongson_encoder_init
 *
 * @dev: point to the drm_device structure
 *
 * Init encoder
 */
struct drm_encoder *loongson_encoder_init(struct drm_device *dev, unsigned int encoder_id)
{
	struct drm_encoder *encoder;
	struct loongson_encoder *loongson_encoder;
	struct loongson_drm_device *ldev = dev->dev_private;

	loongson_encoder = kzalloc(sizeof(struct loongson_encoder), GFP_KERNEL);
	if (!loongson_encoder)
		return NULL;

	loongson_encoder->encoder_id = encoder_id;
	loongson_encoder->lcrtc = &ldev->lcrtc[encoder_id];
	encoder = &loongson_encoder->base;
	encoder->possible_crtcs = BIT(1) | BIT(0);
	encoder->possible_clones = BIT(1) | BIT(0);

	drm_encoder_helper_add(encoder, &loongson_encoder_helper_funcs);
	drm_encoder_init(dev, encoder, &loongson_encoder_encoder_funcs,
			 DRM_MODE_ENCODER_DAC, NULL);

	return encoder;
}
