/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  cursor.h --- SMI DDK 
*  This file contains the definitions for the cursor functions.
* 
*******************************************************************/
#ifndef _DDK768_CURSOR_H_
#define _DDK768_CURSOR_H_

#include "ddk768_mode.h"

#define CURSOR_DISABLE	0x00
#define CURSOR_ALPHA	0x01
#define CURSOR_MONO		0x02
#define CURSOR_MASK		0x03

/*
 * This function initializes the cursor attributes.
 */
void ddk768_initCursor(
    disp_control_t dispControl,     /* Display control (CHANNEL0_CTRL or CHANNEL1_CTRL) */
    unsigned long base,             /* Base Address */ 
    unsigned long color1,           /* Cursor color 1 in RGB 5:6:5 format */
    unsigned long color2,           /* Cursor color 2 in RGB 5:6:5 format */
    unsigned long color3            /* Cursor color 3 in RGB 5:6:5 format */
);

/*
 * This function sets the cursor position.
 */
void ddk768_setCursorPosition(
    disp_control_t dispControl,     /* Display control (CHANNEL0_CTRL or CHANNEL1_CTRL) */
    unsigned long dx,               /* X Coordinate of the cursor */
    unsigned long dy,               /* Y Coordinate of the cursor */
    unsigned char topOutside,       /* Top Boundary Select: either partially outside (= 1) 
                                       or within the screen top boundary (= 0) */
    unsigned char leftOutside       /* Left Boundary Select: either partially outside (= 1) 
                                       or within the screen left boundary (= 0) */
);

/*
 * This function enables/disables the cursor.
 */
void ddk768_enableCursor(
    disp_control_t dispControl,     /* Display control (CHANNEL0_CTRL or CHANNEL1_CTRL) */
    unsigned long enable
);

#endif /* _CURSOR_H_ */
