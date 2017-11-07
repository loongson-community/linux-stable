/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  MODE.C --- SMI DDK 
*  This file contains the source code for the mode table.
* 
*******************************************************************/
//#include <string.h>
#include "ddk750_defs.h"
#include "ddk750_chip.h"
#include "ddk750_clock.h"
#include "ddk750_hardware.h"
#include "ddk750_helper.h"
#include "ddk750_power.h"
#include "ddk750_mode.h"
#include "ddk750_help.h"

//#include "ddk750_os.h"

//#include "ddkdebug.h"

#define SCALE_CONSTANT                      (1 << 12)

/* Maximum panel size scaling */
#define MAX_PANEL_SIZE_WIDTH                1920
#define MAX_PANEL_SIZE_HEIGHT               1440

/* The valid signature of the user data pointer  for the setmode function. 
   The following definition is ASCII representation of the word 'USER'
 */
#define MODE_USER_DATA_SIGNATURE            0x55534552

static mode_parameter_t gPrimaryModeParamTable[MAX_SMI_DEVICE][MAX_MODE_TABLE_ENTRIES] =
{
    {
    /* End of table */
     { 0, 0, 0, 0, NEG, 0, 0, 0, 0, NEG, 0, 0, 0, NEG},
    },
    {
    /* End of table */
     { 0, 0, 0, 0, NEG, 0, 0, 0, 0, NEG, 0, 0, 0, NEG},
    },
    {
    /* End of table */
     { 0, 0, 0, 0, NEG, 0, 0, 0, 0, NEG, 0, 0, 0, NEG},
    },
    {
    /* End of table */
     { 0, 0, 0, 0, NEG, 0, 0, 0, 0, NEG, 0, 0, 0, NEG},
    }
};

static mode_parameter_t gSecondaryModeParamTable[MAX_SMI_DEVICE][MAX_MODE_TABLE_ENTRIES] =
{
    {
    /* End of table */
     { 0, 0, 0, 0, NEG, 0, 0, 0, 0, NEG, 0, 0, 0, NEG},
    },
    {
    /* End of table */
     { 0, 0, 0, 0, NEG, 0, 0, 0, 0, NEG, 0, 0, 0, NEG},
    },
    {
    /* End of table */
     { 0, 0, 0, 0, NEG, 0, 0, 0, 0, NEG, 0, 0, 0, NEG},
    },
    {
    /* End of table */
     { 0, 0, 0, 0, NEG, 0, 0, 0, 0, NEG, 0, 0, 0, NEG},
    }
};

/* Static variable to store the mode information. */
static mode_parameter_t gPrimaryCurrentModeParam[MAX_SMI_DEVICE];
static mode_parameter_t gSecondaryCurrentModeParam[MAX_SMI_DEVICE];

/*
 *  getUserDataSignature
 *      This function gets the user data mode signature
 *
 *  Output:
 *      The signature to be filled in the user_data_mode_t structure to be considered
 *      a valid structure.
 */
unsigned long getUserDataSignature(void)
{
    return MODE_USER_DATA_SIGNATURE;
}

/*
 *  findModeParamFromTable
 *      This function locates the requested mode in the given parameter table
 *
 *  Input:
 *      width           - Mode width
 *      height          - Mode height
 *      refresh_rate    - Mode refresh rate
 *      index           - Index that is used for multiple search of the same mode 
 *                        that have the same width, height, and refresh rate, 
 *                        but have different timing parameters.
 *
 *  Output:
 *      Success: return a pointer to the mode_parameter_t entry.
 *      Fail: a NULL pointer.
 */
mode_parameter_t *findModeParamFromTable(
    unsigned long width, 
    unsigned long height, 
    unsigned long refresh_rate,
    unsigned short index,
    mode_parameter_t *pModeTable
)
{
    unsigned short modeIndex = 0, tempIndex = 0;
    
    /* Walk the entire mode table. */    
    while (pModeTable[modeIndex].pixel_clock != 0)
    {
        if (((width == (unsigned long)(-1)) || (pModeTable[modeIndex].horizontal_display_end == width)) &&
            ((height == (unsigned long)(-1)) || (pModeTable[modeIndex].vertical_display_end == height)) &&
            ((refresh_rate == (unsigned long)(-1)) || (pModeTable[modeIndex].vertical_frequency == refresh_rate)))
        {
            //if (tempIndex < index)
            //    tempIndex++;
           // else
                return (&pModeTable[modeIndex]);
        }
        
        /* Next entry */
        modeIndex++;
    }

    /* No match, return NULL pointer */
    return((mode_parameter_t *)0);
}

/*
 *  Locate in-stock parameter table for the requested mode.
 *  Success: return a pointer to the mode_parameter_t entry.
 *  Fail: a NULL pointer.
 */
mode_parameter_t *findModeParam(
    disp_control_t dispCtrl,
    unsigned long width, 
    unsigned long height, 
    unsigned long refresh_rate,
    unsigned short index
)
{
    return findModeParamFromTable(width, height, refresh_rate, index, getStockModeParamTableEx(dispCtrl));
}

