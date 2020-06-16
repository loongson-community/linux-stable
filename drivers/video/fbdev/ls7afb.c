/*
 *  linux/drivers/video/ls7a_fb.c -- Virtual frame buffer device
 *
 *  This file is subject to the terms and conditions of the GNU General Public
 *  License. See the file COPYING in the main directory of this archive for
 *  more details.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/console.h>
#include <linux/dma-mapping.h>
#include <linux/i2c.h>
#include <linux/pci.h>
#include <linux/pci_ids.h>

#include <linux/fb.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <asm/addrspace.h>

#include <loongson-pch.h>
#include "edid.h"

#ifdef LS7A_FB_DEBUG
#define LS7A_DEBUG(frm, arg...)	\
	printk("ls7a_fb: %s %d: "frm, __func__, __LINE__, ##arg);
#else
#define LS7A_DEBUG(frm, arg...)
#endif /* LS7A_FB_DEBUG */

#define ON	1
#define OFF	0

#define CUR_WIDTH_SIZE		32
#define CUR_HEIGHT_SIZE		32
#define DEFAULT_BITS_PER_PIXEL	32

#define HT1LO_PCICFG_BASE		0x1a000000
#define LS7A_PCH_CFG_SPACE_REG		(TO_UNCAC(HT1LO_PCICFG_BASE)|0x0000a810)
#define LS7A_PCH_CFG_REG_BASE		((*(volatile unsigned int *)(LS7A_PCH_CFG_SPACE_REG))&0xfffffff0)

#define LS7A_PIX0_PLL			(void *)TO_UNCAC(LS7A_PCH_CFG_REG_BASE + 0x04b0)
#define LS7A_PIX1_PLL			(void *)TO_UNCAC(LS7A_PCH_CFG_REG_BASE + 0x04c0)

u64 ls7afb_mem;
u32 ls7afb_dma;
EXPORT_SYMBOL_GPL(ls7afb_dma);
EXPORT_SYMBOL_GPL(ls7afb_mem);

#define LS7AFB_OFFSET	(0xf << 20)
#define LS7AFB_GPU_MASK	(~(0xffUL << 56))
u64 ls7a_cursor_mem;
u32 ls7a_cursor_dma;
u64 ls7a_fb_mem;
u64 ls7a_phy_addr;
u32 ls7a_fb_dma;

#define DEFAULT_CURSOR_MEM		ls7a_cursor_mem
#define DEFAULT_CURSOR_DMA		ls7a_cursor_dma
#define DEFAULT_FB_MEM			ls7a_fb_mem
#define DEFAULT_PHY_ADDR		ls7a_phy_addr
#define DEFAULT_FB_DMA			ls7a_fb_dma

#define LS7A_FB_CFG_DVO0_REG		(0x1240)
#define LS7A_FB_CFG_DVO1_REG		(0x1250)
#define LS7A_FB_ADDR0_DVO0_REG		(0x1260)
#define LS7A_FB_ADDR0_DVO1_REG		(0x1270)
#define LS7A_FB_STRI_DVO0_REG		(0x1280)
#define LS7A_FB_STRI_DVO1_REG		(0x1290)
#define LS7A_FB_DITCFG_DVO0_REG		(0x1360)
#define LS7A_FB_DITCFG_DVO1_REG		(0x1370)
#define LS7A_FB_DITTAB_LO_DVO0_REG	(0x1380)
#define LS7A_FB_DITTAB_LO_DVO1_REG	(0x1390)
#define LS7A_FB_DITTAB_HI_DVO0_REG	(0x13a0)
#define LS7A_FB_DITTAB_HI_DVO1_REG	(0x13b0)
#define LS7A_FB_PANCFG_DVO0_REG		(0x13c0)
#define LS7A_FB_PANCFG_DVO1_REG		(0x13d0)
#define LS7A_FB_PANTIM_DVO0_REG		(0x13e0)
#define LS7A_FB_PANTIM_DVO1_REG		(0x13f0)
#define LS7A_FB_HDISPLAY_DVO0_REG	(0x1400)
#define LS7A_FB_HDISPLAY_DVO1_REG	(0x1410)
#define LS7A_FB_HSYNC_DVO0_REG		(0x1420)
#define LS7A_FB_HSYNC_DVO1_REG		(0x1430)
#define LS7A_FB_VDISPLAY_DVO0_REG	(0x1480)
#define LS7A_FB_VDISPLAY_DVO1_REG	(0x1490)
#define LS7A_FB_VSYNC_DVO0_REG		(0x14a0)
#define LS7A_FB_VSYNC_DVO1_REG		(0x14b0)
#define LS7A_FB_GAMINDEX_DVO0_REG	(0x14e0)
#define LS7A_FB_GAMINDEX_DVO1_REG	(0x14f0)
#define LS7A_FB_GAMDATA_DVO0_REG	(0x1500)
#define LS7A_FB_GAMDATA_DVO1_REG	(0x1510)
#define LS7A_FB_CUR_CFG_REG		(0x1520)
#define LS7A_FB_CUR_ADDR_REG		(0x1530)
#define LS7A_FB_CUR_LOC_ADDR_REG	(0x1540)
#define LS7A_FB_CUR_BACK_REG		(0x1550)
#define LS7A_FB_CUR_FORE_REG		(0x1560)
#define LS7A_FB_INT_REG			(0x1570)
#define LS7A_FB_ADDR1_DVO0_REG		(0x1580)
#define LS7A_FB_ADDR1_DVO1_REG		(0x1590)
#define LS7A_FB_DAC_CTRL_REG		(0x1600)
#define LS7A_FB_DVO_OUTPUT_REG		(0x1630)

struct pix_pll {
	unsigned int l2_div;
	unsigned int l1_loopc;
	unsigned int l1_frefc;
};

static struct eep_info {
	struct i2c_adapter *adapter;
	unsigned short addr;
} eeprom_info[2];

struct ls7a_fb_par {
	struct pci_dev *pdev;
	struct fb_info *fb_info;
	volatile void *reg_base;
	unsigned int irq;
	unsigned int htotal;
	unsigned int vtotal;
	u8 *edid;
};

