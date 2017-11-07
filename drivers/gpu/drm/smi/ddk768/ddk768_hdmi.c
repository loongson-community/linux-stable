/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  Hdmi.c ---  
*  This file contains the source code for HDMI functions.
* 
*******************************************************************/



#include <linux/delay.h>
#include <linux/timer.h>

#include "ddk768_help.h"
#include "ddk768_hdmi.h"
#include "ddk768_reg.h"
#include "ddk768_power.h"
#include "hdmiregs.h"


//-----------------------------------------------------------------------------
// Parameter Table
//-----------------------------------------------------------------------------

/*
 *  Hdmi parameter for some popular modes.
 *  Note that the most timings in this table is made according to CEA standard
 *  parameters for the popular modes.
 */
static hdmi_PHY_param_t gHdmiPHYParamTable[] =
{
/* TMDS clock range 0-50 MHz [8bpc] */
 { 0x22, 0x06, 0x00, 0x40, 0x40, 0x1E, 0x94, 0x2E, 0x30, 0x00},

/* TMDS clock range 50-100 MHz [8bpc] */
 { 0x22, 0x0A, 0x00, 0x40, 0x40, 0x1E, 0x94, 0x2E, 0x30, 0x00},

/* TMDS clock range 100-150 MHz [8bpc] */
 { 0x22, 0x0E, 0x00, 0x40, 0x40, 0x1E, 0x92, 0x2E, 0x71, 0x00},

/* TMDS clock range 150-200 MHz [8bpc] */
 { 0x22, 0x0E, 0x00, 0x40, 0x40, 0x1E, 0x94, 0x2E, 0x71, 0x00},

/* TMDS clock range 200-250 MHz [8bpc] */
 { 0x22, 0x0E, 0x00, 0x40, 0x40, 0x1E, 0x94, 0x2E, 0x71, 0x00},

/* 4K mode  [8bpc] */
 { 0x22, 0x0E, 0x00, 0x40, 0x40, 0x1E, 0x95, 0x2E, 0x71, 0x00},

/* End of table. */
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};


//-----------------------------------------------------------------------------
// Global Variable
//-----------------------------------------------------------------------------

BYTE g_INT_94h = 0;
BYTE g_INT_95h = 0;
BYTE PowerMode = PowerMode_A;
BYTE g_ucHDMIedidRead=0;
BYTE AudioMode = 0;
BYTE gEdidBuffer[256] = {0};



void Delay (void)
{
    unsigned long d;
    for (d = 0; d < 0x13FF; d++)
        ;
}

void DelayMs (BYTE millisecond)
{
	mdelay(millisecond);
}

//-----------------------------------------------------------------------------
// HDMI Interrupt functions
//-----------------------------------------------------------------------------
typedef struct _hdmi_interrupt_t
{
    struct _hdmi_interrupt_t *next;
    void (*handler)(void);
}
hdmi_interrupt_t;

static hdmi_interrupt_t *g_pHdmiIntHandlers = ((hdmi_interrupt_t *)0);

/* HDMI Interrupt Service Routine */
void hdmiISR(
    unsigned long status
)
{
    hdmi_interrupt_t *pfnHandler;

    if (FIELD_GET(status, INT_STATUS, HDMI) == INT_STATUS_HDMI_ACTIVE)
    {
        /* Walk all registered handlers for handlers that support this interrupt status */
        for (pfnHandler = g_pHdmiIntHandlers; pfnHandler != ((hdmi_interrupt_t *)0); pfnHandler = pfnHandler->next)
            pfnHandler->handler();
        
        // Save interrupt status value for further usage. 
        if (PowerMode == PowerMode_A)
        {
            // PS mode a->b
            HDMI_System_PD(PowerMode_B);
            DelayMs(1);
        }
        g_INT_94h = readHDMIRegister (X94_INT1_ST);
        g_INT_95h = readHDMIRegister (X95_INT2_ST);

        // clear all interrupt status
        writeHDMIRegister (X94_INT1_ST, 0xFF);
        writeHDMIRegister (X95_INT2_ST, 0xFF);
    }            
}

/*
 * This is the main interrupt hook for HDMI engine.
 */
long hookHDMIInterrupt(
    void (*handler)(void)
)
{

#if 0
	hdmi_interrupt_t *pfnNewHandler, *pfnHandler;
    unsigned short returnValue = 0;

    /* Allocate a new interrupt structure */
    pfnNewHandler = (hdmi_interrupt_t *)malloc(sizeof(hdmi_interrupt_t));
    if (pfnNewHandler == ((hdmi_interrupt_t *)0))
    {
        /* No more memory */
        return (-1);
    }

    /* Make sure that it has not been register more than once */
    for (pfnHandler = g_pHdmiIntHandlers; pfnHandler != ((hdmi_interrupt_t *)0); pfnHandler = pfnHandler->next)
    {
        if (pfnHandler->handler == handler)
            return (-1);                   /* Duplicate entry */
    }
        
    /* If this is the first interrupt handler, register this panel VSync ISR */
    if (g_pHdmiIntHandlers == ((hdmi_interrupt_t *)0))
        returnValue = registerHandler(hdmiISR, FIELD_SET(0, INT_MASK, HDMI, ENABLE));

    if (returnValue == 0)
    {
        /* Fill interrupt structure. */
        pfnNewHandler->next = g_pHdmiIntHandlers;
        pfnNewHandler->handler = handler;
        g_pHdmiIntHandlers = pfnNewHandler;
    }
#endif   
    return 0;//returnValue;
}