/*
 *  Use the
 *  Locate timing parameter for the requested mode from the default mode table.
 *  Success: return a pointer to the mode_parameter_t entry.
 *  Fail: a NULL pointer.
 */
mode_parameter_t *findVesaModeParam(
    unsigned long width, 
    unsigned long height, 
    unsigned long refresh_rate
)
{
	switch(ddk750_getChipType())
	{
		default: /* Normal SM750/SM718 */
    		return findModeParamFromTable(width, height, refresh_rate, 0, gDefaultModeParamTable);
	}
}

/*
 * (Obsolete) 
 * Return a point to the gDefaultModeParamTable.
 * Function in other files used this to get the mode table pointer.
 */
mode_parameter_t *getStockModeParamTable(void)
{
	switch(ddk750_getChipType())
	{
		default:
    		return (gDefaultModeParamTable);
	}	
}

/*
 * (Obsolete)
 * Return the size of the Stock Mode Param Table
 */
unsigned long getStockModeParamTableSize(void)
{

    return (sizeof(gDefaultModeParamTable) / sizeof(mode_parameter_t) - 1);

}

/* 
 *  getStockModeParamTableEx
 *      This function gets the mode parameters table associated to the
 *      display control (PRIMARY_CTRL or SECONDAR_CTRL).
 *
 *  Input:
 *      dispCtrl    - Display Control of the mode table that is associated to.
 *
 *  Output:
 *      Pointer to the mode table
 */
mode_parameter_t *getStockModeParamTableEx(
    disp_control_t dispCtrl
)
{
    mode_parameter_t *pModeTable;
    pModeTable = getStockModeParamTable();
#if 0 
    if (dispCtrl == PRIMARY_CTRL)
        pModeTable = (mode_parameter_t *)&gPrimaryModeParamTable[getCurrentDevice()];
    else
        pModeTable = (mode_parameter_t *)&gSecondaryModeParamTable[getCurrentDevice()];
        
    /* Check if the table exist by checking the first entry. 
       If it doesn't, then use the default mode table. */
    if (pModeTable->pixel_clock == 0)
	{
#if 1 /* Cheok_2013_0118 */
		pModeTable = getStockModeParamTable();
#else
		if (ddk750_getChipType() == SM750LE)
        	pModeTable = (mode_parameter_t *)&gSM750LEModeParamTable;
		else
        	pModeTable = (mode_parameter_t *)&gDefaultModeParamTable;
#endif
	}
#endif   
    return (pModeTable);
}

/*
 *  getStockModeParamTableSizeEx
 *      This function gets the size of the mode parameter table associated with
 *      specific display control
 *
 *  Input:
 *      dispCtrl    - Display control of the mode param table that is associated to.
 *
 *  Output:
 *      Size of the requeted mode param table.
 */
unsigned long getStockModeParamTableSizeEx(
    disp_control_t dispCtrl
)
{
    unsigned long tableSize;
    mode_parameter_t *pModeTable;
    
    /* Get the mode table */
    pModeTable = getStockModeParamTableEx(dispCtrl);
    
    /* Calculate the table size by finding the end of table entry indicated by all zeroes. */    
    tableSize = 0;
    while (pModeTable[tableSize].pixel_clock != 0)
        tableSize++;
        
    return tableSize;
}


/* 
 * This function returns the current mode.
 */
mode_parameter_t getCurrentModeParam(
    disp_control_t dispCtrl
)
{
    if (dispCtrl == PRIMARY_CTRL)
        return gPrimaryCurrentModeParam[getCurrentDevice()];
    else
        return gSecondaryCurrentModeParam[getCurrentDevice()];
}

/*
 *  Convert the timing into possible SM750 timing.
 *  If actual pixel clock is not equal to timing pixel clock.
 *  other parameter like horizontal total and sync have to be changed.
 *
 *  Input: Pointer to a mode parameters.
 *         Pointer to a an empty mode parameter structure to be filled.
 *         Actual pixel clock generated by SMI hardware.
 *
 *  Output:
 *      1) Fill up input structure mode_parameter_t with possible timing for SM750.
 */
