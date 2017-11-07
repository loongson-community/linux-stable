/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  MODE.H --- SMI DDK 
*  This file contains the definitions for the mode tables.
* 
*******************************************************************/
#ifndef _MODE_H_
#define _MODE_H_

/* Maximum parameters those can be saved in the mode table. */
#define MAX_MODE_TABLE_ENTRIES              60

/* Set the alignment to 1-bit aligned. Otherwise, the memory compare used in the
   modeext to compare timing will not work. */
#pragma pack(1)

typedef enum _spolarity_t
{
    POS, /* positive */
    NEG, /* negative */
}
spolarity_t;

typedef struct _mode_parameter_t
{
    /* Horizontal timing. */
    unsigned long horizontal_total;
    unsigned long horizontal_display_end;
    unsigned long horizontal_sync_start;
    unsigned long horizontal_sync_width;
    spolarity_t horizontal_sync_polarity;

    /* Vertical timing. */
    unsigned long vertical_total;
    unsigned long vertical_display_end;
    unsigned long vertical_sync_start;
    unsigned long vertical_sync_height;
    spolarity_t vertical_sync_polarity;

    /* Refresh timing. */
    unsigned long pixel_clock;
    unsigned long horizontal_frequency;
    unsigned long vertical_frequency;
    
    /* Clock Phase. This clock phase only applies to Panel. */
    spolarity_t clock_phase_polarity;
}
mode_parameter_t;

typedef enum _disp_control_t
{
    PRIMARY_CTRL = 0,
    SECONDARY_CTRL   = 1,
#if 0    
    VGA_CTRL   = 2
#endif
}
disp_control_t;

typedef struct _logicalMode_t
{
    unsigned long x;            /* X resolution */
    unsigned long y;            /* Y resolution */
    unsigned long bpp;          /* Bits per pixel */
    unsigned long hz;           /* Refresh rate */

    unsigned long baseAddress;  /* Offset from beginning of frame buffer.
                                   It is used to control the starting location of a mode.
                                   Calling function must initialize this field.
                                 */

    unsigned long pitch;        /* Mode pitch in byte.
                                   If initialized to 0, setMode function will set
                                   up this field.
                                   If not zero, setMode function will use this value.
                                 */

    disp_control_t dispCtrl;    /* SECONDARY or PRIMARY display control channel */
    
    /* These two parameters are used in the setModeEx. */
    unsigned long xLCD;         /* Panel width */
    unsigned long yLCD;         /* Panel height */
    
    void *userData;             /* Not used now, set it to 0 (for future used only) */
}
logicalMode_t;

/*
 * ID of the modeInfoID used in the userDataParam_t
 */
#define MODE_INFO_INDEX         0x01

typedef struct _userDataParam_t
{
    unsigned long size;         /* Size of this parameter */
    unsigned char modeInfoID;   /* Mode information ID */
    
    /* As the modeInfoID might be expanded in the future to support more stuffs as necessary, 
       the following parameter list might be expanded in the future. */
    union
    {
        unsigned char index;    /* The index of the mode timing of the given resolution:
                                        0   - The first mode timing found in the table
                                        1   - The second mode timing found in the table
                                        etc...
                                 */
        /* Add more user data information parameters here as necessary. */
    } paramInfo;
} userDataParam_t;

/* 
 * Structure that is used as userData pointer in the logicalMode_t 
 */
typedef struct _userData_t
{
    unsigned long signature;    /* Signature of the userData pointer. 
                                   It has to be filled with user data Signature to be a valid
                                   structure pointer. The signature can be obtained by
                                   calling getUserDataSignature function.
                                 */
    unsigned long size;         /* Size of this data structure. */
    userDataParam_t paramList;  /* List of parameters those are associated with this userData 
                                   Currently, only one modeInfoID is supported. Later on, when
                                   the ID is expanded, then the paramList might be expanded
                                   as an array. */
} 
userData_t;

/* Restore alignment */
#pragma pack()

/*
 *  getUserDataSignature
 *      This function gets the user data mode signature
 *
 *  Output:
 *      The signature to be filled in the user_data_mode_t structure to be considered
 *      a valid structure.
 */
unsigned long getUserDataSignature(void);

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
);

