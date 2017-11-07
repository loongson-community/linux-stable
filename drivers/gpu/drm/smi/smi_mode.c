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

#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,17,0)
#include <drm/drm_gem.h>
#include <drm/drm_plane_helper.h>
#include <drm/drm_atomic_helper.h>
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,1,0)
#include <drm/drm_probe_helper.h>
#endif
#include "smi_drv.h"
#include "hw750.h"
#include "hw768.h"
#include "ddk768/ddk768_video.h"


struct smi_crtc * smi_crtc_tab[MAX_CRTC];
struct drm_encoder * smi_enc_tab[MAX_ENCODER];

int g_m_connector = 0;//bit 0: DVI, bit 1: VGA, bit 2: HDMI.

int smi_calc_hdmi_ctrl(int m_connector)
{
		int smi_ctrl = 0;

		if(m_connector==USE_DVI_HDMI) // //vga is empty, dvi is occupied , HDMI use ctrl 1;
			smi_ctrl = 1;
		else
			smi_ctrl = 0;
			
		return smi_ctrl;

}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)

/*
 * This file contains setup code for the CRTC.
 */
static void smi_crtc_load_lut(struct drm_crtc *crtc)
{
}
#endif

/*
 * The DRM core requires DPMS functions, but they make little sense in our
 * case and so are just stubs
 */

static void smi_crtc_dpms(struct drm_crtc *crtc, int mode)
{
	ENTER();
	LEAVE();
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)

/*
 * The core passes the desired mode to the CRTC code to see whether any
 * CRTC-specific modifications need to be made to it. We're in a position
 * to just pass that straight through, so this does nothing
 */
static bool smi_crtc_mode_fixup(struct drm_crtc *crtc,
				   const struct drm_display_mode *mode,
				   struct drm_display_mode *adjusted_mode)
{
	return true;
}
#endif
 
/* smi is different - we will force move buffers out of VRAM */
static int smi_crtc_do_set_base(struct drm_crtc *crtc,
				struct drm_framebuffer *old_fb,
				int x, int y, int atomic, int dst_ctrl)
{
	int ret, pitch;
	u64 gpu_addr;
	struct smi_bo *bo;
	struct smi_framebuffer *smi_fb;
	struct smi_crtc *smi_crtc = to_smi_crtc(crtc);

	ENTER();
	if (old_fb) {
		smi_fb = to_smi_framebuffer(old_fb);
		bo = gem_to_smi_bo(smi_fb->obj);
		ret = smi_bo_reserve(bo, false);
		if (ret) {
			DRM_ERROR("failed to reserve old_fb bo\n");
		} else {
			smi_bo_unpin(bo);
			smi_bo_unreserve(bo);
		}
	}

#if LINUX_VERSION_CODE > KERNEL_VERSION(3,14,0)	
	smi_fb = to_smi_framebuffer(crtc->primary->fb);
#else
	smi_fb = to_smi_framebuffer(crtc->fb);
#endif
	bo = gem_to_smi_bo(smi_fb->obj);
	dbg_msg("bo addr:0x%x\n",bo);	
	ret = smi_bo_reserve(bo, false);
	if (ret)
	{
		dbg_msg("smi_bo_reserve failed\n");
		LEAVE(ret);
	}
	ret = smi_bo_pin(bo, TTM_PL_FLAG_VRAM, &gpu_addr);
	if (ret) {
		dbg_msg("smi_bo_pin failed\n");
		smi_bo_unreserve(bo);
		LEAVE(ret);
	}

	smi_bo_unreserve(bo);

#if LINUX_VERSION_CODE > KERNEL_VERSION(3,14,0)	
	pitch = crtc->primary->fb->pitches[0];
#else
	pitch = crtc->fb->pitches[0] ;
#endif
	// Known issue: when setting 4K+1080p or 2K+1080p, the total pitch exceeds 4096.
	// The max pitch SM768's register supports is 4096, the issue will cause screen garbage.
	int win_width = x * smi_bpp/8;
	int align_width = (win_width + 15)&~15; 	
	int align_offset = align_width - win_width;
	unsigned long base_addr = gpu_addr + y*pitch + align_width;

	
	 if(g_specId == SPC_SM750)
	{
		
		if(crtc == smi_enc_tab[0]->crtc)
		{
			hw750_set_base(SMI0_CTRL,pitch,base_addr);
		}
		if(crtc == smi_enc_tab[1]->crtc)
		{
			hw750_set_base(SMI1_CTRL,pitch,base_addr);
		}
	}
	else
	{
		hw768_set_base(dst_ctrl,pitch,base_addr);
	}

	smi_crtc->CursorOffset = align_offset/(smi_bpp/8);
	
	LEAVE(0);
}
static int smi_crtc_mode_set_base(struct drm_crtc *crtc, int x, int y,
			     struct drm_framebuffer *old_fb)
{
	int i, ctrl_index, dst_ctrl, ret, max_index;