long adjustModeParam(
mode_parameter_t *pModeParam,/* Pointer to mode parameter */
mode_parameter_t *pMode,     /* Pointer to mode parameter to be updated here */
unsigned long ulPClk         /* real pixel clock feasible by SM750 */
)
{
    unsigned long blank_width, sync_start, sync_width;

    /* Sanity check */
    if ( pModeParam == (mode_parameter_t *)0 ||
         pMode      == (mode_parameter_t *)0 ||
         ulPClk     == 0)
    {
        return -1;
    }

    /* Copy VESA mode into SM750 mode. */
    *pMode = *pModeParam;

    /* If it can generate the vesa required pixel clock, and there are a minimum of
       24 pixel difference between the horizontal sync start and the horizontal display
       end, then there is nothing to change */
    if ((ulPClk == pModeParam->pixel_clock) && 
        ((pModeParam->horizontal_sync_start - pModeParam->horizontal_display_end) > 24))
        return 0;

    pMode->pixel_clock = ulPClk; /* Update actual pixel clock into mode */

    /* Calculate the sync percentages of the VESA mode. */
    blank_width = pModeParam->horizontal_total - pModeParam->horizontal_display_end;
    sync_start = roundedDiv((pModeParam->horizontal_sync_start -
                       pModeParam->horizontal_display_end) * 100, blank_width);
    sync_width = roundedDiv(pModeParam->horizontal_sync_width * 100, blank_width);

     /* Calculate the horizontal total based on the actual pixel clock and VESA line frequency. */
    pMode->horizontal_total = roundedDiv(pMode->pixel_clock,
                                    pModeParam->horizontal_frequency);

    /* Calculate the sync start and width based on the VESA percentages. */
    blank_width = pMode->horizontal_total - pMode->horizontal_display_end;

    if (ddk750_getChipType() == SM750)
    {
        unsigned long sync_adjustment;
            
        /* There is minimum delay of 22 pixels between the horizontal display end 
           to the horizontal sync start. Therefore, the horizontal sync start value
           needs to be adjusted if the value falls below 22 pixels.
           The 22 pixels comes from the propagating delay from the CRT display to
           all the 11 display pipes inside SM750. The factor of 2 is caused by the
           double pixel support in SM750. The value used here is 24 to align to 
           8 bit character width.
         */
        sync_adjustment = roundedDiv(blank_width * sync_start, 100);
        if (sync_adjustment < 24)
            sync_adjustment = 24;
        pMode->horizontal_sync_start = pMode->horizontal_display_end + sync_adjustment;
    
        /* Check if the adjustment of the sync start will cause the sync width to go
           over the horizontal total. If it is, then reduce the width instead of
           changing the horizontal total.
         */
        /* Maximum value for sync width and back porch. */
        sync_adjustment = blank_width - sync_adjustment;
        pMode->horizontal_sync_width = roundedDiv(blank_width * sync_width, 100);
        if (sync_adjustment <= pMode->horizontal_sync_width)
            pMode->horizontal_sync_width = sync_adjustment/2;
    }
    else
    {
        /* SM718 does not have the above restriction. */
        pMode->horizontal_sync_start = pMode->horizontal_display_end + roundedDiv(blank_width * sync_start, 100);
        pMode->horizontal_sync_width = roundedDiv(blank_width * sync_width, 100);
    }
    
    /* Calculate the line and screen frequencies. */
    pMode->horizontal_frequency = roundedDiv(pMode->pixel_clock,
                                        pMode->horizontal_total);
    pMode->vertical_frequency = roundedDiv(pMode->horizontal_frequency,
                                      pMode->vertical_total);
    return 0;
}


/*
 *	This function gets the display status
 *
 *	Input:
 *		dispControl		- display control of which display status to be retrieved.
 *
 *  Output:
 *      0   - Display is pending
 *     -1   - Display is not pending
 */
long isCurrentDisplayPending(
    disp_control_t dispControl
)
{
    unsigned long value;

    /* Get the display status */
    if (dispControl == PRIMARY_CTRL)
    {
        if (FIELD_GET(peekRegisterDWord(PRIMARY_FB_ADDRESS), PRIMARY_FB_ADDRESS, STATUS) == PRIMARY_FB_ADDRESS_STATUS_PENDING)
            return 0;
    }
	else if (dispControl == SECONDARY_CTRL)
    {
        if (FIELD_GET(peekRegisterDWord(SECONDARY_FB_ADDRESS), SECONDARY_FB_ADDRESS, STATUS) == SECONDARY_FB_ADDRESS_STATUS_PENDING)
            return 0;
    }

    return (-1);
}

/*
 *	This function sets the display base address
 *
 *	Input:
 *		dispControl		- display control of which base address to be set.
 *		ulBaseAddress	- Base Address value to be set.
 */
void setDisplayBaseAddress(
	disp_control_t dispControl,
	unsigned long ulBaseAddress
)
{
	if (dispControl == PRIMARY_CTRL)
	{
		/* Frame buffer base for this mode */
	    pokeRegisterDWord(PRIMARY_FB_ADDRESS,
              FIELD_SET(0, PRIMARY_FB_ADDRESS, STATUS, PENDING)
            | FIELD_SET(0, PRIMARY_FB_ADDRESS, EXT, LOCAL)
            | FIELD_VALUE(0, PRIMARY_FB_ADDRESS, ADDRESS, ulBaseAddress));
	}
	else if (dispControl == SECONDARY_CTRL)
	{
        /* Frame buffer base for this mode */
        pokeRegisterDWord(SECONDARY_FB_ADDRESS,
              FIELD_SET(0, SECONDARY_FB_ADDRESS, STATUS, PENDING)
            | FIELD_SET(0, SECONDARY_FB_ADDRESS, EXT, LOCAL)
            | FIELD_VALUE(0, SECONDARY_FB_ADDRESS, ADDRESS, ulBaseAddress));
	}
}


/* 
 * Program the hardware for a specific video mode
 */
