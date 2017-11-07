/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  DISPLAY.C --- Voyager GX SDK 
*  This file contains the source code for the panel and CRT functions.
* 
*******************************************************************/
#include "ddk750_defs.h"
#include "ddk750_hardware.h"
#include "ddk750_chip.h"
//#include "ddk750/ddk750_swi2c.h"
#include "ddk750_display.h"
//#include "ddk750/ddk750_dvi.h"
#include "ddk750_power.h"
#include "ddk750_help.h"

#define Validate_718_AA     1

/* When use actual Panel, then do not need to turn on the DAC. */
#define ENABLE_PANEL_DAC 

/* Monitor Detection RGB Default Threshold values */
#define DEFAULT_MON_DETECTION_THRESHOLD         0x64


/*
 * This function checks whether dual panel is enabled or not
 *
 * Output:
 *      0   - Not Enable
 *      1   - Enable  
 */
unsigned char isDualPanelEnable(void)
{
    unsigned long value;

    value = FIELD_GET(peekRegisterDWord(PRIMARY_DISPLAY_CTRL), PRIMARY_DISPLAY_CTRL, DUAL_DISPLAY);
    
    return ((value == PRIMARY_DISPLAY_CTRL_DUAL_DISPLAY_ENABLE) ? 1 : 0);
}

/*
 * This function gets the panel type
 *
 * Output:
 *      panelType   - The type of the panel to be set  
 */
panel_type_t getPanelType(void)
{
    if (FIELD_GET(peekRegisterDWord(PRIMARY_DISPLAY_CTRL), PRIMARY_DISPLAY_CTRL, DOUBLE_PIXEL) == 
        PRIMARY_DISPLAY_CTRL_DOUBLE_PIXEL_ENABLE)
    {
        return TFT_36BIT;
    }
    else if (isDualPanelEnable() == 1)
        return TFT_18BIT;
    else
        return TFT_24BIT;
}

/*
 * This function sets the panel type
 *
 * Input:
 *      panelType   - The type of the panel to be set  
 */
long setPanelType(
    panel_type_t panelType
)
{
    unsigned long value;
            
    /* Set the panel type. */
    value = peekRegisterDWord(PRIMARY_DISPLAY_CTRL);
    switch (panelType)
    {
        case TFT_18BIT:
            value = FIELD_SET(value, PRIMARY_DISPLAY_CTRL, DUAL_DISPLAY, ENABLE);
            value = FIELD_SET(value, PRIMARY_DISPLAY_CTRL, DOUBLE_PIXEL, DISABLE);
            break;
        case TFT_24BIT:
            value = FIELD_SET(value, PRIMARY_DISPLAY_CTRL, DUAL_DISPLAY, DISABLE);
            value = FIELD_SET(value, PRIMARY_DISPLAY_CTRL, DOUBLE_PIXEL, DISABLE);
            break;
        case TFT_36BIT:
            value = FIELD_SET(value, PRIMARY_DISPLAY_CTRL, DUAL_DISPLAY, DISABLE);
            value = FIELD_SET(value, PRIMARY_DISPLAY_CTRL, DOUBLE_PIXEL, ENABLE);
            break;
        default:
            return (-1);
    }
    pokeRegisterDWord(PRIMARY_DISPLAY_CTRL, value);
    
    //DDKDEBUGPRINT((DISPLAY_LEVEL, "(setPanelType)PRIMARY_DISPLAY_CTRL: %x\n", peekRegisterDWord(PRIMARY_DISPLAY_CTRL)));
    
    return 0;
}

/*
 * Use vertical sync as time delay function.
 *
 * Input:
 *          dispControl - Display Control (either panel or crt) 
 *          vsync_count - Number of vertical sync to wait.
 *
 * Note:
 *      This function is waiting for the next vertical sync.         
 */
void waitNextVerticalSync(disp_control_t dispControl, unsigned long vsync_count)
{
    unsigned long status;
    unsigned long ulLoopCount = 0;
    static unsigned long ulDeadLoopCount = 10000;
    
    if (dispControl == PRIMARY_CTRL)
    {
        /* Do not wait when the Primary PLL is off or display control is already off. 
           This will prevent the software to wait forever. */
        if ((FIELD_GET(peekRegisterDWord(PRIMARY_PLL_CTRL), PRIMARY_PLL_CTRL, POWER) ==
             PRIMARY_PLL_CTRL_POWER_OFF) ||
            (FIELD_GET(peekRegisterDWord(PRIMARY_DISPLAY_CTRL), PRIMARY_DISPLAY_CTRL, TIMING) ==
             PRIMARY_DISPLAY_CTRL_TIMING_DISABLE))
        {
            return;
        }

        while (vsync_count-- > 0)
        {
            ulLoopCount = 0;
            /* Wait for end of vsync. */
            do
            {
                status = FIELD_GET(peekRegisterDWord(SYSTEM_CTRL),
                                   SYSTEM_CTRL,
                                   PRIMARY_VSYNC);
                if(ulLoopCount++ > ulDeadLoopCount)
                    break;
            }
            while (status == SYSTEM_CTRL_PRIMARY_VSYNC_ACTIVE);

            ulLoopCount = 0;
            /* Wait for start of vsync. */
            do
            {
                status = FIELD_GET(peekRegisterDWord(SYSTEM_CTRL),
                                   SYSTEM_CTRL, 
                                   PRIMARY_VSYNC);
                if(ulLoopCount++ > ulDeadLoopCount)
                    break;
            }
            while (status == SYSTEM_CTRL_PRIMARY_VSYNC_INACTIVE);
        }
    }
    else
    {
        /* Do not wait when the display control is already off. This will prevent
           the software to wait forever. */
        if ((FIELD_GET(peekRegisterDWord(SECONDARY_PLL_CTRL), SECONDARY_PLL_CTRL, POWER) ==
             SECONDARY_PLL_CTRL_POWER_OFF) ||
            (FIELD_GET(peekRegisterDWord(SECONDARY_DISPLAY_CTRL), SECONDARY_DISPLAY_CTRL, TIMING) ==
             SECONDARY_DISPLAY_CTRL_TIMING_DISABLE))
        {
            return;
        }

        while (vsync_count-- > 0)
        {
            ulLoopCount = 0;
            /* Wait for end of vsync. */
            do
            {
                status = FIELD_GET(peekRegisterDWord(SYSTEM_CTRL),
                                   SYSTEM_CTRL,
                                   SECONDARY_VSYNC);
                if(ulLoopCount++ > ulDeadLoopCount)
                    break;                
            }
            while (status == SYSTEM_CTRL_SECONDARY_VSYNC_ACTIVE);

            ulLoopCount = 0;
            /* Wait for start of vsync. */
            do
            {
                status = FIELD_GET(peekRegisterDWord(SYSTEM_CTRL),
                                   SYSTEM_CTRL, 
                                   SECONDARY_VSYNC);
                if(ulLoopCount++ > ulDeadLoopCount)
                    break;                
            }
            while (status == SYSTEM_CTRL_SECONDARY_VSYNC_INACTIVE);
        }
    }
}

/*
 * Use Primary vertical sync as time delay function.
 * Input: Number of vertical sync to wait.
 */
void primaryWaitVerticalSync(unsigned long vsync_count)
{
    waitNextVerticalSync(PRIMARY_CTRL, vsync_count);
}

/*
 * Use crt vertical sync as time delay function.
 * Input: Number of vertical sync to wait.
 */
void secondaryWaitVerticalSync(unsigned long vsync_count)
{
    waitNextVerticalSync(SECONDARY_CTRL, vsync_count);
}

/*
 * Use panel vertical sync line as time delay function.
 * This function does not wait for the next VSync. Instead, it will wait
 * until the current line reaches the Vertical Sync line.
 * This function is really useful when flipping display to prevent tearing.
 *
 * Input: display control (PRIMARY_CTRL or SECONDARY_CTRL)
 */
void waitVSyncLine(disp_control_t dispControl)
{
    unsigned long value;
    mode_parameter_t modeParam;
    
    /* Get the current mode parameter of the specific display control */
    modeParam = getCurrentModeParam(dispControl);
    
    do
    {
        if (dispControl == PRIMARY_CTRL)
            value = FIELD_GET(peekRegisterDWord(PRIMARY_CURRENT_LINE), PRIMARY_CURRENT_LINE, LINE);
        else
            value = FIELD_GET(peekRegisterDWord(SECONDARY_CURRENT_LINE), SECONDARY_CURRENT_LINE, LINE);
    }
    while (value < modeParam.vertical_sync_start);
}


void swPanelPowerSequence_SM750LE(disp_state_t dispState, unsigned long vsync_delay)
{
    unsigned long ulDispControl, ulMask;

    ulDispControl = peekRegisterDWord(DISPLAY_CONTROL_750LE);

    if (dispState == DISP_ON)
    {
        ulMask = FIELD_SET(0, DISPLAY_CONTROL_750LE, EN,   HIGH) |
                 FIELD_SET(0, DISPLAY_CONTROL_750LE, BIAS, HIGH) |
                 FIELD_SET(0, DISPLAY_CONTROL_750LE, DATA, ENABLE) |
                 FIELD_SET(0, DISPLAY_CONTROL_750LE, VDD,  HIGH);
        
        pokeRegisterDWord(DISPLAY_CONTROL_750LE, (ulDispControl | ulMask));
    }
    else
    {
        ulMask = FIELD_SET(0, DISPLAY_CONTROL_750LE, EN,   LOW) |
                 FIELD_SET(0, DISPLAY_CONTROL_750LE, BIAS, LOW) |
                 FIELD_SET(0, DISPLAY_CONTROL_750LE, DATA, DISABLE) |
                 FIELD_SET(0, DISPLAY_CONTROL_750LE, VDD,  LOW);
        
        pokeRegisterDWord(DISPLAY_CONTROL_750LE, (ulDispControl & ulMask));
    }
}

