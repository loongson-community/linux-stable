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

#include <linux/export.h>
#include <linux/i2c.h>
#include <linux/i2c-algo-bit.h>
#include <linux/pm_runtime.h>
#include <drm/drmP.h>
#include <drm/drm_plane_helper.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_edid.h>

#include "loongson_drv.h"

#define DVO_I2C_NAME "loongson_dvo_i2c"
#define adapter_to_i2c_client(d) container_of(d, struct i2c_client, adapter)

static struct eep_info{
	struct i2c_adapter *adapter;
	unsigned short addr;
}eeprom_info[2];

static DEFINE_MUTEX(edid_lock);
struct i2c_client *loongson_drm_i2c_client[2];

/**
 * loongson_connector_best_encoder
 *
 * @connector: point to the drm_connector structure
 *
 * Select the best encoder for the given connector. Used by both the helpers in
 * drm_atomic_helper_check_modeset() and legacy CRTC helpers
 */
static struct drm_encoder *loongson_connector_best_encoder(struct drm_connector
						  *connector)
{
	int enc_id = connector->encoder_ids[0];
	/* pick the encoder ids */
	if (enc_id)
		return drm_encoder_find(connector->dev, NULL, enc_id);
	return NULL;
}

/**
 * loongson_do_probe_ddc_edid
 *
 * @adapter: I2C device adapter
 *
 * Try to fetch EDID information by calling I2C driver functions
 */
static int loongson_do_probe_ddc_edid(struct i2c_adapter *adapter, unsigned int id, unsigned char *buf)
{
	unsigned char start = 0x0;
	unsigned int i, che_tmp = 0;
	struct i2c_msg msgs[] = {
		{
			.addr = 0x50,
			.flags = 0,
			.len = 1,
			.buf = &start,
		},{
			.addr = 0x50,
			.flags = I2C_M_RD,
			.len = EDID_LENGTH * 2,
			.buf = buf,
		}
	};

	mutex_lock(&edid_lock);
	i = i2c_transfer(adapter, msgs, 2);
	mutex_unlock(&edid_lock);

	if (i != 2) {
		dev_warn_once(&adapter->dev, "Unable to read EDID block.\n");
		return -1;
	} else {
		if(buf[126] != 0){
			buf[126] = 0;
			che_tmp = 0;
			for(i = 0;i < 127;i++){
				che_tmp += buf[i];
			}
			buf[127] = 256-(che_tmp)%256;
		}
		if (!drm_edid_block_valid(buf, 0, false, NULL)) {
			dev_warn_once(&adapter->dev, "Invalid EDID block.\n");
			return -1;
		}
	}

	return 0;
}

/**
 * loongson_i2c_connector
 *
 * According to i2c bus,acquire screen information
 */
static int loongson_i2c_connector(unsigned int id, unsigned char *buf)
{
	DRM_DEBUG("edid entry, id = %d\n", id);
	if (!eeprom_info[id].adapter)
		return -ENODEV;

	return loongson_do_probe_ddc_edid(eeprom_info[id].adapter, id, buf);
}

/**
 * loongson_vga_get_modes
 *
 * @connetcor: central DRM connector control structure
 *
 * Fill in all modes currently valid for the sink into the connector->probed_modes list.
 * It should also update the EDID property by calling drm_connector_update_edid_property().
 */
static int loongson_vga_get_modes(struct drm_connector *connector)
{
	int id, ret = 0;
	enum loongson_edid_method ledid_method;
	unsigned char *buf = kmalloc(EDID_LENGTH * 2, GFP_KERNEL);
	struct edid *edid = NULL;
	struct drm_device *dev = connector->dev;
	struct loongson_drm_device *ldev = dev->dev_private;

	if (!buf){
		dev_warn(&dev->dev, "Unable to allocate memory"
			" for EDID block.\n");
		return 0;
	}

	id = drm_connector_index(connector);

	DRM_DEBUG("connector_id = %d\n", id);
	ledid_method = ldev->connector_vbios[id]->edid_method;
	if (ledid_method == edid_method_i2c) {
		if (loongson_i2c_connector(id, buf) == 0) {
			edid = (struct edid *)buf;
			drm_connector_update_edid_property(connector, edid);
			ret = drm_add_edid_modes(connector, edid);
		}
	}

	kfree(buf);
	DRM_DEBUG("the vga get modes ret is %d\n", ret);

	return ret;
}

/*
 * loongson_vga_mode_valid
 *
 * @connector: point to the drm connector
 * @mode: point to the drm_connector structure
 *
 * Validate a mode for a connector, irrespective of the specific display configuration
 */
static int loongson_vga_mode_valid(struct drm_connector *connector,
				 struct drm_display_mode *mode)
{
	int id = drm_connector_index(connector);
	struct drm_device *dev = connector->dev;
	struct loongson_drm_device *ldev = (struct loongson_drm_device*)dev->dev_private;

	if (mode->hdisplay % 64)
		return MODE_BAD;
	if (mode->hdisplay > ldev->crtc_vbios[id]->crtc_max_weight)
		return MODE_BAD;
	if (mode->vdisplay > ldev->crtc_vbios[id]->crtc_max_height)
		return MODE_BAD;

	return MODE_OK;
}

/**
 * loongson_i2c_create
 *
 * @dev: point to drm_device structure
 *
 * Create i2c adapter
 */
