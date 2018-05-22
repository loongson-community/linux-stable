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

#include <loongson-pch.h>
#include <drm/drmP.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_plane_helper.h>
#include "loongson_drv.h"

/**
 * This file contains setup code for the CRTC
 */

DEFINE_SPINLOCK(loongson_crtc_lock);

/**
 * loongson_crtc_load_lut
 *
 * @ctrc: point to a drm_crtc srtucture
 *
 * Load a LUT
 */
static void loongson_crtc_load_lut(struct drm_crtc *crtc)
{

}

/*
   This is how the framebuffer base address is stored in g200 cards:
   * Assume @offset is the gpu_addr variable of the framebuffer object
   * Then addr is the number of _pixels_ (not bytes) from the start of
     VRAM to the first pixel we want to display. (divided by 2 for 32bit
     framebuffers)
   * addr is stored in the CRTCEXT0, CRTCC and CRTCD registers
   addr<20> -> CRTCEXT0<6>
   addr<19-16> -> CRTCEXT0<3-0>
   addr<15-8> -> CRTCC<7-0>
   addr<7-0> -> CRTCD<7-0>
   CRTCEXT0 has to be programmed last to trigger an update and make the
   new addr variable take effect.
 */
static void loongson_set_start_address(struct drm_crtc *crtc, unsigned offset)
{
	volatile void *base;
	unsigned int crtc_id;
	struct loongson_drm_device *ldev = crtc->dev->dev_private;
	struct loongson_crtc *loongson_crtc = to_loongson_crtc(crtc);

	base = ldev->rmmio;
	crtc_id = loongson_crtc->crtc_id;
	DRM_DEBUG("crtc_gpu_addr = 0x%x\n", offset);
	if(crtc_id == 0){
		writel(offset, base + LS_FB_ADDR0_DVO0_REG);
		writel(offset, base + LS_FB_ADDR1_DVO0_REG);
	}else{
		writel(offset, base + LS_FB_ADDR0_DVO1_REG);
		writel(offset, base + LS_FB_ADDR1_DVO1_REG);
	}
}


/**
 * loongson_crtc_do_set_base
 *
 * @crtc: point to a drm_crtc structure
 * @fb: point to a drm_framebuffer structure
 * @x: x position on screen
 * @y: y position on screen
 * @atomic: int variable
 *
 * Ast is different - we will force move buffers out of VRAM
 */
