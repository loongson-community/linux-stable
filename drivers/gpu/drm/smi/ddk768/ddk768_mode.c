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

#include "linux/string.h"

#include "ddk768_reg.h"

#include "ddk768_chip.h"
#include "ddk768_clock.h"
#include "ddk768_power.h"
#include "ddk768_mode.h"

#include "ddk768_help.h"



/* The valid signature of the user data pointer  for the setmode function. 
   The following definition is ASCII representation of the word 'USER'
 */
#define MODE_USER_DATA_SIGNATURE            0x55534552

/*
 *  Default Timing parameter for some popular modes.
 *  Note that the most timings in this table is made according to standard VESA 
 *  parameters for the popular modes.
 */
static mode_parameter_t gDefaultModeParamTable[] =
{
/* 640 x 480  [4:3] */
/* The first 2 commented lines below are taken from SM502, the rest timing are
   taken from the VESA Monitor Timing Standard */
/* { 840, 640, 680, 64, NEG, 500, 480, 481, 3, NEG, 31500000, 37500, 75, NEG}, */
/* { 832, 640, 700, 56, NEG, 509, 480, 481, 3, NEG, 36000000, 43269, 85, NEG}, */
 { 800, 640, 656, 96, NEG, 525, 480, 490, 2, NEG, 25175000, 31469, 60, NEG},
 { 840, 640, 656, 64, NEG, 500, 480, 481, 3, NEG, 31500000, 37500, 75, NEG},
 { 832, 640, 696, 56, NEG, 509, 480, 481, 3, NEG, 36000000, 43269, 85, NEG},

/* 720 x 480  [3:2] */
 { 889, 720, 738,108, POS, 525, 480, 490, 2, NEG, 28000000, 31496, 60, NEG},

/* 720 x 540  [4:3] -- Not a popular mode */
 { 886, 720, 740, 96, POS, 576, 540, 545, 2, POS, 30600000, 34537, 60, NEG},

/*720 x 576 [5:4] -- Not a popular mode*/
 { 912, 720, 744, 96, POS, 597, 576, 580, 1, NEG, 32670000, 35820, 60, POS}, 

/* 800 x 480  [5:3] -- Not a popular mode */
 { 973, 800, 822, 56, POS, 524, 480, 490, 2, NEG, 30600000, 31449, 60, NEG},

/* 800 x 600  [4:3] */
/* The first 2 commented lines below are taken from SM502, the rest timing are
   taken from the VESA Monitor Timing Standard */
/* {1062, 800, 840,128, POS, 628, 600, 601, 4, POS, 40000000, 37665, 60, NEG}, */
/* {1054, 800, 842, 64, POS, 625, 600, 601, 3, POS, 56000000, 53131, 85, NEG}, */
 {1056, 800, 840,128, POS, 628, 600, 601, 4, POS, 40000000, 37879, 60, NEG},
 {1056, 800, 816, 80, POS, 625, 600, 601, 3, POS, 49500000, 46875, 75, NEG},
 {1048, 800, 832, 64, POS, 631, 600, 601, 3, POS, 56250000, 53674, 85, NEG},

/* 960 x 720  [4:3] -- Not a popular mode */
 {1245, 960, 992, 64, POS, 750, 720, 721, 3, POS, 56000000, 44980, 60, NEG},
      
/* 1024 x 600  [16:9] 1.7 */
 {1313,1024,1064,104, POS, 622, 600, 601, 3, POS, 49000000, 37319, 60, NEG},
     
/* 1024 x 768  [4:3] */
/* The first 2 commented lines below are taken from SM502, the rest timing are
   taken from the VESA Monitor Timing Standard */
/* {1340,1024,1060,136, NEG, 809, 768, 772, 6, NEG, 65000000, 48507, 60, NEG}, */
/* {1337,1024,1072, 96, NEG, 808, 768, 780, 3, NEG, 81000000, 60583, 75, NEG}, */
 {1344,1024,1048,136, NEG, 806, 768, 771, 6, NEG, 65000000, 48363, 60, NEG},
 {1312,1024,1040, 96, POS, 800, 768, 769, 3, POS, 78750000, 60023, 75, NEG},
 {1376,1024,1072, 96, POS, 808, 768, 769, 3, POS, 94500000, 68677, 85, NEG},
  
/* 1152 x 864  [4:3] -- Widescreen eXtended Graphics Array */
/* {1475,1152,1208, 96, NEG, 888, 864, 866, 3, NEG, 78600000, 53288, 60, NEG},*/
 {1475,1152,1208, 96, POS, 888, 864, 866, 3, POS, 78600000, 53288, 60, NEG},
 {1600,1152,1216,128, POS, 900, 864, 865, 3, POS,108000000, 67500, 75, NEG},
 
/* 1280 x 720  [16:9] -- HDTV (WXGA) */
 {1664,1280,1336,136, POS, 746, 720, 721, 3, POS, 74481000, 44760, 60, NEG},

/* 1280 x 768  [5:3] -- Not a popular mode */
 {1678,1280,1350,136, POS, 795, 768, 769, 3, POS, 80000000, 47676, 60, NEG},

/* 1280 x 800  [8:5] -- Not a popular mode */
 {1650,1280,1344,136, NEG, 824, 800, 800, 3, NEG, 81600000, 49455, 60, NEG},

/* 1280 x 960  [4:3] */
/* The first commented line below are taken from SM502, the rest timing are
   taken from the VESA Monitor Timing Standard */
/* {1618,1280,1330, 96, NEG, 977, 960, 960, 2, NEG, 94500000, 59259, 60, NEG},*/
 {1800,1280,1376,112, POS,1000, 960, 961, 3, POS,108000000, 60000, 60, NEG},
 {1728,1280,1344,160, POS,1011, 960, 961, 3, POS,148500000, 85938, 85, NEG},
    
/* 1280 x 1024 [5:4] */
#if 1
 /* GTF with C = 40, M = 600, J = 20, K = 128 */
 {1712,1280,1360,136, NEG,1060,1024,1025, 3, POS,108883200, 63600, 60, NEG},
 {1728,1280,1368,136, NEG,1069,1024,1025, 3, POS,138542400, 80175, 75, NEG},
 {1744,1280,1376,136, NEG,1075,1024,1025, 3, POS,159358000, 91375, 85, NEG},
#else
 /* VESA Standard */
 {1688,1280,1328,112, POS,1066,1024,1025, 3, POS,108000000, 63981, 60, NEG},
 {1688,1280,1296,144, POS,1066,1024,1025, 3, POS,135000000, 79976, 75, NEG},
 {1728,1280,1344,160, POS,1072,1024,1025, 3, POS,157500000, 91146, 85, NEG},
#endif

/* 1360 x 768 [16:9] */
#if 1
 /* GTF with C = 40, M = 600, J = 20, K = 128 */
 //{1776,1360,1432,136, NEG, 795, 768, 769, 3, POS, 84715200, 47700, 60, NEG},
 
 /* GTF with C = 30, M = 600, J = 20, K = 128 */
 {1664,1360,1384,128, NEG, 795, 768, 769, 3, POS, 79372800, 47700, 60, NEG},
#else
 /* Previous Calculation */
 {1776,1360,1424,144, POS, 795, 768, 769, 3, POS, 84715000, 47700, 60, NEG},
#endif
 
/* 1366 x 768  [16:9] */
 /* Previous Calculation  */
 {1722,1366,1424,112, NEG, 784, 768, 769, 3, NEG, 81000000, 47038, 60, NEG},
 
/* 1400 x 1050 [4:3] -- Hitachi TX38D95VC1CAH -- It is not verified yet, therefore
   temporarily disabled. */
 //{1688,1400,1448,112, NEG,1068,1050,1051, 3, NEG,108000000, 64000, 60, NEG},
 //{1688,1400,1464,112, NEG,1068,1050,1051, 3, NEG,108167040, 64080, 60, NEG},
 
 /* Taken from the www.tinyvga.com */
 {1880,1400,1488,152, NEG,1087,1050,1051, 3, POS,122610000, 65218, 60, NEG},
 
/* 1440 x 900  [8:5] -- Widescreen Super eXtended Graphics Array (WSXGA) */
 {1904,1440,1520,152, NEG, 932, 900, 901, 3, POS,106470000, 55919, 60, NEG},

/* 1440 x 960 [3:2] -- Not a popular mode */
 {1920,1440,1528,152, POS, 994, 960, 961, 3, POS,114509000, 59640, 60, NEG},

/* 1600 x 900 */
{2128,1600,1664,192, POS,932,900,901, 3, POS,119000000, 56000, 60, NEG},


/* 1600 x 1200 [4:3]. -- Ultra eXtended Graphics Array */
 /* VESA */
 {2160,1600,1664,192, POS,1250,1200,1201, 3, POS,162000000, 75000, 60, POS},
 {2160,1600,1664,192, POS,1250,1200,1201, 3, POS,202500000, 93750, 75, POS},
 {2160,1600,1664,192, POS,1250,1200,1201, 3, POS,229500000,106250, 85, POS},

/* 
 * The timing below is taken from the www.tinyvga.com/vga-timing.
 * With the exception of 1920x1080.
 */
 
/* 1680 x 1050 [8:5]. -- Widescreen Super eXtended Graphics Array Plus (WSXGA+) */ 
/* The first commented timing might be used for DVI LCD Monitor timing. */
/* {1840,1680,1728, 32, NEG,1080,1050,1053, 6, POS,119232000, 64800, 60, NEG}, */
 /* GTF with C = 30, M = 600, J = 20, K = 128 */
 {2256,1680,1784,184, NEG,1087,1050,1051, 3, POS,147140000, 65222, 60, POS},
/*
 {2272,1680,1792,184, NEG,1093,1050,1051, 3, POS,173831000, 76510, 70, NEG},
 {2288,1680,1800,184, NEG,1096,1050,1051, 3, POS,188074000, 82200, 75, NEG},
*/
 
/* 1792 x 1344 [4:3]. -- Not a popular mode */ 
 {2448,1792,1920,200, NEG,1394,1344,1345, 3, POS,204800000, 83660, 60, NEG},
 {2456,1792,1888,216, NEG,1417,1344,1345, 3, POS,261000000,106270, 75, NEG},
 
/* 1856 x 1392 [4:3]. -- Not a popular mode 
   The 1856 x 1392 @ 75Hz has not been tested due to high Horizontal Frequency
   where not all monitor can support it (including the developer monitor)
 */
 {2528,1856,1952,224, NEG,1439,1392,1393, 3, POS,218300000, 86353, 60, NEG},
/* {2560,1856,1984,224, NEG,1500,1392,1393, 3, POS,288000000,112500, 75, NEG},*/

/* 1920 x 1080 [16:9]. This is a make-up value, need to be proven. 
   The Pixel clock is calculated based on the maximum resolution of
   "Single Link" DVI, which support a maximum 165MHz pixel clock.
   The second values are taken from:
   http://www.tek.com/Measurement/App_Notes/25_14700/eng/25W_14700_3.pdf
 */
/* {2560,1920,2048,208, NEG,1125,1080,1081, 3, POS,172800000, 67500, 60, NEG}, */
 {2142,1920,2008, 44, POS,1100,1080,1081, 3, POS,141400000, 67500, 60, NEG},

/* 1920 x 1200 [8:5]. -- Widescreen Ultra eXtended Graphics Array (WUXGA) */
 {2592,1920,2048,208, NEG,1242,1200,1201, 3, POS,193160000, 74522, 60, NEG},

/* 1920 x 1440 [4:3]. */
/* In the databook, it mentioned only support up to 1920x1440 @ 60Hz. 
   The timing for 75 Hz is provided here if necessary to do testing. - Some underflow
   has been noticed. */
 {2600,1920,2048,208, NEG,1500,1440,1441, 3, POS,234000000, 90000, 60, NEG},
/* {2640,1920,2064,224, NEG,1500,1440,1441, 3, POS,297000000,112500, 75, NEG}, */

/* 2560x1440 [16:9]. */
/* 2K mode */
 {2720,2560,2608,32, POS,1481,1440,1443, 5, NEG,241500000, 88000, 60, NEG},

/* 3840 x 2160 (UHD) */
 {4400,3840,4016,88, POS,2250,2160,2168, 10, POS,297000000, 67500, 30, NEG},


/* End of table. */
 { 0, 0, 0, 0, NEG, 0, 0, 0, 0, NEG, 0, 0, 0, NEG},
};