struct loongson_i2c_chan *loongson_i2c_create(struct drm_device *dev)
{
	int ret, data, clock;
	struct loongson_i2c_chan *i2c;
	struct loongson_drm_device *ldev = dev->dev_private;

	return i2c;
}

/**
 * loongson_i2c_destroy
 *
 * @i2c: point to loongson_i2c_chan
 *
 * Destroy i2c adapter
 */
void loongson_i2c_destroy(struct loongson_i2c_chan *i2c)
{
	if (!i2c)
		return;
	i2c_del_adapter(&i2c->adapter);
	kfree(i2c);
}

/**
 * loongson_vga_detect
 *
 * @connector: point to drm_connector
 * @force: bool
 *
 * Check to see if anything is attached to the connector.
 * The parameter force is set to false whilst polling,
 * true when checking the connector due to a user request
 */
static enum drm_connector_status loongson_vga_detect(struct drm_connector
						   *connector, bool force)
{
	int i, r;
	enum loongson_edid_method ledid_method;
	enum drm_connector_status ret = connector_status_connected;
	struct drm_device *dev = connector->dev;
	struct loongson_drm_device *ldev = dev->dev_private;
 
	i = drm_connector_index(connector);

	ledid_method = ldev->connector_vbios[i]->edid_method;
	DRM_DEBUG("loongson_vga_detect connector_id=%d, ledid_method=%d\n", i, ledid_method);

	if (ledid_method == edid_method_i2c) {
		if (ldev->dev->pdev) {
			r = pm_runtime_get_sync(connector->dev->dev);
			if (r < 0)
				return connector_status_disconnected;
		}

		r = loongson_vga_get_modes(connector);
		if (r)
			ret = connector_status_connected;
		else
			ret = connector_status_disconnected;

		if (ldev->dev->pdev) {
			pm_runtime_mark_last_busy(connector->dev->dev);
			pm_runtime_put_autosuspend(connector->dev->dev);
		}
	}

	return ret;
}


/**
 * loongson_connector_destroy
 *
 * @connector: point to the drm_connector structure
 *
 * Clean up connector resources
 */
static void loongson_connector_destroy(struct drm_connector *connector)
{
	struct loongson_connector *loongson_connector = to_loongson_connector(connector);
	loongson_i2c_destroy(loongson_connector->i2c);
	drm_connector_cleanup(connector);
	kfree(connector);
}


/**
 * These provide the minimum set of functions required to handle a connector
 *
 * Helper operations for connectors.These functions are used
 * by the atomic and legacy modeset helpers and by the probe helpers.
 */
static const struct drm_connector_helper_funcs loongson_vga_connector_helper_funcs = {
        .get_modes = loongson_vga_get_modes,
        .mode_valid = loongson_vga_mode_valid,
        .best_encoder = loongson_connector_best_encoder,
};

/**
 * These provide the minimum set of functions required to handle a connector
 *
 * Control connectors on a given device.
 * The functions below allow the core DRM code to control connectors,
 * enumerate available modes and so on.
 */
static const struct drm_connector_funcs loongson_vga_connector_funcs = {
        .dpms = drm_helper_connector_dpms,
        .detect = loongson_vga_detect,
        .fill_modes = drm_helper_probe_single_connector_modes,
        .destroy = loongson_connector_destroy,
};

static const unsigned short normal_i2c[] = { 0x50, I2C_CLIENT_END };

/**
 * loongson_vga_init
 *
 * @dev: drm device
 * @connector_id:
 *
 * Vga is the interface between host and monitor
 * This function is to init vga
 */
struct drm_connector *loongson_vga_init(struct drm_device *dev, unsigned int connector_id)
{
	struct i2c_adapter *i2c_adap;
	struct i2c_board_info i2c_info;
	struct drm_connector *connector;
	struct loongson_connector *loongson_connector;
	struct loongson_drm_device *ldev = (struct loongson_drm_device*)dev->dev_private;

	loongson_connector = kzalloc(sizeof(struct loongson_connector), GFP_KERNEL);
	if (!loongson_connector)
		return NULL;

	i2c_adap = i2c_get_adapter(ldev->connector_vbios[connector_id]->i2c_id);
	memset(&i2c_info, 0, sizeof(struct i2c_board_info));
	i2c_info.addr = normal_i2c[0];
	strlcpy(i2c_info.type, DVO_I2C_NAME, I2C_NAME_SIZE);
	loongson_drm_i2c_client[connector_id] = i2c_new_device(i2c_adap, &i2c_info);
	i2c_put_adapter(i2c_adap);

	if(ldev->connector_vbios[0]->i2c_id == ldev->connector_vbios[1]->i2c_id)
		loongson_drm_i2c_client[1] = loongson_drm_i2c_client[0];

	if (!loongson_drm_i2c_client[connector_id])
		return NULL;
	else {
		eeprom_info[connector_id].addr = 0x50;
		eeprom_info[connector_id].adapter= loongson_drm_i2c_client[connector_id]->adapter;
	}

	connector = &loongson_connector->base;

	drm_connector_init(dev, connector,
			   &loongson_vga_connector_funcs, DRM_MODE_CONNECTOR_VGA);

	drm_connector_helper_add(connector, &loongson_vga_connector_helper_funcs);

	drm_connector_register(connector);

	return connector;
}