void programModeRegisters(
mode_parameter_t *pModeParam,   /* mode information about pixel clock, horizontal total, etc. */
unsigned long ulBpp,            /* Color depth for this mode */
unsigned long ulBaseAddress,    /* Offset in frame buffer */
unsigned long ulPitch,          /* Mode pitch value in byte: no of bytes between two lines. */
pll_value_t *pPLL               /* Pre-calculated values for the PLL */
)
{
    unsigned long ulTmpValue, ulReg, ulReservedBits;
    unsigned long palette_ram;
    unsigned long offset;
	logical_chip_type_t chipType;

    /* Enable display power gate */
    ulTmpValue = peekRegisterDWord(CURRENT_GATE);
    ulTmpValue = FIELD_SET(ulTmpValue, CURRENT_GATE, DISPLAY, ON);
    setCurrentGate(ulTmpValue);

    if (pPLL->clockType == SECONDARY_PLL)
    {
   // printk("func[%s], secondary reg, ulPitch=[%d]\n", __func__, ulPitch);
        /* Secondary Display Control: SECONDARY_PLL */
        pokeRegisterDWord(SECONDARY_PLL_CTRL, formatPllReg(pPLL)); 

        /* Frame buffer base for this mode */
        //setDisplayBaseAddress(SECONDARY_CTRL, ulBaseAddress);//move by ilena

        /* Pitch value (Sometime, hardware people calls it Offset) */
       // pokeRegisterDWord(SECONDARY_FB_WIDTH,
         //     FIELD_VALUE(0, SECONDARY_FB_WIDTH, WIDTH, ulPitch)
           // | FIELD_VALUE(0, SECONDARY_FB_WIDTH, OFFSET, ulPitch));

        pokeRegisterDWord(SECONDARY_HORIZONTAL_TOTAL,
              FIELD_VALUE(0, SECONDARY_HORIZONTAL_TOTAL, TOTAL, pModeParam->horizontal_total - 1)
            | FIELD_VALUE(0, SECONDARY_HORIZONTAL_TOTAL, DISPLAY_END, pModeParam->horizontal_display_end - 1));

        pokeRegisterDWord(SECONDARY_HORIZONTAL_SYNC,
              FIELD_VALUE(0, SECONDARY_HORIZONTAL_SYNC, WIDTH, pModeParam->horizontal_sync_width)
            | FIELD_VALUE(0, SECONDARY_HORIZONTAL_SYNC, START, pModeParam->horizontal_sync_start - 1));

        pokeRegisterDWord(SECONDARY_VERTICAL_TOTAL,
              FIELD_VALUE(0, SECONDARY_VERTICAL_TOTAL, TOTAL, pModeParam->vertical_total - 1)
            | FIELD_VALUE(0, SECONDARY_VERTICAL_TOTAL, DISPLAY_END, pModeParam->vertical_display_end - 1));

        pokeRegisterDWord(SECONDARY_VERTICAL_SYNC,
              FIELD_VALUE(0, SECONDARY_VERTICAL_SYNC, HEIGHT, pModeParam->vertical_sync_height)
            | FIELD_VALUE(0, SECONDARY_VERTICAL_SYNC, START, pModeParam->vertical_sync_start - 1));

        /* Set control register value */
        ulTmpValue =        
            (pModeParam->vertical_sync_polarity == POS
            ? FIELD_SET(0, SECONDARY_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_HIGH)
            : FIELD_SET(0, SECONDARY_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_LOW))
          | (pModeParam->horizontal_sync_polarity == POS
            ? FIELD_SET(0, SECONDARY_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_HIGH)
            : FIELD_SET(0, SECONDARY_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_LOW))
          | FIELD_SET(0, SECONDARY_DISPLAY_CTRL, SELECT, SECONDARY)
          | FIELD_SET(0, SECONDARY_DISPLAY_CTRL, TIMING, ENABLE)
          | FIELD_SET(0, SECONDARY_DISPLAY_CTRL, PLANE, ENABLE) 
          | (ulBpp == 8
            ? FIELD_SET(0, SECONDARY_DISPLAY_CTRL, FORMAT, 8)
            : (ulBpp == 16
            ? FIELD_SET(0, SECONDARY_DISPLAY_CTRL, FORMAT, 16)
            : FIELD_SET(0, SECONDARY_DISPLAY_CTRL, FORMAT, 32)));

		chipType = ddk750_getChipType();

		if (chipType == SM750 || chipType == SM718)
        {
            /* TODO: Check if the auto expansion bit can be cleared here */
            ulReg = peekRegisterDWord(SECONDARY_DISPLAY_CTRL)
              & FIELD_CLEAR(SECONDARY_DISPLAY_CTRL, VSYNC_PHASE)
              & FIELD_CLEAR(SECONDARY_DISPLAY_CTRL, HSYNC_PHASE)
              & FIELD_CLEAR(SECONDARY_DISPLAY_CTRL, SELECT)
              & FIELD_CLEAR(SECONDARY_DISPLAY_CTRL, TIMING)
              & FIELD_CLEAR(SECONDARY_DISPLAY_CTRL, PLANE)
              & FIELD_CLEAR(SECONDARY_DISPLAY_CTRL, FORMAT)
              & FIELD_CLEAR(SECONDARY_DISPLAY_CTRL, LOCK_TIMING)
              & FIELD_CLEAR(SECONDARY_DISPLAY_CTRL, EXPANSION);

            pokeRegisterDWord(SECONDARY_DISPLAY_CTRL, ulTmpValue | ulReg);
        }
		

        /* Palette RAM. */
        palette_ram = SECONDARY_PALETTE_RAM;
        
        /* Save the current mode param */
        gSecondaryCurrentModeParam[getCurrentDevice()] = *pModeParam;
    }
    else 
    {
        /* Primary display control clock: PRIMARY_PLL */
        pokeRegisterDWord(PRIMARY_PLL_CTRL, formatPllReg(pPLL));

        /* Program primary PLL, if applicable */
        if (pPLL->clockType == PRIMARY_PLL)
        {
            pokeRegisterDWord(PRIMARY_PLL_CTRL, formatPllReg(pPLL));

            /* Program to Non-VGA mode when using primary PLL */
            pokeRegisterDWord(VGA_CONFIGURATION, 
                FIELD_SET(peekRegisterDWord(VGA_CONFIGURATION), VGA_CONFIGURATION, PLL, PRIMARY));
        }
        
        /* Frame buffer base for this mode */
		//setDisplayBaseAddress(PRIMARY_CTRL, ulBaseAddress);// move by iena
		//printk("func[%s], primary reg, ulPitch=[%d]\n", __func__, ulPitch);
        /* Pitch value (Sometime, hardware people calls it Offset) */
        //pokeRegisterDWord(PRIMARY_FB_WIDTH,
          //    FIELD_VALUE(0, PRIMARY_FB_WIDTH, WIDTH, ulPitch)
            //| FIELD_VALUE(0, PRIMARY_FB_WIDTH, OFFSET, ulPitch));

        pokeRegisterDWord(PRIMARY_WINDOW_WIDTH,
              FIELD_VALUE(0, PRIMARY_WINDOW_WIDTH, WIDTH, pModeParam->horizontal_display_end - 1)
            | FIELD_VALUE(0, PRIMARY_WINDOW_WIDTH, X, 0));

        pokeRegisterDWord(PRIMARY_WINDOW_HEIGHT,
              FIELD_VALUE(0, PRIMARY_WINDOW_HEIGHT, HEIGHT, pModeParam->vertical_display_end - 1)
            | FIELD_VALUE(0, PRIMARY_WINDOW_HEIGHT, Y, 0));

        pokeRegisterDWord(PRIMARY_PLANE_TL,
              FIELD_VALUE(0, PRIMARY_PLANE_TL, TOP, 0)
            | FIELD_VALUE(0, PRIMARY_PLANE_TL, LEFT, 0));

        pokeRegisterDWord(PRIMARY_PLANE_BR, 
              FIELD_VALUE(0, PRIMARY_PLANE_BR, BOTTOM, pModeParam->vertical_display_end - 1)
            | FIELD_VALUE(0, PRIMARY_PLANE_BR, RIGHT, pModeParam->horizontal_display_end - 1));

        pokeRegisterDWord(PRIMARY_HORIZONTAL_TOTAL,
              FIELD_VALUE(0, PRIMARY_HORIZONTAL_TOTAL, TOTAL, pModeParam->horizontal_total - 1)
            | FIELD_VALUE(0, PRIMARY_HORIZONTAL_TOTAL, DISPLAY_END, pModeParam->horizontal_display_end - 1));

        pokeRegisterDWord(PRIMARY_HORIZONTAL_SYNC,
              FIELD_VALUE(0, PRIMARY_HORIZONTAL_SYNC, WIDTH, pModeParam->horizontal_sync_width)
            | FIELD_VALUE(0, PRIMARY_HORIZONTAL_SYNC, START, pModeParam->horizontal_sync_start - 1));

        pokeRegisterDWord(PRIMARY_VERTICAL_TOTAL,
              FIELD_VALUE(0, PRIMARY_VERTICAL_TOTAL, TOTAL, pModeParam->vertical_total - 1)
            | FIELD_VALUE(0, PRIMARY_VERTICAL_TOTAL, DISPLAY_END, pModeParam->vertical_display_end - 1));

        pokeRegisterDWord(PRIMARY_VERTICAL_SYNC,
              FIELD_VALUE(0, PRIMARY_VERTICAL_SYNC, HEIGHT, pModeParam->vertical_sync_height)
            | FIELD_VALUE(0, PRIMARY_VERTICAL_SYNC, START, pModeParam->vertical_sync_start - 1));

        /* Set control register value */
        ulTmpValue =
            (pModeParam->clock_phase_polarity == POS
            ? FIELD_SET(0, PRIMARY_DISPLAY_CTRL, CLOCK_PHASE, ACTIVE_HIGH)
            : FIELD_SET(0, PRIMARY_DISPLAY_CTRL, CLOCK_PHASE, ACTIVE_LOW))
          | (pModeParam->vertical_sync_polarity == POS
            ? FIELD_SET(0, PRIMARY_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_HIGH)
            : FIELD_SET(0, PRIMARY_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_LOW))
          | (pModeParam->horizontal_sync_polarity == POS
            ? FIELD_SET(0, PRIMARY_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_HIGH)
            : FIELD_SET(0, PRIMARY_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_LOW))
          | FIELD_SET(0, PRIMARY_DISPLAY_CTRL, TIMING, ENABLE)
          | FIELD_SET(0, PRIMARY_DISPLAY_CTRL, PLANE, ENABLE)
          | (ulBpp == 8
            ? FIELD_SET(0, PRIMARY_DISPLAY_CTRL, FORMAT, 8)
            : (ulBpp == 16
            ? FIELD_SET(0, PRIMARY_DISPLAY_CTRL, FORMAT, 16)
            : FIELD_SET(0, PRIMARY_DISPLAY_CTRL, FORMAT, 32)));

        /* Added some masks to mask out the reserved bits. 
         * Sometimes, the reserved bits are set/reset randomly when 
         * writing to the PRIMARY_DISPLAY_CTRL, therefore, the register
         * reserved bits are needed to be masked out.
         */
        ulReservedBits = FIELD_SET(0, PRIMARY_DISPLAY_CTRL, RESERVED_1_MASK, ENABLE) |
                         FIELD_SET(0, PRIMARY_DISPLAY_CTRL, RESERVED_2_MASK, ENABLE) |
                         FIELD_SET(0, PRIMARY_DISPLAY_CTRL, RESERVED_3_MASK, ENABLE);

        ulReg = (peekRegisterDWord(PRIMARY_DISPLAY_CTRL) & ~ulReservedBits)
              & FIELD_CLEAR(PRIMARY_DISPLAY_CTRL, CLOCK_PHASE)
              & FIELD_CLEAR(PRIMARY_DISPLAY_CTRL, VSYNC_PHASE)
              & FIELD_CLEAR(PRIMARY_DISPLAY_CTRL, HSYNC_PHASE)
              & FIELD_CLEAR(PRIMARY_DISPLAY_CTRL, TIMING)
              & FIELD_CLEAR(PRIMARY_DISPLAY_CTRL, VERTICAL_PAN)
              & FIELD_CLEAR(PRIMARY_DISPLAY_CTRL, HORIZONTAL_PAN)
              & FIELD_CLEAR(PRIMARY_DISPLAY_CTRL, PLANE)
              & FIELD_CLEAR(PRIMARY_DISPLAY_CTRL, FORMAT);

        pokeRegisterDWord(PRIMARY_DISPLAY_CTRL, ulTmpValue | ulReg);

        /* 
         * PRIMARY_DISPLAY_CTRL register seems requiring few writes
         * before a value can be succesfully written in.
         * Added some masks to mask out the reserved bits.
         * Note: This problem happens by design. The hardware will wait for the
         *       next vertical sync to turn on/off the plane.
         */
        while((peekRegisterDWord(PRIMARY_DISPLAY_CTRL) & ~ulReservedBits) != (ulTmpValue|ulReg))
        {
            pokeRegisterDWord(PRIMARY_DISPLAY_CTRL, ulTmpValue | ulReg);
        }

        /* Palette RAM */
        palette_ram = PRIMARY_PALETTE_RAM;
        
        /* Save the current mode param */
        gPrimaryCurrentModeParam[getCurrentDevice()] = *pModeParam;
    }

    /* In case of 8-bpp, fill palette */
    if (ulBpp==8)
    {
        /* Start with RGB = 0,0,0. */
        unsigned char red = 0, green = 0, blue = 0;
        unsigned long gray = 0;
        for (offset = 0; offset < 256 * 4; offset += 4)
        {
            /* Store current RGB value. */
            pokeRegisterDWord(palette_ram + offset, gray
                                ? RGB((gray + 50) / 100,
                                      (gray + 50) / 100,
                                      (gray + 50) / 100)
                                : RGB(red, green, blue));

            if (gray)
            {
                /* Walk through grays (40 in total). */
                gray += 654;
            }

            else
            {
                /* Walk through colors (6 per base color). */
                if (blue != 255)
                {
                    blue += 51;
                }
                else if (green != 255)
                {
                    blue = 0;
                    green += 51;
                }
                else if (red != 255)
                {
                    green = blue = 0;
                    red += 51;
                }
                else
                {
                    gray = 1;
                }
            }
        }
    }

    /* For 16- and 32-bpp,  fill palette with gamma values. */
    else
    {
        /* Start with RGB = 0,0,0. */
        ulTmpValue = 0x000000;
        for (offset = 0; offset < 256 * 4; offset += 4)
        {
            pokeRegisterDWord(palette_ram + offset, ulTmpValue);

            /* Advance RGB by 1,1,1. */
            ulTmpValue += 0x010101;
        }
    }
}