static int loongson_crtc_do_set_base(struct drm_crtc *crtc,
				struct drm_framebuffer *fb,
				int x, int y, int atomic)
{
	int ret;
	volatile void *base;
	unsigned long gpu_addr;
	unsigned int width, depth, pitch;
	unsigned int crtc_id, crtc_address;
	struct drm_gem_object *obj;
	struct loongson_bo *bo;
	struct loongson_framebuffer *loongson_fb;
	struct loongson_drm_device *ldev = crtc->dev->dev_private;
	struct loongson_crtc *loongson_crtc = to_loongson_crtc(crtc);

	base = ldev->rmmio;
	crtc_id = loongson_crtc->crtc_id;
	ldev = crtc->dev->dev_private;
	width = crtc->primary->fb->width;
	depth = crtc->primary->fb->format->cpp[0] << 3;
	pitch = crtc->primary->fb->pitches[0];

	/* push the previous fb to system ram */
	if (!atomic && fb) {
		loongson_fb = to_loongson_framebuffer(fb);
		obj = loongson_fb->obj;
		bo = gem_to_loongson_bo(obj);
		ret = loongson_bo_reserve(bo, false);
		if (ret)
			return ret;
		loongson_bo_unpin(bo);
		loongson_bo_unreserve(bo);
	}

	DRM_DEBUG("crtc width = %d, height = %d\n", width,crtc->primary->fb->height);
	DRM_DEBUG("crtc pitches[0] = %d\n", crtc->primary->fb->pitches[0]);
	loongson_fb = to_loongson_framebuffer(crtc->primary->fb);

	if(ldev->mode_info[0].connector->base.status == connector_status_connected
		       	&& ldev->mode_info[1].connector->base.status == connector_status_connected
		       	&& loongson_fb->base.width == crtc->mode.hdisplay && loongson_fb->base.height == crtc->mode.vdisplay && x == 0 && y == 0){
		DRM_DEBUG("use clone mode\n");
		ldev->clone_mode = true;
	}else{
		DRM_DEBUG("use non-clone mode\n");
		ldev->clone_mode = false;
	}

	obj = loongson_fb->obj;
	bo = gem_to_loongson_bo(obj);

	ret = loongson_bo_reserve(bo, false);
	if (ret)
		return ret;

	ret = loongson_bo_pin(bo, TTM_PL_FLAG_VRAM, &gpu_addr);
	if (ret) {
		loongson_bo_unreserve(bo);
		return ret;
	}

	DRM_DEBUG("gpu_addr = 0x%lx\n", gpu_addr);
	ldev->fb_vram_base = gpu_addr;
	if (&ldev->lfbdev->lfb == loongson_fb) {
		/* if pushing console in kmap it */
		ret = ttm_bo_kmap(&bo->bo, 0, bo->bo.num_pages, &bo->kmap);
		if (ret)
			DRM_ERROR("failed to kmap fbcon\n");

	}
	loongson_bo_unreserve(bo);

	if (crtc_id == 0) {
		switch (depth) {
		case 12:
			writel(0x00100101, base + LS_FB_CFG_DVO0_REG);
			writel((width * 2 + 255) & ~255, base + LS_FB_STRI_DVO0_REG);
			crtc_address = (u32)gpu_addr + y * pitch + ALIGN(x, 64) * 2;
			break;
		case 15:
			writel(0x00100102, base + LS_FB_CFG_DVO0_REG);
			writel((width * 2 + 255) & ~255, base + LS_FB_STRI_DVO0_REG);
			crtc_address = (u32)gpu_addr + y * pitch + ALIGN(x, 64) * 2;
			break;
		case 16:
			writel(0x00100103, base + LS_FB_CFG_DVO0_REG);
			writel((width * 2 + 255) & ~255, base + LS_FB_STRI_DVO0_REG);
			crtc_address = (u32)gpu_addr + y * pitch + ALIGN(x, 64) * 2;
			break;
		case 24:
		case 32:
		default:
			writel(0x00100104, base + LS_FB_CFG_DVO0_REG);
			writel(crtc->primary->fb->pitches[0], base + LS_FB_STRI_DVO0_REG);
			crtc_address = (u32)gpu_addr + y * pitch + ALIGN(x, 64) * 4;
			break;
		}

	} else {
		if (ldev->clone_mode == false) {
			switch (depth) {
			case 12:
				writel(0x00100101, base + LS_FB_CFG_DVO1_REG);
				writel((width * 2 + 255) & ~255, base + LS_FB_STRI_DVO1_REG);
				crtc_address = (u32)gpu_addr + y * pitch + ALIGN(x, 64) * 2;
				break;
			case 15:
				writel(0x00100102, base + LS_FB_CFG_DVO1_REG);
				writel((width * 2 + 255) & ~255, base + LS_FB_STRI_DVO1_REG);
				crtc_address = (u32)gpu_addr + y * pitch + ALIGN(x, 64) * 2;
				break;
			case 16:
				writel(0x00100103, base + LS_FB_CFG_DVO1_REG);
				writel((width * 2 + 255) & ~255, base + LS_FB_STRI_DVO1_REG);
				crtc_address = (u32)gpu_addr + y * pitch + ALIGN(x, 64) * 2;
				break;
			case 24:
			case 32:
			default:
				writel(0x00100104, base + LS_FB_CFG_DVO1_REG);
				writel(crtc->primary->fb->pitches[0], base + LS_FB_STRI_DVO1_REG);
				crtc_address = (u32)gpu_addr + y * pitch + ALIGN(x, 64) * 4;
				break;
			}
		} else {
			switch (depth) {
			case 12:
				writel(0x00100301, base + LS_FB_CFG_DVO1_REG);
				writel((width * 2 + 255) & ~255, base + LS_FB_STRI_DVO1_REG);
				crtc_address = (u32)gpu_addr + y * pitch + ALIGN(x, 64) * 2;
				break;
			case 15:
				writel(0x00100302, base + LS_FB_CFG_DVO1_REG);
				writel((width * 2 + 255) & ~255, base + LS_FB_STRI_DVO1_REG);
				crtc_address = (u32)gpu_addr + y * pitch + ALIGN(x, 64) * 2;
				break;
			case 16:
				writel(0x00100303, base + LS_FB_CFG_DVO1_REG);
				writel((width * 2 + 255) & ~255, base + LS_FB_STRI_DVO1_REG);
				crtc_address = (u32)gpu_addr + y * pitch + ALIGN(x, 64) * 2;
				break;
			case 24:
			case 32:
			default:
				writel(0x00100304, base + LS_FB_CFG_DVO1_REG);
				writel(crtc->primary->fb->pitches[0], base + LS_FB_STRI_DVO1_REG);
				crtc_address = (u32)gpu_addr + y * pitch + ALIGN(x, 64) * 4;
				break;
			}
		}
	}

	mdelay(10); /* Wait config registers to be stable */
	loongson_set_start_address(crtc, (u32)crtc_address);
	ldev->cursor_crtc_id = ldev->num_crtc;
	ldev->cursor_showed = false;

	return 0;
}