static bool edid_flag = 0;
static char *mode_option = NULL;
static void *cursor_mem;
static unsigned int cursor_size = 0x1000;
static void *videomemory;
static dma_addr_t video_dma;
static dma_addr_t cursor_dma;

static u_long videomemorysize = 0;
module_param(videomemorysize, ulong, 0);

static DEFINE_SPINLOCK(fb_lock);
static void config_pll(volatile void *pll_base, struct pix_pll *pll_cfg);

static struct fb_var_screeninfo ls7a_fb_default = {
	.xres		= 1280,
	.yres		= 1024,
	.xres_virtual	= 1280,
	.yres_virtual	= 1024,
	.xoffset	= 0,
	.yoffset	= 0,
	.bits_per_pixel = DEFAULT_BITS_PER_PIXEL,
	.red		= { 11, 5 ,0},
	.green		= { 5, 6, 0 },
	.blue		= { 0, 5, 0 },
	.activate	= FB_ACTIVATE_NOW,
	.height		= -1,
	.width		= -1,
	.pixclock	= 9259,
	.left_margin	= 248,
	.right_margin	= 48,
	.upper_margin	= 38,
	.lower_margin	= 1,
	.hsync_len	= 112,
	.vsync_len	= 3,
	.sync		= 3,
	.vmode =	FB_VMODE_NONINTERLACED,
};
static struct fb_fix_screeninfo ls7a_fb_fix = {
	.id =		"LS7A FB",
	.type =		FB_TYPE_PACKED_PIXELS,
	.visual =	FB_VISUAL_TRUECOLOR,
	.xpanstep =	1,
	.ypanstep =	1,
	.ywrapstep =	1,
	.accel =	FB_ACCEL_NONE,
};

static bool ls7a_fb_enable = 0;	/* disabled by default */
module_param(ls7a_fb_enable, bool, 0);

/*
 *  Internal routines
 */

static u_long get_line_length(int xres_virtual, int bpp)
{
	u_long length;

	length = xres_virtual * bpp;
	length = (length + 31) & ~31;
	length >>= 3;
	return (length);
}

/*
 *  Setting the video mode has been split into two parts.
 *  First part, xxxfb_check_var, must not write anything
 *  to hardware, it should only verify and adjust var.
 *  This means it doesn't alter par but it does use hardware
 *  data from it to check this var.
 */

static int ls7a_fb_check_var(struct fb_var_screeninfo *var,
			 struct fb_info *info)
{
	u_long line_length;

	/*
	 *  FB_VMODE_CONUPDATE and FB_VMODE_SMOOTH_XPAN are equal!
	 *  as FB_VMODE_SMOOTH_XPAN is only used internally
	 */

	if (var->vmode & FB_VMODE_CONUPDATE) {
		var->vmode |= FB_VMODE_YWRAP;
		var->xoffset = info->var.xoffset;
		var->yoffset = info->var.yoffset;
	}

	/*
	 *  Some very basic checks
	 */
	if (!var->xres)
		var->xres = 640;
	if (!var->yres)
		var->yres = 480;
	if (var->xres > var->xres_virtual)
		var->xres_virtual = var->xres;
	if (var->yres > var->yres_virtual)
		var->yres_virtual = var->yres;
	if (var->bits_per_pixel <= 1)
		var->bits_per_pixel = 1;
	else if (var->bits_per_pixel <= 8)
		var->bits_per_pixel = 8;
	else if (var->bits_per_pixel <= 16)
		var->bits_per_pixel = 16;
	else if (var->bits_per_pixel <= 24)
		var->bits_per_pixel = 24;
	else if (var->bits_per_pixel <= 32)
		var->bits_per_pixel = 32;
	else
		return -EINVAL;

	if (var->xres_virtual < var->xoffset + var->xres)
		var->xres_virtual = var->xoffset + var->xres;
	if (var->yres_virtual < var->yoffset + var->yres)
		var->yres_virtual = var->yoffset + var->yres;

	/*
	 *  Memory limit
	 */
	line_length =
		get_line_length(var->xres_virtual, var->bits_per_pixel);
	if (videomemorysize &&  line_length * var->yres_virtual > videomemorysize)
		return -ENOMEM;

	/*
	 * Now that we checked it we alter var. The reason being is that the video
	 * mode passed in might not work but slight changes to it might make it
	 * work. This way we let the user know what is acceptable.
	 */
	switch (var->bits_per_pixel) {
	case 1:
	case 8:
		var->red.offset = 0;
		var->red.length = 8;
		var->green.offset = 0;
		var->green.length = 8;
		var->blue.offset = 0;
		var->blue.length = 8;
		var->transp.offset = 0;
		var->transp.length = 0;
		break;
	case 15:		/* RGBA 555 */
		var->red.offset = 10;
		var->red.length = 5;
		var->green.offset = 5;
		var->green.length = 5;
		var->blue.offset = 0;
		var->blue.length = 5;
		break;
	case 16:		/* BGR 565 */
		var->red.offset = 11;
		var->red.length = 5;
		var->green.offset = 5;
		var->green.length = 6;
		var->blue.offset = 0;
		var->blue.length = 5;
		var->transp.offset = 0;
		var->transp.length = 0;
		break;
	case 24:		/* RGB 888 */
		var->red.offset = 16;
		var->red.length = 8;
		var->green.offset = 8;
		var->green.length = 8;
		var->blue.offset = 0;
		var->blue.length = 8;
		var->transp.offset = 0;
		var->transp.length = 0;
		break;
	case 32:		/* ARGB 8888 */
		var->red.offset = 16;
		var->red.length = 8;
		var->green.offset = 8;
		var->green.length = 8;
		var->blue.offset = 0;
		var->blue.length = 8;
		var->transp.offset = 24;
		var->transp.length = 8;
		break;
	}
	var->red.msb_right = 0;
	var->green.msb_right = 0;
	var->blue.msb_right = 0;
	var->transp.msb_right = 0;

	return 0;
}

