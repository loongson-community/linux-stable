/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  2d.C --- SM750/SM718 DDK 
*  This file contains the definitions for the 2D functions.
* 
*******************************************************************/
#include "linux/string.h"
#include "ddk768_reg.h"

#include "ddk768_chip.h"
#include "ddk768_power.h"
#include "ddk768_2d.h"
#include "ddk768_help.h"

/* Blt Direction definitions */
#define TOP_TO_BOTTOM 0
#define LEFT_TO_RIGHT 0
#define BOTTOM_TO_TOP 1
#define RIGHT_TO_LEFT 1

#if 0 /* Cheok: Block it out to test if it is problem in new chip */

/* Flag to enable the 192 bytes patch to workaround the 2D errata, where the engine
   will draws incorrectly for BITBLT function that involves READ from memory. 
   Currently, this definition flag is only used for testing. */
#define ENABLE_192_BYTES_PATCH
#endif

/* Static macro */
#define BYTE_PER_PIXEL(bpp)         (bpp / 8)

/*
 * 2D Engine Initialization.
 * This function must be called before other 2D functions.
 * Assumption: A specific video mode has been properly set up.
 */
void ddk768_deInit()
{
    ddk768_enable2DEngine(1);

    ddk768_deReset(); /* Just be sure no left-over operations from other applications */

    /* Set up 2D registers that won't change for a specific mode. */

    /* Drawing engine bus and pixel mask, always want to enable. */
    POKE_32(DE_MASKS, 0xFFFFFFFF);

    /* Pixel format, which can be 8, 16 or 32.
       Assuming setmode is call before 2D init, then pixel format
       is available in reg 0x80000 (Panel Display Control)
    */
    POKE_32(DE_STRETCH_FORMAT,
        FIELD_SET  (0, DE_STRETCH_FORMAT, PATTERN_XY,   NORMAL)  |
        FIELD_VALUE(0, DE_STRETCH_FORMAT, PATTERN_Y,    0)       |
        FIELD_VALUE(0, DE_STRETCH_FORMAT, PATTERN_X,    0)       |
        FIELD_SET  (0, DE_STRETCH_FORMAT, ADDRESSING,   XY)      |
        FIELD_VALUE(0, DE_STRETCH_FORMAT, SOURCE_HEIGHT,3));

    /* Clipping and transparent are disable after INIT */
    ddk768_deSetClipping(0, 0, 0, 0, 0);
    ddk768_deSetTransparency(0, 0, 0, 0);
}

/*
 * Reset 2D engine by 
 * 1) Aborting the current 2D operation.
 * 2) Re-enable 2D engine to normal state.
 */
void ddk768_deReset()
{
    unsigned long sysCtrl;
	logical_chip_type_t chipType = ddk768_getChipType();

#if 0 /* Cheok(2/11/2014): Not sure which registers are used for Falcon */

	if (chipType == SM750 || chipType == SM718)
	{
    	/* Abort current 2D operation */
    	sysCtrl = PEEK_32(SYSTEM_CTRL);
    	sysCtrl = FIELD_SET(sysCtrl, SYSTEM_CTRL, DE_ABORT, ON);
    	POKE_32(SYSTEM_CTRL, sysCtrl);

    	/* Re-enable 2D engine to normal state */
    	sysCtrl = PEEK_32(SYSTEM_CTRL);
    	sysCtrl = FIELD_SET(sysCtrl, SYSTEM_CTRL, DE_ABORT, OFF);
    	POKE_32(SYSTEM_CTRL, sysCtrl);
	}
	else /* For SM750LE and SM750HS series */
	{
	    /* Abort current 2D operation */
		sysCtrl = PEEK_32(DE_STATE1);
		sysCtrl = FIELD_SET(sysCtrl, DE_STATE1, DE_ABORT, ON);
		POKE_32(DE_STATE1, sysCtrl);

		/* Re-enable 2D engine to normal state */
        sysCtrl = PEEK_32(DE_STATE1);
		sysCtrl = FIELD_SET(sysCtrl, DE_STATE1, DE_ABORT, OFF);
		POKE_32(DE_STATE1, sysCtrl);
	}
#endif
}
 
/*
 * Wait until 2D engine is not busy.
 * All 2D operations are recommand to check 2D engine idle before start.
 *
 * Return: 0 = return because engine is idle and normal.
 *        -1 = return because time out (2D engine may have problem).
 */
long ddk768_deWaitForNotBusy(void)
{
	unsigned long dwVal;
    unsigned long i = 0x100000;

    while (i--)
    {
        dwVal = PEEK_32(DE_STATE2);
        if ((FIELD_GET(dwVal, DE_STATE2, DE_STATUS)      == DE_STATE2_DE_STATUS_IDLE) &&
            (FIELD_GET(dwVal, DE_STATE2, DE_FIFO)        == DE_STATE2_DE_FIFO_EMPTY) &&
            (FIELD_GET(dwVal, DE_STATE2, DE_MEM_FIFO)    == DE_STATE2_DE_MEM_FIFO_EMPTY))
        {
            return 0; /* Return because engine idle */
        }
    }
    return -1; /* Return because of timeout */
}

/*
 * This function enable/disable clipping area for the 2d engine.
 * Note that the clipping area is always rectangular.
 * 
 */