/**
 * loongson_crtc_mode_set_base
 *
 * @crtc: point to a drm_crtc structure
 * @old_fb: point to a drm_crtc structure
 *
 * Transfer the function which is loongson_crtc_do_set_base,and used by
 * the legacy CRTC helpers to set a new framebuffer and scanout position
 */
static int loongson_crtc_mode_set_base(struct drm_crtc *crtc, int x, int y,
				  struct drm_framebuffer *old_fb)
{
	return loongson_crtc_do_set_base(crtc, old_fb, x, y, 0);
}

#define PLL_REF_CLK_MHZ    100
#define PCLK_PRECISION_INDICATOR 10000

/**
 * ls2h_cal_freq
 *
 * @pixclock: unsigned int
 *
 * Calculate frequency
 */
static unsigned int ls2h_cal_freq(unsigned int pixclock)
{
	unsigned int pstdiv, pll_ldf, pll_odf, pll_idf;
	unsigned int div = 0, ldf = 0, odf = 0, idf = 0, fref = 0;
	unsigned int min = 100000, a, fvco, fvcof, b;

	for (pstdiv = 1; pstdiv < 32; pstdiv++) {
		a = pixclock * pstdiv;
		for (pll_odf = 1; pll_odf <= 8; pll_odf *= 2) {
			fvco = a * pll_odf;
			if((fvco < 600000) || (fvco > 1800000))
				continue;
			for ( pll_idf = 1; pll_idf < 8; pll_idf++) {
				fref = 100000 / pll_idf;
				for ( pll_ldf = 8; pll_ldf < 256;
						pll_ldf++) {
					fvcof = fref * 2 * pll_ldf;
					if ((fvcof < 600000) ||
						(fvcof > 1800000))
						continue;
					b = (fvcof > fvco)?
						(fvcof - fvco):
						(fvco -fvcof);
					if (b < min) {
						min = b;
						div = pstdiv;
						ldf = pll_ldf;
						odf = pll_odf;
						idf = pll_idf;
					}
				}
			}
		}
	}

	pll_odf = (odf == 8)? 3 : (odf == 4)? 2 : (odf == 2)? 1 : 0;

	return (div << 24) | (ldf << 16) | (pll_odf << 5) | (idf << 2);
}

/**
 * ls7a_cal_freq
 *
 * @pixclock: unsigned int
 * @pll_config: point to the pix_pll structure
 *
 * Calculate frequency
 */
