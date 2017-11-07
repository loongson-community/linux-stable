/*
 * Copyright 2016 SiliconMotion Inc.
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License version 2. See the file COPYING in the main
 * directory of this archive for more details.
 *
 */


#include "ddk768/ddk768_mode.h"
#include "ddk768/ddk768_help.h"
#include "ddk768/ddk768_reg.h"	
#include "ddk768/ddk768_display.h"
#include "ddk768/ddk768_2d.h"
#include "ddk768/ddk768_power.h"
#include "ddk768/ddk768_cursor.h"
#include "ddk768/ddk768_video.h"
#include "ddk768/ddk768_hdmi.h"

struct smi_768_register{
	/* registers for save, copyed from struct smi_750_register in hw750.c */
	uint32_t system_ctrl, misc_ctrl,gpio_mux, localmem_arbitration;
	uint32_t pcimem_arbitration, raw_int, int_status, int_mask;
	uint32_t current_gate, mode0_gate, mode1_gate, power_mode_ctrl;
	uint32_t pci_master_base, primary_pll_ctrl,	secondary_pll_ctrl,	vga_pll0_ctrl;
	uint32_t vga_pll1_ctrl,	mxclk_pll_ctrl,	vga_configuration;
	
	uint32_t de_stretch_format, de_masks, de_window_width, de_control;

	uint32_t primary_display_ctrl,primary_pan_ctrl,primary_color_key,primary_fb_address;
	uint32_t primary_fb_width, primary_window_width,primary_window_height, primary_plane_tl;
	uint32_t primary_plane_br, primary_horizontal_total, primary_horizontal_sync, primary_vertical_total;
	uint32_t primary_vertical_sync, primary_current_line;
	
	uint32_t secondary_display_ctrl, secondary_fb_address, secondary_fb_width;
	uint32_t secondary_horizontal_total, secondary_horizontal_sync;
	uint32_t secondary_vertical_total, secondary_vertical_sync;
	uint32_t secondary_auto_centering_tl, secondary_auto_centering_br;
	uint32_t secondary_scale, secondary_hwc_address, secondary_hwc_location;
	uint32_t secondary_hwc_color_12, secondary_hwc_color_3;
};

void hw768_enable_lvds(int channels)
{
	if(channels == 1){
		pokeRegisterDWord(0x80020,0x31E30000);
		pokeRegisterDWord(0x8002C,0x74001200);
	}else{
		pokeRegisterDWord(0x80020,0x31E3F71D);
		pokeRegisterDWord(0x8002C,0x750FED02);
		unsigned long value = peekRegisterDWord(DISPLAY_CTRL);
		value = FIELD_SET(value, DISPLAY_CTRL, LVDS_OUTPUT_FORMAT, CHANNEL0_48BIT);
		value = FIELD_SET(value, DISPLAY_CTRL, PIXEL_CLOCK_SELECT, HALF);
		value = FIELD_SET(value, DISPLAY_CTRL, DOUBLE_PIXEL_CLOCK, ENABLE);
		pokeRegisterDWord(DISPLAY_CTRL,value);

		value = peekRegisterDWord(DISPLAY_CTRL + CHANNEL_OFFSET);
		value = FIELD_SET(value, DISPLAY_CTRL, LVDS_OUTPUT_FORMAT, CHANNEL0_48BIT);
		pokeRegisterDWord(DISPLAY_CTRL + CHANNEL_OFFSET,value);
	}
}

void hw768_suspend(struct smi_768_register * pSave)
{
	printk("sm768 suspend\n");; 
}

void hw768_resume(struct smi_768_register * pSave)
{
	printk("sm768 resume\n");
}
void hw768_set_base(int display,int pitch,int base_addr)
{	

	if(display == 0)
	{
		/* Frame buffer base */
	    pokeRegisterDWord((FB_ADDRESS),
	          FIELD_SET(0, FB_ADDRESS, STATUS, PENDING)
	        | FIELD_VALUE(0, FB_ADDRESS, ADDRESS, base_addr));

	    /* Pitch value (Hardware people calls it Offset) */
    	pokeRegisterDWord((FB_WIDTH), FIELD_VALUE(peekRegisterDWord(FB_WIDTH), FB_WIDTH, OFFSET, pitch));
	}
	else
	{
		/* Frame buffer base */
	    pokeRegisterDWord((FB_ADDRESS+CHANNEL_OFFSET),
	          FIELD_SET(0, FB_ADDRESS, STATUS, PENDING)
	        | FIELD_VALUE(0, FB_ADDRESS, ADDRESS, base_addr));

		
	    /* Pitch value (Hardware people calls it Offset) */	
	    pokeRegisterDWord((FB_WIDTH+CHANNEL_OFFSET),FIELD_VALUE(peekRegisterDWord(FB_WIDTH+CHANNEL_OFFSET), FB_WIDTH, OFFSET, pitch));

	}
}


