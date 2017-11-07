/*
 * Copyright 2016 SiliconMotion Inc.
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License version 2. See the file COPYING in the main
 * directory of this archive for more details.
 *
 */
#include <drm/drmP.h>
#include <drm/drm_crtc_helper.h>
#include "smi_drv.h"
#include "hw750.h"
#include "hw768.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,15,0)
#include <drm/drm_plane_helper.h>
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,19,0)
#include <drm/drm_atomic_helper.h>
#endif

extern struct smi_crtc * smi_crtc_tab[MAX_CRTC];
extern struct drm_encoder * smi_enc_tab[MAX_ENCODER];
extern int g_m_connector;//bit 0: DVI, bit 1: VGA, bit 2: HDMI.

void colorcur2monocur(void * data)
{
	unsigned int * col = (unsigned int *)data;
	unsigned char * mono = (unsigned char *)data;
	unsigned char pixel;
	char bit_values;

	int i;
	for(i=0;i<64*64;i++)
	{
		if(*col >>24 < 0xe0)
		{
			bit_values = 0;
		}
		else
		{
			int val = *col & 0xff;

			if(val<0x80)
			{
				bit_values = 1;
			}
			else	{
				bit_values = 2;
			}
 		}
 		col++;
		/* Copy bits into cursor byte */
		switch (i & 3)
		{
		  case 0:
		    pixel = bit_values;
		    break;

		  case 1:
		    pixel |= bit_values << 2;
		    break;

		  case 2:
		    pixel |= bit_values  << 4;
		    break;

		  case 3:
		    pixel |= bit_values  << 6;
		    *mono = pixel;
		    mono++;
		    pixel = 0;
		    break;
		}
	}
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,19,0)

int smi_plane_atomic_check(struct drm_plane *plane,
			   struct drm_plane_state *state)
{
	return 0;
}

static void smi_cursor_atomic_update(struct drm_plane *plane,struct drm_plane_state *old_state)
{

	//ENTER();
	struct drm_crtc *crtc = plane->state->crtc;
	struct drm_framebuffer *fb = plane->state->fb;
	struct smi_bo *bo;
	struct drm_gem_object *obj;
	int ret = 0;
	int x, y;
	struct smi_crtc * smi_crtc = to_smi_crtc(crtc);	
	disp_control_t disp_crtc;
	int i, ctrl_index, max_enc;
	ctrl_index = 0;

	if (!plane->state->crtc || !plane->state->fb)
		return;
	

	if(g_specId == SPC_SM750)
		max_enc = MAX_CRTC;
	else
		max_enc = MAX_ENCODER;

	for(i = 0;i < max_enc; i++)
	{
		if(crtc == smi_enc_tab[i]->crtc)
		{
			ctrl_index = i;
			break;
		}
	}
	disp_crtc = (ctrl_index == SMI1_CTRL)?SMI1_CTRL:SMI0_CTRL;

	if(ctrl_index >= MAX_CRTC)  //calc which path should we use for HDMI.
	{
		disp_crtc= (disp_control_t)smi_calc_hdmi_ctrl(g_m_connector);
	}


	if (fb != old_state->fb) {
		dbg_msg("cursor:change the cursor fb\n");
		obj = to_smi_framebuffer(fb)->obj;
		bo = gem_to_smi_bo(obj);
		ret = smi_bo_reserve(bo, false);
		if (ret)
		{
			dbg_msg("smi_bo_reserve failed\n");
			//LEAVE();
		}
	
		if(g_specId == SPC_SM750)
		{
			ret = ttm_bo_kmap(&bo->bo, 0, bo->bo.num_pages, &bo->kmap);
			if (ret)
				dbg_msg("failed to kmap fbcon\n");
			else
			{
				colorcur2monocur(bo->kmap.virtual);
				//memset(bo->kmap.virtual,smi_pat,gem->size);
			}
		}
		smi_bo_unreserve(bo);
	}	

	x = plane->state->crtc_x - smi_crtc->CursorOffset;
	y = plane->state->crtc_y;

	if(g_specId == SPC_SM750)
	{
		ddk750_enableCursor(disp_crtc, 1);
		ddk750_setCursorPosition(disp_crtc, x<0?-x:x, y<0?-y:y, y<0?1:0,x<0?1:0);
	}
	else
	{
		ddk768_enableCursor(disp_crtc, 3);
		ddk768_setCursorPosition(disp_crtc, x<0?-x:x, y<0?-y:y, y<0?1:0,x<0?1:0);
	}

	//LEAVE();
}

void smi_cursor_atomic_disable(struct drm_plane *plane,
			       struct drm_plane_state *old_state)
{

	struct drm_crtc *crtc = plane->state->crtc;
	
	disp_control_t disp_crtc;
	int i, ctrl_index, max_enc;
	ctrl_index = 0;

	
	if (!old_state || !old_state->crtc)
		return;