#define PLL_REF_CLK_MHZ    100
#define PCLK_PRECISION_INDICATOR 10000

/* for PLL using configuration: (refc, loopc, div). Like LS7A1000 PLL
 * for C env without decimal fraction support. */
static unsigned int cal_freq(unsigned int pixclock_khz, struct pix_pll * pll_config)
{
	unsigned int refc_set[] = {4, 5, 3};
	unsigned int prec_set[] = {1, 5, 10, 50, 100};   /*in 1/PCLK_PRECISION_INDICATOR*/
	unsigned int pstdiv, loopc, refc;
	int i, j;
	unsigned int precision_req, precision;
	unsigned int loopc_min, loopc_max, loopc_mid;
	unsigned long long real_dvo, req_dvo;
	int loopc_offset;

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
				if(loopc_offset < 0){
					loopc_offset = -loopc_offset;
				}else{
					loopc_offset = -(loopc_offset+1);
				}

				pstdiv = loopc * PLL_REF_CLK_MHZ * 1000 / refc / pixclock_khz;
				if((pstdiv > 127) || (pstdiv < 1)) continue;

				/*real_freq is float type which is not available, but read_freq * pstdiv is available.*/
				real_dvo = (loopc * PLL_REF_CLK_MHZ * 1000 / refc);
				req_dvo  = (pixclock_khz * pstdiv);
				precision = abs(real_dvo * PCLK_PRECISION_INDICATOR / req_dvo - PCLK_PRECISION_INDICATOR);

				if(precision < precision_req){
					pll_config->l2_div = pstdiv;
					pll_config->l1_loopc = loopc;
					pll_config->l1_frefc = refc;
#ifdef LS_FB_DEBUG
					printk("for pixclock = %d khz, found: pstdiv = %d, "
					      "loopc = %d, refc = %d, precision = %d / %d.\n", pixclock_khz,
					      pll_config->l2_div, pll_config->l1_loopc, pll_config->l1_frefc, precision+1, PCLK_PRECISION_INDICATOR);
#endif
					if(j > 1){
						printk("Warning: PIX clock precision degraded to %d / %d\n", precision_req, PCLK_PRECISION_INDICATOR);
					}
					return 1;
				}
			}
		}
	}
	return 0;
}

static void config_pll(volatile void *pll_base, struct pix_pll *pll_cfg)
{
	unsigned long val;

	/* set sel_pll_out0 0 */
	val = readl(pll_base + 0x4);
	val &= ~(1UL << 8);
	writel(val, pll_base + 0x4);
	/* pll_pd 1 */
	val = readl(pll_base + 0x4);
	val |= (1UL << 13);
	writel(val, pll_base + 0x4);
	/* set_pll_param 0 */
	val = readl(pll_base + 0x4);
	val &= ~(1UL << 11);
	writel(val, pll_base + 0x4);
	/* div ref, loopc, div out */
	val = readl(pll_base + 0x4);
	/* clear old value */
	val &= ~(0x7fUL << 0);
	val |= (pll_cfg->l1_frefc << 0);
	writel(val, pll_base + 0x4);
	val = readl(pll_base + 0x0);
	val &= ~(0x7fUL << 0);
	val |= (pll_cfg->l2_div << 0);
	val &= ~(0x1ffUL << 21);
	val |= (pll_cfg->l1_loopc << 21);
	writel(val, pll_base + 0x0);
	/* set_pll_param 1 */
	val = readl(pll_base + 0x4);
	val |= (1UL << 11);
	writel(val, pll_base + 0x4);
	/* pll_pd 0 */
	val = readl(pll_base + 0x4);
	val &= ~(1UL << 13);
	writel(val, pll_base + 0x4);
	/* wait pll lock */
	while(!(readl(pll_base + 0x4) & 0x80));
	/* set sel_pll_out0 1 */
	val = readl(pll_base + 0x4);
	val |= (1UL << 8);
	writel(val, pll_base + 0x4);
}

static void ls7a_reset_cursor_image(void)
{
	u8 __iomem *addr = (u8 *)DEFAULT_CURSOR_MEM;
	memset(addr, 0, 32*32*4);
}