static mode_parameter_t gChannel0ModeParamTable[MAX_MODE_TABLE_ENTRIES] =
{
    /* End of table */
     { 0, 0, 0, 0, NEG, 0, 0, 0, 0, NEG, 0, 0, 0, NEG},
};

static mode_parameter_t gChannel1ModeParamTable[MAX_MODE_TABLE_ENTRIES] =
{
    /* End of table */
     { 0, 0, 0, 0, NEG, 0, 0, 0, 0, NEG, 0, 0, 0, NEG},
};

/* Static variable to store the mode information. */
static mode_parameter_t gChannel0CurrentModeParam;
static mode_parameter_t gChannel1CurrentModeParam;

/*
 *  ddk768_getUserDataSignature
 *      This function gets the user data mode signature
 *
 *  Output:
 *      The signature to be filled in the user_data_mode_t structure to be considered
 *      a valid structure.
 */
unsigned long ddk768_getUserDataSignature()
{
    return MODE_USER_DATA_SIGNATURE;
}

/*
 *  compareModeParam
 *      This function compares two mode parameters
 *
 *  Input:
 *      pModeParam1 - Pointer to the first mode parameter to be compared
 *      pModeParam2 - Pointer to the second mode parameter to be compared
 *
 *  Output:
 *      0   - Identical mode
 *     -1   - Mode is not identical
 */