long ddk768_deSetClipping(
unsigned long enable, /* 0 = disable clipping, 1 = enable clipping */
unsigned long x1,     /* x1, y1 is the upper left corner of the clipping area */
unsigned long y1,     /* Note that the region includes x1 and y1 */
unsigned long x2,     /* x2, y2 is the lower right corner of the clippiing area */
unsigned long y2)     /* Note that the region will not include x2 and y2 */
{
    if (ddk768_deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* ddk768_deReset(); */
    }

    /* Upper left corner and enable/disable bit
       Note: This module defautls to clip outside region.
       "Clip inside" is not a useful feature since nothing gets drawn.
     */
    POKE_32(DE_CLIP_TL,
        FIELD_VALUE(0, DE_CLIP_TL, TOP, y1) |
        ((enable)?
          FIELD_SET(0, DE_CLIP_TL, STATUS, ENABLE)
        : FIELD_SET(0, DE_CLIP_TL, STATUS, DISABLE))|
        FIELD_SET  (0, DE_CLIP_TL, INHIBIT,OUTSIDE) |
        FIELD_VALUE(0, DE_CLIP_TL, LEFT, x1));

    /* Lower right corner */
    POKE_32(DE_CLIP_BR,
        FIELD_VALUE(0, DE_CLIP_BR, BOTTOM,y2) |
        FIELD_VALUE(0, DE_CLIP_BR, RIGHT, x2));

    return 0;
}

/* 
 * Function description:
 * When transparency is enable, the blt engine compares each pixel value 
 * (either source or destination) with DE_COLOR_COMPARE register.
 * If match, the destination pixel will NOT be updated.
 * If not match, the destination pixel will be updated.
 */
long ddk768_deSetTransparency(
unsigned long enable,     /* 0 = disable, 1 = enable transparency feature */
unsigned long tSelect,    /* 0 = compare source, 1 = compare destination */
unsigned long tMatch,     /* 0 = Opaque mode, 1 = transparent mode */
unsigned long ulColor)    /* Color to compare. */
{
    unsigned long de_ctrl;

    if (ddk768_deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* ddk768_deReset(); */
    }

    /* Set mask */
    if (enable)
    {
        POKE_32(DE_COLOR_COMPARE_MASK, 0x00ffffff);

        /* Set compare color */
        POKE_32(DE_COLOR_COMPARE, ulColor);
    }
    else
    {
        POKE_32(DE_COLOR_COMPARE_MASK, 0x0);
        POKE_32(DE_COLOR_COMPARE, 0x0);
    }

    /* Set up transparency control, without affecting other bits
       Note: There are two operatiing modes: Transparent and Opague.
       We only use transparent mode because Opaque mode may have bug.
    */
    de_ctrl = PEEK_32(DE_CONTROL)
              & FIELD_CLEAR(DE_CONTROL, TRANSPARENCY)
              & FIELD_CLEAR(DE_CONTROL, TRANSPARENCY_MATCH)
              & FIELD_CLEAR(DE_CONTROL, TRANSPARENCY_SELECT);

    /* For DE_CONTROL_TRANSPARENCY_MATCH bit, always set it
       to TRANSPARENT mode, OPAQUE mode don't seem working.
    */
    de_ctrl |=
    ((enable)?
      FIELD_SET(0, DE_CONTROL, TRANSPARENCY, ENABLE)
    : FIELD_SET(0, DE_CONTROL, TRANSPARENCY, DISABLE))        |
    ((tMatch)?
      FIELD_SET(0, DE_CONTROL, TRANSPARENCY_MATCH, TRANSPARENT)
    : FIELD_SET(0, DE_CONTROL, TRANSPARENCY_MATCH, OPAQUE)) |
    ((tSelect)?
      FIELD_SET(0, DE_CONTROL, TRANSPARENCY_SELECT, DESTINATION)
    : FIELD_SET(0, DE_CONTROL, TRANSPARENCY_SELECT, SOURCE));

    POKE_32(DE_CONTROL, de_ctrl);

    return 0;
}

/*
 * This function gets the transparency status from DE_CONTROL register.
 * It returns a double word with the transparent fields properly set,
 * while other fields are 0.
 */
unsigned long ddk768_deGetTransparency(void)
{
    unsigned long de_ctrl;

    de_ctrl = PEEK_32(DE_CONTROL);

    de_ctrl &= 
        FIELD_MASK(DE_CONTROL_TRANSPARENCY_MATCH) | 
        FIELD_MASK(DE_CONTROL_TRANSPARENCY_SELECT)| 
        FIELD_MASK(DE_CONTROL_TRANSPARENCY);

    return de_ctrl;
}

/*
 * This function sets the pixel format that will apply to the 2D Engine.
 */
void ddk768_deSetPixelFormat(
    unsigned long bpp
)
{
    unsigned long de_format;
    
    de_format = PEEK_32(DE_STRETCH_FORMAT);
    
    switch (bpp)
    {
        case 8:
            de_format = FIELD_SET(de_format, DE_STRETCH_FORMAT, PIXEL_FORMAT, 8);
            break;
        default:
        case 16:
            de_format = FIELD_SET(de_format, DE_STRETCH_FORMAT, PIXEL_FORMAT, 16);
            break;
        case 32:
            de_format = FIELD_SET(de_format, DE_STRETCH_FORMAT, PIXEL_FORMAT, 32);
            break;
    }
    
    POKE_32(DE_STRETCH_FORMAT, de_format);
}

/*
 * This function uses 2D engine to fill a rectangular area with a specific color.
 * The filled area includes the starting points.
 */