/*
 *  getDuplicateModeIndex
 *      This function retrieves the index of dupicate modes, but having different timing.
 *
 *  Input:
 *      dispCtrl    - Display Control where the mode table belongs to.
 *      pModeParam  - The mode parameters which index to be checked.
 *
 *  Output:
 *      0xFFFF  - The mode parameter can not be found in the current mode table
 *      Other   - The index of the given parameters among the duplicate modes.
 *                0 means that the mode param is the first mode encountered in the table
 *                1 means that the mode param is the second mode encountered in the table
 *                etc...
 */
unsigned short getDuplicateModeIndex(
    disp_control_t dispCtrl,
    mode_parameter_t *pModeParam
);

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
);

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
);

/*
 *  (Obsolete) --> replace by findModeParam
 *  Locate in-stock parameter table for the requested mode.
 *  Success: return a pointer to the mode_parameter_t entry.
 *  Fail: a NULL pointer.
 */
mode_parameter_t *findVesaModeParam(
    unsigned long width, 
    unsigned long height, 
    unsigned long refresh_rate
);

/* 
 * Return a point to the gModeParamTable.
 * Function in other files used this to get the mode table pointer.
 */
mode_parameter_t *getStockModeParamTable(void);

/*
 * Return the size of the Stock Mode Param Table
 */
unsigned long getStockModeParamTableSize(void);

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
);

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
);

/* 
 * This function returns the current mode.
 */
mode_parameter_t getCurrentModeParam(
    disp_control_t dispCtrl
);

/*
 *  getMaximumModeEntries
 *      This function gets the maximum entries that can be stored in the mode table.
 *
 *  Output:
 *      Total number of maximum entries
 */
unsigned long getMaximumModeEntries(void);

/*
 * Input:
 *     1) pLogicalMode contains information such as x, y resolution and bpp.
 *     2) A user defined parameter table for the mode.
 *
 * This function calculate and programs the hardware to set up the
 * requested mode.
 *
 * This function allows the use of user defined parameter table if
 * predefined Vesa parameter table (gModeParamTable) does not fit.
 *
 * Return: 0 (or NO_ERROR) if mode can be set successfully.
 *         -1 if any set mode error.
 */
long setCustomMode(
    logicalMode_t *pLogicalMode, 
    mode_parameter_t *pUserModeParam
);

/*
 * Input:
 *     1) pLogicalMode contains information such as x, y resolution, bpp, xLCD, and yLCD.
 *     2) A user defined parameter table for the mode.
 *
 * Similar like setCustomMode, this function calculate and programs the hardware 
 * to set up the requested mode and also scale the mode if necessary.
 *
 * This function allows the use of user defined parameter table if
 * predefined Vesa parameter table (gModeParamTable) does not fit.
 *
 * Return: 0 (or NO_ERROR) if mode can be set successfully.
 *         -1 if any set mode error.
 */
long setCustomModeEx(
    logicalMode_t *pLogicalMode, 
    mode_parameter_t *pUserModeParam
);

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
);

/*
 * Input pLogicalMode contains information such as x, y resolution, bpp, 
 * xLCD and yLCD. The main difference between setMode and setModeEx are
 * the xLCD and yLCD parameters. Use this setModeEx API to set the mode
 * and enable expansion. setMode API does not support expansion.
 *
 * This function calculate and programs the hardware to set up the
 * requested mode.
 *
 * Return: 0 (or NO_ERROR) if mode can be set successfully.
 *         -1 if any set mode error.
 */
long setModeEx(
    logicalMode_t *pLogicalMode
);


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
);

/***********************************************************************************
 *
 *  The following function prototypes are the extension of mode.c where it involves
 *  EDID, GTF, and others calculation. Please include modeext.c to use these
 *  functions.
 *
 ***********************************************************************************/