long compareModeParam(
    mode_parameter_t *pModeParam1,
    mode_parameter_t *pModeParam2
)
{
    if ((pModeParam1 != (mode_parameter_t *)0) &&
        (pModeParam2 != (mode_parameter_t *)0))
    {
        if (memcmp((void *)pModeParam1, (void *)pModeParam2, sizeof(mode_parameter_t)) == 0)
            return 0;
    }
        
    return (-1);
}

/*
 *  getDuplicateModeIndex
 *      This function retrieves the index of dupicate modes, but having different timing.
 *
 *  Input:
 *      dispCtrl    - Display Control where the mode table belongs to.
 *      pModeParam  - The mode parameters which index to be checked.
 *
 *  Output:
 *      The index of the given parameters among the duplicate modes.
 *          0 means that the mode param is the first mode encountered in the table
 *          1 means that the mode param is the second mode encountered in the table
 *          etc...
 */
unsigned short getDuplicateModeIndex(
    disp_control_t dispCtrl,
    mode_parameter_t *pModeParam
)
{
    unsigned short index, modeIndex;
    mode_parameter_t *pModeTable;
    
    /* Get the mode table */
    pModeTable = ddk768_getStockModeParamTableEx(dispCtrl);
    
    /* Search the current mode */
    modeIndex = 0;
    index = 0;
    while (pModeTable[modeIndex].pixel_clock != 0)
    {
        if ((pModeTable[modeIndex].horizontal_display_end == pModeParam->horizontal_display_end) &&
            (pModeTable[modeIndex].vertical_display_end == pModeParam->vertical_display_end) &&
            (pModeTable[modeIndex].vertical_frequency == pModeParam->vertical_frequency))
        {
            /* Check if this is the same/identical mode param. */
            if (compareModeParam(&pModeTable[modeIndex], pModeParam) == 0)
                break;
                
            /* Increment the index */
            index++;
        }
        modeIndex++;
    }
    
    return index;
}