long ddk768_deRectFill( /*resolution_t resolution, point_t p0, point_t p1, unsigned long color, unsigned long rop2)*/
unsigned long dBase,  /* Base address of destination surface counted from beginning of video frame buffer */
unsigned long dPitch, /* Pitch value of destination surface in BYTES */
unsigned long bpp,    /* Color depth of destination surface: 8, 16 or 32 */
unsigned long x,
unsigned long y,      /* Upper left corner (X, Y) of rectangle in pixel value */
unsigned long width, 
unsigned long height, /* width and height of rectange in pixel value */
unsigned long color,  /* Color to be filled */
unsigned long rop2)   /* ROP value */
{
    unsigned long de_ctrl, bytePerPixel;

    bytePerPixel = bpp/8;
    
    if (ddk768_deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* ddk768_deReset(); */
    }

    /* 2D Destination Base.
       It is an address offset (128 bit aligned) from the beginning of frame buffer.
    */
    POKE_32(DE_WINDOW_DESTINATION_BASE, dBase);

    /* Program pitch (distance between the 1st points of two adjacent lines).
       Note that input pitch is BYTE value, but the 2D Pitch register uses
       pixel values. Need Byte to pixel convertion.
    */
    POKE_32(DE_PITCH,
        FIELD_VALUE(0, DE_PITCH, DESTINATION, (dPitch/bytePerPixel)) |
        FIELD_VALUE(0, DE_PITCH, SOURCE,      (dPitch/bytePerPixel)));

    /* Screen Window width in Pixels.
       2D engine uses this value to calculate the linear address in frame buffer for a given point.
    */
    POKE_32(DE_WINDOW_WIDTH,
        FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, (dPitch/bytePerPixel)) |
        FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE,      (dPitch/bytePerPixel)));

    POKE_32(DE_FOREGROUND, color);

    /* Set the pixel format of the destination */
    ddk768_deSetPixelFormat(bpp);

#ifdef ENABLE_192_BYTES_PATCH
    /* Workaround for 192 byte requirement when ROP is not COPY */
    if (((rop2 != ROP2_COPY) || (rop2 != ROP2_Sn) || (rop2 != ROP2_Dn) || 
         (rop2 != ROP2_D) || (rop2 != ROP2_BLACK) || (rop2 != ROP2_WHITE)) && 
        ((width * bytePerPixel) > 192))
    {
        /* Perform the ROP2 operation in chunks of (xWidth * nHeight) */
        unsigned long xChunk = 192 / bytePerPixel; /* chunk width is in pixels */
        
        //DDKDEBUGPRINT((DE_LEVEL, "ROP != ROP_COPY, width * bytePerPixel = %x (> 192 bytes)\n", width * bytePerPixel));

        while (1)
        {
            ddk768_deWaitForNotBusy();
            
            POKE_32(DE_DESTINATION,
                FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
                FIELD_VALUE(0, DE_DESTINATION, X,    x)  |
                FIELD_VALUE(0, DE_DESTINATION, Y,    y));
                
            POKE_32(DE_DIMENSION,
                FIELD_VALUE(0, DE_DIMENSION, X,    xChunk) |
                FIELD_VALUE(0, DE_DIMENSION, Y_ET, height));

            de_ctrl = 
                FIELD_SET  (0, DE_CONTROL,  STATUS,     START)          |
                FIELD_SET  (0, DE_CONTROL,  DIRECTION,  LEFT_TO_RIGHT)  |
                //FIELD_SET  (0, DE_CONTROL,LAST_PIXEL, OFF)            |
                FIELD_SET  (0, DE_CONTROL,  COMMAND,    RECTANGLE_FILL) |
                FIELD_SET  (0, DE_CONTROL,  ROP_SELECT, ROP2)           |
                FIELD_VALUE(0, DE_CONTROL,  ROP,        rop2);

            POKE_32(DE_CONTROL, de_ctrl | ddk768_deGetTransparency());

            if (xChunk == width) break;

            x += xChunk;
            width -= xChunk;

            if (xChunk > width)
            {
                /* This is the last chunk. */
                xChunk = width;
            }
        }
    }
    else
#endif
    {
        POKE_32(DE_DESTINATION,
            FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
            FIELD_VALUE(0, DE_DESTINATION, X,    x)       |
            FIELD_VALUE(0, DE_DESTINATION, Y,    y));

        POKE_32(DE_DIMENSION,
            FIELD_VALUE(0, DE_DIMENSION, X,    width) |
            FIELD_VALUE(0, DE_DIMENSION, Y_ET, height));

        de_ctrl = 
            FIELD_SET  (0, DE_CONTROL,  STATUS,     START)          |
            FIELD_SET  (0, DE_CONTROL,  DIRECTION,  LEFT_TO_RIGHT)  |
            //FIELD_SET  (0, DE_CONTROL,LAST_PIXEL, OFF)            |
            FIELD_SET  (0, DE_CONTROL,  COMMAND,    RECTANGLE_FILL) |
            FIELD_SET  (0, DE_CONTROL,  ROP_SELECT, ROP2)           |
            FIELD_VALUE(0, DE_CONTROL,  ROP,        rop2);

        POKE_32(DE_CONTROL, de_ctrl | ddk768_deGetTransparency());
    }
    
    return 0;
}

/*
 * This function uses 2D engine to draw a trapezoid with a specific color.
 * The filled area includes the starting points.
 */