	ENTER();
	ret = 0;
	ctrl_index = 0;
	dst_ctrl = 0;
	if(g_specId == SPC_SM750)
		max_index = 2;
	else
		max_index = MAX_ENCODER;
	for(i = 0;i < MAX_ENCODER; i++)
	{
		if(crtc == smi_enc_tab[i]->crtc)
		{
			ctrl_index = i;
			break;
		}
	}
	
	dst_ctrl = (ctrl_index == SMI1_CTRL)?SMI1_CTRL:SMI0_CTRL;

	if(ctrl_index > SMI1_CTRL)
	{
		printk("Reset HDMI base");
		dst_ctrl= smi_calc_hdmi_ctrl(g_m_connector);
	}
	dbg_msg("set base: dst[%d], con[%d]\n", dst_ctrl, ctrl_index);

	ret = smi_crtc_do_set_base(crtc, old_fb, x, y, 0, dst_ctrl);
	LEAVE(ret);
}

/*
 * The meat of this driver. The core passes us a mode and we have to program
 * it. The modesetting here is the bare minimum required to satisfy the qemu
 * emulation of this hardware, and running this against a real device is
 * likely to result in an inadequately programmed mode. We've already had
 * the opportunity to modify the mode, so whatever we receive here should
 * be something that can be correctly programmed and displayed
 */
static int smi_crtc_mode_set(struct drm_crtc *crtc,
				struct drm_display_mode *mode,
				struct drm_display_mode *adjusted_mode,
				int x, int y, struct drm_framebuffer *old_fb)
{
	u32 refresh_rate = drm_mode_vrefresh(mode);
	struct smi_device *sdev = crtc->dev->dev_private;
	logicalMode_t logicalMode;

	ENTER();
	dbg_msg("***crtc addr:0x%x\n",crtc);
	dbg_msg("x:%d,y:%d\n",x,y);
	
	dbg_msg("encode 0->crtc:[0x%x], 1->crtc:[0x%x] \n",smi_enc_tab[0]->crtc, smi_enc_tab[1]->crtc);
	dbg_msg("Printf g_m_connector = %d,  DVI [%d], VGA[%d], HDMI[%d] \n",g_m_connector, g_m_connector&0x1, g_m_connector&0x2, g_m_connector&0x4);
	
	dbg_msg("wxh:%dx%d@%dHz\n",adjusted_mode->hdisplay,adjusted_mode->vdisplay,refresh_rate);

	if(adjusted_mode->hdisplay == 3840)
		refresh_rate = 30;

	if(g_specId == SPC_SM750)
	{
		if(crtc == smi_enc_tab[0]->crtc)
		{
			logicalMode.baseAddress = 0;
			logicalMode.x = adjusted_mode->hdisplay;
			logicalMode.y = adjusted_mode->vdisplay;
			logicalMode.bpp = smi_bpp;
			logicalMode.dispCtrl = SMI0_CTRL;
			logicalMode.hz = refresh_rate;
			logicalMode.pitch = 0;

			setMode(&logicalMode);

			setDisplayControl(SMI0_CTRL, DISP_ON);           /* Turn on Primary Control */
			setPath(SMI0_PATH, SMI0_CTRL, DISP_ON);     /* Turn on Panel Path and use Primary data */
			smi_crtc_do_set_base(crtc, old_fb, x, y, 0, SMI0_CTRL);	
		}
		if(crtc == smi_enc_tab[1]->crtc)
		{
			logicalMode.baseAddress = 0;
			logicalMode.x = adjusted_mode->hdisplay;
			logicalMode.y = adjusted_mode->vdisplay;
			logicalMode.bpp = smi_bpp;
			logicalMode.dispCtrl = SMI1_CTRL;
			logicalMode.hz = refresh_rate;
			logicalMode.pitch = 0;
			setMode(&logicalMode);

			setDisplayControl(SMI1_CTRL, DISP_ON);         /* Turn on Secondary control */
			setPath(SMI1_PATH, SMI1_CTRL, DISP_ON);     /* Turn on CRT Path and use Secondary data */
			smi_crtc_do_set_base(crtc, old_fb, x, y, 0, SMI1_CTRL);
		}
		
		swPanelPowerSequence(DISP_ON, 4);                   /* Turn on Panel */
		setDAC(DISP_ON);                                    /* Turn on DAC */

#ifdef USE_HDMICHIP
		printk("HDMI set mode\n");
		sii9022xSetMode(5);
#endif

	 }
	else
	{
		int i, ctrl_index, dst_ctrl;
		ctrl_index = 0;
		dst_ctrl = 0;
		for(i = 0;i < MAX_ENCODER; i++)
		{
			if(crtc == smi_enc_tab[i]->crtc)
			{
				ctrl_index = i;
				break;
			}
		}

		dst_ctrl = (ctrl_index == SMI1_CTRL)?SMI1_CTRL:SMI0_CTRL;
		
		if(ctrl_index > SMI1_CTRL)
		{
			dst_ctrl=smi_calc_hdmi_ctrl(g_m_connector);
			dbg_msg("hdmi use channel %d\n",dst_ctrl);
	
		}
		
		logicalMode.baseAddress = 0;
		logicalMode.x = adjusted_mode->hdisplay;
		logicalMode.y = adjusted_mode->vdisplay;
		logicalMode.bpp = smi_bpp;
		logicalMode.hz = refresh_rate;
		logicalMode.pitch = 0;
		logicalMode.dispCtrl = dst_ctrl;
		ddk768_setMode(&logicalMode);
		DisableDoublePixel(0);
		DisableDoublePixel(1);
		smi_crtc_do_set_base(crtc, old_fb, x, y, 0, dst_ctrl);
		setSingleViewOn(dst_ctrl);

		if((g_m_connector & USE_HDMI)&&(ctrl_index > SMI1_CTRL))
		{
			printk("starting init HDMI!dst=[%d]\n", dst_ctrl);
			int ret=hw768_set_hdmi_mode(&logicalMode, sdev->is_hdmi);
			if (ret != 0)
			{
				printk("HDMI Mode not supported!\n");
			}
		}

		if(lvds_channel == 1)
			hw768_enable_lvds(1);
		else if(lvds_channel == 2){
			hw768_enable_lvds(2);
			EnableDoublePixel(0);
		}

		if(force_connect)
		{
			logicalMode.dispCtrl = 0;
			printk("starting init HDMI!dst=[%d]\n", dst_ctrl);
			int ret=hw768_set_hdmi_mode(&logicalMode, true);
			if (ret != 0)
			{
				printk("HDMI Mode not supported!\n");
			}
		}

	}
	LEAVE(0);
 }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,17,0)

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,12,0)
int smi_crtc_page_flip(struct drm_crtc *crtc,struct drm_framebuffer *fb,
	struct drm_pending_vblank_event *event, uint32_t page_flip_flags)