/* 
 * This function gets the available clock type
 *
 */
clock_type_t getClockType(disp_control_t dispCtrl)
{
    clock_type_t clockType;

    switch (dispCtrl)
    {
        case PRIMARY_CTRL:
            clockType = PRIMARY_PLL;
            break;
        default:
        case SECONDARY_CTRL:
            clockType = SECONDARY_PLL;
            break;
    }
    return clockType;
}

/*
 * Input:
 *     1) pLogicalMode contains information such as x, y resolution and bpp.
 *     2) A user defined parameter table for the mode.
 *
 * This function calculate and programs the hardware to set up the
 * requested mode.
 *
 * This function allows the use of user defined parameter table if
 * predefined Vesa parameter table (gDefaultModeParamTable) does not fit.
 *
 * Return: 0 (or NO_ERROR) if mode can be set successfully.
 *         -1 if any set mode error.
 */
long setCustomMode(
	logicalMode_t *pLogicalMode, 
	mode_parameter_t *pUserModeParam
)
{
    mode_parameter_t pModeParam; /* physical parameters for the mode */
    pll_value_t pll;
    unsigned long ulActualPixelClk, ulTemp, ulAddress;

    /*
     * Minimum check on mode base address.
     * At least it shouldn't be bigger than the size of frame buffer.
     */
    if (ddk750_getFrameBufSize() <= pLogicalMode->baseAddress)
    	{printk("in func [%s], line[%d], return error\n", __func__, __LINE__);
        return -1;
    	}

    /*
     * Set up PLL, a structure to hold the value to be set in clocks.
     */
    pll.inputFreq = DEFAULT_INPUT_CLOCK; /* Defined in CLOCK.H */

    /* Get the Clock Type */
    pll.clockType = getClockType(pLogicalMode->dispCtrl);

    /* 
     * Call calcPllValue() to fill up the other fields for PLL structure.
     * Sometime, the chip cannot set up the exact clock required by User.
     * Return value from calcPllValue() gives the actual possible pixel clock.
     */
    ulActualPixelClk = calcPllValue(pUserModeParam->pixel_clock, &pll);
    //DDKDEBUGPRINT((DISPLAY_LEVEL, "Actual Pixel Clock: %d\n", ulActualPixelClk));

    /* 
     * Adjust Vesa mode parameter to feasible mode parameter for SMI hardware.
     */
    if (adjustModeParam(pUserModeParam, &pModeParam, ulActualPixelClk) != 0 )
    {
    //printk("in func [%s], line[%d], return error\n", __func__, __LINE__);
        return -1;
    }

    /* If calling function don't have a preferred pitch value, 
       work out a 16 byte aligned pitch value.
    */	
    //printk("func[%s], ulPitch=[%d]\n", __func__, pLogicalMode->pitch);
    if (pLogicalMode->pitch <= 0)
    {
        /* 
         * Pitch value calculation in Bytes.
         * Usually, it is (screen width) * (byte per pixel).
         * However, there are cases that screen width is not 16 pixel aligned, which is
         * a requirement for some OS and the hardware itself.
         * For standard 4:3 resolutions: 320, 640, 800, 1024 and 1280, they are all
         * 16 pixel aligned and pitch is simply (screen width) * (byte per pixel).
         *   
         * However, 1366 resolution, for example, has to be adjusted for 16 pixel aligned.
         */

        ulTemp = (pLogicalMode->x + 15) & ~15; /* This calculation has no effect on 640, 800, 1024 and 1280. */
        pLogicalMode->pitch = ulTemp * (pLogicalMode->bpp / 8);
    }
	//printk("func[%s], ulPitch=[%d]\n", __func__, pLogicalMode->pitch);

    /* Program the hardware to set up the mode. */
    programModeRegisters( 
        &pModeParam,
        pLogicalMode->bpp, 
        pLogicalMode->baseAddress, 
        pLogicalMode->pitch, 
        &pll);
        
    return (0);
}

