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
#include "ddk750_defs.h"
#include "ddk750_hardware.h"
#include "ddk750_cursor.h"
#include "ddk750_help.h"
/*
 * This function initializes the cursor attributes.
 */
void ddk750_initCursor(
    disp_control_t dispControl,     /* Display control (PRIMARY_CTRL or SECONDARY_CTRL) */
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
        (dispControl == PRIMARY_CTRL) ? PRIMARY_HWC_ADDRESS : SECONDARY_HWC_ADDRESS,
        FIELD_SET(0, PRIMARY_HWC_ADDRESS, EXT, LOCAL) |
        FIELD_VALUE(0, PRIMARY_HWC_ADDRESS, ADDRESS, base));
        
    /*
     * 2. Set the cursor color composition 
     */
    pokeRegisterDWord(
        (dispControl == PRIMARY_CTRL) ? PRIMARY_HWC_COLOR_12 : SECONDARY_HWC_COLOR_12, 
        FIELD_VALUE(0, PRIMARY_HWC_COLOR_12, 1_RGB565, color1) |
        FIELD_VALUE(0, PRIMARY_HWC_COLOR_12, 2_RGB565, color2) );

    pokeRegisterDWord(
        (dispControl == PRIMARY_CTRL) ? PRIMARY_HWC_COLOR_3 : SECONDARY_HWC_COLOR_3,
        FIELD_VALUE(0, PRIMARY_HWC_COLOR_3, RGB565, color3));
}

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
)
{  
    unsigned long value;

    /* Set the XY coordinate */
    value = FIELD_VALUE(0, PRIMARY_HWC_LOCATION, X, dx) |
            FIELD_VALUE(0, PRIMARY_HWC_LOCATION, Y, dy);
    
    /* Set the top boundary select either partially outside the top boundary
       screen or inside */
    if (topOutside)
        value = FIELD_SET(value, PRIMARY_HWC_LOCATION, TOP, OUTSIDE);
    else         
        value = FIELD_SET(value, PRIMARY_HWC_LOCATION, TOP, INSIDE);

    /* Set the left boundary select either partially outside the left boundary
       screen or inside */
    if (leftOutside)
        value = FIELD_SET(value, PRIMARY_HWC_LOCATION, LEFT, OUTSIDE);
    else        
        value = FIELD_SET(value, PRIMARY_HWC_LOCATION, LEFT, INSIDE);

    /* Set the register accordingly, either Panel cursor or CRT cursor */
    pokeRegisterDWord((dispControl == PRIMARY_CTRL) ? PRIMARY_HWC_LOCATION : SECONDARY_HWC_LOCATION, value);
}

/*
 * This function enables/disables the cursor.
 */
void ddk750_enableCursor(
    disp_control_t dispControl,     /* Display control (PRIMARY_CTRL or SECONDARY_CTRL) */
    unsigned long enable
)
{
    unsigned long cursorRegister, value;

    cursorRegister = (dispControl == PRIMARY_CTRL) ? PRIMARY_HWC_ADDRESS : SECONDARY_HWC_ADDRESS;
    
	value = peekRegisterDWord(cursorRegister);
    if (enable) 
        value = FIELD_SET(value, PRIMARY_HWC_ADDRESS, ENABLE, ENABLE);
    else
        value = FIELD_SET(value, PRIMARY_HWC_ADDRESS, ENABLE, DISABLE);
    
    pokeRegisterDWord(cursorRegister, value);
}