/*
 * This functions uses software sequence to turn on/off the panel.
 */
void swPanelPowerSequence(disp_state_t dispState, unsigned long vsync_delay)
{
    unsigned long primaryControl = peekRegisterDWord(PRIMARY_DISPLAY_CTRL);

    //DDKDEBUGPRINT((DISPLAY_LEVEL, "swPanelPowerSequence +\n"));

    if (dispState == DISP_ON)
    {
        /* Turn on FPVDDEN. */
        primaryControl = FIELD_SET(primaryControl, PRIMARY_DISPLAY_CTRL, FPVDDEN, HIGH);
        pokeRegisterDWord(PRIMARY_DISPLAY_CTRL, primaryControl);
        primaryWaitVerticalSync(vsync_delay);

        /* Turn on FPDATA. */
        primaryControl = FIELD_SET(primaryControl, PRIMARY_DISPLAY_CTRL, DATA, ENABLE);
        pokeRegisterDWord(PRIMARY_DISPLAY_CTRL, primaryControl);
        primaryWaitVerticalSync(vsync_delay);

        /* Turn on FPVBIAS. */
        primaryControl = FIELD_SET(primaryControl, PRIMARY_DISPLAY_CTRL, VBIASEN, HIGH);
        pokeRegisterDWord(PRIMARY_DISPLAY_CTRL, primaryControl);
        primaryWaitVerticalSync(vsync_delay);

        /* Turn on FPEN. */
        primaryControl = FIELD_SET(primaryControl, PRIMARY_DISPLAY_CTRL, FPEN, HIGH);
        pokeRegisterDWord(PRIMARY_DISPLAY_CTRL, primaryControl);
    }

    else
    {
        /* Turn off FPEN. */
        primaryControl = FIELD_SET(primaryControl, PRIMARY_DISPLAY_CTRL, FPEN, LOW);
        pokeRegisterDWord(PRIMARY_DISPLAY_CTRL, primaryControl);
        primaryWaitVerticalSync(vsync_delay);

        /* Turn off FPVBIASEN. */
        primaryControl = FIELD_SET(primaryControl, PRIMARY_DISPLAY_CTRL, VBIASEN, LOW);
        pokeRegisterDWord(PRIMARY_DISPLAY_CTRL, primaryControl);
        primaryWaitVerticalSync(vsync_delay);

        /* Turn off FPDATA. */
        primaryControl = FIELD_SET(primaryControl, PRIMARY_DISPLAY_CTRL, DATA, DISABLE);
        pokeRegisterDWord(PRIMARY_DISPLAY_CTRL, primaryControl);
        primaryWaitVerticalSync(vsync_delay);

        /* Turn off FPVDDEN. */
        primaryControl = FIELD_SET(primaryControl, PRIMARY_DISPLAY_CTRL, FPVDDEN, LOW);
        pokeRegisterDWord(PRIMARY_DISPLAY_CTRL, primaryControl);
    }
    
    //DDKDEBUGPRINT((DISPLAY_LEVEL, "swPanelPowerSequence -\n"));
}

/* 
 * This function turns on/off the DAC for CRT display control.
 * Input: On or off
 */
void setDAC(disp_state_t state)
{
    //DDKDEBUGPRINT((DISPLAY_LEVEL, "setDAC: %s\n", (state == DISP_ON) ? "on" : "off"));

    if (state == DISP_ON)
    {
        pokeRegisterDWord(MISC_CTRL, FIELD_SET(peekRegisterDWord(MISC_CTRL),
                                               MISC_CTRL,
                                               DAC_POWER,
                                               ON));
    }
    else
    {
        pokeRegisterDWord(MISC_CTRL, FIELD_SET(peekRegisterDWord(MISC_CTRL),
                                               MISC_CTRL,
                                               DAC_POWER,
                                               OFF));
    }
}

/*
 * This function turns on/off the display control.
 * Currently, it for CRT and Panel controls only.
 * Input: Panel or CRT, or ...
 *        On or Off.
 *
 * This function manipulate the physical display channels 
 * and devices.
 *
 * Note:
 *      Turning on/off the timing and the plane requires programming sequence.
 *      The plane can not be changed without turning on the timing. However,
 *      changing the plane has no effect when the timing (clock) is off. Below,
 *      is the description of the timing and plane combination setting.
 *
 *      +-----------+-----------+-----------------------------------------------+
 *      |  Timing   |   Plane   |                    Description                |
 *      +-----------+-----------+-----------------------------------------------+
 *      |    ON     |    OFF    | no Display but clock is on (consume power)    |
 *      |    ON     |    ON     | normal display                                |
 *      |    OFF    |    OFF    | no display and no clock (power down)          |
 *      |    OFF    |    ON     | no display and no clock (same as power down)  |
 *      +-----------+-----------+-----------------------------------------------+
 */
void setDisplayControl(disp_control_t dispControl, disp_state_t dispState)
{
    unsigned long ulDisplayCtrlReg, ulReservedBits;
    unsigned long ulCount = 0;

    /* Set the primary display control */
    if (dispControl == PRIMARY_CTRL)
    {
        ulDisplayCtrlReg = peekRegisterDWord(PRIMARY_DISPLAY_CTRL);

        //DDKDEBUGPRINT((DISPLAY_LEVEL, "(setDispCtrl) PRIMARY_DISPLAY_CTRL before set: %x\n", peekRegisterDWord(PRIMARY_DISPLAY_CTRL)));

        /* Turn on/off the Panel display control */
        if (dispState == DISP_ON)
        {
            /* Timing should be enabled first before enabling the plane because changing at the
               same time does not guarantee that the plane will also enabled or disabled. 
             */
            ulDisplayCtrlReg = FIELD_SET(ulDisplayCtrlReg, PRIMARY_DISPLAY_CTRL, TIMING, ENABLE);
            pokeRegisterDWord(PRIMARY_DISPLAY_CTRL, ulDisplayCtrlReg);
            
            ulDisplayCtrlReg = FIELD_SET(ulDisplayCtrlReg, PRIMARY_DISPLAY_CTRL, PLANE, ENABLE);
            
            /* Added some masks to mask out the reserved bits. 
             * Sometimes, the reserved bits are set/reset randomly when 
             * writing to the PRIMARY_DISPLAY_CTRL, therefore, the register
             * reserved bits are needed to be masked out.
             */
            ulReservedBits = FIELD_SET(0, PRIMARY_DISPLAY_CTRL, RESERVED_1_MASK, ENABLE) |
                             FIELD_SET(0, PRIMARY_DISPLAY_CTRL, RESERVED_2_MASK, ENABLE) |
                             FIELD_SET(0, PRIMARY_DISPLAY_CTRL, RESERVED_3_MASK, ENABLE);
                             
            /* Somehow the register value on the plane is not set until a few delay. Need to write
               and read it a couple times*/
            pokeRegisterDWord(PRIMARY_DISPLAY_CTRL, ulDisplayCtrlReg);
            primaryWaitVerticalSync(2);
        }
        else
        {
            /* When turning off, there is no rule on the programming sequence since whenever the
               clock is off, then it does not matter whether the plane is enabled or disabled.
               Note: Modifying the plane bit will take effect on the next vertical sync. Need to
                     find out if it is necessary to wait for 1 vsync before modifying the timing
                     enable bit. 
             */
            ulDisplayCtrlReg = FIELD_SET(ulDisplayCtrlReg, PRIMARY_DISPLAY_CTRL, PLANE, DISABLE);
            pokeRegisterDWord(PRIMARY_DISPLAY_CTRL, ulDisplayCtrlReg);

            ulDisplayCtrlReg = FIELD_SET(ulDisplayCtrlReg, PRIMARY_DISPLAY_CTRL, TIMING, DISABLE);
            pokeRegisterDWord(PRIMARY_DISPLAY_CTRL, ulDisplayCtrlReg);
        }

        //DDKDEBUGPRINT((DISPLAY_LEVEL, "(setDispCtrl) PRIMARY_DISPLAY_CTRL after set: %x\n", peekRegisterDWord(PRIMARY_DISPLAY_CTRL)));
    }
    /* Set the secondary display control */
    else
    {
        ulDisplayCtrlReg = peekRegisterDWord(SECONDARY_DISPLAY_CTRL);

        //DDKDEBUGPRINT((DISPLAY_LEVEL, "(setDispCtrl) SECONDARY_DISPLAY_CTRL before set: %x\n", peekRegisterDWord(SECONDARY_DISPLAY_CTRL)));

        if (dispState == DISP_ON)
        {
            /* Timing should be enabled first before enabling the plane because changing at the
               same time does not guarantee that the plane will also enabled or disabled. 
             */
            ulDisplayCtrlReg = FIELD_SET(ulDisplayCtrlReg, SECONDARY_DISPLAY_CTRL, TIMING, ENABLE);
            pokeRegisterDWord(SECONDARY_DISPLAY_CTRL, ulDisplayCtrlReg);
                        
            ulDisplayCtrlReg = FIELD_SET(ulDisplayCtrlReg, SECONDARY_DISPLAY_CTRL, PLANE, ENABLE);
            
            /* Added some masks to mask out the reserved bits. 
             * Sometimes, the reserved bits are set/reset randomly when 
             * writing to the PRIMARY_DISPLAY_CTRL, therefore, the register
             * reserved bits are needed to be masked out.
             */
            ulReservedBits = FIELD_SET(0, SECONDARY_DISPLAY_CTRL, RESERVED_1_MASK, ENABLE) |
                             FIELD_SET(0, SECONDARY_DISPLAY_CTRL, RESERVED_2_MASK, ENABLE) |
                             FIELD_SET(0, SECONDARY_DISPLAY_CTRL, RESERVED_3_MASK, ENABLE) |
                             FIELD_SET(0, SECONDARY_DISPLAY_CTRL, RESERVED_4_MASK, ENABLE);
            
            
            pokeRegisterDWord(SECONDARY_DISPLAY_CTRL, ulDisplayCtrlReg);
            secondaryWaitVerticalSync(2);
        }
        else
        {
            /* When turning off, there is no rule on the programming sequence since whenever the
               clock is off, then it does not matter whether the plane is enabled or disabled.
               Note: Modifying the plane bit will take effect on the next vertical sync. Need to
                     find out if it is necessary to wait for 1 vsync before modifying the timing
                     enable bit.
             */
            ulDisplayCtrlReg = FIELD_SET(ulDisplayCtrlReg, SECONDARY_DISPLAY_CTRL, PLANE, DISABLE);
            pokeRegisterDWord(SECONDARY_DISPLAY_CTRL, ulDisplayCtrlReg);

            ulDisplayCtrlReg = FIELD_SET(ulDisplayCtrlReg, SECONDARY_DISPLAY_CTRL, TIMING, DISABLE);
            pokeRegisterDWord(SECONDARY_DISPLAY_CTRL, ulDisplayCtrlReg);
        }

        //DDKDEBUGPRINT((DISPLAY_LEVEL, "(setDispCtrl) SECONDARY_DISPLAY_CTRL after set: %x\n", peekRegisterDWord(SECONDARY_DISPLAY_CTRL)));
    }
}

