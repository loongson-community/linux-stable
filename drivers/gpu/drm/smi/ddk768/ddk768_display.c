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
#include "ddk768_reg.h"


#include "ddk768_chip.h"
#include "ddk768_power.h"
#include "ddk768_display.h"
#include "ddk768_timer.h"

#include "ddk768_help.h"


/* Monitor Detection RGB Default Threshold values */
#define DEFAULT_MON_DETECTION_THRESHOLD         0x64

/*
 * This function initializes the display.
 *
 * Output:
 *      0   - Success
 *      1   - Fail 
 */
long initDisplay()
{
#if 0
    /* set 80024[30:28] and 88024[30:28] to 0x3 in order for the DAC to output stronger signal. */
    unsigned long value;
    value = peekRegisterDWord(MONITOR_DETECT);
    value &= 0xCFFFFFFF;
    value |= 0x30000000;
    pokeRegisterDWord(MONITOR_DETECT, value);
    value = peekRegisterDWord(MONITOR_DETECT + CHANNEL_OFFSET);
    value &= 0xCFFFFFFF;
    value |= 0x30000000;
    pokeRegisterDWord(MONITOR_DETECT + CHANNEL_OFFSET, value);
#endif
    return 0;

}

/* New for Falcon: DPMS control is moved to display controller.
 * This function sets the display DPMS state 
 * It is used to set CRT monitor to On, off, or suspend states, 
 * while display channel are still active.
 */
void setDisplayDPMS(
   disp_control_t dispControl, /* Channel 0 or Channel 1) */
   DISP_DPMS_t state /* DPMS state */
   )
{
    unsigned long ulDispCtrlAddr;
    unsigned long ulDispCtrlReg;

	/* Get the control register for channel 0 or 1. */
    ulDispCtrlAddr = (dispControl == CHANNEL0_CTRL)? DISPLAY_CTRL : (DISPLAY_CTRL+CHANNEL_OFFSET);
    ulDispCtrlReg = peekRegisterDWord(ulDispCtrlAddr);

    switch (state)
    {
       case DISP_DPMS_ON:
        ulDispCtrlReg = FIELD_SET(ulDispCtrlReg, DISPLAY_CTRL, DPMS, VPHP);
        break;

       case DISP_DPMS_STANDBY:
        ulDispCtrlReg = FIELD_SET(ulDispCtrlReg, DISPLAY_CTRL, DPMS, VPHN);
        break;

       case DISP_DPMS_SUSPEND:
        ulDispCtrlReg = FIELD_SET(ulDispCtrlReg, DISPLAY_CTRL, DPMS, VNHP);
        break;

       case DISP_DPMS_OFF:
        ulDispCtrlReg = FIELD_SET(ulDispCtrlReg, DISPLAY_CTRL, DPMS, VNHN);
        break;
    }

    pokeRegisterDWord(ulDispCtrlAddr, ulDispCtrlReg);
}

/* 
 * New for Falcon.
 * This funciton configures:
 * 1. Output from channel 0 or channel 1 is 24 single or 48 double pixel.
 * 2. Output data comes from data path of channel 0 or channel 1.
 *
 * Input: See the commnet in the input parameter below.
 *
 * Return: 0 is OK, -1 is error.
 */