/*
 *  ddk768_findModeParamFromTable
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
mode_parameter_t *ddk768_findModeParamFromTable(
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
            if (tempIndex < index)
                tempIndex++;
            else
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
mode_parameter_t *ddk768_findModeParam(
    disp_control_t dispCtrl,
    unsigned long width, 
    unsigned long height, 
    unsigned long refresh_rate,
    unsigned short index
)
{
    return ddk768_findModeParamFromTable(width, height, refresh_rate, index, ddk768_getStockModeParamTableEx(dispCtrl));
}

/*
 *  Use the
 *  Locate timing parameter for the requested mode from the default mode table.
 *  Success: return a pointer to the mode_parameter_t entry.
 *  Fail: a NULL pointer.
 */
mode_parameter_t *ddk768_findVesaModeParam(
    unsigned long width, 
    unsigned long height, 
    unsigned long refresh_rate
)
{
    return ddk768_findModeParamFromTable(width, height, refresh_rate, 0, gDefaultModeParamTable);
}

/*
 * (Obsolete) 
 * Return a point to the gDefaultModeParamTable.
 * Function in other files used this to get the mode table pointer.
 */
mode_parameter_t *ddk768_getStockModeParamTable()
{
    return(gDefaultModeParamTable);
}

/*
 * (Obsolete)
 * Return the size of the Stock Mode Param Table
 */
