/*******************************************************************
* 
*         Copyright (c) 2009 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  HDMI.H --- SMI DDK 
*  This file contains the source code for the HDMI controller chip
* 
*******************************************************************/

#ifndef  _HDMI_HEADER_H_
#define  _HDMI_HEADER_H_

#include "ddk768_mode.h"

#define SMI_HDMI_LIB_VERSION    ("2.0")

#ifndef BYTE
typedef unsigned char   BYTE;
#endif

// This macro is for HDMI PNP detection - interrupt mode
//#define HDMI_PNP_USE_INT

//-----------------------------------------------------------------------------
// Global Constants
//-----------------------------------------------------------------------------

// Video setting constants
#define VID_01_640x480p     1
#define VID_02_720x480p     2
#define VID_03_720x480p     3
#define VID_04_1280x720p    4
#define VID_05_1920x1080i   5
#define VID_06_720x480i     6
#define VID_07_720x480i     7
#define VID_08_720x240p     8
#define VID_09_720x240p     9
#define VID_10_2880x480i    10
#define VID_11_2880x480i    11
#define VID_12_2880x240p    12
#define VID_13_2880x240p    13
#define VID_14_1440x480p    14
#define VID_15_1440x480p    15
#define VID_16_1920x1080p   16
#define VID_17_720x576p     17
#define VID_18_720x576p     18
#define VID_19_1280x720p    19
#define VID_20_1920x1080i   20
#define VID_21_720x576i     21
#define VID_22_720x576i     22
#define VID_23_720x288p     23
#define VID_24_720x288p     24
#define VID_25_2880x576i    25
#define VID_26_2880x576i    26
#define VID_27_2880x288p    27
#define VID_28_2880x288p    28
#define VID_29_1440x576p    29
#define VID_30_1440x576p    30
#define VID_31_1920x1080p   31
#define VID_32_1920x1080p   32
#define VID_33_1920x1080p   33
#define VID_34_1920x1080p   34
#define VID_35_2880x480p    35
#define VID_36_2880x480p    36
#define VID_37_2880x576p    37
#define VID_38_2880x567p    38
#define VID_39_1920x1080i   39
#define VID_40_1920x1080i   40
#define VID_41_1280x720p    41
#define VID_42_720x576p     42
#define VID_43_720x576p     43
#define VID_44_720x576i     44
#define VID_45_720x576i     45
#define VID_46_1920x1080i   46
#define VID_47_1280x720p    47
#define VID_48_720x480p     48
#define VID_49_720x480p     49
#define VID_50_720x480i     50
#define VID_51_720x480i     51
#define VID_52_720x576p     52
#define VID_53_720x576p     53
#define VID_54_720x576i     54
#define VID_55_720x576i     55
#define VID_56_720x480p     56
#define VID_57_720x480p     57
#define VID_58_720x480i     58
#define VID_59_720x480i     59

// Audio setting constants
#define AUD_48K         0x10
#define AUD_96K         0x20
#define AUD_192K        0x30
#define AUD_2CH         0x40    // 2ch audio
#define AUD_8CH         0x80    // 8ch audio
//#define DS_none           0x00    // No Downsampling
#define DS_none         0x01    // No Downsampling
#define DS_2            0x04    // Downsampling 96k to 48k
#define DS_4            0x08    // DOwnsampling 192k to 48k
#define AUD_SPDIF       0x01
#define AUD_I2S         0x02

// Power Mode - System Control
#define PowerMode_A             0x11
#define PowerMode_B             0x21
#define PowerMode_D             0x41
#define PowerMode_E             0x81

// Audio Mode - For HDMI_Set_Mode's use, in case audio command is received before mode setting
#define Audio_Mute               0x00
#define Audio_Unmute             0x01

// Output Format
#define FORMAT_RGB              0
#define FORMAT_YCC422           1
#define FORMAT_YCC444           2

// Deep Color Bit,
// Should follow GCP_CD[3:0] definition
#define DEEP_COLOR_8BIT         4
#define DEEP_COLOR_10BIT        5
#define DEEP_COLOR_12BIT        6