long setDisplayFormat(
   disp_control_t outputInterface, /* Use the output of channel 0 or 1 */
   disp_control_t dataPath,        /* Use the data path from channel 0 or 1 */
   disp_format_t dispFormat         /* 24 bit single or 48 bit double pixel */
   )
{
   unsigned long ulDispCtrlAddr, ulDispCtrlReg;

   ulDispCtrlAddr = (outputInterface == CHANNEL0_CTRL)? DISPLAY_CTRL : (DISPLAY_CTRL+CHANNEL_OFFSET);
   ulDispCtrlReg = peekRegisterDWord(ulDispCtrlAddr);

   if (dispFormat == DOUBLE_PIXEL_48BIT)
   {
      if (dataPath == CHANNEL0_CTRL)
           ulDispCtrlReg = FIELD_SET(ulDispCtrlReg, DISPLAY_CTRL, CHANNEL_OUTPUT_FORMAT, CHANNEL0_48BIT);
      else
           ulDispCtrlReg = FIELD_SET(ulDispCtrlReg, DISPLAY_CTRL, CHANNEL_OUTPUT_FORMAT, CHANNEL1_48BIT);

        // When in 48 bit mode, enable double pixel clock
        ulDispCtrlReg = FIELD_SET(ulDispCtrlReg, DISPLAY_CTRL, DOUBLE_PIXEL_CLOCK, ENABLE);
   }
   else
   {
      if (dataPath == CHANNEL0_CTRL)
           ulDispCtrlReg = FIELD_SET(ulDispCtrlReg, DISPLAY_CTRL, CHANNEL_OUTPUT_FORMAT, CHANNEL0_24BIT);
      else
           ulDispCtrlReg = FIELD_SET(ulDispCtrlReg, DISPLAY_CTRL, CHANNEL_OUTPUT_FORMAT, CHANNEL1_24BIT);

        // When in 24 bit mode, disable double pixel.
        ulDispCtrlReg = FIELD_SET(ulDispCtrlReg, DISPLAY_CTRL, DOUBLE_PIXEL_CLOCK, DISABLE);
   }

   pokeRegisterDWord(ulDispCtrlAddr, ulDispCtrlReg);

   return 0;
}

/*
 * Wait number of Vertical Vsync
 *
 * Input:
 *          dispControl - Display Control (either channel 0 or 1)
 *          vSyncCount  - Number of vertical sync to wait.
 *
 * Note:
 *      This function is waiting for the next vertical sync.         
 */
void waitDispVerticalSync(disp_control_t dispControl, unsigned long vSyncCount)
{
    unsigned long ulDispCtrlAddr;
    unsigned long status;
    unsigned long ulLoopCount = 0;
    static unsigned long ulDeadLoopCount = 10;
    
    if (dispControl == CHANNEL0_CTRL)
    {
        // There is no Vsync when PLL is off
        if ((FIELD_GET(peekRegisterDWord(CLOCK_ENABLE), CLOCK_ENABLE, DC0) == CLOCK_ENABLE_DC0_OFF))
            return;

        ulDispCtrlAddr = DISPLAY_CTRL;
    }
    else
    {
        // There is no Vsync when PLL is off
        if ((FIELD_GET(peekRegisterDWord(CLOCK_ENABLE), CLOCK_ENABLE, DC1) == CLOCK_ENABLE_DC1_OFF))
            return;

        ulDispCtrlAddr = DISPLAY_CTRL+CHANNEL_OFFSET;
    }

    //There is no Vsync when display timing is off. 
    if ((FIELD_GET(peekRegisterDWord(ulDispCtrlAddr), DISPLAY_CTRL, TIMING) ==
         DISPLAY_CTRL_TIMING_DISABLE))
    {
            return;
    }

    /* Count number of Vsync. */
    while (vSyncCount-- > 0)
    {
        /* If VSync is active when entering this function. Ignore it and
           wait for the next.
        */
        ulLoopCount = 0;
        do
        {
            status = FIELD_GET(peekRegisterDWord(ulDispCtrlAddr), DISPLAY_CTRL, VSYNC);
            //Insert delay to reduce number of Vsync checks
            timerWaitTicks(3, 0xffff);
            if(ulLoopCount++ > ulDeadLoopCount) break;
        }
        while (status == DISPLAY_CTRL_VSYNC_ACTIVE);

        /* Wait for end of vsync or timeout */
        ulLoopCount = 0;
        do
        {
            status = FIELD_GET(peekRegisterDWord(ulDispCtrlAddr), DISPLAY_CTRL, VSYNC);
            timerWaitTicks(3, 0xffff);
            if(ulLoopCount++ > ulDeadLoopCount) break;
        }
        while (status == DISPLAY_CTRL_VSYNC_INACTIVE);
    }
}