/*
 * This function set the display path together with the HSync and VSync.
 *
 * Note:
 *     This function has to be called last after setting all the display Control
 *     and display output. 
 */
void setPath(
    disp_path_t dispPath, 
    disp_control_t dispControl, 
    disp_state_t dispState
)
{
    unsigned long control;
    mode_parameter_t modeParam;
    unsigned long clock0, clock1, MiscControl;    

    /* Get the current mode parameter of the specific display control */
    modeParam = getCurrentModeParam(dispControl);
    
    if (dispPath == PANEL_PATH)
    {
        control = peekRegisterDWord(PRIMARY_DISPLAY_CTRL);
        if (dispState == DISP_ON)
        {
            /* Adjust the Clock polarity */
            if (modeParam.clock_phase_polarity == POS)
                control = FIELD_SET(control, PRIMARY_DISPLAY_CTRL, CLOCK_PHASE, ACTIVE_HIGH);
            else
                control = FIELD_SET(control, PRIMARY_DISPLAY_CTRL, CLOCK_PHASE, ACTIVE_LOW);
                        
            /* Adjust the VSync polarity */
            if (modeParam.vertical_sync_polarity == POS)
                control = FIELD_SET(control, PRIMARY_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_HIGH);
            else
                control = FIELD_SET(control, PRIMARY_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_LOW);

            /* Adjust the HSync polarity */
            if (modeParam.horizontal_sync_polarity == POS)
                control = FIELD_SET(control, PRIMARY_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_HIGH);
            else
                control = FIELD_SET(control, PRIMARY_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_LOW);

            /* Display control is not swapped, so use the normal display data flow */
            if (dispControl == PRIMARY_CTRL)
                control = FIELD_SET(control, PRIMARY_DISPLAY_CTRL, SELECT, PRIMARY);                    
            else    /* Secondary Control */
                control = FIELD_SET(control, PRIMARY_DISPLAY_CTRL, SELECT, SECONDARY);
        }

#if Validate_718_AA
        //if (hwDeviceExtension1[0]->ChipID==0x718 ||
        //    hwDeviceExtension1[1]->ChipID==0x718)
        {
            //patch for sm718aa, when set 80028[28] from 0 to 1, failed some time. needs to set the master
            //clock to 56mhz.
            clock0 = peekRegisterDWord(MODE0_GATE);
            pokeRegisterDWord(MODE0_GATE, FIELD_SET(clock0, MODE0_GATE, MCLK, 42MHZ)); 
            clock1 = peekRegisterDWord(MODE1_GATE);
            pokeRegisterDWord(MODE1_GATE, FIELD_SET(clock1, MODE1_GATE, MCLK, 42MHZ));                 
        }
#endif
        pokeRegisterDWord(PRIMARY_DISPLAY_CTRL, control);
#if Validate_718_AA
        //if (hwDeviceExtension1[0]->ChipID==0x718 ||
        //    hwDeviceExtension1[1]->ChipID==0x718)
        {
             //patch for sm718aa, restore mclk.
            pokeRegisterDWord(MODE0_GATE, clock0); 
            pokeRegisterDWord(MODE1_GATE, clock1); 

            //Reset video memory, otherwise, screen would show garbage.
            MiscControl= peekRegisterDWord(MISC_CTRL);
            MiscControl= FIELD_SET(MiscControl, MISC_CTRL, LOCALMEM_RESET, RESET);
            pokeRegisterDWord(MISC_CTRL, MiscControl);
            MiscControl= FIELD_SET(MiscControl, MISC_CTRL, LOCALMEM_RESET, NORMAL);
            pokeRegisterDWord(MISC_CTRL, MiscControl);            
        }
#endif

       // DDKDEBUGPRINT((DISPLAY_LEVEL, "(setPath) PRIMARY_DISPLAY_CTRL: %x\n", peekRegisterDWord(PRIMARY_DISPLAY_CTRL)));
    }
    else    /* CRT Path */
    {
        control = peekRegisterDWord(SECONDARY_DISPLAY_CTRL);
    
        if (dispState == DISP_ON)
        {
            /* Adjust the Clock polarity */
            if (modeParam.clock_phase_polarity == POS)
                control = FIELD_SET(control, SECONDARY_DISPLAY_CTRL, CLOCK_PHASE, ACTIVE_HIGH);
            else
                control = FIELD_SET(control, SECONDARY_DISPLAY_CTRL, CLOCK_PHASE, ACTIVE_LOW);
                
            /* Adjust the VSync polarity */
            if (modeParam.vertical_sync_polarity == POS)
                control = FIELD_SET(control, SECONDARY_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_HIGH);
            else
                control = FIELD_SET(control, SECONDARY_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_LOW);
    
            /* Adjust the HSync polarity */
            if (modeParam.horizontal_sync_polarity == POS)
                control = FIELD_SET(control, SECONDARY_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_HIGH);
            else
                control = FIELD_SET(control, SECONDARY_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_LOW);

            /* Display control is not swapped, so use the normal display data flow */
            if (dispControl == PRIMARY_CTRL)
                control = FIELD_SET(control, SECONDARY_DISPLAY_CTRL, SELECT, PRIMARY);
            else
                control = FIELD_SET(control, SECONDARY_DISPLAY_CTRL, SELECT, SECONDARY);
            
            /* Enable the CRT Pixel */
            control = FIELD_SET(control, SECONDARY_DISPLAY_CTRL, BLANK, OFF);
        }
        else
        {
            /* Disable the CRT Pixel */
            control = FIELD_SET(control, SECONDARY_DISPLAY_CTRL, BLANK, ON);
        }    

        pokeRegisterDWord(SECONDARY_DISPLAY_CTRL, control);

        //DDKDEBUGPRINT((DISPLAY_LEVEL, "(setPath) SECONDARY_DISPLAY_CTRL: %x\n", peekRegisterDWord(SECONDARY_DISPLAY_CTRL)));
    }
}

/*
 * This function gets the CRT2 Monitor Detection Threshold value.
 *
 * Input:
 *      pRedValue   - Pointer to a variable to store the Red color threshold
 *                    value.
 *      pGreenValue - Pointer to a variable to store the Green color threshold
 *                    value.
 *      pBlueValue  - Pointer to a variable to store the Blue color threshold
 *                    value. 
 */
void getCRTDetectThreshold(
    unsigned char *pRedValue,
    unsigned char *pGreenValue,
    unsigned char *pBlueValue
)
{
    unsigned long value;
    
    value = peekRegisterDWord(SECONDARY_MONITOR_DETECT);

    if (pRedValue != (unsigned char *)0)
        *pRedValue = (unsigned char)FIELD_GET(value, SECONDARY_MONITOR_DETECT, RED);
        
    if (pGreenValue != (unsigned char *)0)
        *pGreenValue = (unsigned char)FIELD_GET(value, SECONDARY_MONITOR_DETECT, GREEN);
        
    if (pBlueValue != (unsigned char *)0)
        *pBlueValue = (unsigned char)FIELD_GET(value, SECONDARY_MONITOR_DETECT, BLUE);
}

/*
 * This function detects if the CRT monitor is attached.
 *
 * Input:
 *      redValue    - Threshold value to be detected on the red color.
 *      greenValue  - Threshold value to be detected on the green color.
 *      blueValue   - Threshold value to be detected on the blue color.
 *
 * Output:
 *      0   - Success
 *     -1   - Fail 
 */
