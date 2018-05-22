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
#include <drm/drm_crtc_helper.h>
#include "loongson_drv.h"
/*
 * The encoder comes after the CRTC in the output pipeline, but before
 * the connector. It's responsible for ensuring that the digital
 * stream is appropriately converted into the output format. Setup is
 * very simple in this case - all we have to do is inform qemu of the
 * colour depth in order to ensure that it displays appropriately
 */

/*
 * These functions are analagous to those in the CRTC code, but are intended
 * to handle any encoder-specific limitations
 */


/**
 * loongson_encoder_mode_set
 *
 * @encoder: encoder object
 * @mode: display mode
 * @adjusted_mode: point to the drm_display_mode structure
 *
 * Used to update the display mode of an encoder
 */
static void loongson_encoder_mode_set(struct drm_encoder *encoder,
				struct drm_display_mode *mode,
				struct drm_display_mode *adjusted_mode)
{

}


/**
 * loongson_encoder_dpms
 *
 * @encoder: encoder object
 *
 * Control power levels on the encoder
 *
 */
static void loongson_encoder_dpms(struct drm_encoder *encoder, int state)
{

}


/**
 * loongson_encoder_prepare
 *
 * @encoder: encoder object
 *
 * Prepare the encoder for a subsequent modeset
 */
static void loongson_encoder_prepare(struct drm_encoder *encoder)
{
}


/**
 * loongson_encoder_commit
 *
 * @encoder: point to tne drm_encoder structure
 *
 * Commit the new mode on the encoder after a modeset
 */
static void loongson_encoder_commit(struct drm_encoder *encoder)
{
}


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


/**
 * These provide the minimum set of functions required to handle a encoder
 *
 * Helper operations for encoders
 */
static const struct drm_encoder_helper_funcs loongson_encoder_helper_funcs = {
	.dpms = loongson_encoder_dpms,
	.mode_set = loongson_encoder_mode_set,
	.prepare = loongson_encoder_prepare,
	.commit = loongson_encoder_commit,
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
struct drm_encoder *loongson_encoder_init(struct drm_device *dev,unsigned int encoder_id)
{
	struct drm_encoder *encoder;
	struct loongson_encoder *loongson_encoder;

	loongson_encoder = kzalloc(sizeof(struct loongson_encoder), GFP_KERNEL);
	if (!loongson_encoder)
		return NULL;

	encoder = &loongson_encoder->base;
	encoder->possible_crtcs = 1 << encoder_id;

	drm_encoder_init(dev, encoder, &loongson_encoder_encoder_funcs,
			 DRM_MODE_ENCODER_DAC, NULL);
	drm_encoder_helper_add(encoder, &loongson_encoder_helper_funcs);

	return encoder;
}