	if(g_specId == SPC_SM750)
	{
		ddk750_enableCursor(SMI0_CTRL, 0);
		ddk750_enableCursor(SMI1_CTRL, 0);
	}
	else
	{
		ddk768_enableCursor(SMI0_CTRL, 0);
		ddk768_enableCursor(SMI1_CTRL, 0);
	}	


}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0)
static int smi_plane_prepare_fb(struct drm_plane *plane, const struct drm_plane_state *new_state)
#else
static int smi_plane_prepare_fb(struct drm_plane *plane, struct drm_plane_state *new_state)
#endif
{
	ENTER();

	struct drm_gem_object *obj;
	struct smi_bo *user_bo;
	struct drm_crtc *crtc = new_state->crtc;
	int ret;
	u64 gpu_addr;


	disp_control_t disp_crtc;
	int i, ctrl_index, max_enc;
	ctrl_index = 0;

	if(g_specId == SPC_SM750)
		max_enc = MAX_CRTC;
	else
		max_enc = MAX_ENCODER;

	for(i = 0;i < max_enc; i++)
	{
		if(crtc == smi_enc_tab[i]->crtc)
		{
			ctrl_index = i;
			break;
		}
	}
	disp_crtc = (ctrl_index == SMI1_CTRL)?SMI1_CTRL:SMI0_CTRL;

	if(ctrl_index >= MAX_CRTC)  //calc which path should we use for HDMI.
	{
		disp_crtc= (disp_control_t)smi_calc_hdmi_ctrl(g_m_connector);
	}


	if (!new_state->fb)
		return 0;

	obj = to_smi_framebuffer(new_state->fb)->obj;
	user_bo = gem_to_smi_bo(obj);
	ret = smi_bo_reserve(user_bo, false);
	if (ret)
	{
		dbg_msg("smi_bo_reserve failed\n");
		LEAVE(ret);
	}

	ret = smi_bo_pin(user_bo, TTM_PL_FLAG_VRAM, &gpu_addr);

	if (ret) {
		dbg_msg("smi_bo_pin failed\n");
		smi_bo_unreserve(user_bo);
		LEAVE(ret);
	}

	smi_bo_unreserve(user_bo);

	
	if(g_specId == SPC_SM750)
	{
		ddk750_initCursor(disp_crtc,(u32)gpu_addr,BPP16_BLACK,BPP16_WHITE,BPP16_BLUE);
	}
	else
	{
		ddk768_initCursor(disp_crtc,(u32)gpu_addr,BPP32_BLACK,BPP32_WHITE,BPP32_BLUE);
	}	

	LEAVE(0);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0)
static void smi_plane_cleanup_fb(struct drm_plane *plane, const struct drm_plane_state *old_state)
#else
static void smi_plane_cleanup_fb(struct drm_plane *plane, struct drm_plane_state *old_state)
#endif
{
	ENTER();
	struct drm_gem_object *obj;
	struct smi_bo *user_bo;
	struct drm_crtc *crtc = plane->state->crtc;
	
	if(g_specId == SPC_SM750)
	{
		ddk750_enableCursor(SMI0_CTRL, 0);
		ddk750_enableCursor(SMI1_CTRL, 0);
	}
	else
	{
		ddk768_enableCursor(SMI0_CTRL, 0);
		ddk768_enableCursor(SMI1_CTRL, 0);
	}	

	if (!plane->state->fb) {
		LEAVE();
	}
	
	obj = to_smi_framebuffer(plane->state->fb)->obj;
	user_bo = gem_to_smi_bo(obj);
	smi_bo_unpin(user_bo);	
	
	LEAVE();
}

#endif

static const uint32_t smi_cursor_plane_formats[] = {DRM_FORMAT_RGB565, DRM_FORMAT_BGR565, DRM_FORMAT_ARGB8888};

static const uint32_t smi_formats[] = {DRM_FORMAT_RGB565, DRM_FORMAT_BGR565, DRM_FORMAT_RGB888,
	DRM_FORMAT_BGR888, DRM_FORMAT_XRGB8888, DRM_FORMAT_XBGR8888,
	DRM_FORMAT_RGBA8888, DRM_FORMAT_BGRA8888, DRM_FORMAT_ARGB8888,
	DRM_FORMAT_ABGR8888 };

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,12,0)
static int smi_plane_update(struct drm_plane *plane, struct drm_crtc *crtc, struct drm_framebuffer *fb,
			int crtc_x, int crtc_y, unsigned int crtc_w, unsigned int crtc_h,
			uint32_t src_x, uint32_t src_y, uint32_t src_w, uint32_t src_h)
#else
static int smi_plane_update(struct drm_plane *plane, struct drm_crtc *crtc, struct drm_framebuffer *fb,
			int crtc_x, int crtc_y, unsigned int crtc_w, unsigned int crtc_h,
			uint32_t src_x, uint32_t src_y, uint32_t src_w, uint32_t src_h, struct drm_modeset_acquire_ctx *ctx)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,15,0)
	return 0;
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0)
	return drm_primary_helper_update(plane, crtc, fb,
			crtc_x, crtc_y, crtc_w, crtc_h, src_x, src_y, src_w, src_h);
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4,12,0)
	if (plane->type == DRM_PLANE_TYPE_PRIMARY)
		return drm_primary_helper_update(plane, crtc, fb,
			crtc_x, crtc_y, crtc_w, crtc_h, src_x, src_y, src_w, src_h);
	else
		return drm_plane_helper_update(plane, crtc, fb,
			crtc_x, crtc_y, crtc_w, crtc_h, src_x, src_y, src_w, src_h);
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	if (plane->type == DRM_PLANE_TYPE_PRIMARY)
		return drm_primary_helper_update(plane, crtc, fb,
			crtc_x, crtc_y, crtc_w, crtc_h, src_x, src_y, src_w, src_h, ctx);
	else
		return drm_plane_helper_update(plane, crtc, fb,
			crtc_x, crtc_y, crtc_w, crtc_h, src_x, src_y, src_w, src_h);