static unsigned int ls7a_cal_freq(unsigned int pixclock, struct pix_pll *pll_config)
{
	int i, j, loopc_offset;
	unsigned int refc_set[] = {4, 5, 3};
	unsigned int prec_set[] = {1, 5, 10, 50, 100};   /*in 1/PCLK_PRECISION_INDICATOR*/
	unsigned int pstdiv, loopc, refc;
	unsigned int precision_req, precision;
	unsigned int loopc_min, loopc_max, loopc_mid;
	unsigned long long real_dvo, req_dvo;

	/*try precision from high to low*/
	for (j = 0; j < sizeof(prec_set)/sizeof(int); j++){
		precision_req = prec_set[j];

		/*try each refc*/
		for (i = 0; i < sizeof(refc_set)/sizeof(int); i++) {
			refc = refc_set[i];
			loopc_min = (1200 / PLL_REF_CLK_MHZ) * refc;  /*1200 / (PLL_REF_CLK_MHZ / refc)*/
			loopc_max = (3200 / PLL_REF_CLK_MHZ) * refc;  /*3200 / (PLL_REF_CLK_MHZ / refc)*/
			loopc_mid = (2200 / PLL_REF_CLK_MHZ) * refc;  /*(loopc_min + loopc_max) / 2;*/
			loopc_offset = -1;

			/*try each loopc*/
			for (loopc = loopc_mid; (loopc <= loopc_max) && (loopc >= loopc_min); loopc += loopc_offset) {
				if(loopc_offset < 0)
					loopc_offset = -loopc_offset;
				else
					loopc_offset = -(loopc_offset+1);

				pstdiv = loopc * PLL_REF_CLK_MHZ * 1000 / refc / pixclock;
				if((pstdiv > 127) || (pstdiv < 1))
					continue;

				/*real_freq is float type which is not available, but read_freq * pstdiv is available.*/
				req_dvo  = (pixclock * pstdiv);
				real_dvo = (loopc * PLL_REF_CLK_MHZ * 1000 / refc);
				precision = abs(real_dvo * PCLK_PRECISION_INDICATOR / req_dvo - PCLK_PRECISION_INDICATOR);

				if(precision < precision_req){
					pll_config->l2_div = pstdiv;
					pll_config->l1_loopc = loopc;
					pll_config->l1_frefc = refc;
					if(j > 1)
						printk("Warning: PIX clock precision degraded to %d / %d\n", precision_req, PCLK_PRECISION_INDICATOR);
					return 1;
				}
			}
		}
	}
	return 0;
}

/**
 * ls2h_config_pll
 *
 * @pll_base: represent a long type
 * @out: value to be written to register
 *
 * Config pll apply to ls2h
 */
static void ls2h_config_pll(void *pll_base, unsigned int out)
{
	/* change to refclk */
	writel(0, pll_base + 4);
	/* reset pstiev */
	writel((out | 0xc0000000), pll_base);
	/* wait 10ms */
	mdelay(10);
	/* set pstdiv */
	writel((out | 0x80000000), pll_base);
	/* wait 10ms */
	mdelay(10);
	/* pll_powerdown */
	writel((out | 0x00000080), pll_base);
	/* wait 10ms */
	mdelay(10);
	/* pll_powerup set pll */
	writel(out, pll_base);
	/* wait pll_lock */
	while ((readl(LS2H_CHIP_SAMP0_REG) & 0x00001800) != 0x00001800)
		cpu_relax();
	/* change to pllclk */
	writel(0x1, pll_base + 4);
}

/**
 * ls7a_config_pll
 *
 * @pll_base: represent a long type
 * @pll_cfg: point to the pix_pll srtucture
 *
 * Config pll apply to ls7a
 */
static void ls7a_config_pll(void *pll_base, struct pix_pll *pll_cfg)
{
	unsigned long val;

	/* clear sel_pll_out0 */
	val = readl(pll_base + 0x4);
	val &= ~(1UL << 8);
	writel(val, pll_base + 0x4);
	/* set pll_pd */
	val = readl(pll_base + 0x4);
	val |= (1UL << 13);
	writel(val, pll_base + 0x4);
	/* clear set_pll_param */
	val = readl(pll_base + 0x4);
	val &= ~(1UL << 11);
	writel(val, pll_base + 0x4);
	/* clear old value & config new value */
	val = readl(pll_base + 0x4);
	val &= ~(0x7fUL << 0);
	val |= (pll_cfg->l1_frefc << 0); /* refc */
	writel(val, pll_base + 0x4);
	val = readl(pll_base + 0x0);
	val &= ~(0x7fUL << 0);
	val |= (pll_cfg->l2_div << 0);   /* div */
	val &= ~(0x1ffUL << 21);
	val |= (pll_cfg->l1_loopc << 21);/* loopc */
	writel(val, pll_base + 0x0);
	/* set set_pll_param */
	val = readl(pll_base + 0x4);
	val |= (1UL << 11);
	writel(val, pll_base + 0x4);
	/* clear pll_pd */
	val = readl(pll_base + 0x4);
	val &= ~(1UL << 13);
	writel(val, pll_base + 0x4);
	/* wait pll lock */
	while(!(readl(pll_base + 0x4) & 0x80))
		cpu_relax();
	/* set sel_pll_out0 */
	val = readl(pll_base + 0x4);
	val |= (1UL << 8);
	writel(val, pll_base + 0x4);
}