/*
 * This function un-register HDMI Interrupt handler.
 */
long unhookHDMIInterrupt(
    void (*handler)(void)
)
{
#if 0
    hdmi_interrupt_t *pfnHandler, *prev;

    /* Find the requested handle to unregister */
    for (pfnHandler = g_pHdmiIntHandlers, prev = ((hdmi_interrupt_t *)0); 
         pfnHandler != ((hdmi_interrupt_t *)0); 
         prev = pfnHandler, pfnHandler = pfnHandler->next)
    {
        if (pfnHandler->handler == handler)
        {
            /* Remove the interrupt handler */
            if (prev == ((hdmi_interrupt_t *)0))
                g_pHdmiIntHandlers = pfnHandler->next;
            else
                prev->next = pfnHandler->next;

            free(pfnHandler);
            
            /* If this was the last interrupt handler, remove the IRQ handler */
            if (g_pHdmiIntHandlers == ((hdmi_interrupt_t *)0))
                unregisterHandler(hdmiISR);
            
            /* Success */
            return (0);
        }
    }
#endif
    /* Oops, handler is not registered */
    return (-1);
}

//-----------------------------------------------------------------------------
// HDMI Misc. functions
//-----------------------------------------------------------------------------

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
 *  
 *  WRITE is a 2 step process. For example, write a value of 0x82 to register with index 0x45, the steps are:
 *  (MMIO base + 0x800C0) = 0x00018245
 *  (MMIO base + 0x800C0) = 0x00008245
 *
 */
void writeHDMIRegister(unsigned char addr, unsigned char value)
{

    pokeRegisterDWord(HDMI_CONFIG, 
        FIELD_SET(0, HDMI_CONFIG, WRITE, ENABLE) | 
        FIELD_VALUE(0, HDMI_CONFIG, DATA, value) | 
        FIELD_VALUE(0, HDMI_CONFIG, ADDRESS, addr));

    DelayMs(1);
    
    pokeRegisterDWord(HDMI_CONFIG, 
        FIELD_VALUE(0, HDMI_CONFIG, DATA, value) | 
        FIELD_VALUE(0, HDMI_CONFIG, ADDRESS, addr));

    DelayMs(1);
}

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
 *  
 *  READ is a 3 step process, For example, read a value from register with index 0x45.
 *  (MMIO base + 0x800C0) = 0x20045
 *  (MMIO base + 0x800C0) = 0x00045
 *  Value = (MMIO base + 0x800C0) =0x1845
 *
 */
unsigned char readHDMIRegister(unsigned char addr)
{
    unsigned long value;

    pokeRegisterDWord(HDMI_CONFIG, 
        FIELD_SET(0, HDMI_CONFIG, READ, ENABLE) | 
        FIELD_VALUE(0, HDMI_CONFIG, ADDRESS, addr));
    
    DelayMs(1);
    
    pokeRegisterDWord(HDMI_CONFIG, FIELD_VALUE(0, HDMI_CONFIG, ADDRESS, addr));
    
    DelayMs(1);

    value = peekRegisterDWord(HDMI_CONFIG);
    
    return (unsigned char)((value >> 8) & 0xFF);
}

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
void writeHDMIControlRegister(unsigned char value)
{

    pokeRegisterDWord(HDMI_CONTROL, value); 
    
}

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
unsigned char readHDMIControlRegister(void)
{
    unsigned long value;

    // Need to write 0x800c0[7:0] (config address) to 00 first, otherwise could not read back
    // real value of HDMI control register. 
    pokeRegisterDWord(HDMI_CONFIG, 
        FIELD_SET(0, HDMI_CONFIG, READ, ENABLE) | 
        FIELD_VALUE(0, HDMI_CONFIG, ADDRESS, 0));
    
    DelayMs(1);
    
    value = peekRegisterDWord(HDMI_CONTROL);
    
    return (unsigned char)(value & 0xFF);
}

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
void writeHdmiPHYRegister(unsigned char addr, unsigned char value)
{
    writeHDMIRegister(addr, value);
    
    writeHDMIControlRegister(0x2D);     // PLLA/B reset
    DelayMs(1);     // wait 100us

    writeHDMIControlRegister(0x21);     // PLLA/B release
    DelayMs(1);     // wait 1ms for PLL lock
}

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
void setHDMIChannel(unsigned char Channel)
{
    unsigned long value;

    value = peekRegisterDWord(DISPLAY_CTRL);
    if (Channel == 0)
    {
        pokeRegisterDWord(DISPLAY_CTRL, 
            FIELD_SET(value, DISPLAY_CTRL, HDMI_SELECT, CHANNEL0));
    }
    else
    {
        pokeRegisterDWord(DISPLAY_CTRL, 
            FIELD_SET(value, DISPLAY_CTRL, HDMI_SELECT, CHANNEL1));
    }

}

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
void enableHdmI2C(unsigned long enable)
{
    unsigned long value;

    value = peekRegisterDWord(TEST_CONTROL);

    if (enable)
        value = FIELD_SET(value, TEST_CONTROL, I2C, HDMI);
    else
        value = FIELD_SET(value, TEST_CONTROL, I2C, I2C1);

    pokeRegisterDWord(TEST_CONTROL, value);
}

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
 *  Linda: for debug purpose only
 */
