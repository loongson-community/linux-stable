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
#ifndef _CURSOR_H_
#define _CURSOR_H_

#include "ddk750_mode.h"
/*
 * This function initializes the cursor attributes.
 */
void ddk750_initCursor(
    disp_control_t dispControl,     /* Display control (PRIMARY_CTRL or SECONDARY_CTRL) */
    unsigned long base,             /* Base Address */ 
    unsigned long color1,           /* Cursor color 1 in RGB 5:6:5 format */
    unsigned long color2,           /* Cursor color 2 in RGB 5:6:5 format */
    unsigned long color3            /* Cursor color 3 in RGB 5:6:5 format */
);

/*
 * This function sets the cursor position.
 */
void ddk750_setCursorPosition(
    disp_control_t dispControl,     /* Display control (PRIMARY_CTRL or SECONDARY_CTRL) */
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
void ddk750_enableCursor(
    disp_control_t dispControl,     /* Display control (PRIMARY_CTRL or SECONDARY_CTRL) */
    unsigned long enable
);

#endif /* _CURSOR_H_ */