static void config_pll(int id, unsigned int pix_freq)
{
	unsigned int out;
	struct pix_pll pll_cfg;

	switch (loongson_pch->type) {
	case LS2H:
		out = ls2h_cal_freq(pix_freq);
		if (id == 0)
			ls2h_config_pll(LS2H_PIX0_PLL, out);
		else
			ls2h_config_pll(LS2H_PIX1_PLL, out);
		break;
	case LS7A:
		out = ls7a_cal_freq(pix_freq, &pll_cfg);
		if (id == 0)
			ls7a_config_pll(LS7A_PIX0_PLL, &pll_cfg);
		else
			ls7a_config_pll(LS7A_PIX1_PLL, &pll_cfg);
	default: /* No RS780E's case */
		break;
	}
}

/**
 * loongson_crtc_mode_set
 *
 * @crtc: point to the drm_crtc structure
 * @mode: represent a display mode
 * @adjusted_mode: point to the drm_display_mode structure
 * @old_fb: point to the drm_framebuffer structure
 *
 * Used by the legacy CRTC helpers to set a new mode
 */
static int loongson_crtc_mode_set(struct drm_crtc *crtc,
				struct drm_display_mode *mode,
				struct drm_display_mode *adjusted_mode,
				int x, int y, struct drm_framebuffer *old_fb)
{
	volatile void *base;
	unsigned int hr, hss, hse, hfl;
	unsigned int vr, vss, vse, vfl;
	unsigned int crtc_id, depth, pix_freq;
	struct drm_device *dev = crtc->dev;
	struct loongson_drm_device *ldev = dev->dev_private;
	struct loongson_crtc *loongson_crtc = to_loongson_crtc(crtc);

	base = ldev->rmmio;
	crtc_id = loongson_crtc->crtc_id;

	hr	= mode->hdisplay;
	hss	= mode->hsync_start;
	hse	= mode->hsync_end;
	hfl	= mode->htotal;

	vr	= mode->vdisplay;
	vss	= mode->vsync_start;
	vse	= mode->vsync_end;
	vfl	= mode->vtotal;

	pix_freq = mode->clock;
	depth = crtc->primary->fb->format->cpp[0] << 3;

	DRM_DEBUG("fb width = %d, height = %d\n", crtc->primary->fb->width, crtc->primary->fb->height);
	DRM_DEBUG("crtc_id = %d, hr = %d, hss = %d, hse = %d, hfl = %d, vr = %d, vss = %d, vse = %d, vfl = %d, depth = %d, pix_freq = %d, x = %d, y = %d\n",
			crtc_id, hr, hss, hse, hfl, vr, vss, vse, vfl, depth, pix_freq, x, y);

	loongson_crtc->width = hr;
	loongson_crtc->height = vr;
	
	if (crtc_id == 0) {
		config_pll(0, pix_freq);
	
		loongson_crtc_do_set_base(crtc, old_fb, x, y, 0);
	
		/* These 4 lines cause out of range, because
		 * the hfl hss vfl vss are different with BIOS vgamode cfg.
		 * So the refresh freq in kernel and refresh freq in BIOS are different.
		 * */
		writel(0, base + LS_FB_DITCFG_DVO0_REG);
		writel(0, base + LS_FB_DITTAB_LO_DVO0_REG);
		writel(0, base + LS_FB_DITTAB_HI_DVO0_REG);
		writel(0x80001311, base + LS_FB_PANCFG_DVO0_REG);
		writel(0x00000000, base + LS_FB_PANTIM_DVO0_REG);

		writel((hfl << 16) | hr, base + LS_FB_HDISPLAY_DVO0_REG);
		writel(0x40000000 | (hse << 16) | hss, base + LS_FB_HSYNC_DVO0_REG);
		writel((vfl << 16) | vr, base + LS_FB_VDISPLAY_DVO0_REG);
		writel(0x40000000 | (vse << 16) | vss, base + LS_FB_VSYNC_DVO0_REG);
	}else{
		config_pll(1, pix_freq);
	
		loongson_crtc_do_set_base(crtc, old_fb, x, y, 0);
	
		/* These 4 lines cause out of range, because
		 * the hfl hss vfl vss are different with BIOS vgamode cfg.
		 * So the refresh freq in kernel and refresh freq in BIOS are different.
		 * */
		writel(0, base + LS_FB_DITCFG_DVO1_REG);
		writel(0, base + LS_FB_DITTAB_LO_DVO1_REG);
		writel(0, base + LS_FB_DITTAB_HI_DVO1_REG);
		writel(0x80001311, base + LS_FB_PANCFG_DVO1_REG);
		writel(0x00000000, base + LS_FB_PANTIM_DVO1_REG);

		writel((hfl << 16) | hr, base + LS_FB_HDISPLAY_DVO1_REG);
		writel(0x40000000 | (hse << 16) | hss, base + LS_FB_HSYNC_DVO1_REG);
		writel((vfl << 16) | vr,base + LS_FB_VDISPLAY_DVO1_REG);
		writel(0x40000000 | (vse << 16) | vss, base + LS_FB_VSYNC_DVO1_REG);
	}
	ldev->cursor_crtc_id = ldev->num_crtc;
	ldev->cursor_showed = false;
	return 0;
}