long ddk750_detectCRTMonitor(
    unsigned char redValue,
    unsigned char greenValue,
    unsigned char blueValue
)
{
    unsigned long value, red, green, blue;
    long result = (-1);
    
    /* Use the given red, green, and blue threshold value to detect the monitor or
       default. */
    if (redValue != 0)
        red = redValue;
    else
        red = DEFAULT_MON_DETECTION_THRESHOLD;
        
    if (greenValue != 0)
        green = greenValue;
    else
        green = DEFAULT_MON_DETECTION_THRESHOLD;
        
    if (blueValue != 0)
        blue = blueValue;
    else
        blue = DEFAULT_MON_DETECTION_THRESHOLD;
        
    /* Set the RGB Threshold value and enable the monitor detection. */
    value = ((unsigned long)red << 16) |
            ((unsigned long)green << 8) |
            ((unsigned long)blue);
    value = FIELD_SET(value, SECONDARY_MONITOR_DETECT, ENABLE, ENABLE);
    pokeRegisterDWord(SECONDARY_MONITOR_DETECT, value);
    
    /* Add some delay here. Otherwise, the detection is not stable. */
    value = 0xFFFF;
    while (value--);
    
    /* Check if the monitor is detected. */
    if (FIELD_GET(peekRegisterDWord(SECONDARY_MONITOR_DETECT), SECONDARY_MONITOR_DETECT, VALUE) ==
        SECONDARY_MONITOR_DETECT_VALUE_ENABLE)
    {
        result = 0;
    }
    
    /* Disable the Monitor Detect Enable bit. Somehow, enabling this bit will 
       cause the CRT to lose display. */
    value = peekRegisterDWord(SECONDARY_MONITOR_DETECT);
    value = FIELD_SET(value, SECONDARY_MONITOR_DETECT, ENABLE, DISABLE);
    pokeRegisterDWord(SECONDARY_MONITOR_DETECT, value);

    return result;
}

/*
 * This function set the logical display output.
 *
 * The output is called logical because it is independent of physical implementation.
 * For example, CRT2 only mode is not using the internal secondary control. It uses the
 * Primary Control with its output directed to CRT DAC.
 *
 * Input:
 *      output          - Logical Display output
 *      dispCtrlUsage   - Display Control Flag Usage:
 *                          0 : Use primary display control (PRIMARY_CTRL) to control 
 *                              primary output (LCD1 & CRT1) and use secondary display 
 *                              control (SECONDARY_CTRL) to control secondary output 
 *                              (CRT2 & LCD2)
 *                          1 : Use primary display control (PRIMARY_CTRL) to control 
 *                              secondary output (LCD2 & CRT2) and use secondary display 
 *                              control (SECONDARY_CTRL) to control primary output 
 *                              (LCD1 & CRT1)
 *
 * Output:
 *      0   - Success
 *     -1   - Fail 
 */