static int ls7a_init_regs(struct fb_info *info)
{
	unsigned int pix_freq;
	unsigned int depth;
	unsigned int hr, hss, hse, hfl;
	unsigned int vr, vss, vse, vfl;
	int ret;
	struct pix_pll pll_cfg;
	struct fb_var_screeninfo *var = &info->var;
	struct ls7a_fb_par *par = (struct ls7a_fb_par *)info->par;
	volatile void *base = par->reg_base;

	hr	= var->xres;
	hss	= hr + var->right_margin;
	hse	= hss + var->hsync_len;
	hfl	= hse + var->left_margin;

	vr	= var->yres;
	vss	= vr + var->lower_margin;
	vse	= vss + var->vsync_len;
	vfl	= vse + var->upper_margin;

	depth = var->bits_per_pixel;
	pix_freq = PICOS2KHZ(var->pixclock);

	ret = cal_freq(pix_freq, &pll_cfg);
	if (ret) {
		config_pll(LS7A_PIX0_PLL, &pll_cfg);
		config_pll(LS7A_PIX1_PLL, &pll_cfg);
	}

	writel(video_dma, base + LS7A_FB_ADDR0_DVO0_REG);
	writel(video_dma, base + LS7A_FB_ADDR0_DVO1_REG);
	writel(video_dma, base + LS7A_FB_ADDR1_DVO0_REG);
	writel(video_dma, base + LS7A_FB_ADDR1_DVO1_REG);
	writel(0, base + LS7A_FB_DITCFG_DVO0_REG);
	writel(0, base + LS7A_FB_DITCFG_DVO1_REG);
	writel(0, base + LS7A_FB_DITTAB_LO_DVO0_REG);
	writel(0, base + LS7A_FB_DITTAB_LO_DVO1_REG);
	writel(0, base + LS7A_FB_DITTAB_HI_DVO0_REG);
	writel(0, base + LS7A_FB_DITTAB_HI_DVO1_REG);
	writel(0x80001311, base + LS7A_FB_PANCFG_DVO0_REG);
	writel(0x80001311, base + LS7A_FB_PANCFG_DVO1_REG);
	writel(0x00000000, base + LS7A_FB_PANTIM_DVO0_REG);
	writel(0x00000000, base + LS7A_FB_PANTIM_DVO1_REG);

/* these 4 lines cause out of range, because
 * the hfl hss vfl vss are different with PMON vgamode cfg.
 * So the refresh freq in kernel and refresh freq in PMON are different.
 * */
	writel((hfl << 16) | hr, base + LS7A_FB_HDISPLAY_DVO0_REG);
	writel((hfl << 16) | hr, base + LS7A_FB_HDISPLAY_DVO1_REG);
	writel(0x40000000 | (hse << 16) | hss, base + LS7A_FB_HSYNC_DVO0_REG);
	writel(0x40000000 | (hse << 16) | hss, base + LS7A_FB_HSYNC_DVO1_REG);
	writel((vfl << 16) | vr, base + LS7A_FB_VDISPLAY_DVO0_REG);
	writel((vfl << 16) | vr, base + LS7A_FB_VDISPLAY_DVO1_REG);
	writel(0x40000000 | (vse << 16) | vss, base + LS7A_FB_VSYNC_DVO0_REG);
	writel(0x40000000 | (vse << 16) | vss, base + LS7A_FB_VSYNC_DVO1_REG);

	switch (depth) {
	case 32:
	case 24:
		if (edid_flag == 0) {
			writel(0x00100104, base + LS7A_FB_CFG_DVO0_REG);
			writel(0x00100304, base + LS7A_FB_CFG_DVO1_REG);
		} else {
			writel(0x00100304, base + LS7A_FB_CFG_DVO0_REG);
			writel(0x00100104, base + LS7A_FB_CFG_DVO1_REG);
		}
		writel((hr * 4 + 255) & ~255, base + LS7A_FB_STRI_DVO0_REG);
		writel((hr * 4 + 255) & ~255, base + LS7A_FB_STRI_DVO1_REG);
		break;
	case 16:
		if (edid_flag == 0) {
			writel(0x00100103, base + LS7A_FB_CFG_DVO0_REG);
			writel(0x00100303, base + LS7A_FB_CFG_DVO1_REG);
		} else {
			writel(0x00100303, base + LS7A_FB_CFG_DVO0_REG);
			writel(0x00100103, base + LS7A_FB_CFG_DVO1_REG);
		}
		writel((hr * 2 + 255) & ~255, base + LS7A_FB_STRI_DVO0_REG);
		writel((hr * 2 + 255) & ~255, base + LS7A_FB_STRI_DVO1_REG);
		break;
	case 15:
		if (edid_flag == 0) {
			writel(0x00100102, base + LS7A_FB_CFG_DVO0_REG);
			writel(0x00100302, base + LS7A_FB_CFG_DVO1_REG);
		} else {
			writel(0x00100302, base + LS7A_FB_CFG_DVO0_REG);
			writel(0x00100102, base + LS7A_FB_CFG_DVO1_REG);
		}
		writel((hr * 2 + 255) & ~255, base + LS7A_FB_STRI_DVO0_REG);
		writel((hr * 2 + 255) & ~255, base + LS7A_FB_STRI_DVO1_REG);
		break;
	case 12:
		if (edid_flag == 0) {
			writel(0x00100101, base + LS7A_FB_CFG_DVO0_REG);
			writel(0x00100301, base + LS7A_FB_CFG_DVO1_REG);
		} else {
			writel(0x00100301, base + LS7A_FB_CFG_DVO0_REG);
			writel(0x00100101, base + LS7A_FB_CFG_DVO1_REG);
		}
		writel((hr * 2 + 255) & ~255, base + LS7A_FB_STRI_DVO0_REG);
		writel((hr * 2 + 255) & ~255, base + LS7A_FB_STRI_DVO1_REG);
		break;
	default:
		if (edid_flag == 0) {
			writel(0x00100104, base + LS7A_FB_CFG_DVO0_REG);
			writel(0x00100304, base + LS7A_FB_CFG_DVO1_REG);
		} else {
			writel(0x00100304, base + LS7A_FB_CFG_DVO0_REG);
			writel(0x00100104, base + LS7A_FB_CFG_DVO1_REG);
		}
		writel((hr * 4 + 255) & ~255, base + LS7A_FB_STRI_DVO0_REG);
		writel((hr * 4 + 255) & ~255, base + LS7A_FB_STRI_DVO1_REG);
		break;
	}

	/* cursor */
	/* Select full color ARGB mode */
	if (edid_flag == 0)
		writel(0x00050202,base + LS7A_FB_CUR_CFG_REG);
	else
		writel(0x00050212,base + LS7A_FB_CUR_CFG_REG);
	writel(cursor_dma,base + LS7A_FB_CUR_ADDR_REG);
	writel(0x00060122,base + LS7A_FB_CUR_LOC_ADDR_REG);
	writel(0x00eeeeee,base + LS7A_FB_CUR_BACK_REG);
	writel(0x00aaaaaa,base + LS7A_FB_CUR_FORE_REG);
	ls7a_reset_cursor_image();

	return 0;
}

#ifdef LS7A_FB_DEBUG
void show_var(struct fb_var_screeninfo *var)
{
	printk(" xres: %d\n"
		" yres: %d\n",
		var->xres,
		var->yres);
}
#endif

/* This routine actually sets the video mode. It's in here where we
 * the hardware state info->par and fix which can be affected by the
 * change in par. For this driver it doesn't do much.
 */
static int ls7a_fb_set_par(struct fb_info *info)
{
	unsigned long flags;
	info->fix.line_length = get_line_length(info->var.xres_virtual,
						info->var.bits_per_pixel);
#ifdef LS7A_FB_DEBUG
	show_var(&info->var);
#endif
	spin_lock_irqsave(&fb_lock, flags);

	ls7a_init_regs(info);

	spin_unlock_irqrestore(&fb_lock, flags);
	return 0;
}