/*
 * Input:
 *     1) pLogicalMode contains information such as x, y resolution, bpp, xLCD, and yLCD.
 *     2) A user defined parameter table for the mode.
 *
 * This function calculate and programs the hardware to set up the
 * requested mode and scale the mode if necessary.
 *
 * This function allows the use of user defined parameter table if
 * predefined parameter table does not fit.
 *
 * Return: 0 (or NO_ERROR) if mode can be set successfully.
 *         -1 if any set mode error.
 */
long setCustomModeEx(
	logicalMode_t *pLogicalMode, 
	mode_parameter_t *pUserModeParam
)
{
    long returnValue = 0;

#if 0   /* userData field is used on DDK version 1.1 and above. Thereofre, this checking is
           not needed anymore. */
    /* For the current DDK version, the userData needs to be set to 0. If not, then return error. */
    if (pLogicalMode->userData != (void *)0)
        return (-1);
#endif

    /* Return error when the mode is bigger than the panel size. Might be temporary solution
       depending whether we will support panning or not. */
    if (((pLogicalMode->xLCD != 0) && 
         (pLogicalMode->yLCD != 0)) &&
        ((pLogicalMode->xLCD < pLogicalMode->x) ||
         (pLogicalMode->yLCD < pLogicalMode->y)))
    	{
    	//printk("in func [%s], line[%d], return error\n", __func__, __LINE__);
        return (-1);
    	}
    
    /* Return error when the panel size exceed the maximum mode that we can support. */
    if ((pLogicalMode->xLCD > MAX_PANEL_SIZE_WIDTH) ||
        (pLogicalMode->yLCD > MAX_PANEL_SIZE_HEIGHT))
    	{
    	//printk("in func [%s], line[%d], return error\n", __func__, __LINE__);
        return (-1);    /* Unsupported panel size. */
    	}
    
    /* Set the mode first */
    returnValue = setCustomMode(pLogicalMode, pUserModeParam);

    
    return (returnValue);
}