/*
 *  registerEdidTiming
 *      This function registers timing from EDID, LCD Monitor Timing Extension,
 *      Standard Timing Extension, etc...
 *
 *  Input:
 *      dispCtrl            - Display control where the mode will be associated to
 *      pEDIDBuffer         - Pointer to the display device's EDID Buffer
 *      pLCDTimingExt       - Pointer to LCD Monitor Timing Extension (currently is not supported)
 *      pStdTimingExt       - Pointer to Standard Timing extension (currently is not supported)
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
long registerEdidTiming(
    disp_control_t dispCtrl,
    unsigned char *pEDIDBuffer,
    unsigned char *pLCDTimingExt,
    unsigned char *pStdTimingExt
);

/*
 *  calculateGtfTiming
 *      This function calculate the GTF Timing and produce the SM750 mode parameter
 *      format
 *
 *  Input:
 *      width               - Mode Width
 *      height              - Mode Height
 *      refreshRate         - Mode Refresh Rate
 *      pModeParam          - Pointer to SM750 mode parameter format variable to store
 *                            the calculated value of the GTF
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
long calculateDefaultGtfTiming(
    unsigned long width,
    unsigned long height,
    unsigned long refreshRate,
    mode_parameter_t *pModeParam
);

/*
 *  calculateGtfTiming
 *      This function calculate the GTF Timing and produce the SM750 mode parameter
 *      format
 *
 *  Input:
 *      width               - Mode Width
 *      height              - Mode Height
 *      refreshRate         - Mode Refresh Rate
 *      pModeParam          - Pointer to SM750 mode parameter format variable to store
 *                            the calculated value of the GTF
 *      offset              - The value of GTF Offset parameter
 *      gradient            - The value of GTF Gradient parameter
 *      scalingFactor       - The value of the GTF scaling factor parameter
 *      scalingFactorWeight - The value of the GTF scaling Factor Weight parameter
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
long calculateGtfTiming(
    unsigned long width,
    unsigned long height,
    unsigned long refreshRate,
    mode_parameter_t *pModeParam,
    unsigned char offset,
    unsigned short gradient,
    unsigned char scalingFactor,
    unsigned char scalingFactorWeight
    
);

/*
 *  isDefaultGtfTiming
 *      Check if the given timing is default GTF timing. 
 *      Default here means it uses the following configuration to calculate the timing:
 *          Offset (C) = 40%
 *          Gradient (M) = 600%/kHz
 *          Scaling Factor (K) = 128
 *          Weighted Scaling Factor (J) = 20%
 *
 *  Input:
 *      pModeParam  - Mode Parameter that will be checked whether is default GTF or not
 *
 *  Output:
 *      0       - Indicates that the given mode parameter is a NOT a default GTF Timing
 *      Other   - Indicates that the given mode parameter is a default GTF Timing
 */
long isDefaultGtfTiming(
    mode_parameter_t *pModeParam
);

/*
 *  isSecondaryGtfTiming
 *      Check if the given timing is the secondary GTF timing calculated based on the
 *      given offset (C), gradient (M), scaling factor (K), and weighted scaling factor (J). 
 *
 *  Input:
 *      pModeParam          - Mode Parameter that will be checked whether is secondary GTF or not
 *      offset              - Blanking formula offset used to calculate the secondary GTF mode param.
 *      gradient            - Blanking formula gradient used to calculate the secondary GTF 
 *                            mode param.
 *      scalingFactor       - Blanking formula scaling factor used to calculate the secondary 
 *                            GTF mode param
 *      scalingFactorWeight - Blanking formula scaling factor weighting used to calculate the
 *                            secondary GTF mode param
 *
 *  Output:
 *      0       - Indicates that the given mode parameter is NOT a default GTF Timing
 *      Other   - Indicates that the given mode parameter is a default GTF Timing
 */
long isSecondaryGtfTiming(
    mode_parameter_t *pModeParam,
    unsigned char offset,
    unsigned short gradient,
    unsigned char scalingFactor,
    unsigned char scalingFactorWeight
);

/*
 *  isVesaTiming
 *      Check if the given timing is the VESA Discrete Monitor Timing Standard. 
 *
 *  Input:
 *      pModeParam  - Mode Parameter that will be checked whether is VESA Timing or not
 *
 *  Output:
 *      0       - Indicates that the given mode parameter is NOT a VESA DMT Timing
 *      Other   - Indicates that the given mode parameter is a VESA DMT Timing
 */
long isVesaTiming(
    mode_parameter_t *pModeParam
);

/*
 *  isEdidPreferredTiming
 *      Check if the given timing is the EDID preferred timing. 
 *
 *  Input:
 *      pEDIDBuffer - Pointer to an EDID Buffer. 
 *      pModeParam  - Mode Parameter that will be checked whether is EDID
 *                    preferred timing or not.
 *
 *  Output:
 *     >0   - Indicates that the given mode parameter is a default GTF Timing
 *      0   - Indicates that the given mode parameter is NOT a default GTF Timing
 */
long isEdidPreferredTiming(
    unsigned char *pEDIDBuffer,
    mode_parameter_t *pModeParam
);

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
);

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
);

#endif /* _MODE_H_ */