/*
 *  Set a single color register. The values supplied are already
 *  rounded down to the hardware's capabilities (according to the
 *  entries in the var structure). Return != 0 for invalid regno.
 */

static int ls7a_fb_setcolreg(u_int regno, u_int red, u_int green, u_int blue,
			 u_int transp, struct fb_info *info)
{
	if (regno >= 256)	/* no. of hw registers */
		return 1;
	/*
	 * Program hardware... do anything you want with transp
	 */

	/* grayscale works only partially under directcolor */
	if (info->var.grayscale) {
		/* grayscale = 0.30*R + 0.59*G + 0.11*B */
		red = green = blue =
			(red * 77 + green * 151 + blue * 28) >> 8;
	}

	/* Directcolor:
	 *   var->{color}.offset contains start of bitfield
	 *   var->{color}.length contains length of bitfield
	 *   {hardwarespecific} contains width of RAMDAC
	 *   cmap[X] is programmed to (X << red.offset) | (X << green.offset) | (X << blue.offset)
	 *   RAMDAC[X] is programmed to (red, green, blue)
	 *
	 * Pseudocolor:
	 *    uses offset = 0 && length = RAMDAC register width.
	 *    var->{color}.offset is 0
	 *    var->{color}.length contains widht of DAC
	 *    cmap is not used
	 *    RAMDAC[X] is programmed to (red, green, blue)
	 * Truecolor:
	 *    does not use DAC. Usually 3 are present.
	 *    var->{color}.offset contains start of bitfield
	 *    var->{color}.length contains length of bitfield
	 *    cmap is programmed to (red << red.offset) | (green << green.offset) |
	 *                      (blue << blue.offset) | (transp << transp.offset)
	 *    RAMDAC does not exist
	 */
#define CNVT_TOHW(val,width) ((((val)<<(width))+0x7FFF-(val))>>16)
	switch (info->fix.visual) {
	case FB_VISUAL_TRUECOLOR:
	case FB_VISUAL_PSEUDOCOLOR:
		red = CNVT_TOHW(red, info->var.red.length);
		green = CNVT_TOHW(green, info->var.green.length);
		blue = CNVT_TOHW(blue, info->var.blue.length);
		transp = CNVT_TOHW(transp, info->var.transp.length);
		break;
	case FB_VISUAL_DIRECTCOLOR:
		red = CNVT_TOHW(red, 8);	/* expect 8 bit DAC */
		green = CNVT_TOHW(green, 8);
		blue = CNVT_TOHW(blue, 8);
		/* hey, there is bug in transp handling... */
		transp = CNVT_TOHW(transp, 8);
		break;
	}
#undef CNVT_TOHW
	/* Truecolor has hardware independent palette */
	if (info->fix.visual == FB_VISUAL_TRUECOLOR) {
		u32 v;

		if (regno >= 16)
			return 1;

		v = (red << info->var.red.offset) |
			(green << info->var.green.offset) |
			(blue << info->var.blue.offset) |
			(transp << info->var.transp.offset);
		switch (info->var.bits_per_pixel) {
		case 8:
			break;
		case 16:
			((u32 *) (info->pseudo_palette))[regno] = v;
			break;
		case 24:
		case 32:
			((u32 *) (info->pseudo_palette))[regno] = v;
			break;
		}
		return 0;
	}
	return 0;
}

static int ls7a_fb_blank (int blank_mode, struct fb_info *info)
{
	return 0;
}

/*************************************************************
 *                Hardware Cursor Routines                   *
 *************************************************************/

/**
 * ls7a_enable_cursor - show or hide the hardware cursor
 * @mode: show (1) or hide (0)
 *
 * Description:
 * Shows or hides the hardware cursor
 */
static void ls7a_enable_cursor(int mode, volatile void *base)
{
	unsigned int tmp = readl(base + LS7A_FB_CUR_CFG_REG);
	tmp &= ~0xff;
	if (edid_flag == 0)
		writel(mode?(tmp|0x02):(tmp|0x00), base + LS7A_FB_CUR_CFG_REG);
	else
		writel(mode?(tmp|0x12):(tmp|0x10), base + LS7A_FB_CUR_CFG_REG);
}

static void ls7a_load_cursor_image(int width, int height, u8 *data)
{
	u32 __iomem *addr = (u32 *)DEFAULT_CURSOR_MEM;
	int row, col, i, j, bit = 0;
	col = (width > CUR_HEIGHT_SIZE)? CUR_HEIGHT_SIZE : width;
	row = (height > CUR_WIDTH_SIZE)? CUR_WIDTH_SIZE : height;

	for (i = 0; i < CUR_HEIGHT_SIZE; i++) {
		for (j = 0; j < CUR_WIDTH_SIZE; j++) {
			if (i < height && j < width) {
				bit = data[(i * width + width - j) >> 3] &
					(1 << ((i * width + width - j) & 0x7));
				addr[i * CUR_WIDTH_SIZE + j] =
					bit ? 0xffffffff : 0;
			 } else {
				addr[i * CUR_WIDTH_SIZE + j] = 0x0;
			 }
		}
	}
}