unsigned long ddk768_getStockModeParamTableSize()
{
    return (sizeof(gDefaultModeParamTable) / sizeof(mode_parameter_t) - 1);
}

/* 
 *  ddk768_getStockModeParamTableEx
 *      This function gets the mode parameters table associated to the
 *      display control (CHANNEL0_CTRL or SECONDAR_CTRL).
 *
 *  Input:
 *      dispCtrl    - Display Control of the mode table that is associated to.
 *
 *  Output:
 *      Pointer to the mode table
 */
mode_parameter_t *ddk768_getStockModeParamTableEx(
    disp_control_t dispCtrl
)
{
    mode_parameter_t *pModeTable;
    
    if (dispCtrl == CHANNEL0_CTRL)
        pModeTable = (mode_parameter_t *)gChannel0ModeParamTable;
    else
        pModeTable = (mode_parameter_t *)gChannel1ModeParamTable;
        
    /* Check if the table exist by checking the first entry. 
       If it doesn't, then use the default mode table. */

    if (pModeTable->pixel_clock == 0)
    {
        pModeTable = ddk768_getStockModeParamTable();
    }
        
    return (pModeTable);
}

/*
 *  ddk768_getStockModeParamTableSizeEx
 *      This function gets the size of the mode parameter table associated with
 *      specific display control
 *
 *  Input:
 *      dispCtrl    - Display control of the mode param table that is associated to.
 *
 *  Output:
 *      Size of the requeted mode param table.
 */
unsigned long ddk768_getStockModeParamTableSizeEx(
    disp_control_t dispCtrl
)
{
    unsigned long tableSize;
    mode_parameter_t *pModeTable;
    
    /* Get the mode table */
    pModeTable = ddk768_getStockModeParamTableEx(dispCtrl);
    
    /* Calculate the table size by finding the end of table entry indicated by all zeroes. */    
    tableSize = 0;
    while (pModeTable[tableSize].pixel_clock != 0)
        tableSize++;
        
    return tableSize;
}

/*
 *  getMaximumModeEntries
 *      This function gets the maximum entries that can be stored in the mode table.
 *
 *  Output:
 *      Total number of maximum entries
 */
unsigned long getMaximumModeEntries()
{
    return MAX_MODE_TABLE_ENTRIES;
}

/* 
 * This function returns the current mode.
 */
mode_parameter_t ddk768_getCurrentModeParam(
    disp_control_t dispCtrl
)
{
    if (dispCtrl == CHANNEL0_CTRL)
        return gChannel0CurrentModeParam;
    else
        return gChannel1CurrentModeParam;
}