long ddk768_deStartTrapezoidFill(
    unsigned long dBase,  /* Base address of destination surface counted from beginning of video frame buffer */
    unsigned long dPitch, /* Pitch value of destination surface in BYTES */
    unsigned long bpp,    /* Color depth of destination surface: 8, 16 or 32 */
    unsigned long x,
    unsigned long y,      /* Starting (X, Y) coordinate inside the polygon to be filled */
    unsigned long length, /* Length of the line */
    unsigned long color,  /* Color to be filled */
    unsigned long rop2    /* ROP value */
)
{
    unsigned long dx, dy;
    unsigned long de_ctrl =
        FIELD_SET  (0, DE_CONTROL, STATUS,      START)           |
        FIELD_SET  (0, DE_CONTROL, QUICK_START, ENABLE)          |
        FIELD_SET  (0, DE_CONTROL, LAST_PIXEL,  OFF)             |
        FIELD_SET  (0, DE_CONTROL, DIRECTION,   LEFT_TO_RIGHT)   |
        FIELD_SET  (0, DE_CONTROL, COMMAND,     TRAPEZOID_FILL)  |
        FIELD_SET  (0, DE_CONTROL, ROP_SELECT,  ROP2)            |
        FIELD_VALUE(0, DE_CONTROL, ROP,         rop2);

    if (ddk768_deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* ddk768_deReset(); */
    }
    
    /* 2D Destination Base.
       It is an address offset (128 bit aligned) from the beginning of frame buffer.
    */
    POKE_32(DE_WINDOW_DESTINATION_BASE, dBase);

    /* Program pitch (distance between the 1st points of two adjacent lines).
       Note that input pitch is BYTE value, but the 2D Pitch register uses
       pixel values. Need Byte to pixel convertion.
    */
    POKE_32(DE_PITCH,
        FIELD_VALUE(0, DE_PITCH, DESTINATION, dPitch / BYTE_PER_PIXEL(bpp)) |
        FIELD_VALUE(0, DE_PITCH, SOURCE, dPitch / BYTE_PER_PIXEL(bpp)));

    /* Screen Window width in Pixels.
       2D engine uses this value to calculate the linear address in frame buffer for a given point.
    */
    POKE_32(DE_WINDOW_WIDTH,
        FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, dPitch / BYTE_PER_PIXEL(bpp)) |
        FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE,      dPitch / BYTE_PER_PIXEL(bpp)));

    /* Set the Line Color */
    POKE_32(DE_FOREGROUND, color);
    
    /* Set the destination coordinate */
    POKE_32(DE_DESTINATION,
        FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
        FIELD_VALUE(0, DE_DESTINATION, X,    x)       |
        FIELD_VALUE(0, DE_DESTINATION, Y,    y));
    
    /* Set the pixel format of the destination */
    ddk768_deSetPixelFormat(bpp);
    
    /* Set the line length and width */
    POKE_32(DE_DIMENSION,
        FIELD_VALUE(0, DE_DIMENSION, X,    length) |
        FIELD_VALUE(0, DE_DIMENSION, Y_ET, 0));

    /* Enable the 2D Engine. */
    POKE_32(DE_CONTROL, de_ctrl | ddk768_deGetTransparency());
    
    return 0;
}

/* 
 * Function to continue drawing a line using Trapezoid Fill method.
 */
long ddk768_deNextTrapezoidFill(
    unsigned long x,            /* Starting X location. */
    unsigned long length        /* Line length */
)
{
    unsigned long de_ctrl;
    
    if (ddk768_deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* ddk768_deReset(); */
    }
    
    /* Set the X destination coordinate */
    POKE_32(DE_DESTINATION,
        FIELD_VALUE(PEEK_32(DE_DESTINATION), DE_DESTINATION, X,    x));
        
    /* Set the line length */
    POKE_32(DE_DIMENSION,
        FIELD_VALUE(PEEK_32(DE_DIMENSION), DE_DIMENSION, X,    length));
        
    return 0;
}

/* 
 * Function to stop the Trapezoid Fill drawing.
 * This function has to be called to end the Trapezoid Fill drawing.
 * Otherwise, the next 2D function might still use this function.
 */
long ddk768_deStopTrapezoidFill()
{
    if (ddk768_deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* ddk768_deReset(); */
    }
    
    POKE_32(DE_CONTROL, FIELD_SET(PEEK_32(DE_CONTROL), DE_CONTROL, QUICK_START, DISABLE));
    
    return 0;
}

/*
 * Video Memory to Video Memory data transfer.
 * Note: 
 *        It works whether the Video Memroy is off-screeen or on-screen.
 *        This function is a one to one transfer without stretching or 
 *        mono expansion.
 */
long ddk768_deVideoMem2VideoMemBlt(
unsigned long sBase,  /* Address of source: offset in frame buffer */
unsigned long sPitch, /* Pitch value of source surface in BYTE */
unsigned long sx,
unsigned long sy,     /* Starting coordinate of source surface */
unsigned long dBase,  /* Address of destination: offset in frame buffer */
unsigned long dPitch, /* Pitch value of destination surface in BYTE */
unsigned long bpp,    /* Color depth of destination surface */
unsigned long dx,
unsigned long dy,     /* Starting coordinate of destination surface */
unsigned long width, 
unsigned long height, /* width and height of rectangle in pixel value */
unsigned long rop2)   /* ROP value */
{
    unsigned long nDirection, de_ctrl, bytePerPixel;
    long opSign;

    if (ddk768_deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* ddk768_deReset(); */
    }

    nDirection = LEFT_TO_RIGHT;
    opSign = 1;    /* Direction of ROP2 operation: 1 = Left to Right, (-1) = Right to Left */
    bytePerPixel = bpp/8;
    de_ctrl = 0;

    /* If source and destination are the same surface, need to check for overlay cases */
    if (sBase == dBase && sPitch == dPitch)
    {
        /* Determine direction of operation */
        if (sy < dy)
        {
            /* +----------+
               |S         |
               |   +----------+
               |   |      |   |
               |   |      |   |
               +---|------+   |
                   |         D|
                   +----------+ */
    
            nDirection = BOTTOM_TO_TOP;
        }
        else if (sy > dy)
        {
            /* +----------+
               |D         |
               |   +----------+
               |   |      |   |
               |   |      |   |
               +---|------+   |
                   |         S|
                   +----------+ */
    
            nDirection = TOP_TO_BOTTOM;
        }
        else
        {
            /* sy == dy */
    
            if (sx <= dx)
            {
                /* +------+---+------+
                   |S     |   |     D|
                   |      |   |      |
                   |      |   |      |
                   |      |   |      |
                   +------+---+------+ */
    
                nDirection = RIGHT_TO_LEFT;
            }
            else
            {
                /* sx > dx */
    
                /* +------+---+------+
                   |D     |   |     S|
                   |      |   |      |
                   |      |   |      |
                   |      |   |      |
                   +------+---+------+ */
    
                nDirection = LEFT_TO_RIGHT;
            }
        }
    }

    if ((nDirection == BOTTOM_TO_TOP) || (nDirection == RIGHT_TO_LEFT))
    {
        sx += width - 1;
        sy += height - 1;
        dx += width - 1;
        dy += height - 1;
        opSign = (-1);
    }

    /* Note:
       DE_FOREGROUND are DE_BACKGROUND are don't care.
       DE_COLOR_COMPARE and DE_COLOR_COMPARE_MAKS are set by set ddk768_deSetTransparency().
    */

    /* 2D Source Base.
       It is an address offset (128 bit aligned) from the beginning of frame buffer.
    */
    POKE_32(DE_WINDOW_SOURCE_BASE, sBase);

    /* 2D Destination Base.
       It is an address offset (128 bit aligned) from the beginning of frame buffer.
    */
    POKE_32(DE_WINDOW_DESTINATION_BASE, dBase);

    /* Program pitch (distance between the 1st points of two adjacent lines).
       Note that input pitch is BYTE value, but the 2D Pitch register uses
       pixel values. Need Byte to pixel convertion.
    */
    POKE_32(DE_PITCH,
        FIELD_VALUE(0, DE_PITCH, DESTINATION, (dPitch/bytePerPixel)) |
        FIELD_VALUE(0, DE_PITCH, SOURCE,      (sPitch/bytePerPixel)));

    /* Screen Window width in Pixels.
       2D engine uses this value to calculate the linear address in frame buffer for a given point.
    */
    POKE_32(DE_WINDOW_WIDTH,
        FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, (dPitch/bytePerPixel)) |
        FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE,      (sPitch/bytePerPixel)));

    /* Set the pixel format of the destination */
    ddk768_deSetPixelFormat(bpp);
    