void HDMI_Dump_Registers (void)
{
    unsigned char i = 0, j = 0;

    printk("++++++++++ HDMI Regsiters 0x00 - 0x7F ++++++++++\n");

    for (i = 0; i < 16; i++)
    {
        printk("Offset 0x%02x:  ", (i * 8));
        for (j = 0; j < 8; j++)
            printk("%02x  ", readHDMIRegister((j + i * 8)));
        printk("\n");
    }
}


//-----------------------------------------------------------------------------
// HDMI Functions
//-----------------------------------------------------------------------------

//
// Parameters   : unsigned char mode. 4 modes available.
//                  MODE_A (sleep), MODE_B (register access), MODE_D (clock), MODE_E (active).
//
void HDMI_System_PD (unsigned char mode) 
{
    PowerMode = mode;

    // PLL A/B Reset
    if (PowerMode != PowerMode_A)
    {
        writeHDMIControlRegister (mode | 0x0C);
       	DelayMs(8);    // wait 8ms
    }
    // PLL A/B Release
    writeHDMIControlRegister (mode);
    DelayMs(10);    // wait 10ms


}

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
void HDMI_Init(void)
{
    unsigned char temp;
    
    // Enable HDMI clock
     ddk768_enableHDMI(1);
    
    // select channel 0 to HDMI by default
    setHDMIChannel(0);
    
    // set INT polarity to Active High
    temp = readHDMIControlRegister();
    writeHDMIControlRegister (temp | 0x01);
    
    // Set to power mode B, in order to read/write to registers
    HDMI_System_PD (PowerMode_B);
    DelayMs(1);

    // Setting AVI InfoFrame
    writeHDMIRegister (X5F_PACKET_INDEX, AVI_INFO_PACKET); // Index.6 AVI InfoFrame
    writeHDMIRegister (X60_PACKET_HB0, 0x82); // HB0
    writeHDMIRegister (X61_PACKET_HB1, 0x02); // HB1
    writeHDMIRegister (X62_PACKET_HB2, 0x0D); // HB2
    writeHDMIRegister (X63_PACKET_PB0, 0x16); // PB0
    writeHDMIRegister (X64_PACKET_PB1, 0x00); // PB1
    writeHDMIRegister (X65_PACKET_PB2, 0x00); // PB2
    writeHDMIRegister (X66_PACKET_PB3, 0x12); // PB3
    writeHDMIRegister (X67_PACKET_PB4, 0x00); // PB4
    writeHDMIRegister (X68_PACKET_PB5, 0x00); // PB5
    writeHDMIRegister (X69_PACKET_PB6, 0xe4); // PB6
    writeHDMIRegister (X6A_PACKET_PB7, 0xb5); // PB7
    writeHDMIRegister (X6B_PACKET_PB8, 0x4e); // PB8
    writeHDMIRegister (X6C_PACKET_PB9, 0x59); // PB9
    writeHDMIRegister (X6D_PACKET_PB10, 0xd2); // PB10
    writeHDMIRegister (X6E_PACKET_PB11, 0xeb); // PB11
    writeHDMIRegister (X6F_PACKET_PB12, 0x18); // PB12
    writeHDMIRegister (X70_PACKET_PB13, 0x1d); // PB13

    // Setting Audio InfoFrame
    writeHDMIRegister (X5F_PACKET_INDEX, AUDIO_INFO_PACKET); // Index.8 Audio
    writeHDMIRegister (X60_PACKET_HB0, 0x84); // HB0
    writeHDMIRegister (X61_PACKET_HB1, 0x01); // HB1
    writeHDMIRegister (X62_PACKET_HB2, 0x0A); // HB2
    writeHDMIRegister (X63_PACKET_PB0, 0x70); // PB0
    writeHDMIRegister (X64_PACKET_PB1, 0x01); // PB1
    writeHDMIRegister (X65_PACKET_PB2, 0x00); // PB2
    writeHDMIRegister (X66_PACKET_PB3, 0x00); // PB3
    writeHDMIRegister (X67_PACKET_PB4, 0x00); // PB4
    writeHDMIRegister (X68_PACKET_PB5, 0x00); // PB5

    // init DDC bus frequency
    writeHDMIRegister (X81_ISRC2_PB0, 0x20);
    writeHDMIRegister (X82_ISRC2_PB1, 0x00);

    // Unmask MSENS detect interrupt. Hot plug interrupt is enough for hot-plug
    // detection, we don't need to detect both at the same time.
    // writeHDMIRegister (X92_INT_MASK1, 0x80);
}

/*
 *  Function:
 *      HDMI_Control_Packet_Auto_Send
 *
 *  Input:
 *      None
 *
 *  Output:
 *      None
 *
 */
void HDMI_Control_Packet_Auto_Send (void)
{
    writeHDMIRegister (X42_AUTO_CHECKSUM, 0x01);    // enable auto checksum
    writeHDMIRegister (X40_CTRL_PKT_EN, 0x00);
}

/*
 *  Function:
 *      HDMI_Audio_Setting_44100Hz
 *
 *  Input:
 *      pLogicalMode
 *
 *  Output:
 *      None
 *
 *  Return:
 *      None
 *
 */