#else
int smi_crtc_page_flip(struct drm_crtc *crtc,struct drm_framebuffer *fb,
	struct drm_pending_vblank_event *event, uint32_t page_flip_flags, struct drm_modeset_acquire_ctx *ctx)
#endif
{	
	struct drm_device *dev = crtc->dev;
	unsigned long flags;
	struct drm_framebuffer *old_fb = crtc->primary->fb;
	
	smi_crtc_mode_set_base(crtc, 0, 0, old_fb);
	
	spin_lock_irqsave(&dev->event_lock, flags);
	if (event)
			drm_crtc_send_vblank_event(crtc, event);
	spin_unlock_irqrestore(&dev->event_lock, flags);
	
	crtc->primary->fb = fb;
	
	return 0;
}

#endif

/*
 * This is called before a mode is programmed. A typical use might be to
 * enable DPMS during the programming to avoid seeing intermediate stages,
 * but that's not relevant to us
 */
static void smi_crtc_prepare(struct drm_crtc *crtc)
{
}

/*
 * This is called after a mode is programmed. It should reverse anything done
 * by the prepare function
 */
static void smi_crtc_commit(struct drm_crtc *crtc)
{
}

/* Simple cleanup function */
static void smi_crtc_destroy(struct drm_crtc *crtc)
{
	struct smi_crtc *smi_crtc = to_smi_crtc(crtc);

	drm_crtc_cleanup(crtc);
	kfree(smi_crtc);
}

/* These provide the minimum set of functions required to handle a CRTC */
static const struct drm_crtc_funcs smi_crtc_funcs = {
 	.set_config = drm_crtc_helper_set_config,//kernel: deprecated. will be instead of drm_atomic_helper_set_config
	.destroy = smi_crtc_destroy,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,17,0)	
	.page_flip = smi_crtc_page_flip,
#endif
};

static const struct drm_crtc_helper_funcs smi_helper_funcs = {
	.dpms = smi_crtc_dpms,
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)		
	.mode_fixup = smi_crtc_mode_fixup,
#endif	
	.mode_set = smi_crtc_mode_set,
	.mode_set_base = smi_crtc_mode_set_base,
	.prepare = smi_crtc_prepare,
	.commit = smi_crtc_commit,
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
	.load_lut = smi_crtc_load_lut,
#endif
};


/* CRTC setup */
static struct smi_crtc * smi_crtc_init(struct drm_device *dev, int crtc_id)
{
	struct smi_device *cdev = dev->dev_private;
	struct smi_crtc *smi_crtc;
	struct drm_plane *primary, *cursor = NULL;
	int r;
	
 	smi_crtc = kzalloc(sizeof(struct smi_crtc) +
			      sizeof(struct drm_connector *),
			      GFP_KERNEL);
 	if (smi_crtc == NULL)
		return NULL;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,15,0)
	primary = smi_plane_init(cdev, 1 << crtc_id);
#else
	primary = smi_plane_init(cdev, 1 << crtc_id, DRM_PLANE_TYPE_PRIMARY);
#endif
	if (IS_ERR(primary)) {
		r = -ENOMEM;
		goto free_mem;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,2,0)
	cursor = smi_plane_init(cdev, 1 << crtc_id, DRM_PLANE_TYPE_CURSOR);
	if (IS_ERR(cursor)) {
		r = -ENOMEM;
		goto clean_primary;
	}