static int ls7a_fb_cursor(struct fb_info *info, struct fb_cursor *cursor)
{
	struct ls7a_fb_par *par = (struct ls7a_fb_par *)info->par;
	volatile void *base = par->reg_base;

	if (cursor->image.width > CUR_WIDTH_SIZE ||
			cursor->image.height > CUR_HEIGHT_SIZE)
		return -ENXIO;

	ls7a_enable_cursor(OFF, base);

	if (cursor->set & FB_CUR_SETPOS) {
		u32 tmp;

		tmp = (cursor->image.dx - info->var.xoffset) & 0xffff;
		tmp |= (cursor->image.dy - info->var.yoffset) << 16;
		writel(tmp, base + LS7A_FB_CUR_LOC_ADDR_REG);
	}

	if (cursor->set & FB_CUR_SETSIZE)
		ls7a_reset_cursor_image();

	if (cursor->set & FB_CUR_SETHOT) {
		u32 hot = (cursor->hot.x << 16) | (cursor->hot.y << 8);
		u32 con = readl(base + LS7A_FB_CUR_CFG_REG) & 0xff;
		writel(hot | con, base + LS7A_FB_CUR_CFG_REG);
	}

	if (cursor->set & (FB_CUR_SETSHAPE | FB_CUR_SETIMAGE)) {
		int size = ((cursor->image.width + 7) >> 3) *
			cursor->image.height;
		int i;
		u8 *data = kmalloc(32 * 32 * 4, GFP_ATOMIC);

		if (data == NULL)
			return -ENOMEM;

		switch (cursor->rop) {
		case ROP_XOR:
			for (i = 0; i < size; i++)
				data[i] = cursor->image.data[i] ^ cursor->mask[i];
			break;
		case ROP_COPY:
		default:
			for (i = 0; i < size; i++)
				data[i] = cursor->image.data[i] & cursor->mask[i];
			break;
		}

		ls7a_load_cursor_image(cursor->image.width,
				       cursor->image.height, data);
		kfree(data);
	}

	if (cursor->enable)
		ls7a_enable_cursor(ON, base);

	return 0;
}

struct cursor_req {
	u32 x;
	u32 y;
};

#define CURIOSET_CORLOR		0x4607
#define CURIOSET_POSITION	0x4608
#define CURIOLOAD_ARGB		0x4609
#define CURIOLOAD_IMAGE		0x460A
#define CURIOHIDE_SHOW		0x460B
#define FBEDID_GET		0x860C

static int ls7a_fb_ioctl(struct fb_info *info, unsigned int cmd,
		                        unsigned long arg)
{
	u32 tmp;
	struct cursor_req req;
	struct ls7a_fb_par *par = (struct ls7a_fb_par *)info->par;
	volatile void *base = par->reg_base;
	void __user *argp = (void __user *)arg;
	u8 *cursor_base = (u8 *)DEFAULT_CURSOR_MEM;

	switch (cmd) {
	case CURIOSET_CORLOR:
		break;
	case CURIOSET_POSITION:
		LS7A_DEBUG("CURIOSET_POSITION\n");
		if (copy_from_user(&req, argp, sizeof(struct cursor_req)))
			return -EFAULT;
		tmp = (req.x - info->var.xoffset) & 0xffff;
		tmp |= (req.y - info->var.yoffset) << 16;
		writel(tmp, base + LS7A_FB_CUR_LOC_ADDR_REG);
		break;
	case CURIOLOAD_ARGB:
		LS7A_DEBUG("CURIOLOAD_ARGB\n");
		if (copy_from_user(cursor_base, argp, 32 * 32 * 4))
			return -EFAULT;
		break;
	case CURIOHIDE_SHOW:
		LS7A_DEBUG("CURIOHIDE_SHOW:%s\n", arg ? "show" : "hide");
		ls7a_enable_cursor(arg, base);
		break;
	case FBEDID_GET:
		LS7A_DEBUG("COPY EDID TO USER\n");
		if (copy_to_user(argp, par->edid, EDID_LENGTH))
			return -EFAULT;
	default:
		return -ENOTTY;

	}

	return 0;
}

/*
 *  Pan or Wrap the Display
 *
 *  This call looks only at xoffset, yoffset and the FB_VMODE_YWRAP flag
 */

static int ls7a_fb_pan_display(struct fb_var_screeninfo *var,
			struct fb_info *info)
{
	if (var->vmode & FB_VMODE_YWRAP) {
		if (var->yoffset < 0
			|| var->yoffset >= info->var.yres_virtual
			|| var->xoffset)
			return -EINVAL;
	} else {
		if (var->xoffset + var->xres > info->var.xres_virtual ||
			var->yoffset + var->yres > info->var.yres_virtual)
			return -EINVAL;
	}
	info->var.xoffset = var->xoffset;
	info->var.yoffset = var->yoffset;
	if (var->vmode & FB_VMODE_YWRAP)
		info->var.vmode |= FB_VMODE_YWRAP;
	else
		info->var.vmode &= ~FB_VMODE_YWRAP;
	return 0;
}

#ifndef MODULE
static int __init ls7a_fb_setup(char *options)
{
	char *this_opt;
	ls7a_fb_enable = 1;

	if (!options || !*options)
		return 1;

	while ((this_opt = strsep(&options, ",")) != NULL) {
		if (!*this_opt)
			continue;
		if (!strncmp(this_opt, "disable", 7))
			ls7a_fb_enable = 0;
		else
			mode_option = this_opt;
	}
	return 1;
}
#endif  /* MODULE */

static unsigned char *fb_do_probe_ddc_edid(struct i2c_adapter *adapter, int id)
{
	unsigned char start = 0x0;
	unsigned char *buf = kmalloc(EDID_LENGTH, GFP_KERNEL);
	struct i2c_msg msgs[] = {
		{
			.addr = eeprom_info[id].addr,
			.flags = 0,
			.len = 1,
			.buf = &start,
		},{
			.addr = eeprom_info[id].addr,
			.flags = I2C_M_RD,
			.len = EDID_LENGTH,
			.buf = buf,
		}
	};
	if (!buf){
		dev_warn(&adapter->dev, "unable to allocate memory for EDID "
			"block.\n");
		return NULL;
	}
	if (i2c_transfer(adapter, msgs, 2) == 2){
		return buf;
	}
	dev_warn(&adapter->dev, "unable to read EDID block.\n");
	kfree(buf);
	return NULL;
}

static unsigned char *ls7a_fb_i2c_connector(struct ls7a_fb_par *fb_par)
{
	unsigned char *edid = NULL;

	LS7A_DEBUG("edid entry\n");

	if (eeprom_info[0].adapter)
		edid = fb_do_probe_ddc_edid(eeprom_info[0].adapter, 0);

	if (!edid) {
		edid_flag = 1;
		if (eeprom_info[1].adapter)
			edid = fb_do_probe_ddc_edid(eeprom_info[1].adapter, 1);
	}

	fb_par->edid = edid;

	return edid;
}