void HDMI_Audio_Setting_44100Hz (mode_parameter_t *pModeParam)
{
    unsigned long N = 6272, CTS = 0;    // default N value is 6272
    unsigned char regValue = 0;

    // calculate CTS with recommended N
    // recommended N value is 4704 for 4K mode
    if ((pModeParam->horizontal_display_end >= 3840 && pModeParam->horizontal_display_end < 5120) && 
        (pModeParam->vertical_display_end >= 2160 && pModeParam->vertical_display_end < 2880))
    {
        N = 4704;
    }
    
    // CTS = (fTMDS_clock x N) / (128 X fS)
    CTS = ((pModeParam->pixel_clock / 1000) * N / 128) * 10 / 441;
    //printf("44100Hz: N = %d, CTS = %d\n", N, CTS);

    // set N and CTS into registers
    regValue = (unsigned char)((N >> 16) & 0x0F);
    writeHDMIRegister(X01_N19_16, regValue);
    regValue = (unsigned char)(N >> 8);
    writeHDMIRegister(X02_N15_8, regValue);
    regValue = (unsigned char)N;
    writeHDMIRegister(X03_N7_0, regValue);

    regValue = (unsigned char)((CTS >> 16) & 0x0F);
    writeHDMIRegister(X07_CTS_EXT, regValue);
    regValue = (unsigned char)(CTS >> 8);
    writeHDMIRegister(X08_CTS_EXT, regValue);
    regValue = (unsigned char)CTS;
    writeHDMIRegister(X09_CTS_EXT, regValue);

    // set audio setting registers
    writeHDMIRegister(X0A_AUDIO_SOURCE, 0x00);      // internal CTS
    writeHDMIRegister(X0B_AUDIO_SET2, 0x40);
    writeHDMIRegister(X0C_I2S_MODE, 0x04);      // I2S 2ch (0x3C for 8ch) + I2S
    //writeHDMIRegister(X0D_DSD_MODE, 0x00);      // DSD audio disabled
    writeHDMIRegister(X10_I2S_PINMODE, 0x00);      // I2S input pin swap
    writeHDMIRegister(X11_ASTATUS1, 0x0F);      // Original frequency not indicated(defult)
    writeHDMIRegister(X12_ASTATUS2, 0x22);
    writeHDMIRegister(X13_CAT_CODE, 0x00);
    writeHDMIRegister(X14_A_SOURCE, 0x02);
    
    regValue = (readHDMIRegister(X15_AVSET1) & 0x0F);       // set freq 44.1kHz
    writeHDMIRegister(X15_AVSET1, regValue);

    regValue = readHDMIRegister(X0A_AUDIO_SOURCE) & 0x9F;
    writeHDMIRegister(X0A_AUDIO_SOURCE, regValue);      // dounsampling none (bit 6:5 = 00)
    
    regValue = readHDMIRegister(X0A_AUDIO_SOURCE) & 0xF7;
    writeHDMIRegister(X0A_AUDIO_SOURCE, regValue);      // disable SPDIF
    
}

/*
 *  Function:
 *      HDMI_Video_Setting
 *
 *  Input:
 *      pModeParam
 *
 *  Output:
 *      None
 *
 *  Return:
 *      None
 *
 */
void HDMI_Video_Setting (mode_parameter_t *pModeParam, bool isHDMI)
{
    unsigned long temp = 0;
    unsigned char regValue = 0;

    // video set timing
    regValue = readHDMIRegister(X30_EXT_VPARAMS);
    regValue &= 0xF2;
    regValue = 
        (pModeParam->vertical_sync_polarity == POS
        ? FIELD_SET(regValue, X30_EXT_VPARAMS, VSYNC_PHASE, POS)
        : FIELD_SET(regValue, X30_EXT_VPARAMS, VSYNC_PHASE, NEG))
        | (pModeParam->horizontal_sync_polarity == POS
        ? FIELD_SET(regValue, X30_EXT_VPARAMS, HSYNC_PHASE, POS)
        : FIELD_SET(regValue, X30_EXT_VPARAMS, HSYNC_PHASE, NEG))
        | FIELD_SET(regValue, X30_EXT_VPARAMS, USE, EXTERNAL);
    writeHDMIRegister(X30_EXT_VPARAMS, regValue);

    writeHDMIRegister(X31_EXT_HTOTAL, (pModeParam->horizontal_total & 0xFF));   // horizontal total
    writeHDMIRegister(X32_EXT_HTOTAL, (pModeParam->horizontal_total & 0xFF00)>>8);

    temp = pModeParam->horizontal_total - pModeParam->horizontal_display_end;   // horizontal blanking
    writeHDMIRegister(X33_EXT_HBLANK, (temp & 0xFF));
    writeHDMIRegister(X34_EXT_HBLANK, (temp & 0xFF00)>>8);

    temp = pModeParam->horizontal_total - pModeParam->horizontal_sync_start;    // horizontal delay
    writeHDMIRegister(X35_EXT_HDLY, (temp & 0xFF));
    writeHDMIRegister(X36_EXT_HDLY, (temp & 0xFF00)>>8);

    writeHDMIRegister(X37_EXT_HS_DUR, (pModeParam->horizontal_sync_width & 0xFF));  // horizontal duration
    writeHDMIRegister(X38_EXT_HS_DUR, (pModeParam->horizontal_sync_width & 0xFF00)>>8);

    writeHDMIRegister(X39_EXT_VTOTAL, (pModeParam->vertical_total & 0xFF)); // vertical total
    writeHDMIRegister(X3A_EXT_VTOTAL, (pModeParam->vertical_total & 0xFF00)>>8);

    temp = pModeParam->vertical_total - pModeParam->vertical_display_end;   // vertical blanking
    writeHDMIRegister(X3D_EXT_VBLANK, temp);

    temp = pModeParam->vertical_total - pModeParam->vertical_sync_start;   // vertical delay
    writeHDMIRegister(X3E_EXT_VDLY, temp);

    writeHDMIRegister(X3F_EXT_VS_DUR, pModeParam->vertical_sync_height);    // vertical duration

    // video set color - deep_color_8bit
    regValue = readHDMIRegister(X17_DC_REG);
    regValue = (regValue & 0x3F) | 0x00;
    writeHDMIRegister(X17_DC_REG, regValue);
    
    writeHDMIRegister(X16_VIDEO1, 0x30);

    // video set format
    regValue = (readHDMIRegister(X15_AVSET1) & 0xF0);
    regValue |= 0x01;       // set RGB & external DE
    writeHDMIRegister(X15_AVSET1, regValue);
    writeHDMIRegister(X3B_AVSET2, 0x40);
    //writeHDMIRegister(X40_CTRL_PKT_EN, 0x00);
    //writeHDMIRegister(X45_VIDEO2, 0x83);
    writeHDMIRegister(X46_OUTPUT_OPTION, 0x04);
    writeHDMIRegister(XD3_CSC_CONFIG1, 0x01);

    // video set output - setting to HDMI/DVI mode
    regValue = readHDMIRegister(XAF_HDCP_CTRL);
    if (isHDMI)
        writeHDMIRegister(XAF_HDCP_CTRL, (regValue | 0x02));
    else
        writeHDMIRegister(XAF_HDCP_CTRL, (regValue & 0xFD));

    // set DDC bus access frequency control register based on pixel clock value (400kHz is preferred)
    // At mode_d/mode_e: 
    // DDC Bus access frequency = TDMS_CK input clock frequency / (register value) / 4
    temp = pModeParam ->pixel_clock / 4 / 400000;
    writeHDMIRegister(X81_ISRC2_PB0, (temp & 0xFF));    // LSB
    writeHDMIRegister(X82_ISRC2_PB1, (temp & 0xFF00)>>8);   // MSB
    
}