#ifdef ENABLE_192_BYTES_PATCH
    /* This bug is fixed in SM718 for 16 and 32 bpp. However, in 8-bpp, the problem still exists. 
       The Version AA also have this problem on higher clock with 32-bit memory data bus, 
       therefore, it needs to be enabled here. 
       In version AA, the problem happens on the following configurations:
        1. M2XCLK = 336MHz w/ 32-bit, MCLK = 112MHz, and color depth set to 32bpp
        2. M2XCLK = 336MHz w/ 32-bit, MCLK = 84MHz, and color depth set to 16bpp or 32bpp.
       Somehow, the problem does not appears in 64-bit memory setting.
     */

    /* Workaround for 192 byte requirement when ROP is not COPY */
    if ((rop2 != ROP2_COPY) && ((width * bytePerPixel) > 192))
    {
        /* Perform the ROP2 operation in chunks of (xWidth * nHeight) */
        unsigned long xChunk = 192 / bytePerPixel; /* chunk width is in pixels */
        
        //DDKDEBUGPRINT((DE_LEVEL, "ROP != ROP_COPY, width * bytePerPixel = %x (> 192 bytes)\n", width * bytePerPixel));

        while (1)
        {
            ddk768_deWaitForNotBusy();
            POKE_32(DE_SOURCE,
                FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE) |
                FIELD_VALUE(0, DE_SOURCE, X_K1, sx)   |
                FIELD_VALUE(0, DE_SOURCE, Y_K2, sy));
            POKE_32(DE_DESTINATION,
                FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
                FIELD_VALUE(0, DE_DESTINATION, X,    dx)  |
                FIELD_VALUE(0, DE_DESTINATION, Y,    dy));
            POKE_32(DE_DIMENSION,
                FIELD_VALUE(0, DE_DIMENSION, X,    xChunk) |
                FIELD_VALUE(0, DE_DIMENSION, Y_ET, height));

            de_ctrl = 
                FIELD_VALUE(0, DE_CONTROL, ROP, rop2) |
                FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2) |
                FIELD_SET(0, DE_CONTROL, COMMAND, BITBLT) |
                ((nDirection == RIGHT_TO_LEFT) ? 
                FIELD_SET(0, DE_CONTROL, DIRECTION, RIGHT_TO_LEFT)
                : FIELD_SET(0, DE_CONTROL, DIRECTION, LEFT_TO_RIGHT)) |
                FIELD_SET(0, DE_CONTROL, STATUS, START);

            POKE_32(DE_CONTROL, de_ctrl | ddk768_deGetTransparency());

            if (xChunk == width) break;

            sx += (opSign * xChunk);
            dx += (opSign * xChunk);
            width -= xChunk;

            if (xChunk > width)
            {
                /* This is the last chunk. */
                xChunk = width;
            }
        }
    }
    else
#endif
    {
        ddk768_deWaitForNotBusy();

        POKE_32(DE_SOURCE,
            FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE) |
            FIELD_VALUE(0, DE_SOURCE, X_K1, sx)   |
            FIELD_VALUE(0, DE_SOURCE, Y_K2, sy));
        POKE_32(DE_DESTINATION,
            FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
            FIELD_VALUE(0, DE_DESTINATION, X,    dx)  |
            FIELD_VALUE(0, DE_DESTINATION, Y,    dy));
        POKE_32(DE_DIMENSION,
            FIELD_VALUE(0, DE_DIMENSION, X,    width) |
            FIELD_VALUE(0, DE_DIMENSION, Y_ET, height));

        de_ctrl = 
            FIELD_VALUE(0, DE_CONTROL, ROP, rop2) |
            FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2) |
            FIELD_SET(0, DE_CONTROL, COMMAND, BITBLT) |
            ((nDirection == RIGHT_TO_LEFT) ? 
            FIELD_SET(0, DE_CONTROL, DIRECTION, RIGHT_TO_LEFT)
            : FIELD_SET(0, DE_CONTROL, DIRECTION, LEFT_TO_RIGHT)) |
            FIELD_SET(0, DE_CONTROL, STATUS, START);

        POKE_32(DE_CONTROL, de_ctrl | ddk768_deGetTransparency());
    }

    return 0;
}
#if 0