#define LS7A_HT1_BASE 0x90000e0000000000

static void ls7a_fb_address_init(void)
{
         struct pci_dev *pdev;

         pdev = pci_get_device(PCI_VENDOR_ID_LOONGSON, PCI_DEVICE_ID_LOONGSON_GPU, NULL);
         if(pdev){
                 /*get frame buffer address from memory of GPU device*/
                 ls7a_cursor_dma = pci_resource_start(pdev, 2);
                 ls7a_cursor_mem = LS7A_HT1_BASE|ls7a_cursor_dma;

                 ls7a_fb_mem     = ls7a_cursor_mem + LS7AFB_OFFSET;
                 ls7a_fb_dma     = ls7a_cursor_dma + LS7AFB_OFFSET;
                 ls7a_phy_addr   = ls7a_fb_mem & LS7AFB_GPU_MASK;

                 pci_enable_device_mem(pdev);
         }
}

static void ls7a_find_init_mode(struct fb_info *info)
{
        int found = 0;
	unsigned char *edid;
        struct fb_videomode mode;
        struct fb_var_screeninfo var;
        struct fb_monspecs *specs = &info->monspecs;
	struct ls7a_fb_par *par = info->par;
        INIT_LIST_HEAD(&info->modelist);

	ls7a_fb_address_init();
        memset(&mode, 0, sizeof(struct fb_videomode));
        memset(&var, 0, sizeof(struct fb_var_screeninfo));
	var.bits_per_pixel = DEFAULT_BITS_PER_PIXEL;
	edid = ls7a_fb_i2c_connector(par);
	if (!edid){
		goto def;
	}
	fb_edid_to_monspecs(par->edid, specs);
	if (specs->modedb == NULL){
		printk("ls7a-fb: Unable to get Mode Database\n");
		goto def;
	}
	fb_videomode_to_modelist(specs->modedb,specs->modedb_len,
					&info->modelist);
	if (specs->modedb != NULL){
		const struct fb_videomode *m;
		if (!found){
			m = fb_find_best_display(&info->monspecs, &info->modelist);
			mode = *m;
			found= 1;
		}
		fb_videomode_to_var(&var,&mode);
	}
	if (mode_option) {
			printk("mode_option: %s\n", mode_option);
			fb_find_mode(&var, info, mode_option, specs->modedb,
					specs->modedb_len, (found) ? &mode : NULL,info->var.bits_per_pixel);
			info->var = var;
	}
	else{
		info->var = ls7a_fb_default;
	}
	info->var = var;
	fb_destroy_modedb(specs->modedb);
	specs->modedb = NULL;
	return;
def:
	info->var = ls7a_fb_default;
	return;
}

/* irq */
static irqreturn_t ls7afb_irq(int irq, void *dev_id)
{
	unsigned int val, cfg;
	unsigned long flags;
	struct fb_info *info = (struct fb_info *) dev_id;
	struct ls7a_fb_par *par = (struct ls7a_fb_par *)info->par;
	volatile void *base = par->reg_base;

	spin_lock_irqsave(&fb_lock, flags);

	val = readl(base + LS7A_FB_INT_REG);
	writel(val & (0xffff << 16), base + LS7A_FB_INT_REG);

	cfg = readl(base + LS7A_FB_CFG_DVO1_REG);
	/* if underflow, reset VGA */
	if (val & 0x280) {
		writel(0, base + LS7A_FB_CFG_DVO1_REG);
		writel(cfg, base + LS7A_FB_CFG_DVO1_REG);
	}

	spin_unlock_irqrestore(&fb_lock, flags);

	return IRQ_HANDLED;
}

static struct fb_ops ls7a_fb_ops = {
	.owner			= THIS_MODULE,
	.fb_check_var		= ls7a_fb_check_var,
	.fb_set_par		= ls7a_fb_set_par,
	.fb_setcolreg		= ls7a_fb_setcolreg,
	.fb_blank		= ls7a_fb_blank,
	.fb_pan_display		= ls7a_fb_pan_display,
	.fb_fillrect		= cfb_fillrect,
	.fb_copyarea		= cfb_copyarea,
	.fb_imageblit		= cfb_imageblit,
	.fb_cursor		= ls7a_fb_cursor,
	.fb_ioctl		= ls7a_fb_ioctl,
	.fb_compat_ioctl	= ls7a_fb_ioctl,
};

static struct pci_device_id ls7a_fb_devices[] = {
	{PCI_DEVICE(PCI_VENDOR_ID_LOONGSON, PCI_DEVICE_ID_LOONGSON_DC)},
	{0, 0, 0, 0, 0, 0, 0}
};

/*
 *  Initialisation
 */