long setLogicalDispOutput(

    disp_output_t output,
    unsigned char dispCtrlUsage
)
{
    unsigned long ulReg;

    //DDKDEBUGPRINT((DISPLAY_LEVEL, "setLogicalDispOutput\n"));

    switch (output)
    {
        case NO_DISPLAY:
        {
            //DDKDEBUGPRINT((DISPLAY_LEVEL, "NO_DISPLAY\n"));
        
            /* In here, all the display device has to be turned off first before the
               the display control. */
            swPanelPowerSequence(DISP_OFF, 4);                  /* Turn off Panel */
            setDAC(DISP_OFF);                                   /* Turn off DAC */
            setDPMS(DPMS_OFF);                                  /* Turn off DPMS */
    
            setDisplayControl(PRIMARY_CTRL, DISP_OFF);          /* Turn off Panel control */
            setDisplayControl(SECONDARY_CTRL, DISP_OFF);        /* Turn off CRT control */

            setPath(PANEL_PATH, PRIMARY_CTRL, DISP_OFF);        /* Turn off Panel path */
            setPath(CRT_PATH, PRIMARY_CTRL, DISP_OFF);          /* Turn off CRT path */
            break;
        }    
        case LCD1_ONLY:
        {
            //DDKDEBUGPRINT((DISPLAY_LEVEL, "LCD1_ONLY\n"));

            /* 
             * 1. Set all the display control and the display path 
             */
            if (dispCtrlUsage == 0)
            {
                /* Turn on Primary control */
                setDisplayControl(PRIMARY_CTRL, DISP_ON);
                if (isScalingEnabled(PRIMARY_CTRL) == 1)
                {
                    setDisplayControl(SECONDARY_CTRL, DISP_ON);     /* Turn on CRT control */
                    setPath(PANEL_PATH, SECONDARY_CTRL, DISP_ON);   /* Turn on Panel Path and use Secondary data */
                }
                else
                {
                    setDisplayControl(SECONDARY_CTRL, DISP_OFF);    /* Turn off CRT control */
                    setPath(PANEL_PATH, PRIMARY_CTRL, DISP_ON);     /* Turn on Panel Path and use panel data */
                }
                setPath(CRT_PATH, PRIMARY_CTRL, DISP_OFF);          /* Turn off CRT Path */
            }
            else
            {
                setDisplayControl(PRIMARY_CTRL, DISP_OFF);          /* Turn off Primary control */
                setDisplayControl(SECONDARY_CTRL, DISP_ON);         /* Turn on Secondary control */
                setPath(PANEL_PATH, SECONDARY_CTRL, DISP_ON);       /* Turn on Panel Path and use Secondary data */
                setPath(CRT_PATH, SECONDARY_CTRL, DISP_OFF);        /* Turn off CRT Path */
            }
            
            /* 
             * 2. Enable/disable the display devices. 
             */
            swPanelPowerSequence(DISP_ON, 4);                   /* Turn on Panel */
#ifdef ENABLE_PANEL_DAC
            /* When using Analog CRT connected to the DVI channel, the DAC needs
               to be turned on. The power is using the same power as CRT DAC. */
            setDAC(DISP_ON);                                    /* Turn on DAC */
#else
            setDAC(DISP_OFF);                                   /* Turn off DAC */
#endif
            setDPMS(DPMS_OFF);                                  /* Turn off DPMS */
            break;
        }    
        case LCD2_ONLY:
        {
            //DDKDEBUGPRINT((DISPLAY_LEVEL, "LCD2_ONLY\n"));

            /* 
             * 1. Check the conditions 
             */
            /* Can not enable LCD22 when the panel type is not 18-bit panel. */            
            if (getPanelType() != TFT_18BIT)
                return (-1);
            
            /* 
             * 2. Set all the display control and the display path 
             */
            if (dispCtrlUsage == 0)
            {
                setDisplayControl(PRIMARY_CTRL, DISP_ON);           /* Turn on Primary control */                
                if (isScalingEnabled(PRIMARY_CTRL) == 1)
                {
                    setDisplayControl(SECONDARY_CTRL, DISP_ON);     /* Turn on Secondary control */
                    setPath(CRT_PATH, SECONDARY_CTRL, DISP_ON);     /* Turn on CRT Path and use Secondary data */
                }
                else
                {
                    setDisplayControl(SECONDARY_CTRL, DISP_OFF);    /* Turn off Secondary control */
                    setPath(CRT_PATH, PRIMARY_CTRL, DISP_ON);       /* Turn on CRT Path and use primary data */
                }
                setPath(PANEL_PATH, PRIMARY_CTRL, DISP_OFF);        /* Turn off Panel Path */
            }
            else
            {
                setDisplayControl(PRIMARY_CTRL, DISP_OFF);          /* Turn off Primary control */                
                setDisplayControl(SECONDARY_CTRL, DISP_ON);         /* Turn on Secondary control */
                setPath(CRT_PATH, SECONDARY_CTRL, DISP_ON);         /* Turn off CRT Path and use secondary data */
                setPath(PANEL_PATH, SECONDARY_CTRL, DISP_OFF);      /* Turn off Panel Path */
            }
            
            /* 
             * 3. Enable/disable the display devices. 
             */
            swPanelPowerSequence(DISP_OFF, 4);                  /* Turn off Panel 1 */
            setDAC(DISP_OFF);                                   /* Turn off DAC */
            setDPMS(DPMS_OFF);                                  /* Turn off DPMS */
            break;
        }
        case CRT2_ONLY:
        {
            //DDKDEBUGPRINT((DISPLAY_LEVEL, "CRT2_ONLY\n"));

            /* 
             * 1. Set all the display control and the display path 
             */
            if (dispCtrlUsage == 0)
            {
                setDisplayControl(PRIMARY_CTRL, DISP_ON);           /* Turn on Primary Control */
                setDisplayControl(SECONDARY_CTRL, DISP_OFF);        /* Turn off Secondary control */
                setPath(PANEL_PATH, SECONDARY_CTRL, DISP_OFF);      /* Turn off Panel Path */
                setPath(CRT_PATH, PRIMARY_CTRL, DISP_ON);           /* Turn off CRT Path and use Primary data */
            }
            else
            {
                setDisplayControl(PRIMARY_CTRL, DISP_OFF);          /* Turn off Primary Control */
                setDisplayControl(SECONDARY_CTRL, DISP_ON);         /* Turn on Secondary control */
                setPath(PANEL_PATH, SECONDARY_CTRL, DISP_OFF);      /* Turn off Panel Path */
                setPath(CRT_PATH, SECONDARY_CTRL, DISP_ON);         /* Turn off CRT Path and use Primary data */
            }
            
            /* 
             * 2. Enable/disable the display devices. 
             */
            swPanelPowerSequence(DISP_OFF, 4);                  /* Turn off Panel */
            setDAC(DISP_ON);                                    /* Turn on DAC */
            setDPMS(DPMS_ON);                                   /* Turn on DPMS to drive CRT */
            break;
        }
        case LCD1_CRT2_SIMUL: /* Panel and CRT same content */
        {
            //DDKDEBUGPRINT((DISPLAY_LEVEL, "LCD1_CRT2_SIMUL\n"));
            
            /* 
             * 1. Set all the display control and the display path 
             */
            if (dispCtrlUsage == 0)
            {
                setDisplayControl(PRIMARY_CTRL, DISP_ON);           /* Turn on Primary Control */
                if (isScalingEnabled(PRIMARY_CTRL) == 1)
                {
                    setDisplayControl(SECONDARY_CTRL, DISP_ON);     /* Turn on Secondary control */
                    setPath(PANEL_PATH, SECONDARY_CTRL, DISP_ON);   /* Turn on Panel Path and use Secondary data */
                }
                else
                {
                    setDisplayControl(SECONDARY_CTRL, DISP_OFF);    /* Turn off Secondary control */
                    setPath(PANEL_PATH, PRIMARY_CTRL, DISP_ON);     /* Turn on Panel Path and use Primary data */
                } 
                setPath(CRT_PATH, PRIMARY_CTRL, DISP_ON);           /* Turn on CRT Path and use Primary data */
            }
            else
            {
                setDisplayControl(PRIMARY_CTRL, DISP_OFF);          /* Turn off Primary Control */
                setDisplayControl(SECONDARY_CTRL, DISP_ON);         /* Turn on Secondary control */
                setPath(PANEL_PATH, SECONDARY_CTRL, DISP_ON);       /* Turn on Panel Path and use Secondary data */
                setPath(CRT_PATH, SECONDARY_CTRL, DISP_ON);         /* Turn on CRT Path and use Primary data */
            }
            
            /* 
             * 2. Enable/disable the display devices. 
             */
            swPanelPowerSequence(DISP_ON, 4);                   /* Turn on Panel */
            setDAC(DISP_ON);                                    /* Turn on DAC */
            setDPMS(DPMS_ON);                                   /* Turn on DPMS to drive CRT */
            break;
        }    
        case LCD1_LCD2_SIMUL:
        {
            //DDKDEBUGPRINT((DISPLAY_LEVEL, "LCD1_LCD2_SIMUL\n"));
            
            /* 
             * 1. Check the conditions 
             */
            /* Can not enable PANEL2 when the panel type is not 18-bit panel. */            
            if (getPanelType() != TFT_18BIT)
                return (-1);
            
            /* 
             * 2. Set all the display control and the display path 
             */
            if (dispCtrlUsage == 0)
            {    
                setDisplayControl(PRIMARY_CTRL, DISP_ON);           /* Turn on Primary Control */
                if (isScalingEnabled(PRIMARY_CTRL) == 1)
                {
                    setDisplayControl(SECONDARY_CTRL, DISP_ON);     /* Turn on Secondary control */
                    setPath(PANEL_PATH, SECONDARY_CTRL, DISP_ON);   /* Turn on Panel Path and use Secondary data */
                    setPath(CRT_PATH, SECONDARY_CTRL, DISP_ON);     /* Turn on CRT Path and use Secondary data */
                }
                else
                {
                    setDisplayControl(SECONDARY_CTRL, DISP_OFF);    /* Turn off Secondary control */
                    setPath(PANEL_PATH, PRIMARY_CTRL, DISP_ON);     /* Turn on Panel Path and use Primary data */
                    setPath(CRT_PATH, PRIMARY_CTRL, DISP_ON);       /* Turn on CRT Path and use Primary data */
                }
            }
            else
            {
                setDisplayControl(PRIMARY_CTRL, DISP_OFF);          /* Turn off Primary Control */
                setDisplayControl(SECONDARY_CTRL, DISP_ON);         /* Turn on Secondary control */
                setPath(PANEL_PATH, SECONDARY_CTRL, DISP_ON);       /* Turn on Panel Path and use Secondary data */
                setPath(CRT_PATH, SECONDARY_CTRL, DISP_ON);         /* Turn on CRT Path and use Secondary data */
            }
            
            /* 
             * 3. Enable/disable the display devices. 
             */
            swPanelPowerSequence(DISP_ON, 4);                   /* Turn on Panel */
#ifdef ENABLE_PANEL_DAC
            /* When using Analog CRT connected to the DVI channel, the DAC needs
               to be turned on. The power is using the same power as CRT DAC. */
            setDAC(DISP_ON);                                    /* Turn on DAC */
#else
            setDAC(DISP_OFF);                                   /* Turn off DAC */
#endif
            setDPMS(DPMS_OFF);                                  /* Turn off DPMS */
            break;
        }    
        case CRT2_LCD2_SIMUL:
        {
            //DDKDEBUGPRINT((DISPLAY_LEVEL, "CRT2_LCD2_SIMUL\n"));
            
            /* 
             * 1. Check the conditions 
             */
            /* Can not enable PANEL2 when the panel type is not 18-bit panel. */            
            if (getPanelType() != TFT_18BIT)
                return (-1);
            
            /* 
             * 2. Set all the display control and the display path 
             */
            if (dispCtrlUsage == 0)
            {
                setDisplayControl(PRIMARY_CTRL, DISP_ON);           /* Turn on Primary Control */
                if (isScalingEnabled(PRIMARY_CTRL) == 1)
                {
                    setDisplayControl(SECONDARY_CTRL, DISP_ON);     /* Turn on Secondary control */
                    setPath(CRT_PATH, SECONDARY_CTRL, DISP_ON);     /* Turn on CRT Path and use Secondary data */
                }
                else
                {
                    setDisplayControl(SECONDARY_CTRL, DISP_OFF);    /* Turn off Secondary control */
                    setPath(CRT_PATH, PRIMARY_CTRL, DISP_ON);       /* Turn on CRT Path and use Primary data */
                }
                setPath(PANEL_PATH, PRIMARY_CTRL, DISP_OFF);        /* Turn on Panel Path and use Primary data */
            }
            else
            {
                setDisplayControl(PRIMARY_CTRL, DISP_OFF);          /* Turn off Primary Control */
                setDisplayControl(SECONDARY_CTRL, DISP_ON);         /* Turn on Secondary control */
                setPath(CRT_PATH, SECONDARY_CTRL, DISP_ON);         /* Turn on CRT Path and use Secondary data */
                setPath(PANEL_PATH, SECONDARY_CTRL, DISP_OFF);      /* Turn on Panel Path and use Primary data */
            }
            
            /* 
             * 3. Enable/disable the display devices. 
             */
            swPanelPowerSequence(DISP_ON, 4);                   /* Turn on Panel */
            setDAC(DISP_ON);                                    /* Turn on DAC */
            setDPMS(DPMS_ON);                                   /* Turn on DPMS to drive CRT */
            break;
        }    
        case LCD1_LCD2_CRT2_SIMUL:
        {
            //DDKDEBUGPRINT((DISPLAY_LEVEL, "LCD1_LCD2_CRT2_SIMUL\n"));
            
            /* 
             * 1. Check the conditions
             */
            /* Can not enable PANEL2 when the panel type is not 18-bit panel. */            
            if (getPanelType() != TFT_18BIT)
                return (-1);
            
            /* 
             * 2. Set all the display control and the display path 
             */
            if (dispCtrlUsage == 0)
            {            
                setDisplayControl(PRIMARY_CTRL, DISP_ON);           /* Turn on Primary Control */
                if (isScalingEnabled(PRIMARY_CTRL) == 1)
                {
                    setDisplayControl(SECONDARY_CTRL, DISP_ON);     /* Turn on Secondary control */
                    setPath(PANEL_PATH, SECONDARY_CTRL, DISP_ON);   /* Turn on Panel Path and use Secondary data */
                    setPath(CRT_PATH, SECONDARY_CTRL, DISP_ON);     /* Turn on CRT Path and use Secondary data */
                }
                else
                {
                    setDisplayControl(SECONDARY_CTRL, DISP_OFF);    /* Turn off Secondary control */
                    setPath(PANEL_PATH, PRIMARY_CTRL, DISP_ON);     /* Turn on Panel Path and use Primary data */
                    setPath(CRT_PATH, PRIMARY_CTRL, DISP_ON);       /* Turn on CRT Path and use Primary data */
                }
            }
            else
            {
                setDisplayControl(PRIMARY_CTRL, DISP_OFF);          /* Turn off Primary Control */
                setDisplayControl(SECONDARY_CTRL, DISP_ON);         /* Turn on Secondary control */
                setPath(PANEL_PATH, SECONDARY_CTRL, DISP_ON);       /* Turn on Panel Path and use Secondary data */
                setPath(CRT_PATH, SECONDARY_CTRL, DISP_ON);         /* Turn on CRT Path and use Secondary data */
            }
            
            /* 
             * 3. Enable/disable the display devices. 
             */
            swPanelPowerSequence(DISP_ON, 4);                   /* Turn on Panel */
            setDAC(DISP_ON);                                    /* Turn on DAC */
            setDPMS(DPMS_ON);                                   /* Turn on DPMS to drive CRT */
            break;
        }
        case LCD1_CRT2_DUAL: /* Panel and CRT different content */
        {
            //DDKDEBUGPRINT((DISPLAY_LEVEL, "LCD1_CRT2_DUAL\n"));
            
            /* 
             * 1. Check the conditions
             */
            /* This combination is not valid if panel 1 requires scaling and display
               control is not swapped. */
          //  if (isScalingEnabled(PRIMARY_CTRL) == 1)
          //      return (-1);

            /* 
             * 2. Set all the display control and the display path 
             */
            setDisplayControl(PRIMARY_CTRL, DISP_ON);           /* Turn on Primary Control */
            setDisplayControl(SECONDARY_CTRL, DISP_ON);         /* Turn on Secondary control */


            if (dispCtrlUsage == 0)
            {
                setPath(PANEL_PATH, PRIMARY_CTRL, DISP_ON);     /* Turn on Panel Path and use Primary data */
                setPath(CRT_PATH, SECONDARY_CTRL, DISP_ON);     /* Turn on CRT Path and use Secondary data */


            }
            else
            {
                setPath(PANEL_PATH, SECONDARY_CTRL, DISP_ON);   /* Turn on Panel Path and use Secondary data */
                setPath(CRT_PATH, PRIMARY_CTRL, DISP_ON);       /* Turn on CRT Path and use Primary data */


            }
            
            /* 
             * 3. Enable/disable the display devices. 
             */
            swPanelPowerSequence(DISP_ON, 4);                   /* Turn on Panel */
            setDAC(DISP_ON);                                    /* Turn on DAC */
            setDPMS(DPMS_ON);                                   /* Turn on DPMS to drive CRT */
            break;
        }    
        case LCD1_LCD2_DUAL:
        {
            //DDKDEBUGPRINT((DISPLAY_LEVEL, "LCD1_LCD2_DUAL\n"));
            
            /* 
             * 1. Check the conditions
             */    
            /* This combination is not valid if panel requires scaling and display
               control is not swapped. */
           // if (isScalingEnabled(PRIMARY_CTRL) == 1)
           //     return (-1);
                
            /* Can not enable PANEL2 when the panel type is not 18-bit panel. */            
            if (getPanelType() != TFT_18BIT)
                return (-1);
            
            /* 
             * 2. Set all the display control and the display path 
             */
            setDisplayControl(PRIMARY_CTRL, DISP_ON);           /* Turn on Primary Control */
            setDisplayControl(SECONDARY_CTRL, DISP_ON);         /* Turn on Secondary control */
            if (dispCtrlUsage == 0)
            {
                setPath(PANEL_PATH, PRIMARY_CTRL, DISP_ON);     /* Turn on Panel Path and use Primary data */
                setPath(CRT_PATH, SECONDARY_CTRL, DISP_ON);     /* Turn on CRT Path and use Secondary data */
            }
            else
            {
                setPath(PANEL_PATH, SECONDARY_CTRL, DISP_ON);   /* Turn on Panel Path and use Secondary data */
                setPath(CRT_PATH, PRIMARY_CTRL, DISP_ON);       /* Turn on CRT Path and use Primary data */
            }
            
            /* 
             * 3. Enable/disable the display devices. 
             */
            swPanelPowerSequence(DISP_ON, 4);                   /* Turn on Panel */
#ifdef ENABLE_PANEL_DAC
            /* When using Analog CRT connected to the DVI channel, the DAC needs
               to be turned on. The power is using the same power as CRT DAC. */
            setDAC(DISP_ON);                                    /* Turn on DAC */
#else
            setDAC(DISP_OFF);                                   /* Turn off DAC */
#endif
            setDPMS(DPMS_OFF);                                  /* Turn off DPMS to drive CRT */
            break;
        }
        case LCD1_LCD2_CRT2_DUAL:
        {
            //DDKDEBUGPRINT((DISPLAY_LEVEL, "LCD1_LCD2_CRT2_DUAL\n"));
            
            /* 
             * 1. Check the conditions
             */    
            /* This combination is not valid if panel requires scaling and display
               control is not swapped. */
            //if (isScalingEnabled(PRIMARY_CTRL) == 1)
              //  return (-1);
                
            /* Can not enable PANEL2 when the panel type is not 18-bit panel. */            
            if (getPanelType() != TFT_18BIT)
                return (-1);
            
            /* 
             * 2. Set all the display control and the display path 
             */
            setDisplayControl(PRIMARY_CTRL, DISP_ON);           /* Turn on Primary Control */
            setDisplayControl(SECONDARY_CTRL, DISP_ON);         /* Turn on Secondary control */            
            if (dispCtrlUsage == 0)
            {
                setPath(PANEL_PATH, PRIMARY_CTRL, DISP_ON);     /* Turn on Panel Path and use Primary data */
                setPath(CRT_PATH, SECONDARY_CTRL, DISP_ON);     /* Turn on CRT Path and use Secondary data */
            }
            else
            {
                setPath(PANEL_PATH, SECONDARY_CTRL, DISP_ON);   /* Turn on Panel Path and use Secondary data */
                setPath(CRT_PATH, PRIMARY_CTRL, DISP_ON);       /* Turn on CRT Path and use Primary data */
            }
            
            /* 
             * 3. Enable/disable the display devices. 
             */
            swPanelPowerSequence(DISP_ON, 4);                   /* Turn on Panel */
            setDAC(DISP_ON);                                    /* Turn on DAC */
            setDPMS(DPMS_ON);                                   /* Turn on DPMS to drive CRT */
            break;
        }
    }
    
    return 0;
}