/*
 * Use panel vertical sync line as time delay function.
 * This function does not wait for the next VSync. Instead, it will wait
 * until the current line reaches the Vertical Sync line.
 * This function is really useful when flipping display to prevent tearing.
 *
 * Input: display control (CHANNEL0_CTRL or CHANNEL1_CTRL)
 */
void ddk768_waitVSyncLine(disp_control_t dispControl)
{
    unsigned long ulDispCtrlAddr;
    unsigned long value;
    mode_parameter_t modeParam;
    
    ulDispCtrlAddr = (dispControl == CHANNEL0_CTRL)? CURRENT_LINE : (CURRENT_LINE+CHANNEL_OFFSET);

    /* Get the current mode parameter of the specific display control */
    modeParam = ddk768_getCurrentModeParam(dispControl);
    
    do
    {
    	value = FIELD_GET(peekRegisterDWord(ulDispCtrlAddr), CURRENT_LINE, LINE);
    }
    while (value < modeParam.vertical_sync_start);
}

/*
 * Get current display line number
 */
unsigned long getDisplayLine(disp_control_t dispControl)
{
    unsigned long ulRegAddr;
    unsigned long ulRegValue;
    
    ulRegAddr = (dispControl == CHANNEL0_CTRL)? CURRENT_LINE : (CURRENT_LINE+DC_OFFSET);
    ulRegValue = FIELD_GET(peekRegisterDWord(ulRegAddr), CURRENT_LINE, LINE);

    return(ulRegValue);
}

/*
 * This functions uses software sequence to turn on/off the panel of the digital interface.
 */
void ddk768_swPanelPowerSequence(disp_control_t dispControl, disp_state_t dispState, unsigned long vSyncDelay)
{
    unsigned long ulDispCtrlAddr;
    unsigned long ulDispCtrlReg;


    ulDispCtrlAddr = (dispControl == CHANNEL0_CTRL)? DISPLAY_CTRL : (DISPLAY_CTRL+CHANNEL_OFFSET);
    ulDispCtrlReg = peekRegisterDWord(ulDispCtrlAddr);

    if (dispState == DISP_ON)
    {
        //If bit 27:24 are already ON. Don't need to set them again 
        //because setting panel seq is time consuming.
        if ((ulDispCtrlReg & 0x0f000000) == 0x0f000000) return;

        /* Turn on FPVDDEN. */
        ulDispCtrlReg = FIELD_SET(ulDispCtrlReg, DISPLAY_CTRL, FPVDDEN, HIGH);
        pokeRegisterDWord(ulDispCtrlAddr, ulDispCtrlReg);
		waitDispVerticalSync(dispControl, vSyncDelay);

        /* Turn on FPDATA. */
        ulDispCtrlReg = FIELD_SET(ulDispCtrlReg, DISPLAY_CTRL, DATA, ENABLE);
        pokeRegisterDWord(ulDispCtrlAddr, ulDispCtrlReg);
		waitDispVerticalSync(dispControl, vSyncDelay);

        /* Turn on FPVBIAS. */
        ulDispCtrlReg = FIELD_SET(ulDispCtrlReg, DISPLAY_CTRL, VBIASEN, HIGH);
        pokeRegisterDWord(ulDispCtrlAddr, ulDispCtrlReg);
		waitDispVerticalSync(dispControl, vSyncDelay);

        /* Turn on FPEN. */
        ulDispCtrlReg = FIELD_SET(ulDispCtrlReg, DISPLAY_CTRL, FPEN, HIGH);
        pokeRegisterDWord(ulDispCtrlAddr, ulDispCtrlReg);
    }
    else
    {
        //If bit 27:24 are already OFF. Don't need to clear them again.
        if ((ulDispCtrlReg & 0x0f000000) == 0x00000000) return;

        /* Turn off FPEN. */
        ulDispCtrlReg = FIELD_SET(ulDispCtrlReg, DISPLAY_CTRL, FPEN, LOW);
        pokeRegisterDWord(ulDispCtrlAddr, ulDispCtrlReg);
		waitDispVerticalSync(dispControl, vSyncDelay);


        /* Turn off FPVBIASEN. */
        ulDispCtrlReg = FIELD_SET(ulDispCtrlReg, DISPLAY_CTRL, VBIASEN, LOW);
        pokeRegisterDWord(ulDispCtrlAddr, ulDispCtrlReg);
		waitDispVerticalSync(dispControl, vSyncDelay);

        /* Turn off FPDATA. */
        ulDispCtrlReg = FIELD_SET(ulDispCtrlReg, DISPLAY_CTRL, DATA, DISABLE);
        pokeRegisterDWord(ulDispCtrlAddr, ulDispCtrlReg);
		waitDispVerticalSync(dispControl, vSyncDelay);

        /* Turn off FPVDDEN. */
        ulDispCtrlReg = FIELD_SET(ulDispCtrlReg, DISPLAY_CTRL, FPVDDEN, LOW);
        pokeRegisterDWord(ulDispCtrlAddr, ulDispCtrlReg);
    }

}