/*
 *  Function:
 *      HDMI_PHY_Setting
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
long HDMI_PHY_Setting (mode_parameter_t *pModeParam)
{
    unsigned long clkIndex;
    hdmi_PHY_param_t *pPHYParamTable;

    // Decide which PHY params to use depend on TMDS clock range. 
    if (pModeParam->pixel_clock > 0 && pModeParam->pixel_clock < 50000000)
    {
        clkIndex = CLK_0_to_50;
    }
    else if (pModeParam->pixel_clock >= 50000000 && pModeParam->pixel_clock < 100000000)
    {
        clkIndex = CLK_50_to_100;
    }
    else if (pModeParam->pixel_clock >= 100000000 && pModeParam->pixel_clock < 150000000)
    {
        clkIndex = CLK_100_to_150;
    }
    else if (pModeParam->pixel_clock >= 150000000 && pModeParam->pixel_clock < 200000000)
    {
        clkIndex = CLK_150_to_200;
    }
    else if (pModeParam->pixel_clock >= 200000000 && pModeParam->pixel_clock < 250000000)
    {
        clkIndex = CLK_200_to_250;
    }
    else if (pModeParam->pixel_clock >= 250000000 && pModeParam->pixel_clock <= 297000000)
    {
        clkIndex = CLK_4Kmode;
    }
    else
    {
        // Does not support TMDS clock larger than 297MHz.
        return (-1);
    }

    pPHYParamTable = (hdmi_PHY_param_t *)gHdmiPHYParamTable;
    
    // load PHY parameters into registers
    writeHdmiPHYRegister(X57_PHY_CTRL, pPHYParamTable[clkIndex].X57_PHY_value);
    writeHdmiPHYRegister(X58_PHY_CTRL, pPHYParamTable[clkIndex].X58_PHY_value);
    writeHdmiPHYRegister(X59_PHY_CTRL, pPHYParamTable[clkIndex].X59_PHY_value);
    writeHdmiPHYRegister(X5A_PHY_CTRL, pPHYParamTable[clkIndex].X5A_PHY_value);
    writeHdmiPHYRegister(X5B_PHY_CTRL, pPHYParamTable[clkIndex].X5B_PHY_value);
    writeHdmiPHYRegister(X5C_PHY_CTRL, pPHYParamTable[clkIndex].X5C_PHY_value);
    writeHdmiPHYRegister(X5D_PHY_CTRL, pPHYParamTable[clkIndex].X5D_PHY_value);
    writeHdmiPHYRegister(X5E_PHY_CTRL, pPHYParamTable[clkIndex].X5E_PHY_value);
    writeHdmiPHYRegister(X56_PHY_CTRL, pPHYParamTable[clkIndex].X56_PHY_value);
    writeHdmiPHYRegister(X17_DC_REG, pPHYParamTable[clkIndex].X17_PHY_value);
    
    return 0;
}

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
long HDMI_Set_Mode (logicalMode_t *pLogicalMode, bool isHDMI)
{
    mode_parameter_t *pModeParam;
    unsigned char temp = 0;
    unsigned long ret = 0;
    
    // set mode b
    if (PowerMode != PowerMode_B)
    {
        HDMI_System_PD (PowerMode_B);
    }

    // find mode parameters for input mode
    pModeParam = ddk768_findModeParam(pLogicalMode->dispCtrl, pLogicalMode->x, pLogicalMode->y, pLogicalMode->hz, 0);
    if (pModeParam == (mode_parameter_t *)0)
        return -1;

    // set video param
    HDMI_Video_Setting(pModeParam, isHDMI);

    // set audio param
    HDMI_Audio_Setting_44100Hz(pModeParam);
    
    // control packet auto send
    HDMI_Control_Packet_Auto_Send();

    // set PHY param
    ret = HDMI_PHY_Setting(pModeParam);
    if (ret != 0)
    {
        return ret;
    }

    // disable video & audio output: write 11b to #45h[1:0]
    temp = readHDMIRegister(X45_VIDEO2);
    writeHDMIRegister(X45_VIDEO2, (temp | 0x03));

    // set Channel # to HDMI
    setHDMIChannel((unsigned char)pLogicalMode->dispCtrl);
    
    // mode b->d: (0x4d, 100us) -> (0x49, 100us) -> 0x41
    PowerMode = PowerMode_D;
    writeHDMIControlRegister (PowerMode | 0x0C);
    DelayMs (1);
    writeHDMIControlRegister (PowerMode | 0x08);
    DelayMs (1);
    writeHDMIControlRegister (PowerMode);
    DelayMs (1);

    // mode d->e: 0x81
    HDMI_System_PD (PowerMode_E);

    if (AudioMode)
    {
        // enable video & audio output: write 00b to #45h[1:0]
        temp = readHDMIRegister(X45_VIDEO2);
        writeHDMIRegister(X45_VIDEO2, (temp & (~0x03)));

        // Audio reset/release
        // Audio is mute after reset of audio is set.
        // Therefore, set it in the following procedures.
        //   Audio:  Save value of now => Audio Reset => Audio Active => Set value again
        temp = readHDMIRegister(X45_VIDEO2);
        writeHDMIRegister(X45_VIDEO2, temp | 0x04 );   // Reset
        DelayMs(1);                                    // Followed by 1ms wait time
        writeHDMIRegister(X45_VIDEO2, temp & 0xFB );   // Reset Release and Audio Mute)
    }
    else
    {
        // enable video output: write 0b to #45h[0]
        temp = readHDMIRegister(X45_VIDEO2);
        writeHDMIRegister(X45_VIDEO2, (temp & (~0x01)));
    }

#if 0   // for debug only
    HDMI_Dump_Registers();
#endif
    
    return 0;
}
void HDMI_Enable_Output(void)
{
    unsigned char temp = 0;

	if (PowerMode == PowerMode_E)
		return;

	if (PowerMode == PowerMode_B){
	    // mode b->d: (0x4d, 100us) -> (0x49, 100us) -> 0x41
	    PowerMode = PowerMode_D;
	    writeHDMIControlRegister (PowerMode | 0x0C);
	    DelayMs (1);
	    writeHDMIControlRegister (PowerMode | 0x08);
	    DelayMs (1);
	    writeHDMIControlRegister (PowerMode);
	    DelayMs (1);

	    // mode d->e: 0x81
	    HDMI_System_PD (PowerMode_E);
	}

    // enable video & audio output: write 00b to #45h[1:0]
    temp = readHDMIRegister(X45_VIDEO2);
    writeHDMIRegister(X45_VIDEO2, (temp & (~0x03)));
    
    // Audio reset/release
    // Audio is mute after reset of audio is set.
    // Therefore, set it in the following procedures.
    //   Audio:  Save value of now => Audio Reset => Audio Active => Set value again
    temp = readHDMIRegister(X45_VIDEO2);
    writeHDMIRegister(X45_VIDEO2, temp | 0x04 );   // Reset
    DelayMs(1);                                    // Followed by 1ms wait time
    writeHDMIRegister(X45_VIDEO2, temp & 0xFB );   // Reset Release and Audio Mute)

#if 0   // for debug only
    HDMI_Dump_Registers();
#endif
    
    return ;

}

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
void HDMI_Disable_Output (void)
{
    unsigned char temp = 0;
    
    // disable video & audio output: write 11b to #45h[1:0]
    temp = readHDMIRegister(X45_VIDEO2);
    writeHDMIRegister(X45_VIDEO2, (temp | 0x03));

    // audio reset: write 1b to #45h[2], followed by 500us wait time
    temp = readHDMIRegister(X45_VIDEO2);
    writeHDMIRegister(X45_VIDEO2, (temp | 0x04));
    DelayMs(1);

    // PS mode e->d->b
    HDMI_System_PD (PowerMode_D);
    HDMI_System_PD (PowerMode_B);
}

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
void HDMI_Unplugged (void)
{
    unsigned char temp = 0;
    
    // disable video & audio output: write 11b to #45h[1:0]
    temp = readHDMIRegister(X45_VIDEO2);
    writeHDMIRegister(X45_VIDEO2, (temp | 0x03));

    // PS mode e->d->b->a
    HDMI_System_PD (PowerMode_D);
    HDMI_System_PD (PowerMode_B);
    HDMI_System_PD (PowerMode_A);
}

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
void HDMI_Audio_Mute (void)
{
    unsigned char temp = 0;

    if (PowerMode == PowerMode_E)
    {
        // disable audio output: write 1b to #45h[1]
        temp = readHDMIRegister(X45_VIDEO2);
        writeHDMIRegister(X45_VIDEO2, (temp | 0x02));
    }

	 AudioMode = Audio_Mute;
}

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
void HDMI_Audio_Unmute (void)
{
    unsigned char temp = 0;

    if (PowerMode == PowerMode_E)
    {
        // enable audio output: write 0b to #45h[1]
        temp = readHDMIRegister(X45_VIDEO2);
        writeHDMIRegister(X45_VIDEO2, (temp & (~0x02)));

        // Audio reset/release
        // Audio is mute after reset of audio is set.
        // Therefore, set it in the following procedures.
        //   Audio:  Save value of now => Audio Reset => Audio Active => Set value again
        temp = readHDMIRegister(X45_VIDEO2);
        writeHDMIRegister(X45_VIDEO2, temp | 0x04 );   // Reset
        DelayMs(1);                                    // Followed by 1ms wait time
        writeHDMIRegister(X45_VIDEO2, temp & (~0x04) );   // Reset Release and Audio Mute)
    }
	AudioMode = Audio_Unmute;
}

/*
 *  Function:
 *      HDMI_Edid_ReadFirstByte
 *
 *  Input:
 *      None
 *
 *  Return:
 *      Fisrt byte of HDMI EDID FIFO
 *
 */
