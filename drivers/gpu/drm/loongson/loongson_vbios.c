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

#define VBIOS_START_ADDR	0x1000
#define VBIOS_SIZE 		0x1E000

static const char *phy_type_names[1] = {
	"NONE or TRANSPARENT PHY",
};

static const char *get_edid_method_names[4] = {
	"No EDID",
	"Reading EDID via built-in I2C",
	"Use the VBIOS built-in EDID information",
	"Get EDID via phy chip",
};

static const char *crtc_version_name[] = {
	"default version",
};

int have_table = 0;
uint32_t table[256];
uint32_t POLYNOMIAL = 0xEDB88320;

void make_table(void)
{
	int i, j;

	have_table = 1;
	for (i = 0 ; i < 256 ; i++)
		for (j = 0, table[i] = i; j < 8; j++)
			table[i] = (table[i]>>1)^((table[i]&1)?POLYNOMIAL:0);
}

uint lscrc32(uint crc, char *buff, int len)
{
	int i;

	if (!have_table)
		make_table();

	crc = ~crc;
	for (i = 0; i < len; i++)
		crc = (crc >> 8) ^ table[(crc ^ buff[i]) & 0xff];

	return ~crc;
}

void *loongson_vbios_default(void)
{
	int i;
	unsigned char *vbios_start;
	char *title = "Loongson-VBIOS";
	struct loongson_vbios *vbios;
	struct loongson_vbios_crtc *crtc_vbios[2];
	struct loongson_vbios_connector *connector_vbios[2];
	struct loongson_vbios_phy *phy_vbios[2];

	vbios = kzalloc(120*1024,GFP_KERNEL);
	vbios_start = (unsigned char *)vbios;

	i = 0;
	while (*title != '\0') {
		if(i > 15){
			vbios->title[15] = '\0';
			break;
		}
		vbios->title[i++] = *title;
		title++;
	}

	/* Build loongson_vbios struct */
	vbios->version_major = 0;
	vbios->version_minor = 1;
	vbios->crtc_num = 2;
	vbios->crtc_offset = sizeof(struct loongson_vbios);
	vbios->connector_num = 2;
	vbios->connector_offset = sizeof(struct loongson_vbios) + 2 * sizeof(struct loongson_vbios_crtc);
	vbios->phy_num = 2;
	vbios->phy_offset =
		sizeof(struct loongson_vbios) + 2 * sizeof(struct loongson_vbios_crtc) + 2 * sizeof(struct loongson_vbios_connector);


	/* Build loongson_vbios_crtc struct */
	crtc_vbios[0] = (struct loongson_vbios_crtc *)(vbios_start + vbios->crtc_offset);
	crtc_vbios[1] = (struct loongson_vbios_crtc *)(vbios_start + vbios->crtc_offset + sizeof(struct loongson_vbios_crtc));

	crtc_vbios[0]->next_crtc_offset = sizeof(struct loongson_vbios) + sizeof(struct loongson_vbios_crtc);
	crtc_vbios[0]->crtc_id = 0;
	crtc_vbios[0]->crtc_version = default_version;
	crtc_vbios[0]->crtc_max_weight = 2048;
	crtc_vbios[0]->crtc_max_height = 2048;
	crtc_vbios[0]->connector_id = 0;
	crtc_vbios[0]->phy_num = 1;
	crtc_vbios[0]->phy_id[0] = 0;

	crtc_vbios[1]->next_crtc_offset = NULL;
	crtc_vbios[1]->crtc_id = 1;
	crtc_vbios[1]->crtc_version = default_version;
	crtc_vbios[1]->crtc_max_weight = 2048;
	crtc_vbios[1]->crtc_max_height = 2048;
	crtc_vbios[1]->connector_id = 1;
	crtc_vbios[1]->phy_num = 1;
	crtc_vbios[1]->phy_id[0] = 1;

	/* Build loongson_vbios_connector struct */
	connector_vbios[0] = (struct loongson_vbios_connector *)(vbios_start + vbios->connector_offset);
	connector_vbios[1] = (struct loongson_vbios_connector *)(vbios_start + vbios->connector_offset + sizeof(struct loongson_vbios_connector));

	connector_vbios[0]->next_connector_offset = vbios->connector_offset + sizeof(struct loongson_vbios_connector);
	connector_vbios[1]->next_connector_offset = NULL;

	connector_vbios[0]->crtc_id = 0;
	connector_vbios[1]->crtc_id = 1;

	connector_vbios[0]->edid_method = edid_method_i2c;
	connector_vbios[1]->edid_method = edid_method_i2c;

	switch (loongson_pch->type) {
	case LS2H:
		connector_vbios[0]->i2c_id = 1;
		connector_vbios[1]->i2c_id = 1;
		break;
	case LS7A:
	default: /* No RS780E's case */
		connector_vbios[0]->i2c_id = 6;
		connector_vbios[1]->i2c_id = 7;
		break;
	}

	connector_vbios[0]->i2c_type = i2c_type_gpio;
	connector_vbios[1]->i2c_type = i2c_type_gpio;
		
	/* Build loongson_vbios_phy struct */
	phy_vbios[0] = (struct loongson_vbios_phy *)(vbios_start + vbios->phy_offset);
	phy_vbios[1] = (struct loongson_vbios_phy *)(vbios_start + vbios->phy_offset + sizeof(struct loongson_vbios_phy));
		
	phy_vbios[0]->next_phy_offset = vbios->phy_offset + sizeof(struct loongson_vbios_phy);
	phy_vbios[1]->next_phy_offset = NULL;

	phy_vbios[0]->phy_type = phy_transparent;
	phy_vbios[1]->phy_type = phy_transparent;

	phy_vbios[0]->crtc_id = 0;
	phy_vbios[1]->crtc_id = 1;

	phy_vbios[0]->connector_id = 0;
	phy_vbios[1]->connector_id = 1;

	return (void *)vbios;
}