// Register Values for Register 94h: Interrupt Status 1
#define INT1_RSVD               0x09
#define HOT_PLUG                0x80
#define HPG_MSENS               0xC0
#define EDID_ERR                0x02
#define EDID_RDY                0x04
#define VSYNC                   0x20

// Register Values for Register 95h: Interrupt Status 2
#define INT2_RSVD               0x07
#define HDCP_ERR                0x80
#define BKSV_RDY                0x60
#define HDCP_AUTH               0x08
#define HDCP_DONE               0x10

// STATE
#define HDMI_STATE_IDLE         0x01
#define HDMI_STATE_HOTPLUG      0x02
#define HDMI_STATE_EDID_START   0x03
#define HDMI_STATE_EDID_READY   0x04
#define HDMI_STATE_EDID_READ    0x05
#define HDMI_STATE_EDID_PROCESS 0x06
#define HDMI_STATE_TX_SETTING   0x07
#define HDMI_STATE_TX_START     0x08
#define HDMI_STATE_TX_RUNNING   0x09
#define HDMI_STATE_HDCP_START   0x0A
#define HDMI_STATE_HDCP_READY   0x0B
#define HDMI_STATE_HDCP_READ    0x0C
#define HDMI_STATE_HDCP_AUTH    0x0D
//#define HDMI_STATE_PHY_RESET    0x0E
#define HDMI_STATE_ERROR        0x00
#define STATE_DEBUG             0xFF


//-----------------------------------------------------------------------------
// Type Definition
//-----------------------------------------------------------------------------

typedef enum _aspect_ratio_t
{
    AR_4to3, /* 4:3 */
    AR_16to9, /* 16:9 */
}
aspect_ratio_t;

typedef enum _TMDS_clk_t
{
    CLK_0_to_50, /* 0-50MHz TMDS clock */
    CLK_50_to_100, /* 50-100MHz TMDS clock */
    CLK_100_to_150, /* 100-150MHz TMDS clock */
    CLK_150_to_200, /* 150-200MHz TMDS clock */
    CLK_200_to_250, /* 200-250MHz TMDS clock */
    CLK_4Kmode, /* 4K mode uses seperate PHY parameters */
}
TMDS_clk_t;

typedef struct _hdmi_PHY_param_t
{
    unsigned char X17_PHY_value;
    unsigned char X56_PHY_value;
    unsigned char X57_PHY_value;
    unsigned char X58_PHY_value;
    unsigned char X59_PHY_value;
    unsigned char X5A_PHY_value;
    unsigned char X5B_PHY_value;
    unsigned char X5C_PHY_value;
    unsigned char X5D_PHY_value;
    unsigned char X5E_PHY_value;
}
hdmi_PHY_param_t;

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------

/*
 * This is the main interrupt hook for HDMI engine.
 */
long hookHDMIInterrupt(
    void (*handler)(void)
);

/*
 * This function un-register HDMI Interrupt handler.
 */
long unhookHDMIInterrupt(
    void (*handler)(void)
);

/*
 *  Function: 
 *      writeHDMIRegister
 *
 *  Input:
 *      addr    - HDMI register index (could be from 0x01 to 0xFF)
 *      value   - HDMI register value
 *
 *  Output:
 *      None
 *
 */
void writeHDMIRegister(unsigned char addr, unsigned char value);

/*
 *  Function: 
 *      readHDMIRegister
 *
 *  Input:
 *      addr    - HDMI register index (could be from 0x01 to 0xFF)
 *
 *  Output:
 *      register value
 *
 */
unsigned char readHDMIRegister(unsigned char addr);

/*
 *  Function: 
 *      writeHDMIControlRegister
 *      This functions writes HDMI System Control register (index 0x00). 
 *
 *  Input:
 *      value   - HDMI register 0x00 value
 *
 *  Output:
 *      None
 *  
 */
void writeHDMIControlRegister(unsigned char value);

