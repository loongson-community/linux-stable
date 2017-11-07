/*
 * Copyright 2016 SiliconMotion Inc.
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License version 2. See the file COPYING in the main
 * directory of this archive for more details.
 *
 */



#ifndef LYNX_HW_COM_H__
#define LYNX_HW_COM_H__

typedef enum _disp_path_t
{
    SMI0_PATH = 0,
    SMI1_PATH  = 1,
}
disp_path_t;

typedef enum _disp_control_t
{
    SMI0_CTRL = 0,
    SMI1_CTRL = 1,
    SMI2_CTRL = 2,
    ERROR_CTRL = 3,
}
disp_control_t;

typedef enum _disp_state_t
{
    DISP_OFF = 0,
    DISP_ON  = 1,
}
disp_state_t;
typedef enum _DPMS_t
{
    DPMS_ON,
    DPMS_STANDBY,
    DPMS_SUSPEND,
    DPMS_OFF
}
DPMS_t;

typedef enum _DISP_DPMS_t
{
    DISP_DPMS_ON,
    DISP_DPMS_STANDBY,
    DISP_DPMS_SUSPEND,
    DISP_DPMS_OFF
}
DISP_DPMS_t;



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

#endif