/*
 * System Memory to Video Memory data transfer.
 * Only works in D, S, ~D, and ~S ROP.
 */
long ddk768_deSystemMem2VideoMemBusMasterBlt(
    unsigned char *pSBase,  /* Address of source in the system memory.
                               The memory must be a continuous physical address. */
    unsigned long sPitch,   /* Pitch value of source surface in BYTE */
    unsigned long sx,
    unsigned long sy,       /* Starting coordinate of source surface */
    unsigned long dBase,    /* Address of destination in frame buffer */
    unsigned long dPitch,   /* Pitch value of destination surface in BYTE */
    unsigned long bpp,      /* Color depth of destination surface */
    unsigned long dx,
    unsigned long dy,       /* Starting coordinate of destination surface */
    unsigned long width, 
    unsigned long height,   /* width and height of rectangle in pixel value */
    unsigned long rop2      /* ROP value */
)
{
    unsigned long de_ctrl, bytePerPixel;
    unsigned long value, pciMasterBaseAddress;
    long opSign;

    if (ddk768_deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* ddk768_deReset(); */
    }

    bytePerPixel = bpp/8;
    de_ctrl = 0;

    /* Note:
       DE_FOREGROUND are DE_BACKGROUND are don't care.
       DE_COLOR_COMPARE and DE_COLOR_COMPARE_MAKS are set by set ddk768_deSetTransparency().
    */

    /* 2D Source Base.
       It is an address offset (128 bit aligned) from the given PCI Master base address
    */
    /* Set 2D Source Base Address */
    value = FIELD_VALUE(0, DE_WINDOW_SOURCE_BASE, ADDRESS, (unsigned long)pSBase);        
    pciMasterBaseAddress = ((unsigned long)pSBase & 0xFC000000) >> 24;
    POKE_32(PCI_MASTER_BASE, FIELD_VALUE(0, PCI_MASTER_BASE, ADDRESS, pciMasterBaseAddress));
    POKE_32(DE_WINDOW_SOURCE_BASE, value);

    /* 2D Destination Base.
       It is an address offset (128 bit aligned) from the beginning of frame buffer.
    */
    POKE_32(DE_WINDOW_DESTINATION_BASE, dBase);

    /* Program pitch (distance between the 1st points of two adjacent lines).
       Note that input pitch is BYTE value, but the 2D Pitch register uses
       pixel values. Need Byte to pixel convertion.
    */
    POKE_32(DE_PITCH,
        FIELD_VALUE(0, DE_PITCH, DESTINATION, (dPitch/bytePerPixel)) |
        FIELD_VALUE(0, DE_PITCH, SOURCE,      (sPitch/bytePerPixel)));

    /* Screen Window width in Pixels.
       2D engine uses this value to calculate the linear address in frame buffer for a given point.
    */
    POKE_32(DE_WINDOW_WIDTH,
        FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, (dPitch/bytePerPixel)) |
        FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE,      (sPitch/bytePerPixel)));

    /* Set the pixel format of the destination */
    ddk768_deSetPixelFormat(bpp);
    
#ifdef ENABLE_192_BYTES_PATCH
    /* This bug is fixed in SM718 for 16 and 32 bpp. However, in 8-bpp, the problem still exists. 
       The Version AA also have this problem on higher clock with 32-bit memory data bus, 
       therefore, it needs to be enabled here. 
       In version AA, the problem happens on the following configurations:
        1. M2XCLK = 336MHz w/ 32-bit, MCLK = 112MHz, and color depth set to 32bpp
        2. M2XCLK = 336MHz w/ 32-bit, MCLK = 84MHz, and color depth set to 16bpp or 32bpp.
       Somehow, the problem does not appears in 64-bit memory setting.
     */

    /* Workaround for 192 byte requirement when ROP is not COPY */
    if ((rop2 != ROP2_COPY) && ((width * bytePerPixel) > 192))
    {
        /* Perform the ROP2 operation in chunks of (xWidth * nHeight) */
        unsigned long xChunk = 192 / bytePerPixel; /* chunk width is in pixels */
        
        //DDKDEBUGPRINT((DE_LEVEL, "ROP != ROP_COPY, width * bytePerPixel = %x (> 192 bytes)\n", width * bytePerPixel));

        while (1)
        {
            ddk768_deWaitForNotBusy();
            POKE_32(DE_SOURCE,
                FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE) |
                FIELD_VALUE(0, DE_SOURCE, X_K1, sx)   |
                FIELD_VALUE(0, DE_SOURCE, Y_K2, sy));
                
            POKE_32(DE_DESTINATION,
                FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
                FIELD_VALUE(0, DE_DESTINATION, X,    dx)  |
                FIELD_VALUE(0, DE_DESTINATION, Y,    dy));
                
            POKE_32(DE_DIMENSION,
                FIELD_VALUE(0, DE_DIMENSION, X,    xChunk) |
                FIELD_VALUE(0, DE_DIMENSION, Y_ET, height));

            de_ctrl = 
                FIELD_VALUE(0, DE_CONTROL, ROP, rop2) |
                FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2) |
                FIELD_SET(0, DE_CONTROL, COMMAND, BITBLT) |
                FIELD_SET(0, DE_CONTROL, DIRECTION, LEFT_TO_RIGHT) |
                FIELD_SET(0, DE_CONTROL, STATUS, START);

            POKE_32(DE_CONTROL, de_ctrl | ddk768_deGetTransparency());

            if (xChunk == width) break;

            sx += xChunk;
            dx += xChunk;
            width -= xChunk;

            if (xChunk > width)
            {
                /* This is the last chunk. */
                xChunk = width;
            }
        }
    }
    else