/*
 * This function set the logical display output.
 *
 * The output is called logical because it is independent of physical implementation.
 * For example, CRT2 only mode is not using the internal secondary control. It uses the
 * Primary Control with its output directed to CRT DAC.
 *
 * Input:
 *		isSnd	   		- is or not the second view
 *      output          - Logical Display output
 *      dispCtrlUsage   - Display Control Flag Usage:
 *                          0 : Use primary display control (PRIMARY_CTRL) to control 
 *                              primary output (LCD1 & CRT1) and use secondary display 
 *                              control (SECONDARY_CTRL) to control secondary output 
 *                              (CRT2 & LCD2)
 *                          1 : Use primary display control (PRIMARY_CTRL) to control 
 *                              secondary output (LCD2 & CRT2) and use secondary display 
 *                              control (SECONDARY_CTRL) to control primary output 
 *                              (LCD1 & CRT1)
 *
 * Output:
 *      0   - Success
 *     -1   - Fail 
 */
long setLogicalDispOutputExt(
    long isSecondDisplay, 
    disp_output_t output,
    unsigned char dispCtrlUsage
)
{
    unsigned long ulReg;

    //DDKDEBUGPRINT((DISPLAY_LEVEL, "setLogicalDispOutputExt\n"));

    switch (output)
    {
        case NO_DISPLAY:
        {
            //DDKDEBUGPRINT((DISPLAY_LEVEL, "NO_DISPLAY\n"));
        
            /* In here, all the display device has to be turned off first before the
               the display control. */
            swPanelPowerSequence(DISP_OFF, 4);                  /* Turn off Panel */
            setDAC(DISP_OFF);                                   /* Turn off DAC */
            setDPMS(DPMS_OFF);                                  /* Turn off DPMS */
    
            setDisplayControl(PRIMARY_CTRL, DISP_OFF);          /* Turn off Panel control */
            setDisplayControl(SECONDARY_CTRL, DISP_OFF);        /* Turn off CRT control */

            setPath(PANEL_PATH, PRIMARY_CTRL, DISP_OFF);        /* Turn off Panel path */
            setPath(CRT_PATH, PRIMARY_CTRL, DISP_OFF);          /* Turn off CRT path */
            break;
        }    
        case LCD1_ONLY:
        {
            //DDKDEBUGPRINT((DISPLAY_LEVEL, "LCD1_ONLY\n"));

            /* 
             * 1. Set all the display control and the display path 
             */
            if (dispCtrlUsage == 0)
            {
                /* Turn on Primary control */
                setDisplayControl(PRIMARY_CTRL, DISP_ON);
                if (isScalingEnabled(PRIMARY_CTRL) == 1)
                {
                    setDisplayControl(SECONDARY_CTRL, DISP_ON);     /* Turn on CRT control */
                    setPath(PANEL_PATH, SECONDARY_CTRL, DISP_ON);   /* Turn on Panel Path and use Secondary data */
                }
                else
                {
                    setDisplayControl(SECONDARY_CTRL, DISP_OFF);    /* Turn off CRT control */
                    setPath(PANEL_PATH, PRIMARY_CTRL, DISP_ON);     /* Turn on Panel Path and use panel data */
                }
                setPath(CRT_PATH, PRIMARY_CTRL, DISP_OFF);          /* Turn off CRT Path */
            }
            else
            {
                setDisplayControl(PRIMARY_CTRL, DISP_OFF);          /* Turn off Primary control */
                setDisplayControl(SECONDARY_CTRL, DISP_ON);         /* Turn on Secondary control */
                setPath(PANEL_PATH, SECONDARY_CTRL, DISP_ON);       /* Turn on Panel Path and use Secondary data */
                setPath(CRT_PATH, SECONDARY_CTRL, DISP_OFF);        /* Turn off CRT Path */
            }
            
            /* 
             * 2. Enable/disable the display devices. 
             */
            swPanelPowerSequence(DISP_ON, 4);                   /* Turn on Panel */
#ifdef ENABLE_PANEL_DAC
            /* When using Analog CRT connected to the DVI channel, the DAC needs
               to be turned on. The power is using the same power as CRT DAC. */
            setDAC(DISP_ON);                                    /* Turn on DAC */
#else
            setDAC(DISP_OFF);                                   /* Turn off DAC */
#endif
            setDPMS(DPMS_OFF);                                  /* Turn off DPMS */
            break;
        }    
        case LCD2_ONLY:
        {
            //DDKDEBUGPRINT((DISPLAY_LEVEL, "LCD2_ONLY\n"));

            /* 
             * 1. Check the conditions 
             */
            /* Can not enable LCD22 when the panel type is not 18-bit panel. */            
            if (getPanelType() != TFT_18BIT)
                return (-1);
            
            /* 
             * 2. Set all the display control and the display path 
             */
            if (dispCtrlUsage == 0)
            {
                setDisplayControl(PRIMARY_CTRL, DISP_ON);           /* Turn on Primary control */                
                if (isScalingEnabled(PRIMARY_CTRL) == 1)
                {
                    setDisplayControl(SECONDARY_CTRL, DISP_ON);     /* Turn on Secondary control */
                    setPath(CRT_PATH, SECONDARY_CTRL, DISP_ON);     /* Turn on CRT Path and use Secondary data */
                }
                else
                {
                    setDisplayControl(SECONDARY_CTRL, DISP_OFF);    /* Turn off Secondary control */
                    setPath(CRT_PATH, PRIMARY_CTRL, DISP_ON);       /* Turn on CRT Path and use primary data */
                }
                setPath(PANEL_PATH, PRIMARY_CTRL, DISP_OFF);        /* Turn off Panel Path */
            }
            else
            {
                setDisplayControl(PRIMARY_CTRL, DISP_OFF);          /* Turn off Primary control */                
                setDisplayControl(SECONDARY_CTRL, DISP_ON);         /* Turn on Secondary control */
                setPath(CRT_PATH, SECONDARY_CTRL, DISP_ON);         /* Turn off CRT Path and use secondary data */
                setPath(PANEL_PATH, SECONDARY_CTRL, DISP_OFF);      /* Turn off Panel Path */
            }
            
            /* 
             * 3. Enable/disable the display devices. 
             */
            swPanelPowerSequence(DISP_OFF, 4);                  /* Turn off Panel 1 */
            setDAC(DISP_OFF);                                   /* Turn off DAC */
            setDPMS(DPMS_OFF);                                  /* Turn off DPMS */
            break;
        }
        case CRT2_ONLY:
        {
            //DDKDEBUGPRINT((DISPLAY_LEVEL, "CRT2_ONLY\n"));

            /* 
             * 1. Set all the display control and the display path 
             */
            if (dispCtrlUsage == 0)
            {
                setDisplayControl(PRIMARY_CTRL, DISP_ON);           /* Turn on Primary Control */
                setDisplayControl(SECONDARY_CTRL, DISP_OFF);        /* Turn off Secondary control */
                setPath(PANEL_PATH, SECONDARY_CTRL, DISP_OFF);      /* Turn off Panel Path */
                setPath(CRT_PATH, PRIMARY_CTRL, DISP_ON);           /* Turn off CRT Path and use Primary data */
            }
            else
            {
                setDisplayControl(PRIMARY_CTRL, DISP_OFF);          /* Turn off Primary Control */
                setDisplayControl(SECONDARY_CTRL, DISP_ON);         /* Turn on Secondary control */
                setPath(PANEL_PATH, SECONDARY_CTRL, DISP_OFF);      /* Turn off Panel Path */
                setPath(CRT_PATH, SECONDARY_CTRL, DISP_ON);         /* Turn off CRT Path and use Primary data */
            }
            
            /* 
             * 2. Enable/disable the display devices. 
             */
            swPanelPowerSequence(DISP_OFF, 4);                  /* Turn off Panel */
            setDAC(DISP_ON);                                    /* Turn on DAC */
            setDPMS(DPMS_ON);                                   /* Turn on DPMS to drive CRT */
            break;
        }
        case LCD1_CRT2_SIMUL: /* Panel and CRT same content */
        {
            //DDKDEBUGPRINT((DISPLAY_LEVEL, "LCD1_CRT2_SIMUL\n"));
            
            /* 
             * 1. Set all the display control and the display path 
             */
            if (dispCtrlUsage == 0)
            {
                setDisplayControl(PRIMARY_CTRL, DISP_ON);           /* Turn on Primary Control */
                if (isScalingEnabled(PRIMARY_CTRL) == 1)
                {
                    setDisplayControl(SECONDARY_CTRL, DISP_ON);     /* Turn on Secondary control */
                    setPath(PANEL_PATH, SECONDARY_CTRL, DISP_ON);   /* Turn on Panel Path and use Secondary data */
                }
                else
                {
                    setDisplayControl(SECONDARY_CTRL, DISP_OFF);    /* Turn off Secondary control */
                    setPath(PANEL_PATH, PRIMARY_CTRL, DISP_ON);     /* Turn on Panel Path and use Primary data */
                }            
                setPath(CRT_PATH, PRIMARY_CTRL, DISP_ON);           /* Turn on CRT Path and use Primary data */
            }
            else
            {
                setDisplayControl(PRIMARY_CTRL, DISP_OFF);          /* Turn off Primary Control */
                setDisplayControl(SECONDARY_CTRL, DISP_ON);         /* Turn on Secondary control */
                setPath(PANEL_PATH, SECONDARY_CTRL, DISP_ON);       /* Turn on Panel Path and use Secondary data */
                setPath(CRT_PATH, SECONDARY_CTRL, DISP_ON);         /* Turn on CRT Path and use Primary data */
            }
            
            /* 
             * 2. Enable/disable the display devices. 
             */
            swPanelPowerSequence(DISP_ON, 4);                   /* Turn on Panel */
            setDAC(DISP_ON);                                    /* Turn on DAC */
            setDPMS(DPMS_ON);                                   /* Turn on DPMS to drive CRT */
            break;
        }    
        case LCD1_LCD2_SIMUL:
        {
            //DDKDEBUGPRINT((DISPLAY_LEVEL, "LCD1_LCD2_SIMUL\n"));
            
            /* 
             * 1. Check the conditions 
             */
            /* Can not enable PANEL2 when the panel type is not 18-bit panel. */            
            if (getPanelType() != TFT_18BIT)
                return (-1);
            
            /* 
             * 2. Set all the display control and the display path 
             */
            if (dispCtrlUsage == 0)
            {    
                setDisplayControl(PRIMARY_CTRL, DISP_ON);           /* Turn on Primary Control */
                if (isScalingEnabled(PRIMARY_CTRL) == 1)
                {
                    setDisplayControl(SECONDARY_CTRL, DISP_ON);     /* Turn on Secondary control */
                    setPath(PANEL_PATH, SECONDARY_CTRL, DISP_ON);   /* Turn on Panel Path and use Secondary data */
                    setPath(CRT_PATH, SECONDARY_CTRL, DISP_ON);     /* Turn on CRT Path and use Secondary data */
                }
                else
                {
                    setDisplayControl(SECONDARY_CTRL, DISP_OFF);    /* Turn off Secondary control */
                    setPath(PANEL_PATH, PRIMARY_CTRL, DISP_ON);     /* Turn on Panel Path and use Primary data */
                    setPath(CRT_PATH, PRIMARY_CTRL, DISP_ON);       /* Turn on CRT Path and use Primary data */
                }
            }
            else
            {
                setDisplayControl(PRIMARY_CTRL, DISP_OFF);          /* Turn off Primary Control */
                setDisplayControl(SECONDARY_CTRL, DISP_ON);         /* Turn on Secondary control */
                setPath(PANEL_PATH, SECONDARY_CTRL, DISP_ON);       /* Turn on Panel Path and use Secondary data */
                setPath(CRT_PATH, SECONDARY_CTRL, DISP_ON);         /* Turn on CRT Path and use Secondary data */
            }
            
            /* 
             * 3. Enable/disable the display devices. 
             */
            swPanelPowerSequence(DISP_ON, 4);                   /* Turn on Panel */
#ifdef ENABLE_PANEL_DAC
            /* When using Analog CRT connected to the DVI channel, the DAC needs
               to be turned on. The power is using the same power as CRT DAC. */
            setDAC(DISP_ON);                                    /* Turn on DAC */
#else
            setDAC(DISP_OFF);                                   /* Turn off DAC */
#endif
            setDPMS(DPMS_OFF);                                  /* Turn off DPMS */
            break;
        }    
        case CRT2_LCD2_SIMUL:
        {
            //DDKDEBUGPRINT((DISPLAY_LEVEL, "CRT2_LCD2_SIMUL\n"));
            
            /* 
             * 1. Check the conditions 
             */
            /* Can not enable PANEL2 when the panel type is not 18-bit panel. */            
            if (getPanelType() != TFT_18BIT)
                return (-1);
            
            /* 
             * 2. Set all the display control and the display path 
             */
            if (dispCtrlUsage == 0)
            {
                setDisplayControl(PRIMARY_CTRL, DISP_ON);           /* Turn on Primary Control */
                if (isScalingEnabled(PRIMARY_CTRL) == 1)
                {
                    setDisplayControl(SECONDARY_CTRL, DISP_ON);     /* Turn on Secondary control */
                    setPath(CRT_PATH, SECONDARY_CTRL, DISP_ON);     /* Turn on CRT Path and use Secondary data */
                }
                else
                {
                    setDisplayControl(SECONDARY_CTRL, DISP_OFF);    /* Turn off Secondary control */
                    setPath(CRT_PATH, PRIMARY_CTRL, DISP_ON);       /* Turn on CRT Path and use Primary data */
                }
                setPath(PANEL_PATH, PRIMARY_CTRL, DISP_OFF);        /* Turn on Panel Path and use Primary data */
            }
            else
            {
                setDisplayControl(PRIMARY_CTRL, DISP_OFF);          /* Turn off Primary Control */
                setDisplayControl(SECONDARY_CTRL, DISP_ON);         /* Turn on Secondary control */
                setPath(CRT_PATH, SECONDARY_CTRL, DISP_ON);         /* Turn on CRT Path and use Secondary data */
                setPath(PANEL_PATH, SECONDARY_CTRL, DISP_OFF);      /* Turn on Panel Path and use Primary data */
            }
            
            /* 
             * 3. Enable/disable the display devices. 
             */
            swPanelPowerSequence(DISP_ON, 4);                   /* Turn on Panel */
            setDAC(DISP_ON);                                    /* Turn on DAC */
            setDPMS(DPMS_ON);                                   /* Turn on DPMS to drive CRT */
            break;
        }    
        case LCD1_LCD2_CRT2_SIMUL:
        {
            //DDKDEBUGPRINT((DISPLAY_LEVEL, "LCD1_LCD2_CRT2_SIMUL\n"));
            
            /* 
             * 1. Check the conditions
             */
            /* Can not enable PANEL2 when the panel type is not 18-bit panel. */            
            if (getPanelType() != TFT_18BIT)
                return (-1);
            
            /* 
             * 2. Set all the display control and the display path 
             */
            if (dispCtrlUsage == 0)
            {            
                setDisplayControl(PRIMARY_CTRL, DISP_ON);           /* Turn on Primary Control */
                if (isScalingEnabled(PRIMARY_CTRL) == 1)
                {
                    setDisplayControl(SECONDARY_CTRL, DISP_ON);     /* Turn on Secondary control */
                    setPath(PANEL_PATH, SECONDARY_CTRL, DISP_ON);   /* Turn on Panel Path and use Secondary data */
                    setPath(CRT_PATH, SECONDARY_CTRL, DISP_ON);     /* Turn on CRT Path and use Secondary data */
                }
                else
                {
                    setDisplayControl(SECONDARY_CTRL, DISP_OFF);    /* Turn off Secondary control */
                    setPath(PANEL_PATH, PRIMARY_CTRL, DISP_ON);     /* Turn on Panel Path and use Primary data */
                    setPath(CRT_PATH, PRIMARY_CTRL, DISP_ON);       /* Turn on CRT Path and use Primary data */
                }
            }
            else
            {
                setDisplayControl(PRIMARY_CTRL, DISP_OFF);          /* Turn off Primary Control */
                setDisplayControl(SECONDARY_CTRL, DISP_ON);         /* Turn on Secondary control */
                setPath(PANEL_PATH, SECONDARY_CTRL, DISP_ON);       /* Turn on Panel Path and use Secondary data */
                setPath(CRT_PATH, SECONDARY_CTRL, DISP_ON);         /* Turn on CRT Path and use Secondary data */
            }
            
            /* 
             * 3. Enable/disable the display devices. 
             */
            swPanelPowerSequence(DISP_ON, 4);                   /* Turn on Panel */
            setDAC(DISP_ON);                                    /* Turn on DAC */
            setDPMS(DPMS_ON);                                   /* Turn on DPMS to drive CRT */
            break;
        }
        case LCD1_CRT2_DUAL: /* Panel and CRT different content */
        {
            //DDKDEBUGPRINT((DISPLAY_LEVEL, "LCD1_CRT2_DUAL\n"));
            
            /* 
             * 1. Check the conditions
             */
            /* This combination is not valid if panel 1 requires scaling and display
               control is not swapped. */
            if (isScalingEnabled(PRIMARY_CTRL) == 1)
                return (-1);

            /* 
             * 2. Set all the display control and the display path 
             */
            if(!isSecondDisplay) 
                setDisplayControl(PRIMARY_CTRL, DISP_ON);           /* Turn on Primary Control */
            else
                setDisplayControl(SECONDARY_CTRL, DISP_ON);         /* Turn on Secondary control */
            if (dispCtrlUsage == 0)
            {
                if(!isSecondDisplay) 
                    setPath(PANEL_PATH, PRIMARY_CTRL, DISP_ON);     /* Turn on Panel Path and use Primary data */
                else
                    setPath(CRT_PATH, SECONDARY_CTRL, DISP_ON);     /* Turn on CRT Path and use Secondary data */
            }
            else
            {
                if(isSecondDisplay)             
                    setPath(PANEL_PATH, SECONDARY_CTRL, DISP_ON);   /* Turn on Panel Path and use Secondary data */
                else
                    setPath(CRT_PATH, PRIMARY_CTRL, DISP_ON);       /* Turn on CRT Path and use Primary data */
            }
            /* 
             * 3. Enable/disable the display devices. 
             */
            swPanelPowerSequence(DISP_ON, 4);                   /* Turn on Panel */
            setDAC(DISP_ON);                                    /* Turn on DAC */
            setDPMS(DPMS_ON);                                   /* Turn on DPMS to drive CRT */
            break;
        }    
        case LCD1_LCD2_DUAL:
        {
            //DDKDEBUGPRINT((DISPLAY_LEVEL, "LCD1_LCD2_DUAL\n"));
            
            /* 
             * 1. Check the conditions
             */    
            /* This combination is not valid if panel requires scaling and display
               control is not swapped. */
            if (isScalingEnabled(PRIMARY_CTRL) == 1)
                return (-1);
                
            /* Can not enable PANEL2 when the panel type is not 18-bit panel. */            
            if (getPanelType() != TFT_18BIT)
                return (-1);
            
            /* 
             * 2. Set all the display control and the display path 
             */
            setDisplayControl(PRIMARY_CTRL, DISP_ON);           /* Turn on Primary Control */
            setDisplayControl(SECONDARY_CTRL, DISP_ON);         /* Turn on Secondary control */
            if (dispCtrlUsage == 0)
            {
                setPath(PANEL_PATH, PRIMARY_CTRL, DISP_ON);     /* Turn on Panel Path and use Primary data */
                setPath(CRT_PATH, SECONDARY_CTRL, DISP_ON);     /* Turn on CRT Path and use Secondary data */
            }
            else
            {
                setPath(PANEL_PATH, SECONDARY_CTRL, DISP_ON);   /* Turn on Panel Path and use Secondary data */
                setPath(CRT_PATH, PRIMARY_CTRL, DISP_ON);       /* Turn on CRT Path and use Primary data */
            }
            
            /* 
             * 3. Enable/disable the display devices. 
             */
            swPanelPowerSequence(DISP_ON, 4);                   /* Turn on Panel */
#ifdef ENABLE_PANEL_DAC
            /* When using Analog CRT connected to the DVI channel, the DAC needs
               to be turned on. The power is using the same power as CRT DAC. */
            setDAC(DISP_ON);                                    /* Turn on DAC */
#else
            setDAC(DISP_OFF);                                   /* Turn off DAC */
#endif
            setDPMS(DPMS_OFF);                                  /* Turn off DPMS to drive CRT */
            break;
        }
        case LCD1_LCD2_CRT2_DUAL:
        {
            //DDKDEBUGPRINT((DISPLAY_LEVEL, "LCD1_LCD2_CRT2_DUAL\n"));
            
            /* 
             * 1. Check the conditions
             */    
            /* This combination is not valid if panel requires scaling and display
               control is not swapped. */
            if (isScalingEnabled(PRIMARY_CTRL) == 1)
                return (-1);
                
            /* Can not enable PANEL2 when the panel type is not 18-bit panel. */            
            if (getPanelType() != TFT_18BIT)
                return (-1);
            
            /* 
             * 2. Set all the display control and the display path 
             */
            setDisplayControl(PRIMARY_CTRL, DISP_ON);           /* Turn on Primary Control */
            setDisplayControl(SECONDARY_CTRL, DISP_ON);         /* Turn on Secondary control */            
            if (dispCtrlUsage == 0)
            {
                setPath(PANEL_PATH, PRIMARY_CTRL, DISP_ON);     /* Turn on Panel Path and use Primary data */
                setPath(CRT_PATH, SECONDARY_CTRL, DISP_ON);     /* Turn on CRT Path and use Secondary data */
            }
            else
            {
                setPath(PANEL_PATH, SECONDARY_CTRL, DISP_ON);   /* Turn on Panel Path and use Secondary data */
                setPath(CRT_PATH, PRIMARY_CTRL, DISP_ON);       /* Turn on CRT Path and use Primary data */
            }
            
            /* 
             * 3. Enable/disable the display devices. 
             */
            swPanelPowerSequence(DISP_ON, 4);                   /* Turn on Panel */
            setDAC(DISP_ON);                                    /* Turn on DAC */
            setDPMS(DPMS_ON);                                   /* Turn on DPMS to drive CRT */
            break;
        }
    }
    
    return 0;
}

/*
 * This function checks whether the scaling is enabled.
 *
 * Input:
 *      dispCtrl    - Display control to be checked.
 *
 * Output:
 *      0   - Scaling is not enabled
 *      1   - Scaling is enabled
 */
unsigned char isScalingEnabled(
    disp_control_t dispCtrl
)
{
    unsigned long value;
    
    /* If display control is not swapped, then check the expansion bit for PRIMARY_CTRL 
       and SECONDARY_SCALE register for SECONDARY_CTRL. */
    if (dispCtrl == PRIMARY_CTRL)
    {
        value = peekRegisterDWord(SECONDARY_DISPLAY_CTRL);
        if (FIELD_GET(value, SECONDARY_DISPLAY_CTRL, EXPANSION) == SECONDARY_DISPLAY_CTRL_EXPANSION_ENABLE)
            return 1;
    }
    else
    {
        value = peekRegisterDWord(SECONDARY_SCALE);
        if ((FIELD_GET(value, SECONDARY_SCALE, VERTICAL_SCALE) != 0) ||
            (FIELD_GET(value, SECONDARY_SCALE, HORIZONTAL_SCALE) != 0))
            return 1;
    }
    
    return 0;
}