#endif
	smi_crtc->CursorOffset = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,15,0)
	r = drm_crtc_init(dev, &smi_crtc->base, &smi_crtc_funcs);
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4,5,0)
	r = drm_crtc_init_with_planes(dev, &smi_crtc->base, primary, cursor,
				      &smi_crtc_funcs);
#else
	r = drm_crtc_init_with_planes(dev, &smi_crtc->base, primary, cursor,
				      &smi_crtc_funcs, NULL);
#endif

	if (r)
		goto clean_cursor;
	
 	drm_crtc_helper_add(&smi_crtc->base, &smi_helper_funcs);
	return smi_crtc;

clean_cursor:
	drm_plane_cleanup(cursor);
	kfree(cursor);
clean_primary:
	drm_plane_cleanup(primary);
	kfree(primary);
free_mem:
	kfree(smi_crtc);
	return NULL;
}

/** Sets the color ramps on behalf of fbcon */
void smi_crtc_fb_gamma_set(struct drm_crtc *crtc, u16 red, u16 green,
			      u16 blue, int regno)
{
	struct smi_crtc *smi_crtc = to_smi_crtc(crtc);

	smi_crtc->lut_r[regno] = red;
	smi_crtc->lut_g[regno] = green;
	smi_crtc->lut_b[regno] = blue;
}

/** Gets the color ramps on behalf of fbcon */
void smi_crtc_fb_gamma_get(struct drm_crtc *crtc, u16 *red, u16 *green,
			      u16 *blue, int regno)
{
	struct smi_crtc *smi_crtc = to_smi_crtc(crtc);

	*red = smi_crtc->lut_r[regno];
	*green = smi_crtc->lut_g[regno];
	*blue = smi_crtc->lut_b[regno];
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
static bool smi_encoder_mode_fixup(struct drm_encoder *encoder,
				      const struct drm_display_mode *mode,
				      struct drm_display_mode *adjusted_mode)
{
	return true;
}
#endif
static void smi_encoder_mode_set(struct drm_encoder *encoder,
				struct drm_display_mode *mode,
				struct drm_display_mode *adjusted_mode)
{

}

static void smi_encoder_dpms(struct drm_encoder *encoder, int mode)
{
	int index =0;

	ENTER();
	if (encoder->encoder_type  == DRM_MODE_ENCODER_LVDS)
		index = 0;
	else if (encoder->encoder_type  == DRM_MODE_ENCODER_DAC)
		index = 1;
	else if (encoder->encoder_type  == DRM_MODE_ENCODER_TMDS)
		index = 2;
	
	dbg_msg("The current connect group = [%d], we deal with con=[%d], mode=[%s]\n", g_m_connector,index, (mode == DRM_MODE_DPMS_OFF)?"Off":"ON");
	if(g_specId == SPC_SM750)
	{	
		if (mode == DRM_MODE_DPMS_OFF) {
			dbg_msg("disable dpms, index=%d\n",index);
			setDisplayControl(index, DISP_OFF); 
		}else
		{
			setDisplayControl(index, DISP_ON); 
			swPanelPowerSequence(DISP_ON, 4); 
			dbg_msg("enable dpms ,index=%d\n",index);
		}
	}else if(g_specId == SPC_SM768)
	{
		if(encoder->encoder_type  == DRM_MODE_ENCODER_LVDS) 
		{
			if(g_m_connector == USE_VGA_HDMI||g_m_connector==USE_HDMI)
			{
				dbg_msg("DVI connector off\n");
				LEAVE(0);
			}
			dbg_msg("DVI connector: index=%d\n",index);
	
		}
		else if(encoder->encoder_type  == DRM_MODE_ENCODER_DAC)
		{
			if(g_m_connector == USE_DVI_HDMI)
			{
				dbg_msg("VGA connector off\n");
				LEAVE(0);
			}
			dbg_msg("VGA connector: index=%d\n",index);
		}
		else if(encoder->encoder_type  == DRM_MODE_ENCODER_TMDS)
		{	
			if(force_connect)
				LEAVE(0);
			if (mode == DRM_MODE_DPMS_OFF)	
				hw768_HDMI_Disable_Output();
			else
				hw768_HDMI_Enable_Output();
			if(g_m_connector == USE_DVI_HDMI){
				index = SMI1_CTRL;
			 	dbg_msg("HDMI connector: index=%d\n",index);
			}
			else if(g_m_connector == USE_VGA_HDMI || g_m_connector==USE_HDMI){
				index = SMI0_CTRL;
			 	dbg_msg("HDMI connector: index=%d\n",index);
			}else{
				dbg_msg("HDMI connector not set dpms\n");
				LEAVE(0);
			}
		}
		
		if (mode == DRM_MODE_DPMS_OFF){
			setDisplayDPMS(index, DISP_DPMS_OFF);
			ddk768_swPanelPowerSequence(index, 0, 4);
		}else{
			setDisplayDPMS(index, DISP_DPMS_ON);
			ddk768_swPanelPowerSequence(index, 1, 4);
		}

		if(lvds_channel == 2)
			EnableDoublePixel(0);
	}
	
	LEAVE();
}

static void smi_encoder_prepare(struct drm_encoder *encoder)
{
	smi_encoder_dpms(encoder, DRM_MODE_DPMS_OFF);

}

static void smi_encoder_commit(struct drm_encoder *encoder)
{
	smi_encoder_dpms(encoder, DRM_MODE_DPMS_ON);

}

void smi_encoder_destroy(struct drm_encoder *encoder)
{
	struct smi_encoder *smi_encoder = to_smi_encoder(encoder);
	drm_encoder_cleanup(encoder);
	kfree(smi_encoder);
}

static const struct drm_encoder_helper_funcs smi_encoder_helper_funcs = {
	.dpms = smi_encoder_dpms,
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
	.mode_fixup = smi_encoder_mode_fixup,
#endif
	.mode_set = smi_encoder_mode_set,
	.prepare = smi_encoder_prepare,
	.commit = smi_encoder_commit,
};

static const struct drm_encoder_funcs smi_encoder_encoder_funcs = {
	.destroy = smi_encoder_destroy,
};

static struct drm_encoder *smi_encoder_init(struct drm_device *dev, int index)
{
	struct drm_encoder *encoder;
	struct smi_encoder *smi_encoder;

	smi_encoder = kzalloc(sizeof(struct smi_encoder), GFP_KERNEL);
	if (!smi_encoder)
		return NULL;

	encoder = &smi_encoder->base;
	encoder->possible_crtcs = (1 << index);

	switch (index)
	{
		case 0:
			drm_encoder_init(dev, encoder, &smi_encoder_encoder_funcs,
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,5,0)		
			 DRM_MODE_ENCODER_LVDS);//DVI,LVDS,
#else
			 DRM_MODE_ENCODER_LVDS, NULL);
#endif	
			break;
		case 1:
			drm_encoder_init(dev, encoder, &smi_encoder_encoder_funcs,
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,5,0)		
			 DRM_MODE_ENCODER_DAC);//VGA