/*
 * Input pLogicalMode contains information such as x, y resolution, bpp, 
 * xLCD and yLCD. The main difference between setMode and setModeEx are
 * these two parameters (xLCD and yLCD). Use this setModeEx API to set
 * expansion while setMode API for regular setmode without any expansion.
 * Refer to MODE.h for the details.
 *
 * This function calculate and programs the hardware to set up the
 * requested mode.
 *
 * Return: 0 (or NO_ERROR) if mode can be set successfully.
 *         -1 if any set mode error.
 */
long setModeEx(
	logicalMode_t *pLogicalMode
)
{
    mode_parameter_t *pModeParam;       /* physical parameters for the mode */
    unsigned short index = 0;
    unsigned long modeWidth, modeHeight;
    userData_t *pUserData;
    
    /* Conditions to set the mode when scaling is needed (xLCD and yLCD is not zeroes)
     *      1. PRIMARY_CTRL
     *          a. Set the primary display control timing to the actual display mode.
     *          b. Set the secondary display control timing to the mode that equals to
     *             the panel size.
     *      2. SECONDARY_CTRL
     *          a. Set the secondary display control timing to the mode that equals to
     *             the panel size.
     */
    if ((pLogicalMode->dispCtrl == SECONDARY_CTRL) &&
        (pLogicalMode->xLCD != 0) && (pLogicalMode->yLCD != 0))
    {
        modeWidth = pLogicalMode->xLCD;
        modeHeight = pLogicalMode->yLCD;
    }
    else
    {
        modeWidth = pLogicalMode->x;
        modeHeight = pLogicalMode->y;
    }
    
    /*
     * Check the validity of the userData pointer and translate the information as necessary
     */
    pUserData = (userData_t *)pLogicalMode->userData;
    if ((pUserData != (userData_t *)0) &&
        (pUserData->signature == getUserDataSignature()) &&
        (pUserData->size == sizeof(userData_t)))
    {
        /* Interpret the userData information */
        if (pUserData->paramList.size == sizeof(userDataParam_t))
        {
            if (pUserData->paramList.modeInfoID == MODE_INFO_INDEX)
                index = pUserData->paramList.paramInfo.index;
        }
    }
    
    /* 
     * Check if we already have physical timing parameter for this mode.
     */
  //   printk("fine ctrl=[%d], width[%d](x=[%d]), height[%d], hz[%d], index[%d], disp=[%d]\n", pLogicalMode->dispCtrl, modeWidth,pLogicalMode->x,modeHeight, pLogicalMode->hz, index,pLogicalMode->dispCtrl);
    pModeParam = findModeParam(pLogicalMode->dispCtrl, modeWidth, modeHeight, pLogicalMode->hz, index);
    if (pModeParam == (mode_parameter_t *)0)
    	{
    	//printk("in func [%s], line[%d], return error\n", __func__, __LINE__);
        return -1;
    	}

    return(setCustomModeEx(pLogicalMode, pModeParam));
}