/*
 *  Function: 
 *      readHDMIRegister
 *      This functions reads HDMI System Control register (index 0x00). 
 *
 *  Input:
 *      None
 *
 *  Output:
 *      register value
 *
 */
unsigned char readHDMIControlRegister(void);

/*
 *  Function: 
 *      writeHdmiPHYRegister
 *
 *  Input:
 *      addr    - HDMI register index (for PHY registers only, could be from 0x57 to 0x5E)
 *      value   - HDMI register value
 *
 *  Output:
 *      None
 *
 */
void writeHdmiPHYRegister(unsigned char addr, unsigned char value);

/*
 *  Function:
 *      setHDMIChannel
 *
 *  Input:
 *      channel number      -   0 = Select Channel 0 to HDMI
 *                                  -   1 = Select Channel 1 to HDMI
 *
 *  Output:
 *      None
 *
 */
void setHDMIChannel(unsigned char Channel);

/*
 *  Function:
 *      enableHdmiI2C
 *
 *  Input:
 *      enable/disable      -   0 = HDMI I2C to GPIO[7:6]
 *                                  -   1 = HW I2C to GPIO[7:6]
 *
 *  Output:
 *      None
 *
 */
void enableHdmI2C(unsigned long enable);

/*
 *  Function:
 *      HDMI_Dump_Registers (Used for debug)
 *
 *  Input:
 *      None
 *
 *  Output:
 *      None
 *
 */
void HDMI_Dump_Registers (void);

//
// Parameters   : unsigned char mode. 4 modes available.
//                  MODE_A (sleep), MODE_B (register access), MODE_D (clock), MODE_E (active).
//
void HDMI_System_PD (unsigned char mode);

/*
 *  Function:
 *      HDMI_Set_Control_Packet
 *
 *  Input:
 *      None
 *
 *  Output:
 *      None
 *
 */
void HDMI_Init (void);

/*
 *  Function:
 *      HDMI_Set_Mode
 *
 *  Input:
 *      pLogicalMode
 *
 *  Output:
 *      None
 *
 *  Return:
 *      0 - Success
 *      -1 - Error 
 *
 */
long HDMI_Set_Mode (logicalMode_t *pLogicalMode, bool isHDMI);

void HDMI_Enable_Output(void);

/*
 *  Function:
 *      HDMI_Disable_Output
 *
 *  Input:
 *      None
 *
 *  Output:
 *      None
 *
 *  Return:
 *      None
 *
 */
void HDMI_Disable_Output (void);

/*
 *  Function:
 *      HDMI_Unplugged
 *
 *  Input:
 *      None
 *
 *  Return:
 *      None
 *
 */
void HDMI_Unplugged (void);

/*
 *  Function:
 *      HDMI_Audio_Mute
 *
 *  Input:
 *      None
 *
 *  Output:
 *      None
 *
 *  Return:
 *      None
 *
 */
void HDMI_Audio_Mute (void);

/*
 *  Function:
 *      HDMI_Audio_Unmute
 *
 *  Input:
 *      None
 *
 *  Output:
 *      None
 *
 *  Return:
 *      None
 *
 */
void HDMI_Audio_Unmute (void);

/*
 *  Function:
 *      HDMI_Read_Edid.
 *		 
 *  Input:
 *      pEDIDBuffer - EDID buffer
 *      bufferSize - EDID buffer size (usually 128-bytes or 256 bytes)
 *  Output:
 *      -1 - Error
 *      0 - exist block0 EDID (128 Bytes)
 *      1 - exist block0 & block1 EDID (256 Bytes)
 */
long HDMI_Read_Edid(BYTE *pEDIDBuffer, unsigned long bufferSize);

/*
 *  Function:
 *      HDMI_hotplug_check
 *		 
 *  Input:
 *      None
 * 
 *  Output:
 *      0 - unplugged
 *      1 - plugged
 * 
 */
BYTE HDMI_hotplug_check (void);

int hdmi_detect(void);


void hdmiHandler(void);
void Delay (void);
void DelayMs (BYTE millisecond);

#endif  /* _HDMI_HEADER_H_ */