#else
			 DRM_MODE_ENCODER_DAC, NULL);
#endif
			break;
		case 2:
            encoder->possible_crtcs = 0x3;
			drm_encoder_init(dev, encoder, &smi_encoder_encoder_funcs,
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,5,0)		
			 DRM_MODE_ENCODER_TMDS);//HDMI.
#else
			 DRM_MODE_ENCODER_TMDS, NULL);
#endif	
			break;
		default:
			printk("error index of Connector\n");
	}

	drm_encoder_helper_add(encoder, &smi_encoder_helper_funcs);
 	return encoder;
}

int smi_connector_get_modes(struct drm_connector *connector)
{
	int ret, count;
	void *edid_buf;
	struct smi_device *sdev = connector->dev->dev_private;

	ENTER();
	dbg_msg("print connector type: [%d], DVI=%d, VGA=%d, HDMI=%d\n",
			connector->connector_type, DRM_MODE_CONNECTOR_DVII, DRM_MODE_CONNECTOR_VGA, DRM_MODE_CONNECTOR_HDMIA);
	
	if(g_specId == SPC_SM750)
	{
		if(connector->connector_type == DRM_MODE_CONNECTOR_DVII)
		{
#ifdef USE_HDMICHIP
			count = drm_add_modes_noedid(connector, 1920, 1080);
			drm_set_preferred_mode(connector, 1024, 768);
#else
			edid_buf = sdev->dvi_edid;
			ret = ddk750_edidReadMonitorEx_HW(SMI0_CTRL, edid_buf, 256, 0);
			dbg_msg("DVI edid size= %d\n",ret);
			if (ret) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,18,0)
				drm_connector_update_edid_property(connector, sdev->dvi_edid);
#else
				drm_mode_connector_update_edid_property(connector, sdev->dvi_edid);
#endif
				count = drm_add_edid_modes(connector, sdev->dvi_edid);
			}
			if (ret == 0 || count == 0) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,18,0)
				drm_connector_update_edid_property(connector, NULL);
#else
				drm_mode_connector_update_edid_property(connector, NULL);
#endif
				count = drm_add_modes_noedid(connector, 1920, 1080);
				drm_set_preferred_mode(connector, 1024, 768);
			}
#endif
		}
		if(connector->connector_type == DRM_MODE_CONNECTOR_VGA)
		{
			edid_buf = sdev->vga_edid;
			ret = ddk750_edidReadMonitorEx(SMI1_CTRL, edid_buf, 256, 0, 17, 18);

			if (ret) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,18,0)
				drm_connector_update_edid_property(connector, sdev->vga_edid);
#else
				drm_mode_connector_update_edid_property(connector, sdev->vga_edid);
#endif
				count = drm_add_edid_modes(connector, sdev->vga_edid);
			}
			if (ret == 0 || count == 0) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,18,0)
				drm_connector_update_edid_property(connector, NULL);