/* 
 * This function turns on/off the DAC for CRT display control.
 * Input: On or off
 */
void ddk768_setDAC(disp_state_t state)
{

#if 1
	/* Cheok(10/16/2013): For Falcon, DAC definition not exist yet.*/

#else
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
#endif
}

/*
 * This function turns on/off the display control of Channel 0 or channel 1.
 *
 * Note:
 *      Turning on/off the timing and the plane requires programming sequence.
 *      The plane can not be changed without turning on the timing. However,
 *      changing the plane has no effect when the timing (clock) is off. Below,
 *      is the description of the timing and plane combination setting.
 */
//Cheok(10/18/2013): New function similar to setDisplayControl()
void ddk768_setDisplayEnable(
   disp_control_t dispControl, /* Channel 0 or Channel 1) */
   disp_state_t dispState      /* ON or OFF */
)
{
    unsigned long ulDispCtrlAddr;
    unsigned long ulDispCtrlReg;

    ulDispCtrlAddr = (dispControl == CHANNEL0_CTRL)? DISPLAY_CTRL : (DISPLAY_CTRL+CHANNEL_OFFSET);
    ulDispCtrlReg = peekRegisterDWord(ulDispCtrlAddr);



    /* Turn on/off the Panel display control */
    if (dispState == DISP_ON)
    {
         /* Timing should be enabled first before enabling the plane because changing at the
            same time does not guarantee that the plane will also enabled or disabled. 
          */
         ulDispCtrlReg = FIELD_SET(ulDispCtrlReg, DISPLAY_CTRL, TIMING, ENABLE);
         ulDispCtrlReg = FIELD_SET(ulDispCtrlReg, DISPLAY_CTRL, DIRECTION, INPUT); 
         pokeRegisterDWord(ulDispCtrlAddr, ulDispCtrlReg);

         ulDispCtrlReg = FIELD_SET(ulDispCtrlReg, DISPLAY_CTRL, PLANE, ENABLE)|        
                            FIELD_SET(0, DISPLAY_CTRL, DATA_PATH, EXTENDED); 
         pokeRegisterDWord(ulDispCtrlAddr, ulDispCtrlReg);
    }
    else
    {
         /* When turning off, there is no rule on the programming sequence since whenever the
            clock is off, then it does not matter whether the plane is enabled or disabled.
            Note: Modifying the plane bit will take effect on the next vertical sync. Need to
                  find out if it is necessary to wait for 1 vsync before modifying the timing
                  enable bit. 
          */
         ulDispCtrlReg = FIELD_SET(ulDispCtrlReg, DISPLAY_CTRL, PLANE, DISABLE);
         pokeRegisterDWord(ulDispCtrlAddr, ulDispCtrlReg);

         ulDispCtrlReg = FIELD_SET(ulDispCtrlReg, DISPLAY_CTRL, TIMING, DISABLE);
         pokeRegisterDWord(ulDispCtrlAddr, ulDispCtrlReg);

    }


}


/*
 * This function set display channel data direction.
 *
 */