#endif
    {
        ddk768_deWaitForNotBusy();

        POKE_32(DE_SOURCE,
            FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE) |
            FIELD_VALUE(0, DE_SOURCE, X_K1, sx)   |
            FIELD_VALUE(0, DE_SOURCE, Y_K2, sy));
        POKE_32(DE_DESTINATION,
            FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
            FIELD_VALUE(0, DE_DESTINATION, X,    dx)  |
            FIELD_VALUE(0, DE_DESTINATION, Y,    dy));
        POKE_32(DE_DIMENSION,
            FIELD_VALUE(0, DE_DIMENSION, X,    width) |
            FIELD_VALUE(0, DE_DIMENSION, Y_ET, height));

        de_ctrl = 
            FIELD_VALUE(0, DE_CONTROL, ROP, rop2) |
            FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2) |
            FIELD_SET(0, DE_CONTROL, COMMAND, BITBLT) |
            FIELD_SET(0, DE_CONTROL, DIRECTION, LEFT_TO_RIGHT) |
            FIELD_SET(0, DE_CONTROL, STATUS, START);

        POKE_32(DE_CONTROL, de_ctrl | ddk768_deGetTransparency());
    }

    return 0;
}
#endif
/* 
 * System memory to Video memory data transfer
 * Note: 
 *         We also call it HOST Blt.
 *        This function is a one to one transfer without stretching or 
 *        mono expansion.
 */
long ddk768_deSystemMem2VideoMemBlt(
    unsigned char *pSrcbuf, /* pointer to source data in system memory */
    long srcDelta,          /* width (in Bytes) of the source data, +ive means top down and -ive mean button up */
    unsigned long dBase,    /* Address of destination: offset in frame buffer */
    unsigned long dPitch,   /* Pitch value of destination surface in BYTE */
    unsigned long bpp,      /* Color depth of destination surface */
    unsigned long dx,
    unsigned long dy,       /* Starting coordinate of destination surface */
    unsigned long width, 
    unsigned long height,   /* width and height of rectange in pixel value */
    unsigned long rop2      /* ROP value */
)
{
    unsigned long bytePerPixel;
    unsigned long ulBytesPerScan;
    unsigned long ul8BytesPerScan;
    unsigned long ulBytesRemain;
    unsigned long de_ctrl = 0;
    unsigned char ajRemain[8];
    long i, j;

    bytePerPixel = bpp/8;

    /* HOST blt data port must take multiple of 8 bytes as input.
       If the source width does not match that requirement,
       we need to split it into two portions. The first portion
       is 8 byte multiple. The 2nd portion is the remaining bytes.
       The remaining bytes will be buffered to an 8 byte array and
       and send it to the host blt data port.
    */
    //ulBytesPerScan = width * bpp / 8;
    ulBytesPerScan = width  / 8;
    ul8BytesPerScan = ulBytesPerScan & ~7;
    ulBytesRemain = ulBytesPerScan & 7;

    /* Program 2D Drawing Engine */
    if (ddk768_deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* ddk768_deReset(); */
    }

    /* 2D Source Base.
       Use 0 for HOST Blt.
    */
    POKE_32(DE_WINDOW_SOURCE_BASE, 0);

    /* 2D Destination Base.
       It is an address offset (128 bit aligned) from the beginning of frame buffer.
    */
    POKE_32(DE_WINDOW_DESTINATION_BASE, dBase);

    /* Program pitch (distance between the 1st points of two adjacent lines).
       Note that input pitch is BYTE value, but the 2D Pitch register uses
       pixel values. Need Byte to pixel convertion.
    */
    POKE_32(DE_PITCH,
        FIELD_VALUE(0, DE_PITCH, DESTINATION, dPitch/bytePerPixel) |
        FIELD_VALUE(0, DE_PITCH, SOURCE,      dPitch/bytePerPixel));

    /* Screen Window width in Pixels.
       2D engine uses this value to calculate the linear address in frame buffer for a given point.
    */
    POKE_32(DE_WINDOW_WIDTH,
        FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, (dPitch/bytePerPixel)) |
        FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE,      (dPitch/bytePerPixel)));

    /* Note: For 2D Source in Host Write, only X_K1 field is needed, and Y_K2 field is not used.
             For 1 to 1 bitmap transfer, use 0 for X_K1 means source alignment from byte 0. */
    POKE_32(DE_SOURCE,
        FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE) |
        FIELD_VALUE(0, DE_SOURCE, X_K1, 0)       |
        FIELD_VALUE(0, DE_SOURCE, Y_K2, 0));

    POKE_32(DE_DESTINATION,
        FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
        FIELD_VALUE(0, DE_DESTINATION, X,    dx)    |
        FIELD_VALUE(0, DE_DESTINATION, Y,    dy));

    POKE_32(DE_DIMENSION,
        FIELD_VALUE(0, DE_DIMENSION, X,    width) |
        FIELD_VALUE(0, DE_DIMENSION, Y_ET, height));
        
    /* Set the pixel format of the destination */
    ddk768_deSetPixelFormat(bpp);

    de_ctrl = 
        FIELD_VALUE(0, DE_CONTROL, ROP, rop2)         |
        FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2)    |
        FIELD_SET(0, DE_CONTROL, COMMAND, HOST_WRITE) |
        FIELD_SET(0, DE_CONTROL, HOST, COLOR)         |
        FIELD_SET(0, DE_CONTROL, STATUS, START);

    POKE_32(DE_CONTROL, de_ctrl | ddk768_deGetTransparency());

    /* Write bitmap/image data (line by line) to 2D Engine data port */
    for (i = 0; i < height; i++)
    {
        /* For each line, send the data in chunks of 4 bytes. */
        for (j=0; j < (ul8BytesPerScan/4);  j++)
            POKE_32(DE_DATA_PORT, *(unsigned long *)(pSrcbuf + (j * 4)));

        if (ulBytesRemain)
        {
            memcpy(ajRemain, pSrcbuf+ul8BytesPerScan, ulBytesRemain);
            POKE_32(DE_DATA_PORT, *(unsigned long *)ajRemain);
            POKE_32(DE_DATA_PORT, *(unsigned long *)(ajRemain+4));
        }

        pSrcbuf += srcDelta;
    }

    return 0;
}