/*
 *  addTiming
 *      This function adds the SM750 mode parameter timing to the specified mode table
 *
 *  Input:
 *      dispCtrl        - Display control where the mode will be associated to
 *      pNewModeList    - Pointer to a list table of SM750 mode parameter to be added 
 *                        to the current specified display control mode table.
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
long addTiming(
    disp_control_t dispCtrl,
    mode_parameter_t *pNewModeList,
    unsigned long totalList,
    unsigned char clearTable
)
{
    mode_parameter_t *pModeParamTable;
    unsigned char index;
    long returnValue = 0;
    
    /* Get the correct table */
    if (dispCtrl == CHANNEL0_CTRL)
        pModeParamTable = (mode_parameter_t *)gChannel0ModeParamTable;
    else
        pModeParamTable = (mode_parameter_t *)gChannel1ModeParamTable;
    
    if (clearTable == 0)
    {    
        /* Find the last index where the timing will be added to */
        index = 0;
        while(pModeParamTable[index].pixel_clock != 0)
            index++;
    }
    else
    {
        /* Clear and reset the mode table first */
        for (index = 0; index < MAX_MODE_TABLE_ENTRIES; index++)
            memset((void*)&pModeParamTable[index], 0, sizeof(mode_parameter_t));
            
        /* Reset index */
        index = 0;
    }
        
    /* Update the number of modes those can be added to the current table. */
    if (totalList > (unsigned long)(MAX_MODE_TABLE_ENTRIES - index))
        totalList = (unsigned long)(MAX_MODE_TABLE_ENTRIES - index);
    else
        returnValue = (-1);
    
    /* Check if totalList is 0, which means that the table is full. */        
    if (totalList == 0)
        returnValue = (-1);
        
    /* Add the list of modes provided by the caller */
    while (totalList--)
    {
        memcpy((void *)&pModeParamTable[index], (void *)&pNewModeList[index], sizeof(mode_parameter_t));
        index++;
    }
        
    return returnValue;
}

/*
 *	This function sets the display base address
 *
 *	Input:
 *		dispControl		- display control of which base address to be set.
 *		ulBaseAddress	- Base Address value to be set.
 */
void ddk768_setDisplayBaseAddress(
	disp_control_t dispControl,
	unsigned long ulBaseAddress
)
{
    unsigned long regFB;

    regFB = (dispControl == CHANNEL0_CTRL) ? FB_ADDRESS : (FB_ADDRESS+CHANNEL_OFFSET);

		/* Frame buffer base for this mode */
	pokeRegisterDWord(regFB,
          FIELD_SET(0, FB_ADDRESS, STATUS, PENDING)
        | FIELD_VALUE(0, FB_ADDRESS, ADDRESS, ulBaseAddress));
}

/*
 *    This function checks if change of "display base address" has effective.
 *    Change of DC base address will not effective until next VSync, SW sets pending bit to 1 during address change.
 *    HW resets pending bit when it starts to use the new address.
 *
 *    Input:
 *        dispControl        - display control of which display status to be retrieved.
 *
 *  Output:
 *      1   - Display is pending
 *      0   - Display is not pending
 */
long isDisplayBasePending(
    disp_control_t dispControl
)
{
    unsigned long regFB;

    regFB = (dispControl == CHANNEL0_CTRL) ? FB_ADDRESS : (FB_ADDRESS+CHANNEL_OFFSET);

    if (FIELD_GET(peekRegisterDWord(regFB), FB_ADDRESS, STATUS) == FB_ADDRESS_STATUS_PENDING)
        return 1;

    return (0);
}


/* 
 * Program the hardware for a specific video mode
 *
 * return:
 *         0 = success
 *        -1 = fail.
 */