/**
 * loongson_crtc_dpms
 *
 * @crtc: point to the drm_crtc structure
 * @mode: represent mode
 *
 * According to mode,represent the power levels on the CRTC
 */
static void loongson_crtc_dpms(struct drm_crtc *crtc, int mode)
{
	volatile void *base;
	unsigned int crtc_id, val;
 	struct drm_device *dev = crtc->dev;
 	struct loongson_drm_device *ldev = dev->dev_private;
 	struct loongson_crtc *loongson_crtc = to_loongson_crtc(crtc);

	if (ldev->inited == false || ldev->clone_mode == true)
		return;

	base = ldev->rmmio;
 	crtc_id = loongson_crtc->crtc_id;

	switch(mode) {
	case DRM_MODE_DPMS_ON:
		if (crtc_id == 0) {
			val = readl(base + LS_FB_CFG_DVO0_REG);
			val |= (1 << 8);
			writel(val, base + LS_FB_CFG_DVO0_REG);
		} else {
			val = readl(base + LS_FB_CFG_DVO1_REG);
			val |= (1 << 8);
			writel(val, base + LS_FB_CFG_DVO1_REG);
		}
		loongson_crtc->enabled = true;
		break;
	case DRM_MODE_DPMS_OFF:
	case DRM_MODE_DPMS_STANDBY:
	case DRM_MODE_DPMS_SUSPEND:
		if (crtc_id == 0) {
			val = readl(base + LS_FB_CFG_DVO0_REG);
			val &= ~(1 << 8);
			writel(val, base + LS_FB_CFG_DVO0_REG);
		} else {
			val = readl(base + LS_FB_CFG_DVO1_REG);
			val &= ~(1 << 8);
			writel(val, base + LS_FB_CFG_DVO1_REG);
		}
		loongson_crtc->enabled = false;
		break;
	}
}

/**
 * loongson_crtc_prepare
 *   
 * @crtc: point to a drm_crtc structure
 *     
 * This is called before a mode is programmed. A typical use might be to
 * enable DPMS during the programming to avoid seeing intermediate stages
 */
static void loongson_crtc_prepare(struct drm_crtc *crtc)
{
	struct drm_crtc *crtci;
	struct drm_device *dev = crtc->dev;

	/*
	 * The hardware wedges sometimes if you reconfigure one CRTC
	 * whilst another is running
	 */
	DRM_DEBUG("loongson_crtc_prepare\n");
	list_for_each_entry(crtci, &dev->mode_config.crtc_list, head) {
		loongson_crtc_dpms(crtci, DRM_MODE_DPMS_OFF);
	}
}