#else
	if (plane->type == DRM_PLANE_TYPE_PRIMARY)
		return drm_primary_helper_update(plane, crtc, fb,
			crtc_x, crtc_y, crtc_w, crtc_h, src_x, src_y, src_w, src_h, ctx);
	else
		return drm_plane_helper_update(plane, crtc, fb,
			crtc_x, crtc_y, crtc_w, crtc_h, src_x, src_y, src_w, src_h, ctx);
#endif
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,12,0)
static int smi_plane_disable(struct drm_plane *plane)
#else
static int smi_plane_disable(struct drm_plane *plane, struct drm_modeset_acquire_ctx *ctx)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,15,0)
	return -EINVAL;
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0)
	return drm_primary_helper_disable(plane);
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4,12,0)
	if (plane->type == DRM_PLANE_TYPE_PRIMARY)
		return drm_primary_helper_disable(plane);
	else
		return drm_plane_helper_disable(plane);
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	if (plane->type == DRM_PLANE_TYPE_PRIMARY)
		return drm_primary_helper_disable(plane, ctx);
	else
		return drm_plane_helper_disable(plane);
#else
	if (plane->type == DRM_PLANE_TYPE_PRIMARY)
		return drm_primary_helper_disable(plane, ctx);
	else
		return drm_plane_helper_disable(plane, ctx);
#endif
}

static void smi_plane_destroy(struct drm_plane *plane)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,12,0)
	smi_plane_disable(plane);
#else
	smi_plane_disable(plane, NULL);
#endif
	drm_plane_cleanup(plane);
	kfree(plane);
}

static struct drm_plane_funcs smi_plane_funcs = {
	.update_plane	= smi_plane_update,
	.disable_plane	= smi_plane_disable,
	.destroy	= smi_plane_destroy,
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,19,0)
static const struct drm_plane_helper_funcs smi_cursor_helper_funcs = {
	.atomic_check = smi_plane_atomic_check,
	.atomic_update = smi_cursor_atomic_update,
	.atomic_disable = smi_cursor_atomic_disable,
	.prepare_fb = smi_plane_prepare_fb,
	.cleanup_fb = smi_plane_cleanup_fb,
};
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,15,0)
struct drm_plane *smi_plane_init(struct smi_device *cdev,unsigned int possible_crtcs)
#else
struct drm_plane *smi_plane_init(struct smi_device *cdev,unsigned int possible_crtcs, enum drm_plane_type type)
#endif
{
	int err;
	int num_formats;
	const uint32_t *formats;
	struct drm_plane *plane;
	const struct drm_plane_funcs *funcs;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,19,0)
	const struct drm_plane_helper_funcs *helper_funcs;
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,19,0)
	funcs = &smi_plane_funcs;
	formats = smi_formats;
	num_formats = ARRAY_SIZE(smi_formats);
#else
	switch (type) {
	case DRM_PLANE_TYPE_PRIMARY:
		funcs = &smi_plane_funcs;
		formats = smi_formats;
		num_formats = ARRAY_SIZE(smi_formats);
		helper_funcs = NULL;
		break;
	case DRM_PLANE_TYPE_CURSOR:
		funcs = &smi_plane_funcs;
		formats = smi_cursor_plane_formats;
		num_formats = ARRAY_SIZE(smi_cursor_plane_formats);
		helper_funcs = &smi_cursor_helper_funcs;
		break;
	default:
		return ERR_PTR(-EINVAL);
	}
#endif
	
	plane = kzalloc(sizeof(*plane), GFP_KERNEL);
	if (!plane)
		return ERR_PTR(-ENOMEM);

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,15,0)
	err = drm_plane_init(cdev->dev, plane, possible_crtcs,
						   funcs, formats, num_formats,
						   false);
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4,5,0)
	err = drm_universal_plane_init(cdev->dev, plane, possible_crtcs,
						   funcs, formats, num_formats,
						   type);
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
	err = drm_universal_plane_init(cdev->dev, plane, possible_crtcs,
						   funcs, formats, num_formats,
						   type, NULL);
#else
	err = drm_universal_plane_init(cdev->dev, plane, possible_crtcs,
						   funcs, formats, num_formats,
						   NULL, type, NULL);
#endif

	if (err)
		goto free_plane;
	
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,19,0)
	drm_plane_helper_add(plane, helper_funcs);
#endif
	
	return plane;
	
free_plane:
	kfree(plane);
	return ERR_PTR(-EINVAL);

}


