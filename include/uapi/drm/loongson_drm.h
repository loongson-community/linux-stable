/*
 * include/uapi/drm/loongson_drm.h
 *
 * Copyright (C) 2011 Loongson Technology
 * Author: Zhu Chen <zhuchen@loongson.cn>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __LOONGSON_DRM_H__
#define __LOONGSON_DRM_H__

#include <drm/drm.h>

/* Please note that modifications to all structs defined here are
 * subject to backwards-compatibility constraints.
 */
struct drm_loongson_param {
	uint64_t param;			/* in */
	uint64_t value;			/* in (set_param), out (get_param) */
};

#define DRM_LOONGSON_GET_FB_VRAM_BASE	0x00
#define DRM_LOONGSON_GET_BO_VRAM_BASE	0x01

#define DRM_IOCTL_LOONGSON_GET_FB_VRAM_BASE	DRM_IOWR(DRM_COMMAND_BASE + DRM_LOONGSON_GET_FB_VRAM_BASE, struct drm_loongson_param)
#define DRM_IOCTL_LOONGSON_GET_BO_VRAM_BASE	DRM_IOWR(DRM_COMMAND_BASE + DRM_LOONGSON_GET_BO_VRAM_BASE, struct drm_loongson_param)

#define DRM_LOONGSON_NUM_IOCTLS		0x02

#endif /* __LOONGSON_DRM_H__ */