#else
				drm_mode_connector_update_edid_property(connector, NULL);
#endif
				count = drm_add_modes_noedid(connector, 1920, 1080);
				drm_set_preferred_mode(connector, 1024, 768);
			}

		}
	}

	else
	{ //SM768 Part
		if(connector->connector_type == DRM_MODE_CONNECTOR_DVII)
		{
			if (lvds_channel) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,18,0)
				drm_connector_update_edid_property(connector, NULL);
#else
				drm_mode_connector_update_edid_property(connector, NULL);
#endif
				count = drm_add_modes_noedid(connector, 1920, 1080);
				drm_set_preferred_mode(connector, 1920, 1080);
			} else {
				edid_buf = sdev->dvi_edid;
#ifdef HW_I2C
				ret = ddk768_edidReadMonitorExHwI2C(edid_buf, 256, 0, 0);
				dbg_msg("DVI edid size= %d\n",ret);
#else
				ddk768_edidReadMonitorEx(edid_buf, 256, 0, 30, 31); // GPIO 30,31 for DVI, HW I2C0
#endif
				if (ret) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,18,0)
					drm_connector_update_edid_property(connector, sdev->dvi_edid);
#else
					drm_mode_connector_update_edid_property(connector, sdev->dvi_edid);
#endif
					count = drm_add_edid_modes(connector, sdev->dvi_edid);
				}
				if (ret == 0 || count == 0) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,18,0)
					drm_connector_update_edid_property(connector, NULL);
#else
					drm_mode_connector_update_edid_property(connector, NULL);
#endif
					count = drm_add_modes_noedid(connector, 1920, 1080);
					drm_set_preferred_mode(connector, 1024, 768);
				}
			}
		}
		if(connector->connector_type == DRM_MODE_CONNECTOR_VGA)
		{
			edid_buf = sdev->vga_edid;
#ifdef HW_I2C		
			ret = ddk768_edidReadMonitorExHwI2C(edid_buf, 256, 0, 1);
			dbg_msg("VGA edid size= %d\n",ret);
#else
			ddk768_edidReadMonitorEx(edid_buf, 256, 0, 6, 7);//GPIO 6,7 for VGA, HW I2C1
#endif			
		       
			if (ret) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,18,0)
				drm_connector_update_edid_property(connector, sdev->vga_edid);
#else
				drm_mode_connector_update_edid_property(connector, sdev->vga_edid);
#endif
				count = drm_add_edid_modes(connector, sdev->vga_edid);
			}
			if (ret == 0 || count == 0) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,18,0)
				drm_connector_update_edid_property(connector, NULL);
#else
				drm_mode_connector_update_edid_property(connector, NULL);
#endif
				count = drm_add_modes_noedid(connector, 1920, 1080);
				drm_set_preferred_mode(connector, 1024, 768);
			}

		}
		if(connector->connector_type == DRM_MODE_CONNECTOR_HDMIA)
		{
			edid_buf = sdev->hdmi_edid;
			ret = ddk768_edidReadMonitorEx(edid_buf, 256, 0, 8, 9); //use GPIO8/9 for HDMI
			dbg_msg("HDMI edid size= %d\n",ret);

			if (ret) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,18,0)
				drm_connector_update_edid_property(connector, sdev->hdmi_edid);
#else
				drm_mode_connector_update_edid_property(connector, sdev->hdmi_edid);
#endif
				count = drm_add_edid_modes(connector, sdev->hdmi_edid);
				sdev->is_hdmi = drm_detect_hdmi_monitor(sdev->hdmi_edid);
				dbg_msg("HDMI connector is %s\n",(sdev->is_hdmi ? "HDMI monitor" : "DVI monitor"));
			}
			if (ret == 0 || count == 0) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,18,0)
				drm_connector_update_edid_property(connector, NULL);
#else
				drm_mode_connector_update_edid_property(connector, NULL);
#endif
				count = drm_add_modes_noedid(connector, 1920, 1080);
				drm_set_preferred_mode(connector, 1024, 768);
				sdev->is_hdmi = true;
			}
		}
	}
	
	LEAVE(count);
}

static int smi_connector_mode_valid(struct drm_connector *connector,
				 struct drm_display_mode *mode)
{
	u32 refresh, vrefresh = drm_mode_vrefresh(mode);
	
	/* Only support and use 30Hz or 60Hz mode currently. */
	switch (vrefresh) {
	case 30:
	case 30-1:
	case 30+1:
		refresh = 30;
		break;
	case 60:
	case 60-1:
	case 60+1:
		refresh = 60;
		break;
	default:
		return MODE_NOMODE;
	}

	/* Bandwidth: 2K-32bpp@60Hz / 4K-16bpp@60Hz / 4K-32bpp@30Hz / 8K-16bpp@30Hz */
	if ((mode->hdisplay * smi_bpp * refresh) > (1920 * 32 * 60))
		return MODE_NOMODE;

#ifdef CONFIG_CPU_LOONGSON3
	if ((mode->hdisplay == 1360) && (mode->vdisplay == 768))
		return MODE_NOMODE;
	if ((mode->hdisplay == 1366) && (mode->vdisplay == 768))
		return MODE_NOMODE;
#endif