void setDataDirection(
   disp_control_t dispControl, /* Channel 0 or Channel 1) */
   disp_state_t dispState      /* ON or OFF */
)
{
    unsigned long ulDispCtrlAddr;
    unsigned long ulDispCtrlReg;
    if (dispState == DISP_ON)
    {
         ulDispCtrlAddr = (dispControl == CHANNEL0_CTRL)? DISPLAY_CTRL : (DISPLAY_CTRL+CHANNEL_OFFSET);
         ulDispCtrlReg = peekRegisterDWord(ulDispCtrlAddr);

         /*set data direction */ 
         ulDispCtrlReg = FIELD_SET(ulDispCtrlReg, DISPLAY_CTRL, DIRECTION, INPUT); 
         pokeRegisterDWord(ulDispCtrlAddr, ulDispCtrlReg);


     }
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
long ddk768_detectCRTMonitor(
    disp_control_t dispControl, 
    unsigned char redValue,
    unsigned char greenValue,
    unsigned char blueValue
)
{
    unsigned long ulMonitorDetectAddr;
    unsigned long value, red, green, blue;
    long result = (-1);

    ulMonitorDetectAddr = (dispControl == CHANNEL0_CTRL)? CRT_DETECT : (CRT_DETECT+DC_OFFSET);
    
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
    value = FIELD_VALUE(0, CRT_DETECT, DATA_RED, red) |
            FIELD_VALUE(0, CRT_DETECT, DATA_GREEN, green) |
            FIELD_VALUE(0, CRT_DETECT, DATA_BLUE, blue) |
            FIELD_SET(value, CRT_DETECT, ENABLE, ENABLE);
    pokeRegisterDWord(ulMonitorDetectAddr, value);
    
    /* Add some delay here. Otherwise, the detection is not stable.
       SM768 has internal timer. It's better than SW countdown loop.
    */
    timerWaitTicks(3, 0x7ffff);
    
    /* Check if the monitor is detected. */
    if (FIELD_GET(peekRegisterDWord(ulMonitorDetectAddr), CRT_DETECT, CRT) ==
        CRT_DETECT_CRT_PRESENT)
    {
        result = 0;
    }
    
    /* Disable the Monitor Detect Enable bit. Somehow, enabling this bit will 
       cause the CRT to lose display. */
    value = peekRegisterDWord(ulMonitorDetectAddr);
    value = FIELD_SET(value, CRT_DETECT, ENABLE, DISABLE);
    pokeRegisterDWord(ulMonitorDetectAddr, value);

    return result;
}


/*
 * This function controls monitor on/off and data path.
 * It can be used to set up any veiws: single view, clone view, dual view, output with channel swap, etc.
 * However, it needs too many input parameter.
 * There are other set view functions with less parameters, but not as flexible as this one.
 *
 */
long setDisplayView(
	disp_control_t dispOutput, 		/* Monitor 0 or 1 */
	disp_state_t dispState,				/* On or off */
	disp_control_t dataPath,			/* Use the data path of channel 0 or channel 1 (optional when OFF) */
	disp_format_t dispFormat)			/* 24 or 48 bit digital interface (optional when OFF */
{

	ddk768_setDisplayEnable(dispOutput, dispState);         /* Enable or disable Channel output timing */
	ddk768_swPanelPowerSequence(dispOutput, dispState, 4);  /* Turn on or off output power */
	setDisplayFormat(dispOutput, dataPath, dispFormat); /* Set dataPath and output pixel format */

#if 0
	if (dispState == DISP_ON)
    	setDisplayDPMS(dispOutput, DISP_DPMS_ON);          /* DPMS on */
#endif
	return 0;
}

/*
 * Convenient function to trun on single view 
 */
long setSingleViewOn(disp_control_t dispOutput)
{
	setDisplayView(
		dispOutput, 			/* Output monitor */
		DISP_ON, 				/* Turn On */
		dispOutput,				/* Assume monitor 0 is using data path 0, and monitor 1 is using data path 1 */
		SINGLE_PIXEL_24BIT);	/* Default to 24 bit single pixel, the most used case */

	return 0;
}

/*
 * Convenient function to trun off single view 
 */
long setSingleViewOff(disp_control_t dispOutput)
{
	setDisplayView(
		dispOutput, 	      /* Output monitor */
		DISP_OFF, 		      /* Turn Off */
		dispOutput,				/* Assume monitor 0 is using data path 0, and monitor 1 is using data path 1 */
		SINGLE_PIXEL_24BIT);	/* Default to 24 bit single pixel, the most used case */

	return 0;
}

/*
 * Convenient function to trun on clone view 
 */
long setCloneViewOn(disp_control_t dataPath)
{
	setDisplayView(
		CHANNEL0_CTRL,			/* For Clone view, monitor 0 has to be ON */
		DISP_ON, 
		dataPath,				/* Use this data path for monitor 0 */
		SINGLE_PIXEL_24BIT);	/* Default to 24 bit single pixel, the most used case */

	setDisplayView(
		CHANNEL1_CTRL,			/* For Clone view, monitor 1 has to be ON */
		DISP_ON, 
		dataPath,				/* Use this data path for monitor 1 */
		SINGLE_PIXEL_24BIT);	/* Default to 24 bit single pixel, the most used case */

	return 0;
}

/*
 * Convenient function to trun on dual view 
 */
long setDualViewOn()
{
	setSingleViewOn(CHANNEL0_CTRL);
	setSingleViewOn(CHANNEL1_CTRL);

	return 0;
}

/*
 * Convenient function to trun off all views
 */
long setAllViewOff()
{
	setSingleViewOff(CHANNEL0_CTRL);	/* Turn Off monitor 0 */
	setSingleViewOff(CHANNEL1_CTRL);	/* Turn Off monitor 1 */

	return 0;
}

/*
 * Disable double pixel clock. 
 * This is a temporary function, used to patch for the random fuzzy font problem. 
 */
void DisableDoublePixel(disp_control_t dispControl)
{
	unsigned long ulDispCtrlAddr;
	unsigned long ulDispCtrlReg;

	if(dispControl == CHANNEL0_CTRL) {
	    ulDispCtrlAddr = DISPLAY_CTRL;
	    ulDispCtrlReg = peekRegisterDWord(ulDispCtrlAddr);
	    ulDispCtrlReg = FIELD_SET(ulDispCtrlReg, DISPLAY_CTRL, DOUBLE_PIXEL_CLOCK, DISABLE);
	    pokeRegisterDWord(ulDispCtrlAddr, ulDispCtrlReg);
	}else{
	    ulDispCtrlAddr = DISPLAY_CTRL+CHANNEL_OFFSET;
	    ulDispCtrlReg = peekRegisterDWord(ulDispCtrlAddr);
	    ulDispCtrlReg = FIELD_SET(ulDispCtrlReg, DISPLAY_CTRL, DOUBLE_PIXEL_CLOCK, DISABLE);
	    pokeRegisterDWord(ulDispCtrlAddr, ulDispCtrlReg);
	}
}

void EnableDoublePixel(disp_control_t dispControl)
{
	unsigned long ulDispCtrlAddr;
	unsigned long ulDispCtrlReg;

	if(dispControl == CHANNEL0_CTRL) {
	    ulDispCtrlAddr = DISPLAY_CTRL;
	    ulDispCtrlReg = peekRegisterDWord(ulDispCtrlAddr);
	    ulDispCtrlReg = FIELD_SET(ulDispCtrlReg, DISPLAY_CTRL, DOUBLE_PIXEL_CLOCK, ENABLE);
	    pokeRegisterDWord(ulDispCtrlAddr, ulDispCtrlReg);
	}else{
	    ulDispCtrlAddr = DISPLAY_CTRL+CHANNEL_OFFSET;
	    ulDispCtrlReg = peekRegisterDWord(ulDispCtrlAddr);
	    ulDispCtrlReg = FIELD_SET(ulDispCtrlReg, DISPLAY_CTRL, DOUBLE_PIXEL_CLOCK, ENABLE);
	    pokeRegisterDWord(ulDispCtrlAddr, ulDispCtrlReg);
	}
}