unsigned char HDMI_Edid_ReadFirstByte(void)
{
    unsigned long value;

    pokeRegisterDWord(HDMI_CONFIG, 
        FIELD_SET(0, HDMI_CONFIG, READ, LATCH) | 
        FIELD_VALUE(0, HDMI_CONFIG, ADDRESS, X80_EDID));
    
    DelayMs(1);
    
    pokeRegisterDWord(HDMI_CONFIG, FIELD_VALUE(0, HDMI_CONFIG, ADDRESS, X80_EDID));
    
    DelayMs(1);
    
    value = peekRegisterDWord(HDMI_CONFIG);

    return (unsigned char)((value >> 8) & 0xFF);
}

/*
 *  Function:
 *      HDMI_Edid_CheckSum
 *
 *  Input:
 *      array - EDID data
 *      size - size of array
 *
 *  Return:
 *      byChecksum
 *
 */
BYTE HDMI_Edid_CheckSum (BYTE* array, unsigned long size)
{
    BYTE i, sum = 0;
    for (i = 0; i < size; i++)
        sum += array[i];

    return sum;
}

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
long HDMI_Read_Edid(BYTE *pEDIDBuffer, unsigned long bufferSize)
{
    BYTE byEDID_current = 0;
    BYTE byEDID_size = 0;
    BYTE byEDID_finish = 0;
    #define EDID_WORD   ((byEDID_current % 2) ? 0x80 : 0x00)
    #define EDID_SEG    (byEDID_current /2)
    #define EDID_EXT    (gEdidBuffer[126])
    
    BYTE byChecksum = 0, regValue;
    unsigned long i = 0, j = 0;
    unsigned long retry = 1000;
            
    if (pEDIDBuffer == (unsigned char *)0)
    {
        printk("buffer is NULL!\n");
        return (-1);
    }
    
    // PS mode a -> b if current power mode is in PS mode a
    if (PowerMode == PowerMode_A)
    {
        HDMI_System_PD (PowerMode_B);
    }

    // clear interrupt status before reading EDID
    writeHDMIRegister (X94_INT1_ST, 0xFF);
    writeHDMIRegister (X95_INT2_ST, 0xFF);

    // Enable EDID interrupt
    regValue = readHDMIRegister (X92_INT_MASK1);
    writeHDMIRegister (X92_INT_MASK1, (regValue | 0x06));

    while(byEDID_finish == 0)
    {			
    	// Set EDID word address (set to 00h for the first 128 bytes)
    	writeHDMIRegister (XC5_EDID_WD_ADDR, EDID_WORD);	
    	// Set EDID segment pointer 0
    	// (Regsiter write to XC4_SEG_PTR will start EDID reading)
    	writeHDMIRegister (XC4_SEG_PTR, EDID_SEG);
        
    	/* Hook the interrupt before going to the while */ 
        //hookHDMIInterrupt(HdmiHandler);
    	
        retry = 1000;
        while(retry)
    	{		
            retry--;

            g_INT_94h = readHDMIRegister (X94_INT1_ST);
            g_INT_95h = readHDMIRegister (X95_INT2_ST);	

            // EDID ERR interrupt, or EDID not ready
            if ((g_INT_94h & EDID_ERR))
            {
                // clear error interrupt 
                writeHDMIRegister (X94_INT1_ST, 0xFF);
                writeHDMIRegister (X95_INT2_ST, 0xFF);
                DelayMs(1);
            }
            else if (g_INT_94h & EDID_RDY)
            {		
                // clear ready interrupt 
                writeHDMIRegister (X94_INT1_ST, 0xFF);
                writeHDMIRegister (X95_INT2_ST, 0xFF);
                
                // Read EDID for current block (128bytes)         
                gEdidBuffer[byEDID_current* 0x80] = HDMI_Edid_ReadFirstByte();
                
                for(i=1;i<128;i++)
                {
                    gEdidBuffer[byEDID_current* 0x80+i] = readHDMIRegister(X80_EDID);
                    DelayMs(1);
                }
                printk("EDID read finish\n");
                
                // Calculate EDID data byChecksum 
                byChecksum = HDMI_Edid_CheckSum(gEdidBuffer + (byEDID_current * 0x80), 128);                     
              //  printf("checksum = %x\n", byChecksum);
                
                if (byChecksum != 0)
                {
                    // Return fail
                    printk("Block %x Checksum != 0, fail.\n",byEDID_current);
                    if (byEDID_current == 0)
                    {
                        // Disable EDID interrupt
                        regValue = readHDMIRegister (X92_INT_MASK1);
                        writeHDMIRegister (X92_INT_MASK1, (regValue & (~0x06)));

                        return (-1);
                    }
                    else
                    {
                        // Disable EDID interrupt
                        regValue = readHDMIRegister (X92_INT_MASK1);
                        writeHDMIRegister (X92_INT_MASK1, (regValue & (~0x06)));

                     //   DDKDEBUGPRINT((DISPLAY_LEVEL, "Return the first 128 bytes only.\n"));
                        // Copy 128 bytes data to the given buffer
                        for (i = 0; i < 128; i++)
                        {
                          //  printf("%x ", gEdidBuffer[i]);
                            pEDIDBuffer[i] = gEdidBuffer[i];
                        }
                        printk("\n");
                        
                        return 0;   // read first 128 bytes successfully, extension 128 bytes failed. 
                    }
                }
                else
                {
                    //Read Extension flag of EDID
                    byEDID_size = EDID_EXT;

                    if ((byEDID_size==1) && (byEDID_current < 1))
                    {
                        // Move to next EDID block
                        byEDID_current++;
                        printk("Exist extern EDID information.\n");
                        // EDID read block1
                        //byEDID_STATE = HDMI_STATE_EDID_START;	
                        break;
                    }
                    else
                    {
                        printk("HDMI EDID Finished.\n");
                        byEDID_finish = 1;
                        break;
                    }  
                    
                }
                            
            }
        }
        
        if (retry == 0)
        {
            printk("Read HDMI EDID fail.\n");
            // Disable EDID interrupt
            regValue = readHDMIRegister (X92_INT_MASK1);
            writeHDMIRegister (X92_INT_MASK1, (regValue & (~0x06)));
            
            return (-1);
        }
        /* unhook the interrupt */
        //unhookHDMIInterrupt(HdmiHandler);
    }
                   	
    // Disable EDID interrupt
    regValue = readHDMIRegister (X92_INT_MASK1);
    writeHDMIRegister (X92_INT_MASK1, (regValue & (~0x06)));

    // Copy data to the given buffer
   // printf("Copy buffer\n");
    for (i = 0; i < bufferSize; i++)
    {
     //   printf("%x ", gEdidBuffer[i]);
        pEDIDBuffer[i] = gEdidBuffer[i];
    }

#if 0   // for debug only
    // Print EDID data
    for (j = 0; j < 8; j++)
    {
        printf("0x%02x: ", (j*16));
        for (i = 0; i < 16; i++)
            printf("%02x ", gEdidBuffer[(j*16+i)]);
        printf("\n");
    }
    if (byEDID_current > 0)
    {
        printf("E_EDID:\n");
        for (j = 8; j < 16; j++)
        {
            printf("0x%02x: ", (j*16));
            for (i = 0; i < 16; i++)
                printf("%02x ", gEdidBuffer[(j*16+i)]);
            printf("\n");
        }
        printf("\n");
    }
#endif

    // Return EDID block number
    return byEDID_current;
}

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
BYTE HDMI_hotplug_check (void)
{
    BYTE STAT_DFh;

    // Wait time before check hot plug & MSENS pin status
    DelayMs (15);

    STAT_DFh = readHDMIRegister (XDF_HPG_STATUS);

    if ((STAT_DFh & HPG_MSENS) == HPG_MSENS)        // HPD & MSENS status both high? 
    {
        // DDC I2C master controller reset ... ddc_ctrl_reset[bit4]
        writeHDMIRegister (X3B_AVSET2, readHDMIRegister (X3B_AVSET2) | 0x10);
        writeHDMIRegister (X3B_AVSET2, readHDMIRegister (X3B_AVSET2) & 0xEF);
        
        return 1;
    }
    else
    {
        return 0;
    }

}


int hdmi_detect(void)
{
    unsigned int intStatus;


    intStatus = peekRegisterDWord(INT_STATUS);
	
    if (FIELD_GET(intStatus, INT_STATUS, HDMI) == INT_STATUS_HDMI_ACTIVE)
    {
        

        if (PowerMode == PowerMode_A)
        {
            // PS mode a->b
            HDMI_System_PD(PowerMode_B);
        }

        // Save interrupt status from the last interrupt
        g_INT_94h = readHDMIRegister(X94_INT1_ST);
        g_INT_95h = readHDMIRegister(X95_INT2_ST);

        // check if plug-in or plug-out detect
        if ((g_INT_94h & HPG_MSENS) == HPG_MSENS)        // HPD & MSENS status both high? 
        {
            // clear all interrupts
            writeHDMIRegister(X94_INT1_ST, 0xFF);
            writeHDMIRegister(X95_INT2_ST, 0xFF);

            if (HDMI_hotplug_check())
            {
                return 1;
            }
            else
            {
                return 0;
            }

        }

    }
   return 3; //nothing to do 
}