void hw768_init_hdmi(void)
{
	HDMI_Init();
}

int hw768_set_hdmi_mode(logicalMode_t *pLogicalMode, bool isHDMI)
{
	int ret = 1;
	if(pLogicalMode->x == 3840)
	{
		printk("Use 4K Mode!\n");
		pLogicalMode->hz = 30;
	}
	else
		pLogicalMode->hz = 60;
	// set HDMI parameters
	HDMI_Disable_Output();
	ret = HDMI_Set_Mode(pLogicalMode, isHDMI);
	return ret;
}

int hw768_en_dis_interrupt(int status, int pipe)
{
	if(status == 0)
	{
		pokeRegisterDWord(INT_MASK, 	(pipe == CHANNEL0_CTRL) ? 
		FIELD_SET(0, INT_MASK, CHANNEL1_VSYNC, DISABLE):
		FIELD_SET(0, INT_MASK, CHANNEL0_VSYNC, DISABLE));
	}
	else
	{
		pokeRegisterDWord(INT_MASK, 	(pipe == CHANNEL1_CTRL) ? 
		FIELD_SET(0, INT_MASK, CHANNEL1_VSYNC, ENABLE):
		FIELD_SET(0, INT_MASK, CHANNEL0_VSYNC, ENABLE));
	}
	return 0;
}
void hw768_HDMI_Enable_Output(void)
{
	HDMI_Enable_Output();
}

void hw768_HDMI_Disable_Output(void)
{
	HDMI_Disable_Output();
}


int hw768_get_hdmi_edid(unsigned char *pEDIDBuffer)
{
    int ret;
    enableHdmI2C(1);
    ret = HDMI_Read_Edid(pEDIDBuffer, 128);
    enableHdmI2C(0);

    return ret;
}

int hw768_check_iis_interrupt(void)
{

	unsigned long value;
		
	value = peekRegisterDWord(INT_STATUS);

	
    if (FIELD_GET(value, INT_STATUS, I2S) == INT_STATUS_I2S_ACTIVE)
		return true;
	else	
		return false;
}


int hw768_check_vsync_interrupt(int path)
{

	unsigned long value1,value2;
		
	value1 = peekRegisterDWord(RAW_INT);
	value2 = peekRegisterDWord(INT_MASK);

	if(path == CHANNEL0_CTRL)
	{
	    if ((FIELD_GET(value1, RAW_INT, CHANNEL0_VSYNC) == RAW_INT_CHANNEL0_VSYNC_ACTIVE)
			&&(FIELD_GET(value2, INT_MASK, CHANNEL0_VSYNC) == INT_MASK_CHANNEL0_VSYNC_ENABLE))
	    {
			return true;
		}
	}else{
		if ((FIELD_GET(value1, RAW_INT, CHANNEL1_VSYNC) == RAW_INT_CHANNEL1_VSYNC_ACTIVE)
			&&(FIELD_GET(value2, INT_MASK, CHANNEL1_VSYNC) == INT_MASK_CHANNEL1_VSYNC_ENABLE))
		{
			return true;
		}
	}
	
	return false;
}


void hw768_clear_vsync_interrupt(int path)
{
	
	unsigned long value;
	
	value = peekRegisterDWord(RAW_INT);

	if(path == CHANNEL0_CTRL)
	{   
		pokeRegisterDWord(RAW_INT, FIELD_SET(value, RAW_INT, CHANNEL0_VSYNC, CLEAR));
			
	}else{	
		pokeRegisterDWord(RAW_INT, FIELD_SET(value, RAW_INT, CHANNEL1_VSYNC, CLEAR));	
	}
	
}



int hdmi_int_status = 0;

inline int hdmi_hotplug_detect(void)
{
		unsigned int intMask = peekRegisterDWord(INT_MASK);
    	intMask = FIELD_SET(intMask, INT_MASK, HDMI, ENABLE);
    	pokeRegisterDWord(INT_MASK, intMask);


		int ret = hdmi_detect();

		if (ret == 1) {
			hdmi_int_status = 1;
		}
		else if(ret == 0){
			hdmi_int_status = 0;
		}
		else{	
			hdmi_int_status = hdmi_int_status & ret;		
		}	

		intMask = peekRegisterDWord(INT_MASK);
    	intMask = FIELD_SET(intMask, INT_MASK, HDMI, DISABLE);
    	pokeRegisterDWord(INT_MASK, intMask);

		return hdmi_int_status;

}

void ddk768_disable_IntMask(void)
{
	
    pokeRegisterDWord(INT_MASK, 0);
}