	return MODE_OK;
}

struct drm_encoder *smi_connector_best_encoder(struct drm_connector
						  *connector)
{
	int enc_id = connector->encoder_ids[0];

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,14,0)
	struct drm_mode_object *obj;
	struct drm_encoder *encoder;

	/* pick the encoder ids */
	if (enc_id) {
		obj =
		    drm_mode_object_find(connector->dev, enc_id,
					 DRM_MODE_OBJECT_ENCODER);
		if (!obj)
			return NULL;
		encoder = obj_to_encoder(obj);
		return encoder;
	}
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0)
	if(enc_id)
		return drm_encoder_find(connector->dev, enc_id);
#else
	if(enc_id)
		return drm_encoder_find(connector->dev, NULL, enc_id);
#endif
	return NULL;
}


static enum drm_connector_status smi_connector_detect(struct drm_connector
						   *connector, bool force)
{
	if(force_connect){
		if(connector->connector_type == DRM_MODE_CONNECTOR_HDMIA){
			g_m_connector = g_m_connector&(~USE_HDMI);
			return connector_status_disconnected;
		}
		else{
			g_m_connector = USE_DVI_VGA;
			return connector_status_connected;
		}
	}
	if(g_specId == SPC_SM750)
	{		
		if(connector->connector_type == DRM_MODE_CONNECTOR_DVII)
		{

#ifdef USE_HDMICHIP
			if(sii9022xIsConnected())
				return connector_status_connected;
#endif
			if(ddk750_edidHeaderReadMonitorExHwI2C()<0)	
			{
				dbg_msg("detect DVI/Panel DO NOT connected.\n");
				return connector_status_disconnected;
			}
			else
			{
				dbg_msg("detect DVI/Panel connected.\n");
				return connector_status_connected;
			}
		}
		
		if(connector->connector_type == DRM_MODE_CONNECTOR_VGA)
		{
			if(ddk750_edidHeaderReadMonitorEx(17,18)<0)	
			{
				dbg_msg("detect CRT DO NOT connected.\n");
				return connector_status_disconnected;
			}
			else
			{
				dbg_msg("detect CRT connected.\n");
				return connector_status_connected;
			}
		}
	}
	else  //SM768 Part
	{
		if(connector->connector_type == DRM_MODE_CONNECTOR_DVII)
		{
			if (lvds_channel) {
				g_m_connector =g_m_connector | USE_DVI;
				return connector_status_connected;
			}
#ifdef HW_I2C	
			if(ddk768_edidHeaderReadMonitorExHwI2C(0)<0)						
#else
			if(ddk768_edidHeaderReadMonitorEx(30,31)<0)	
#endif
			{				
				dbg_msg("detect DVI DO NOT connected. \n");
				g_m_connector = g_m_connector & (~USE_DVI);
				return connector_status_disconnected; 
			}
			else
			{
				dbg_msg("detect DVI connected(GPIO30,31)\n");
				g_m_connector =g_m_connector |USE_DVI;
				return connector_status_connected;
			}
		}
		
		if(connector->connector_type == DRM_MODE_CONNECTOR_VGA)
		{
#ifdef HW_I2C	
			if(ddk768_edidHeaderReadMonitorExHwI2C(1)<0)						
#else
			if(ddk768_edidHeaderReadMonitorEx(6,7)<0) 
#endif
			{				
				dbg_msg("detect CRT DO NOT connected. \n");
				g_m_connector =g_m_connector&(~USE_VGA);
				return connector_status_disconnected;
			}
			else
			{
				dbg_msg("detect CRT connected(GPIO 6, 7)\n");
				g_m_connector = g_m_connector|USE_VGA;
				return connector_status_connected;
			}
		
		}
		if(connector->connector_type == DRM_MODE_CONNECTOR_HDMIA)
		{

			if (g_m_connector == USE_DVI_VGA || g_m_connector == USE_ALL){
				hw768_HDMI_Disable_Output();
				dbg_msg("set HDMI connector_status_disconnected because of VGA+DVI\n");
				g_m_connector = g_m_connector&(~USE_HDMI);
				return connector_status_disconnected;  //If VGA and DVI are both connected, disable HDMI
			}
#if 0//ndef AUDIO_EN
			if (hdmi_hotplug_detect()){
#else
			if(ddk768_edidHeaderReadMonitorEx(8,9)==0){ 
#endif
				dbg_msg("detect HDMI connected(GPIO 6,7) \n");
				g_m_connector = g_m_connector|USE_HDMI;
				return connector_status_connected; 
			}
			else{
				dbg_msg("detect HDMI DO NOT connected. \n");
				g_m_connector = g_m_connector&(~USE_HDMI);
				return connector_status_disconnected;
			}
		
		}
	}
}

static void smi_connector_destroy(struct drm_connector *connector)
{
#if KERNEL_VERSION(3, 17, 0) <= LINUX_VERSION_CODE
	drm_connector_unregister(connector);
#else
	drm_sysfs_connector_remove(connector);
#endif
	drm_connector_cleanup(connector);
	kfree(connector);
}


static const struct drm_connector_helper_funcs smi_vga_connector_helper_funcs = {
	.get_modes = smi_connector_get_modes,
	.mode_valid = smi_connector_mode_valid,
	.best_encoder = smi_connector_best_encoder,
};

static const struct drm_connector_funcs smi_vga_connector_funcs = {
	.dpms = drm_helper_connector_dpms,
	.detect = smi_connector_detect,
	.fill_modes = drm_helper_probe_single_connector_modes,
	.destroy = smi_connector_destroy,
};
static struct drm_connector *smi_connector_init(struct drm_device *dev, int index)
{
	struct drm_connector *connector;
	struct smi_connector *smi_connector;

	smi_connector = kzalloc(sizeof(struct smi_connector), GFP_KERNEL);
	if (!smi_connector)
		return NULL;

	connector = &smi_connector->base;

	switch (index)
	{
		case 0:
			drm_connector_init(dev, connector, &smi_vga_connector_funcs, DRM_MODE_CONNECTOR_DVII);
			break;
		case 1:
			drm_connector_init(dev, connector, &smi_vga_connector_funcs, DRM_MODE_CONNECTOR_VGA);
			break;
		case 2:
			drm_connector_init(dev, connector, &smi_vga_connector_funcs, DRM_MODE_CONNECTOR_HDMIA);
			break;
		default:
			printk("error index of Connector\n");
	}
	drm_connector_helper_add(connector, &smi_vga_connector_helper_funcs);
	connector->polled = DRM_CONNECTOR_POLL_CONNECT | DRM_CONNECTOR_POLL_DISCONNECT;

#if KERNEL_VERSION(3, 17, 0) <= LINUX_VERSION_CODE
	drm_connector_register(connector);
#else
	drm_sysfs_connector_add(connector);
#endif

	
	return connector;
}


int smi_modeset_init(struct smi_device *cdev)
{
	int ret, index, max_index;
	struct drm_encoder *encoder;
	struct drm_connector *connector;
	struct smi_crtc *smi_crtc;

	if(smi_bpp >= 24)
		smi_bpp = 32;

	//in multi-card with Intel, we can only use 32bpp
#ifdef PRIME
	smi_bpp = 32;
#endif

	drm_mode_config_init(cdev->dev);
	cdev->mode_info.mode_config_initialized = true;

	cdev->dev->mode_config.min_width = 0;
	cdev->dev->mode_config.min_height = 0;
	cdev->dev->mode_config.max_width = SMI_MAX_FB_WIDTH;
	cdev->dev->mode_config.max_height = SMI_MAX_FB_HEIGHT;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0)
	cdev->dev->mode_config.cursor_width = 64;
	cdev->dev->mode_config.cursor_height = 64;
#endif

	cdev->dev->mode_config.fb_base = cdev->mc.vram_base;
#ifdef PRIME
	cdev->dev->mode_config.preferred_depth = smi_bpp;
#else
	cdev->dev->mode_config.preferred_depth = min(24, smi_bpp);
#endif
	cdev->dev->mode_config.prefer_shadow = 1;

	for(index = 0; index < MAX_CRTC ; index ++)
	{
		smi_crtc = smi_crtc_init(cdev->dev, index);
		smi_crtc->crtc_index = index;
		smi_crtc_tab[index] = smi_crtc;
		dbg_msg("******smi_crtc_tab[%d]:0x%x******\n",index, smi_crtc_tab[index]);

	}
	if(g_specId == SPC_SM750)
		max_index = 2;
	else
		max_index = MAX_ENCODER;
	for(index = 0; index < max_index ; index ++)
	{
		encoder = smi_encoder_init(cdev->dev, index);
		if (!encoder) {
			DRM_ERROR("smi_encoder_tmds_init failed\n");
			return -1;
		}
		smi_enc_tab[index] = encoder;

		connector = smi_connector_init(cdev->dev, index);
		if (!connector) {
			DRM_ERROR("smi_%s_init failed\n", index?"VGA":"DVI");
			return -1;
		}

#if LINUX_VERSION_CODE > KERNEL_VERSION(4,18,0)
		drm_connector_attach_encoder(connector, encoder);
#else
		drm_mode_connector_attach_encoder(connector, encoder);
#endif
	}

	ret = smi_fbdev_init(cdev);
	if (ret) {
		DRM_ERROR("smi_fbdev_init failed\n");
		return ret;
	}

	return 0;
}

void smi_modeset_fini(struct smi_device *cdev)
{
	smi_fbdev_fini(cdev);

	if (cdev->mode_info.mode_config_initialized) {
		drm_mode_config_cleanup(cdev->dev);
		cdev->mode_info.mode_config_initialized = false;
	}
}