int loongson_vbios_check(struct loongson_vbios *vbios)
{
	int i = 0;
	unsigned int crc;
	char *title="Loongson-VBIOS";

	while (*title != '\0' && i <= 15) {
		if(vbios->title[i++] != *title){
			DRM_ERROR("VBIOS title is wrong!\n");
			return -EINVAL;
		}
		title++;
	}

	crc = lscrc32(0,(unsigned char *)vbios, VBIOS_SIZE - 0x4);
	if(*(unsigned int *)((unsigned char *)vbios + VBIOS_SIZE - 0x4) != crc){
		DRM_ERROR("VBIOS crc is wrong!\n");
		return -EINVAL;
	}

	return 0;
}

int loongson_vbios_init(struct loongson_drm_device *ldev){
	int i;
	unsigned char *vbios_start;
	struct loongson_vbios *vbios;

	if (!loongson_sysconf.vgabios_addr ||
			(loongson_vbios_check((void *)loongson_sysconf.vgabios_addr) < 0)) {
		DRM_INFO("Use default VBIOS!\n");
		ldev->vbios = (struct loongson_vbios *)loongson_vbios_default();
	} else {
		DRM_INFO("Use firmware VBIOS!\n");
		ldev->vbios = (struct loongson_vbios *)loongson_sysconf.vgabios_addr;
	}

	vbios = ldev->vbios;
	if(vbios == NULL)
		return -1;

	vbios_start = (unsigned char *)vbios;

	/*get crtc struct points*/
	ldev->crtc_vbios[0] = (struct loongson_vbios_crtc *)(vbios_start + vbios->crtc_offset);
	if(vbios->crtc_num > 1)
	{
		for(i = 1;i < vbios->crtc_num; i++)
			ldev->crtc_vbios[i] = (struct loongson_vbios_crtc *)(vbios_start + ldev->crtc_vbios[i - 1]->next_crtc_offset);
	}

	/*get connector struct points*/
	ldev->connector_vbios [0] = (struct loongson_vbios_connector *)(vbios_start + vbios->connector_offset);
	if(vbios->connector_num > 1){
		for(i = 1;i < vbios->connector_num; i++)
			ldev->connector_vbios[i] = (struct loongson_vbios_connector *)(vbios_start + ldev->connector_vbios[i - 1]->next_connector_offset);
	}

	/*get phy struct points*/
	ldev->phy_vbios[0] = (struct loongson_vbios_phy *)(vbios_start + vbios->phy_offset);
	if(vbios->phy_num > 1){
		for(i = 1;i < vbios->phy_num; i++)
			ldev->phy_vbios[1] = (struct loongson_vbios_phy *)(vbios_start + ldev->phy_vbios[0]->next_phy_offset);
	}
	loongson_vbios_information_display(ldev);

	return 0;
}