long ddk768_programModeRegisters(
logicalMode_t *pLogicalMode, 
mode_parameter_t *pModeParam,   /* mode information about pixel clock, horizontal total, etc. */
pll_value_t *pPLL               /* Pre-calculated values for the PLL */
)
{
    unsigned long ulTmpValue;
    unsigned long paletteRam;
    unsigned long offset, pllReg;

#if 0 // print UHD register setting for debug.
    if (pLogicalMode->x == 3840)
        return(printModeRegisters(pLogicalMode, pModeParam, pPLL));
#endif

    /*  Make sure normal display channel is used, not VGA channel */
    pokeRegisterDWord(VGA_CONFIGURATION,
          FIELD_SET(0, VGA_CONFIGURATION, PLL, PANEL)
        | FIELD_SET(0, VGA_CONFIGURATION, MODE, GRAPHIC));

    offset = (pLogicalMode->dispCtrl==CHANNEL0_CTRL)? 0 : CHANNEL_OFFSET;
	pllReg = (pLogicalMode->dispCtrl==CHANNEL0_CTRL)? VCLK0_PLL : VCLK1_PLL;

    /* Program PLL */
    pokeRegisterDWord(pllReg, ddk768_formatPllReg(pPLL));
    
#if 0
    /* Frame buffer base */
    pokeRegisterDWord((FB_ADDRESS+offset),
          FIELD_SET(0, FB_ADDRESS, STATUS, PENDING)
        | FIELD_VALUE(0, FB_ADDRESS, ADDRESS, pLogicalMode->baseAddress));

#endif


    /* Pitch value (Hardware people calls it Offset) */
    pokeRegisterDWord((FB_WIDTH+offset),
          FIELD_VALUE(0, FB_WIDTH, WIDTH, pLogicalMode->pitch));

    pokeRegisterDWord((HORIZONTAL_TOTAL+offset),
          FIELD_VALUE(0, HORIZONTAL_TOTAL, TOTAL, pModeParam->horizontal_total - 1)
        | FIELD_VALUE(0, HORIZONTAL_TOTAL, DISPLAY_END, pModeParam->horizontal_display_end - 1));

    pokeRegisterDWord((HORIZONTAL_SYNC+offset),
          FIELD_VALUE(0, HORIZONTAL_SYNC, WIDTH, pModeParam->horizontal_sync_width)
        | FIELD_VALUE(0, HORIZONTAL_SYNC, START, pModeParam->horizontal_sync_start - 1));

    pokeRegisterDWord((VERTICAL_TOTAL+offset),
          FIELD_VALUE(0, VERTICAL_TOTAL, TOTAL, pModeParam->vertical_total - 1)
        | FIELD_VALUE(0, VERTICAL_TOTAL, DISPLAY_END, pModeParam->vertical_display_end - 1));

    pokeRegisterDWord((VERTICAL_SYNC+offset),
          FIELD_VALUE(0, VERTICAL_SYNC, HEIGHT, pModeParam->vertical_sync_height)
        | FIELD_VALUE(0, VERTICAL_SYNC, START, pModeParam->vertical_sync_start - 1));


    
    unsigned long hdmi_channel = FIELD_GET(peekRegisterDWord(DISPLAY_CTRL+offset),
                                   DISPLAY_CTRL,
                                   HDMI_SELECT);    

    /* Set control register value */
    ulTmpValue =       
        (pModeParam->vertical_sync_polarity == POS
        ? FIELD_SET(0, DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_HIGH)
        : FIELD_SET(0, DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_LOW))
      | (pModeParam->horizontal_sync_polarity == POS
        ? FIELD_SET(0, DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_HIGH)
        : FIELD_SET(0, DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_LOW))
      | (pModeParam->clock_phase_polarity== POS
        ? FIELD_SET(0, DISPLAY_CTRL, CLOCK_PHASE, ACTIVE_HIGH)
        : FIELD_SET(0, DISPLAY_CTRL, CLOCK_PHASE, ACTIVE_LOW))
      | FIELD_SET(0, DISPLAY_CTRL, DATA_PATH, EXTENDED)
      | FIELD_SET(0, DISPLAY_CTRL, DIRECTION, INPUT)
      | FIELD_SET(0, DISPLAY_CTRL, TIMING, ENABLE)
      | FIELD_SET(0, DISPLAY_CTRL, PLANE, ENABLE) 
      | (pLogicalMode->bpp == 8
        ? FIELD_SET(0, DISPLAY_CTRL, FORMAT, 8)
        : (pLogicalMode->bpp == 16
        ? FIELD_SET(0, DISPLAY_CTRL, FORMAT, 16)
        : FIELD_SET(0, DISPLAY_CTRL, FORMAT, 32)));

	
	if( hdmi_channel == DISPLAY_CTRL_HDMI_SELECT_CHANNEL0)
		 ulTmpValue= FIELD_SET(ulTmpValue,DISPLAY_CTRL, HDMI_SELECT, CHANNEL0);
	else
		 ulTmpValue= FIELD_SET(ulTmpValue,DISPLAY_CTRL, HDMI_SELECT, CHANNEL1);


    pokeRegisterDWord((DISPLAY_CTRL+offset), ulTmpValue);

    /* Palette RAM. */
    paletteRam = PALETTE_RAM + offset;
    
    /* Save the current mode param */
    if (pLogicalMode->dispCtrl == CHANNEL0_CTRL)
        gChannel0CurrentModeParam = *pModeParam;
    else
        gChannel1CurrentModeParam = *pModeParam;

    /* In case of 8-bpp, fill palette */
    if (pLogicalMode->bpp==8)
    {
        /* Start with RGB = 0,0,0. */
        unsigned char red = 0, green = 0, blue = 0;
        unsigned long gray = 0;
        for (offset = 0; offset < 256 * 4; offset += 4)
        {
            /* Store current RGB value. */
            pokeRegisterDWord(paletteRam + offset, gray
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
            pokeRegisterDWord(paletteRam + offset, ulTmpValue);

            /* Advance RGB by 1,1,1. */
            ulTmpValue += 0x010101;
        }
    }

    return 0;
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
long ddk768_setCustomMode(
    logicalMode_t *pLogicalMode, 
    mode_parameter_t *pUserModeParam
)
{
    mode_parameter_t pModeParam; /* physical parameters for the mode */
    pll_value_t pll;
    unsigned long ulActualPixelClk, ulTemp;

    /*
     * Minimum check on mode base address.
     * At least it shouldn't be bigger than the size of frame buffer.
     */
    if (ddk768_getFrameBufSize() <= pLogicalMode->baseAddress)
        return -1;

    /*
     * Set up PLL, a structure to hold the value to be set in clocks.
     */
    pll.inputFreq = DEFAULT_INPUT_CLOCK; /* Defined in CLOCK.H */



    /* 
     * Call calcPllValue() to fill up the other fields for PLL structure.
     * Sometime, the chip cannot set up the exact clock required by User.
     * Return value from calcPllValue() gives the actual possible pixel clock.
     */
    ulActualPixelClk = ddk768_calcPllValue(pUserModeParam->pixel_clock, &pll);
  



    /* If calling function don't have a preferred pitch value, 
       work out a 16 byte aligned pitch value.
    */
    if (pLogicalMode->pitch == 0)
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
        pLogicalMode->pitch = PITCH(pLogicalMode->x, pLogicalMode->bpp);
    }

    /* Program the hardware to set up the mode. */
    return( ddk768_programModeRegisters( 
            pLogicalMode, 
            pUserModeParam,
            &pll));
}

/*
 * Input pLogicalMode contains information such as x, y resolution and bpp 
 * The main difference between setMode and setModeEx userData.
 *
 * Return: 0 (or NO_ERROR) if mode can be set successfully.
 *         -1 if any set mode error.
 */
long ddk768_setModeEx(
    logicalMode_t *pLogicalMode
)
{
    mode_parameter_t *pModeParam;       /* physical parameters for the mode */
    unsigned short index = 0;
    userData_t *pUserData;

    /*
     * Check the validity of the userData pointer and translate the information as necessary
     */
    pUserData = (userData_t *)pLogicalMode->userData;
    if ((pUserData != (userData_t *)0) &&
        (pUserData->signature == ddk768_getUserDataSignature()) &&
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
    pModeParam = ddk768_findModeParam(pLogicalMode->dispCtrl, pLogicalMode->x, pLogicalMode->y, pLogicalMode->hz, index);
    if (pModeParam == (mode_parameter_t *)0)
        return -1;

    return(ddk768_setCustomMode(pLogicalMode, pModeParam));
}

/*
 * Input pLogicalMode contains information such as x, y resolution and bpp.
 * If there is no special parameters, use this function.
 * If there are special parameters, use setModeEx.
 *
 * Return: 0 (or NO_ERROR) if mode can be set successfully.
 *         -1 if any set mode error.
 */
long ddk768_setMode(
    logicalMode_t *pLogicalMode
)
{
    pLogicalMode->userData = (void *)0;

    /* Call the setModeEx to set the mode. */
    return ddk768_setModeEx(pLogicalMode);
}