/*
 * Input pLogicalMode contains information such as x, y resolution and bpp.
 * Refer to MODE.h for the details.
 *
 * This function calculate and programs the hardware to set up the
 * requested mode.
 *
 * Return: 0 (or NO_ERROR) if mode can be set successfully.
 *         -1 if any set mode error.
 */
long setMode(
	logicalMode_t *pLogicalMode
)
{
    /* Initialize the panel size to 0 */
    pLogicalMode->xLCD = 0;
    pLogicalMode->yLCD = 0;
    pLogicalMode->userData = (void *)0;

    /* Call the setModeEx to set the mode. */
    return setModeEx(pLogicalMode);
}

/*
 *  setInterpolation
 *      This function enables/disables the horizontal and vertical interpolation
 *      for the secondary display control. Primary display control does not have
 *      this capability.
 *
 *  Input:
 *      enableHorzInterpolation - Flag to enable/disable Horizontal interpolation
 *      enableVertInterpolation - Flag to enable/disable Vertical interpolation
 */
void setInterpolation(
    unsigned long enableHorzInterpolation,
    unsigned long enableVertInterpolation
)
{
    unsigned long value;

    value = peekRegisterDWord(SECONDARY_DISPLAY_CTRL);
    
    if (enableHorzInterpolation)
        value = FIELD_SET(value, SECONDARY_DISPLAY_CTRL, HORIZONTAL_MODE, INTERPOLATE);
    else
        value = FIELD_SET(value, SECONDARY_DISPLAY_CTRL, HORIZONTAL_MODE, REPLICATE);
        
    if (enableVertInterpolation)
        value = FIELD_SET(value, SECONDARY_DISPLAY_CTRL, VERTICAL_MODE, INTERPOLATE);
    else
        value = FIELD_SET(value, SECONDARY_DISPLAY_CTRL, VERTICAL_MODE, REPLICATE);
        
    pokeRegisterDWord(SECONDARY_DISPLAY_CTRL, value);
}