/*
 * System memory to Video memory monochrome expansion.
 * Source is monochrome image in system memory.
 * This function expands the monochrome data to color image in video memory.
 */
long ddk768_deSystemMem2VideoMemMonoBlt(
unsigned char *pSrcbuf, /* pointer to start of source buffer in system memory */
long srcDelta,          /* Pitch value (in bytes) of the source buffer, +ive means top down and -ive mean button up */
unsigned long startBit, /* Mono data can start at any bit in a byte, this value should be 0 to 7 */
unsigned long dBase,    /* Address of destination: offset in frame buffer */
unsigned long dPitch,   /* Pitch value of destination surface in BYTE */
unsigned long bpp,      /* Color depth of destination surface */
unsigned long dx,
unsigned long dy,       /* Starting coordinate of destination surface */
unsigned long width, 
unsigned long height,   /* width and height of rectange in pixel value */
unsigned long fColor,   /* Foreground color (corresponding to a 1 in the monochrome data */
unsigned long bColor,   /* Background color (corresponding to a 0 in the monochrome data */
unsigned long rop2)     /* ROP value */
{
    unsigned long bytePerPixel;
    unsigned long ulBytesPerScan;
    unsigned long ul4BytesPerScan;
    unsigned long ulBytesRemain;
    unsigned long de_ctrl = 0;
    unsigned char ajRemain[4];
    long i, j;

    bytePerPixel = bpp/8;

    startBit &= 7; /* Just make sure the start bit is within legal range */
    ulBytesPerScan = (width + startBit + 7) / 8;
    ul4BytesPerScan = ulBytesPerScan & ~3;
    ulBytesRemain = ulBytesPerScan & 3;

    if (ddk768_deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* ddk768_deReset(); */
    }

    /* 2D Source Base.
       Use 0 for HOST Blt.
    */
    POKE_32(DE_WINDOW_SOURCE_BASE, 0);

    /* 2D Destination Base.
       It is an address offset (128 bit aligned) from the beginning of frame buffer.
    */
    POKE_32(DE_WINDOW_DESTINATION_BASE, dBase);

    /* Program pitch (distance between the 1st points of two adjacent lines).
       Note that input pitch is BYTE value, but the 2D Pitch register uses
       pixel values. Need Byte to pixel convertion.
    */
    POKE_32(DE_PITCH,
        FIELD_VALUE(0, DE_PITCH, DESTINATION, dPitch/bytePerPixel) |
        FIELD_VALUE(0, DE_PITCH, SOURCE,      dPitch/bytePerPixel));

    /* Screen Window width in Pixels.
       2D engine uses this value to calculate the linear address in frame buffer for a given point.
    */
    POKE_32(DE_WINDOW_WIDTH,
        FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, (dPitch/bytePerPixel)) |
        FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE,      (dPitch/bytePerPixel)));

    /* Note: For 2D Source in Host Write, only X_K1_MONO field is needed, and Y_K2 field is not used.
             For mono bitmap, use startBit for X_K1. */
    POKE_32(DE_SOURCE,
        FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE)       |
        FIELD_VALUE(0, DE_SOURCE, X_K1_MONO, startBit));

    POKE_32(DE_DESTINATION,
        FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
        FIELD_VALUE(0, DE_DESTINATION, X,    dx)    |
        FIELD_VALUE(0, DE_DESTINATION, Y,    dy));

    POKE_32(DE_DIMENSION,
        FIELD_VALUE(0, DE_DIMENSION, X,    width) |
        FIELD_VALUE(0, DE_DIMENSION, Y_ET, height));

    POKE_32(DE_FOREGROUND, fColor);
    POKE_32(DE_BACKGROUND, bColor);
    
    /* Set the pixel format of the destination */
    ddk768_deSetPixelFormat(bpp);

    de_ctrl = FIELD_VALUE(0, DE_CONTROL, ROP, rop2)         |
              FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2)    |
              FIELD_SET(0, DE_CONTROL, COMMAND, HOST_WRITE) |
              FIELD_SET(0, DE_CONTROL, HOST, MONO)          |
              FIELD_SET(0, DE_CONTROL, STATUS, START);

    POKE_32(DE_CONTROL, de_ctrl | ddk768_deGetTransparency());

    /* Write MONO data (line by line) to 2D Engine data port */
    for (i=0; i<height; i++)
    {
        /* For each line, send the data in chunks of 4 bytes */
        for (j=0; j<(ul4BytesPerScan/4); j++)
        {
            POKE_32(DE_DATA_PORT, *(unsigned long *)(pSrcbuf + (j * 4)));
        }

        if (ulBytesRemain)
        {
            memcpy(ajRemain, pSrcbuf+ul4BytesPerScan, ulBytesRemain);
            POKE_32(DE_DATA_PORT, *(unsigned long *)ajRemain);
        }

        pSrcbuf += srcDelta;
    }

    return 0;
}
