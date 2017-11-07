/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  CURSOR.C --- Voyager GX SDK 
*  This file contains the definitions for the Panel cursor functions.
* 
*******************************************************************/
#include "ddk768.h"

#include "ddk768_cursor.h"

/*
 * This function initializes the cursor attributes.
 */
void ddk768_initCursor(
    disp_control_t dispControl,     /* Display control (CHANNEL0_CTRL or CHANNEL1_CTRL) */
    unsigned long base,             /* Base Address */ 
    unsigned long color1,           /* Cursor color 1 in RGB 5:6:5 format */
    unsigned long color2,           /* Cursor color 2 in RGB 5:6:5 format */
    unsigned long color3            /* Cursor color 3 in RGB 5:6:5 format */
)
{
    /*
     * 1. Set the cursor source address 
     */
    pokeRegisterDWord(
        (dispControl == CHANNEL0_CTRL) ? CHANNEL0_HWC_ADDRESS : CHANNEL1_HWC_ADDRESS,
        FIELD_VALUE(0, CHANNEL0_HWC_ADDRESS, ADDRESS, base));
        
    /*
     * 2. Set the cursor color composition 
     */
    pokeRegisterDWord(
        (dispControl == CHANNEL0_CTRL) ? CHANNEL0_HWC_COLOR_0 : CHANNEL1_HWC_COLOR_0, 
        FIELD_VALUE(0, CHANNEL0_HWC_COLOR_0, RGB888, color1));

    pokeRegisterDWord(
        (dispControl == CHANNEL0_CTRL) ? CHANNEL0_HWC_COLOR_1 : CHANNEL1_HWC_COLOR_1, 
        FIELD_VALUE(0, CHANNEL0_HWC_COLOR_1, RGB888, color2));

}

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
)
{  
    unsigned long value;

    /* Set the XY coordinate */
    value = FIELD_VALUE(0, HWC_LOCATION, X, dx) |
            FIELD_VALUE(0, HWC_LOCATION, Y, dy);
    
    /* Set the top boundary select either partially outside the top boundary
       screen or inside */
    if (topOutside)
        value = FIELD_SET(value, HWC_LOCATION, TOP, OUTSIDE);
    else         
        value = FIELD_SET(value, HWC_LOCATION, TOP, INSIDE);

    /* Set the left boundary select either partially outside the left boundary
       screen or inside */
    if (leftOutside)
        value = FIELD_SET(value, HWC_LOCATION, LEFT, OUTSIDE);
    else        
        value = FIELD_SET(value, HWC_LOCATION, LEFT, INSIDE);

    /* Set the register accordingly, either Panel cursor or CRT cursor */
    pokeRegisterDWord((dispControl == CHANNEL0_CTRL) ? HWC_LOCATION : (HWC_LOCATION+CHANNEL_OFFSET), value);
}

/*
 * This function enables/disables the cursor.
 */
void ddk768_enableCursor(
    disp_control_t dispControl, /* Display control (CHANNEL0_CTRL or CHANNEL1_CTRL) */
    unsigned long mode			/* Cursor type - 00 disable, 0x01 mask cursor, 0x02 mono, 0x03 alpha cursor */
)
{
    unsigned long cursorRegister, value;

    cursorRegister = (dispControl == CHANNEL0_CTRL) ? HWC_CONTROL : (HWC_CONTROL+CHANNEL_OFFSET);
    
	value = peekRegisterDWord(cursorRegister);
    value = FIELD_VALUE(value, HWC_CONTROL, MODE, mode);
    
    pokeRegisterDWord(cursorRegister, value);
}