static int ls7a_fb_pci_register(struct pci_dev *pdev,
				 const struct pci_device_id *ent)
{
	char irq;
	int retval = -ENOMEM;
	struct fb_info *info;
	struct ls7a_fb_par *par;
	struct i2c_adapter *i2c_adap;

	pr_debug("ls7a_fb_pci_register BEGIN\n");

	i2c_adap = i2c_get_adapter(6);
	eeprom_info[0].addr = 0x50;
	eeprom_info[0].adapter = i2c_adap;
	i2c_put_adapter(i2c_adap);

	i2c_adap = i2c_get_adapter(7);
	eeprom_info[1].addr = 0x50;
	eeprom_info[1].adapter = i2c_adap;
	i2c_put_adapter(i2c_adap);

	/* Enable device in PCI config */
	retval = pci_enable_device(pdev);
	if (retval < 0) {
		printk(KERN_ERR "ls7afb (%s): Cannot enable PCI device\n",
		       pci_name(pdev));
		goto err_out;
	}

	/* request the mem regions */
	retval = pci_request_region(pdev, 0, "ls7afb io");
	if (retval < 0) {
		printk(KERN_ERR "ls7afb (%s): cannot request region 0.\n",
			pci_name(pdev));
		goto err_out;
	}

	/* need api from pci irq */
	retval = pci_read_config_byte(pdev, PCI_INTERRUPT_LINE, &irq);
	if (irq < 0) {
		dev_err(&pdev->dev, "no irq for device\n");
		return -ENOENT;
	}

	info = framebuffer_alloc(sizeof(u32) * 256, &pdev->dev);
	if (!info)
		return -ENOMEM;

	info->fix = ls7a_fb_fix;
	info->node = -1;
	info->fbops = &ls7a_fb_ops;
	info->pseudo_palette = info->par;
	info->flags = FBINFO_FLAG_DEFAULT;

	par = kzalloc(sizeof(struct ls7a_fb_par), GFP_KERNEL);
	if (!par) {
		retval = -ENOMEM;
		goto release_info;
	}

	info->par = par;
	par->fb_info = info;
	par->pdev = pdev;
	par->irq = irq;

	par->reg_base = ioremap(pci_resource_start(pdev, 0),
			pci_resource_end(pdev, 0) - pci_resource_start(pdev, 0) + 1);

	ls7a_find_init_mode(info);

	if (!videomemorysize) {
		videomemorysize = info->var.xres_virtual *
					info->var.yres_virtual *
					info->var.bits_per_pixel / 8;
	}

	/*
	 * For real video cards we use ioremap.
	 */
	videomemory = (void *)DEFAULT_FB_MEM;
	video_dma = (dma_addr_t)DEFAULT_FB_DMA;

	pr_info("VideoDMA=%x\n",(int)video_dma);
	pr_info("VideoMemory=%lx\n",(unsigned long)videomemory);
	pr_info("VideoMemorySize=%lx\n",videomemorysize);
	memset(videomemory, 0, videomemorysize);

	cursor_mem = (void *)DEFAULT_CURSOR_MEM;
	cursor_dma = (dma_addr_t)DEFAULT_CURSOR_DMA;
	memset(cursor_mem,0x88FFFF00,cursor_size);

	info->screen_base = (char __iomem *)videomemory;
	info->fix.smem_start = DEFAULT_PHY_ADDR;
	info->fix.smem_len = videomemorysize;

	retval = fb_alloc_cmap(&info->cmap, 32, 0);
	if (retval < 0) goto release_par;

	info->fbops->fb_check_var(&info->var, info);
	par->htotal = info->var.xres;
	par->vtotal = info->var.yres;
	retval = register_framebuffer(info);
	if (retval < 0)
		goto release_map;

	retval = request_irq(irq, ls7afb_irq, 0, pci_name(pdev), info);
	if (retval) {
		dev_err(&pdev->dev, "cannot get irq %d - err %d\n", irq, retval);
		goto unreg_info;
	}

	dev_set_drvdata(&pdev->dev, info);

	pr_info("fb%d: Virtual frame buffer device, using %ldK of"
			"video memory\n", info->node, videomemorysize >> 10);

	return 0;

unreg_info:
	unregister_framebuffer(info);
release_map:
	fb_dealloc_cmap(&info->cmap);
release_par:
	kfree(par);
release_info:
	dev_set_drvdata(&pdev->dev, NULL);
	framebuffer_release(info);
err_out:
	return retval;
}

static void ls7a_fb_pci_unregister(struct pci_dev *pdev)
{
	struct fb_info *info = dev_get_drvdata(&pdev->dev);
	struct ls7a_fb_par *par = info->par;
	int irq = par->irq;

	free_irq(irq, info);
	fb_dealloc_cmap(&info->cmap);
	unregister_framebuffer(info);
	dev_set_drvdata(&pdev->dev, NULL);
	framebuffer_release(info);
	if (par->edid)
		kfree(par->edid);
	kfree(par);
	pci_release_region(pdev, 0);
}

#ifdef	CONFIG_SUSPEND
static u32 output_mode;

int ls7a_fb_pci_suspend(struct pci_dev *pdev, pm_message_t mesg)
{
	struct fb_info *info = dev_get_drvdata(&pdev->dev);
	struct ls7a_fb_par *par = (struct ls7a_fb_par *)info->par;
	volatile void *base = par->reg_base;

	console_lock();
	fb_set_suspend(info, 1);
	console_unlock();

	output_mode = readl(base + LS7A_FB_DVO_OUTPUT_REG);
	pci_save_state(pdev);
	return 0;
}

int ls7a_fb_pci_resume(struct pci_dev *pdev)
{
	struct fb_info *info = dev_get_drvdata(&pdev->dev);
	struct ls7a_fb_par *par = (struct ls7a_fb_par *)info->par;
	volatile void *base = par->reg_base;

	ls7a_fb_set_par(info);
	writel(output_mode, base + LS7A_FB_DVO_OUTPUT_REG);

	console_lock();
	fb_set_suspend(info, 0);
	console_unlock();
	return 0;
}
#endif

static struct pci_driver ls7a_fb_pci_driver = {
	.name		= "ls7a-fb",
	.id_table	= ls7a_fb_devices,
	.probe		= ls7a_fb_pci_register,
	.remove		= ls7a_fb_pci_unregister,
#ifdef	CONFIG_SUSPEND
	.suspend = ls7a_fb_pci_suspend,
	.resume	 = ls7a_fb_pci_resume,
#endif
};

static int __init ls7a_fb_init(void)
{
	int ret;
	struct pci_dev *pdev = NULL;

#ifndef MODULE
	char *option = NULL;

	if (fb_get_options("ls7a-fb", &option))
		return -ENODEV;
	ls7a_fb_setup(option);
#endif
	/* Prefer to use External Graphics Card */
	while ((pdev = pci_get_class(PCI_CLASS_DISPLAY_VGA << 8, pdev))) {
		if (pdev->vendor == PCI_VENDOR_ID_ATI)
			return 0;
		if (pdev->vendor == 0x1a03) /* ASpeed */
			return 0;
	}

	ret = pci_register_driver(&ls7a_fb_pci_driver);

	return ret;
}

static void __exit ls7a_fb_exit (void)
{
	pci_unregister_driver(&ls7a_fb_pci_driver);
}

late_initcall(ls7a_fb_init);
module_exit(ls7a_fb_exit);

MODULE_LICENSE("GPL");