int loongson_vbios_information_display(struct loongson_drm_device *ldev)
{
	int i, j, k;

	DRM_INFO("===========================LOONGSON VBIOS INFO=================================\n");
	DRM_INFO("Title: %s\n", ldev->vbios->title);
	DRM_INFO("Loongson VBIOS version: %d.%d\n", ldev->vbios->version_major,ldev->vbios->version_minor);
	DRM_INFO("VBIOS information: %s\n", (char *)ldev->vbios->information);
	DRM_INFO("CRTC num: %d\n", ldev->vbios->crtc_num);
	DRM_INFO("Connector num: %d\n", ldev->vbios->connector_num);
	DRM_INFO("PHY num: %d\n", ldev->vbios->phy_num);
	DRM_INFO("================================CRTC INFO=====================================\n");
	for(i=0;i<ldev->vbios->crtc_num;i++){
		DRM_INFO("CRTC-%d: max_weight=%d, max_height=%d\n",
				ldev->crtc_vbios[i]->crtc_id,ldev->crtc_vbios[i]->crtc_max_weight,ldev->crtc_vbios[i]->crtc_max_height);
		DRM_INFO("CRTC-%d type is %s\n",
				ldev->crtc_vbios[i]->crtc_id,crtc_version_name[ldev->crtc_vbios[i]->crtc_version]);
		DRM_INFO("Bind connector id is %d\n",ldev->crtc_vbios[i]->connector_id);
		DRM_INFO("Bind phy number is %d\n",ldev->crtc_vbios[i]->phy_num);
		j = ldev->crtc_vbios[i]->phy_num;
		k = 0;
		while(j-- > 0){
			DRM_INFO("Bind phy[%d] ID is:%d\n",k,ldev->crtc_vbios[i]->phy_id[k]);
			k++;
		}
	}
	DRM_INFO("=============================CONNECTOR INFO===================================\n");
	for(i=0;i<ldev->vbios->connector_num;i++){
		DRM_INFO("Connector-%d: %s\n", i, get_edid_method_names[ldev->connector_vbios[i]->edid_method]);
		if(ldev->connector_vbios[i]->edid_method == edid_method_i2c){
			DRM_INFO("Connector-%d use i2c: %d", i, ldev->connector_vbios[i]->i2c_id);
		}
	}
	DRM_INFO("===============================PHY INFO=======================================\n");
	for(i=0;i<ldev->vbios->phy_num;i++){
		DRM_INFO("PHY-%d: %s\n", i, phy_type_names[ldev->phy_vbios[i]->phy_type]);
		DRM_INFO("PHY-%d: bind with CRTC %d\n", i, ldev->phy_vbios[i]->crtc_id);
		DRM_INFO("PHY-%d: bind with connector %d\n", i, ldev->phy_vbios[i]->connector_id);
	}
	DRM_INFO("=================================END==========================================\n");
	return 0;
}