/**
 * loongson_crtc_commit
 *
 * @crtc: point to the drm_crtc structure
 *
 * Commit the new mode on the CRTC after a modeset.This is called after
 * a mode is programmed. It should reverse anything done by the prepare function
 */
static void loongson_crtc_commit(struct drm_crtc *crtc)
{
	struct drm_crtc *crtci;
	struct drm_device *dev = crtc->dev;

	DRM_DEBUG("loongson_crtc_commit\n");
	list_for_each_entry(crtci, &dev->mode_config.crtc_list, head) {
		if (crtci->enabled)
			loongson_crtc_dpms(crtci, DRM_MODE_DPMS_ON);
	}
}


/**
 * loongson_crtc_destroy
 *
 * @crtc: pointer to a drm_crtc struct
 *
 * Destory the CRTC when not needed anymore,and transfer the drm_crtc_cleanup
 * function,the function drm_crtc_cleanup() cleans up @crtc and removes it
 * from the DRM mode setting core.Note that the function drm_crtc_cleanup()
 * does not free the structure itself.
 */
static void loongson_crtc_destroy(struct drm_crtc *crtc)
{
	struct loongson_crtc *loongson_crtc = to_loongson_crtc(crtc);

	drm_crtc_cleanup(crtc);
	kfree(loongson_crtc);
}

/**
 * loongosn_crtc_disable
 *
 * @crtc: DRM CRTC
 *
 * Used to shut down CRTC
 */
static void loongson_crtc_disable(struct drm_crtc *crtc)
{
	int ret;
	DRM_DEBUG_KMS("\n");
	loongson_crtc_dpms(crtc, DRM_MODE_DPMS_OFF);
	if (crtc->primary->fb) {
		struct loongson_framebuffer *mga_fb = to_loongson_framebuffer(crtc->primary->fb);
		struct drm_gem_object *obj = mga_fb->obj;
		struct loongson_bo *bo = gem_to_loongson_bo(obj);
		ret = loongson_bo_reserve(bo, false);
		if (ret)
			return;
		loongson_bo_push_sysram(bo);
		loongson_bo_unreserve(bo);
	}
	crtc->primary->fb = NULL;
}


/**
 * These provide the minimum set of functions required to handle a CRTC
 * Each driver is responsible for filling out this structure at startup time
 *
 * The drm_crtc_funcs structure is the central CRTC management structure
 * in the DRM. Each CRTC controls one or more connectors
 */
static const struct drm_crtc_funcs loongson_crtc_funcs = {
	.cursor_set2 = loongson_crtc_cursor_set2,
	.cursor_move = loongson_crtc_cursor_move,
	.set_config = drm_crtc_helper_set_config,
	.destroy = loongson_crtc_destroy,
};

/**
 * These provide the minimum set of functions required to handle a CRTC
 *   
 * The drm_crtc_helper_funcs is a helper operations for CRTC
 */
static const struct drm_crtc_helper_funcs loongson_helper_funcs = {
	.disable = loongson_crtc_disable,
	.dpms = loongson_crtc_dpms,
	.mode_set = loongson_crtc_mode_set,
	.mode_set_base = loongson_crtc_mode_set_base,
	.prepare = loongson_crtc_prepare,
	.commit = loongson_crtc_commit,
};


/**
 * loongosn_crtc_init
 *
 * @ldev: point to the loongson_drm_device structure
 *
 * Init CRTC
 */
void loongson_crtc_init(struct loongson_drm_device *ldev)
{
	struct loongson_crtc *ls_crtc;
	int i;

	for(i=0;i<ldev->vbios->crtc_num;i++){
		ls_crtc = kzalloc(sizeof(struct loongson_crtc) +
				      (1 * sizeof(struct drm_connector *)),
				      GFP_KERNEL);

		if (ls_crtc == NULL)
			return;
		ls_crtc->crtc_id = ldev->crtc_vbios[i]->crtc_id;
		drm_crtc_init(ldev->dev, &ls_crtc->base, &loongson_crtc_funcs);

		ldev->mode_info[i].crtc = ls_crtc;

		drm_crtc_helper_add(&ls_crtc->base, &loongson_helper_funcs);
	}
}


