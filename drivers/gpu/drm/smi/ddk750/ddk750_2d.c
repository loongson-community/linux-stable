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

#include "ddk750_defs.h"
#include "ddk750_hardware.h"
#include "ddk750_chip.h"
#include "ddk750_power.h"
#include "ddk750_sw2d.h"
#include "ddk750_2d.h"
#include "ddk750_help.h"
#include "ddk750_regde.h"

/* Flag to enable the 192 bytes patch to workaround the 2D errata, where the engine
   will draws incorrectly for BITBLT function that involves READ from memory. 
   Currently, this definition flag is only used for testing. */
#define ENABLE_192_BYTES_PATCH

/* Static macro */
#define BYTE_PER_PIXEL(bpp)         (bpp / 8)

/*
 * 2D Engine Initialization.
 * This function must be called before other 2D functions.
 * Assumption: A specific video mode has been properly set up.
 */
void ddk750_deInit(void)
{
    enable2DEngine(1);

    deReset(); /* Just be sure no left-over operations from other applications */

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
    deSetClipping(0, 0, 0, 0, 0);
    deSetTransparency(0, 0, 0, 0);
}

/*
 * Reset 2D engine by 
 * 1) Aborting the current 2D operation.
 * 2) Re-enable 2D engine to normal state.
 */
void deReset()
{
    unsigned long sysCtrl;
	logical_chip_type_t chipType = ddk750_getChipType();

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
}
 
/*
 * Wait until 2D engine is not busy.
 * All 2D operations are recommand to check 2D engine idle before start.
 *
 * Return: 0 = return because engine is idle and normal.
 *        -1 = return because time out (2D engine may have problem).
 */
long deWaitForNotBusy(void)
{
	unsigned long dwVal;
	logical_chip_type_t chipType;
    unsigned long i = 0x100000;

	chipType = ddk750_getChipType();

	if (chipType == SM750 || chipType == SM718)
	{
    	while (i--)
    	{
    	    dwVal = PEEK_32(SYSTEM_CTRL);
    	    if ((FIELD_GET(dwVal, SYSTEM_CTRL, DE_STATUS)      == SYSTEM_CTRL_DE_STATUS_IDLE) &&
    	        (FIELD_GET(dwVal, SYSTEM_CTRL, DE_FIFO)        == SYSTEM_CTRL_DE_FIFO_EMPTY) &&
    	        (FIELD_GET(dwVal, SYSTEM_CTRL, CSC_STATUS)     == SYSTEM_CTRL_CSC_STATUS_IDLE) &&
    	        (FIELD_GET(dwVal, SYSTEM_CTRL, DE_MEM_FIFO)    == SYSTEM_CTRL_DE_MEM_FIFO_EMPTY))
    	    {
    	        return 0; /* Return because engine idle */
    	    }
    	}

    	return -1; /* Return because time out */
	}
    else /* For SM750LE & SM750HS */
    {
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
}

#if 0 /* Cheok_2013_0118: Delete this funciton since no other functions are calling it. */
/* deWaitIdle() function.
 *
 * This function is same as deWaitForNotBusy(), except application can
 * input the maximum number of times that this function will check 
 * the idle register.
 *
 * Its usage is mainly for debugging purpose.
 *
 * Wait until 2D engine is not busy.
 * All 2D operations are recommand to check 2D engine idle before start.
 *
 * Return: 0 = return because engine is idle and normal.
 *        -1 = return because time out (2D engine may have problem).
 */
long deWaitIdle(unsigned long i)
{
	unsigned long dwVal;

    if (ddk750_getChipType() == SM750LE)
    {
#if 0
		while (i--)
		{
			dwVal = PEEK_32(DE_CONTROL);
			if (FIELD_GET(dwVal, DE_CONTROL, STATUS) == DE_CONTROL_STATUS_STOP)
				return 0;
		}
		return -1;
#else
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

        return -1; /* Return because time out */
#endif
    }

    while (i--)
    {
        dwVal = PEEK_32(SYSTEM_CTRL);
        if ((FIELD_GET(dwVal, SYSTEM_CTRL, DE_STATUS)      == SYSTEM_CTRL_DE_STATUS_IDLE) &&
            (FIELD_GET(dwVal, SYSTEM_CTRL, DE_FIFO)        == SYSTEM_CTRL_DE_FIFO_EMPTY) &&
            (FIELD_GET(dwVal, SYSTEM_CTRL, CSC_STATUS)     == SYSTEM_CTRL_CSC_STATUS_IDLE) &&
            (FIELD_GET(dwVal, SYSTEM_CTRL, DE_MEM_FIFO)    == SYSTEM_CTRL_DE_MEM_FIFO_EMPTY))
        {
            return 0; /* Return because engine idle */
        }
    }

    return -1; /* Return because time out */
}
#endif

/*
 * This function enable/disable clipping area for the 2d engine.
 * Note that the clipping area is always rectangular.
 * 
 */
long deSetClipping(
unsigned long enable, /* 0 = disable clipping, 1 = enable clipping */
unsigned long x1,     /* x1, y1 is the upper left corner of the clipping area */
unsigned long y1,     /* Note that the region includes x1 and y1 */
unsigned long x2,     /* x2, y2 is the lower right corner of the clippiing area */
unsigned long y2)     /* Note that the region will not include x2 and y2 */
{
    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
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
long deSetTransparency(
unsigned long enable,     /* 0 = disable, 1 = enable transparency feature */
unsigned long tSelect,    /* 0 = compare source, 1 = compare destination */
unsigned long tMatch,     /* 0 = Opaque mode, 1 = transparent mode */
unsigned long ulColor)    /* Color to compare. */
{
    unsigned long de_ctrl;

    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
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
unsigned long deGetTransparency(void)
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
void deSetPixelFormat(
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
long deRectFill( /*resolution_t resolution, point_t p0, point_t p1, unsigned long color, unsigned long rop2)*/
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
    
    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
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
    deSetPixelFormat(bpp);

#ifdef ENABLE_192_BYTES_PATCH
    /* Workaround for 192 byte requirement when ROP is not COPY */
    if (((rop2 != ROP2_COPY) || (rop2 != ROP2_Sn) || (rop2 != ROP2_Dn) || 
         (rop2 != ROP2_D) || (rop2 != ROP2_BLACK) || (rop2 != ROP2_WHITE)) && 
        ((width * bytePerPixel) > 192))
    {
        /* Perform the ROP2 operation in chunks of (xWidth * nHeight) */
        unsigned long xChunk = 192 / bytePerPixel; /* chunk width is in pixels */
        
//        DDKDEBUGPRINT((DE_LEVEL, "ROP != ROP_COPY, width * bytePerPixel = %x (> 192 bytes)\n", width * bytePerPixel));

        while (1)
        {
            deWaitForNotBusy();
            
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

            POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());

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

        POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());
    }
    
    return 0;
}

/*
 * This function uses 2D engine to draw a trapezoid with a specific color.
 * The filled area includes the starting points.
 */
long deStartTrapezoidFill(
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

    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
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
    deSetPixelFormat(bpp);
    
    /* Set the line length and width */
    POKE_32(DE_DIMENSION,
        FIELD_VALUE(0, DE_DIMENSION, X,    length) |
        FIELD_VALUE(0, DE_DIMENSION, Y_ET, 0));

    /* Enable the 2D Engine. */
    POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());
    
    return 0;
}

/* 
 * Function to continue drawing a line using Trapezoid Fill method.
 */
long deNextTrapezoidFill(
    unsigned long x,            /* Starting X location. */
    unsigned long length        /* Line length */
)
{
    unsigned long de_ctrl;
    
    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
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
long deStopTrapezoidFill(void)
{
    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
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
long ddk750_deVideoMem2VideoMemBlt(
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

    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
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
       DE_COLOR_COMPARE and DE_COLOR_COMPARE_MAKS are set by set deSetTransparency().
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
    deSetPixelFormat(bpp);
    
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
            deWaitForNotBusy();
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

            POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());

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
        deWaitForNotBusy();

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

        POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());
    }

    return 0;
}

/*
 * System Memory to Video Memory data transfer.
 * Only works in D, S, ~D, and ~S ROP.
 */
long deSystemMem2VideoMemBusMasterBlt(
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

    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
    }

    bytePerPixel = bpp/8;
    de_ctrl = 0;

    /* Note:
       DE_FOREGROUND are DE_BACKGROUND are don't care.
       DE_COLOR_COMPARE and DE_COLOR_COMPARE_MAKS are set by set deSetTransparency().
    */

    /* 2D Source Base.
       It is an address offset (128 bit aligned) from the given PCI Master base address
    */
    /* Set 2D Source Base Address */
    value = FIELD_VALUE(0, DE_WINDOW_SOURCE_BASE, ADDRESS, (unsigned long)pSBase);        
    pciMasterBaseAddress = ((unsigned long)pSBase & 0xFC000000) >> 24;
    POKE_32(PCI_MASTER_BASE, FIELD_VALUE(0, PCI_MASTER_BASE, ADDRESS, pciMasterBaseAddress));
    value = FIELD_SET(value, DE_WINDOW_SOURCE_BASE, EXT, EXTERNAL);
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
    deSetPixelFormat(bpp);
    
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
            deWaitForNotBusy();
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

            POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());

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
        deWaitForNotBusy();

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

        POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());
    }

    return 0;
}

/* 
 * System memory to Video memory data transfer
 * Note: 
 *         We also call it HOST Blt.
 *        This function is a one to one transfer without stretching or 
 *        mono expansion.
 */
long deSystemMem2VideoMemBlt(
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
    ulBytesPerScan = width * bpp / 8;
    ul8BytesPerScan = ulBytesPerScan & ~7;
    ulBytesRemain = ulBytesPerScan & 7;

    /* Program 2D Drawing Engine */
    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
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
    deSetPixelFormat(bpp);

    de_ctrl = 
        FIELD_VALUE(0, DE_CONTROL, ROP, rop2)         |
        FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2)    |
        FIELD_SET(0, DE_CONTROL, COMMAND, HOST_WRITE) |
        FIELD_SET(0, DE_CONTROL, HOST, COLOR)         |
        FIELD_SET(0, DE_CONTROL, STATUS, START);

    POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());

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
long deSystemMem2VideoMemMonoBlt(
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

    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
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
    deSetPixelFormat(bpp);

    de_ctrl = FIELD_VALUE(0, DE_CONTROL, ROP, rop2)         |
              FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2)    |
              FIELD_SET(0, DE_CONTROL, COMMAND, HOST_WRITE) |
              FIELD_SET(0, DE_CONTROL, HOST, MONO)          |
              FIELD_SET(0, DE_CONTROL, STATUS, START);

    POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());

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

#if 0   /* Does not work. Errata. */
/*
 * System memory to Video memory monochrome expansion.
 * Source is the starting location of monochrome image in System memory.
 * This function expands the monochrome data to color image.
 *
 * Note:
 * This fnnction can be used to diaplay a mono-font charater to the screen.
 * Input source points to the starting location of the font character.
 */
long deSystemMem2VideoMemMonoBusMasterBlt(
    unsigned char *pSBase,  /* Address of mono-chrome source data in frame buffer */
    unsigned long sPitch,   /* Pitch value (in bytes) of the source buffer. */
    unsigned long startBit, /* Mono data can start at any bit in a byte, this value should be 0 to 7 */
    unsigned long dBase,    /* Base address of destination in frame buffer */
    unsigned long dPitch,   /* Pitch value of destination surface in BYTE */
    unsigned long bpp,      /* Color depth of destination surface */
    unsigned long dx,
    unsigned long dy,       /* Starting coordinate of destination surface */
    unsigned long width,    /* width of mono-chrome picture in pixel value */
    unsigned long height,   /* height of mono-chrome picture in pixel value */
    unsigned long fColor,   /* Foreground color (corresponding to a 1 in the monochrome data */
    unsigned long bColor,   /* Background color (corresponding to a 0 in the monochrome data */
    unsigned long rop2      /* ROP value */
)
{
    unsigned long bytePerPixel, de_ctrl, packed;
    unsigned long value, pciMasterBaseAddress;

    bytePerPixel = bpp/8;

    switch ( width )
    {
        case 8:
            packed = DE_CONTROL_MONO_DATA_8_PACKED;
            break;
        case 16:
            packed = DE_CONTROL_MONO_DATA_16_PACKED;
            break;
        case 32:
            packed = DE_CONTROL_MONO_DATA_32_PACKED;
            break;
        default:
            packed = DE_CONTROL_MONO_DATA_NOT_PACKED;
    }

    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
    }

    /* Note:
       DE_COLOR_COMPARE and DE_COLOR_COMPARE_MAKS are set by set deSetTransparency().
    */

    /* Monochrome source data in system memory.
       It is an address offset (128 bit aligned) from the given PCI Master base address
    */
    /* Set 2D Source Base Address */
    value = FIELD_VALUE(0, DE_WINDOW_SOURCE_BASE, ADDRESS, (unsigned long)pSBase);        
    pciMasterBaseAddress = ((unsigned long)pSBase & 0xFC000000) >> 24;
    POKE_32(PCI_MASTER_BASE, FIELD_VALUE(0, PCI_MASTER_BASE, ADDRESS, pciMasterBaseAddress));
    value = FIELD_SET(value, DE_WINDOW_SOURCE_BASE, EXT, EXTERNAL);
    POKE_32(DE_WINDOW_SOURCE_BASE, value);

    /* 2D Destination Base.
       It is an address offset (128 bit aligned) from the beginning of frame buffer.
    */
    POKE_32(DE_WINDOW_DESTINATION_BASE, dBase);

    /* Program pitch (distance between the 1st points of two adjacent lines).
       Note that input pitch is BYTE value, but the 2D Pitch register uses
       pixel values. Need Byte to pixel conversion.
    */
    POKE_32(DE_PITCH,
        FIELD_VALUE(0, DE_PITCH, DESTINATION, (dPitch/bytePerPixel)) |
        FIELD_VALUE(0, DE_PITCH, SOURCE,      1));

    /* Screen Window width in Pixels.
       2D engine uses this value to calculate the linear address in frame buffer for a given point.
    */
    POKE_32(DE_WINDOW_WIDTH,
        FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, (dPitch/bytePerPixel)) |
        FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE,      0)); /* Source window width is don't care */

    POKE_32(DE_SOURCE,
        FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE) |
        FIELD_VALUE(0, DE_SOURCE, X_K1, startBit)   |
        FIELD_VALUE(0, DE_SOURCE, Y_K2, 0));

    POKE_32(DE_DESTINATION,
        FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
        FIELD_VALUE(0, DE_DESTINATION, X,    dx)  |
        FIELD_VALUE(0, DE_DESTINATION, Y,    dy));

    POKE_32(DE_DIMENSION,
        FIELD_VALUE(0, DE_DIMENSION, X,    width) |
        FIELD_VALUE(0, DE_DIMENSION, Y_ET, height));

    POKE_32(DE_FOREGROUND, fColor);
    POKE_32(DE_BACKGROUND, bColor);
    
    /* Set the pixel format of the destination */
    deSetPixelFormat(bpp);

    de_ctrl = 
        FIELD_VALUE(0, DE_CONTROL, ROP, rop2) |
        FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2) |
        FIELD_SET(0, DE_CONTROL, COMMAND, FONT) |
        FIELD_SET(0, DE_CONTROL, HOST, MONO) |
        FIELD_VALUE(0, DE_CONTROL, MONO_DATA, packed) |
        FIELD_SET(0, DE_CONTROL, STATUS, START);

    POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());

    return 0;
}
#endif


/*
 * Video memory to Video memory monochrome expansion.
 * Source is the starting location of monochrome image in Video memory.
 * This function expands the monochrome data to color image.
 *
 * Note:
 * This fnnction can be used to diaplay a mono-font charater to the screen.
 * Input source points to the starting location of the font character.
 */
long deVideoMem2VideoMemMonoBlt(
unsigned long sBase,  /* Address of mono-chrome source data in frame buffer */
unsigned long dBase,  /* Base address of destination in frame buffer */
unsigned long dPitch, /* Pitch value of destination surface in BYTE */
unsigned long bpp,    /* Color depth of destination surface */
unsigned long dx,
unsigned long dy,     /* Starting coordinate of destination surface */
unsigned long width,  /* width of mono-chrome picture in pixel value */
unsigned long height, /* height of mono-chrome picture in pixel value */
unsigned long fColor, /* Foreground color (corresponding to a 1 in the monochrome data */
unsigned long bColor, /* Background color (corresponding to a 0 in the monochrome data */
unsigned long rop2)   /* ROP value */
{
    unsigned long bytePerPixel, de_ctrl, packed;

    bytePerPixel = bpp/8;

    switch ( width )
    {
        case 8:
            packed = DE_CONTROL_MONO_DATA_8_PACKED;
            break;
        case 16:
            packed = DE_CONTROL_MONO_DATA_16_PACKED;
            break;
        case 32:
            packed = DE_CONTROL_MONO_DATA_32_PACKED;
            break;
        default:
            packed = DE_CONTROL_MONO_DATA_NOT_PACKED;
    }

    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
    }

    /* Note:
       DE_COLOR_COMPARE and DE_COLOR_COMPARE_MAKS are set by set deSetTransparency().
    */

    /* Monochrome source data in frame buffer.
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
        FIELD_VALUE(0, DE_PITCH, SOURCE,      0));      /* Source pitch is don't care */

    /* Screen Window width in Pixels.
       2D engine uses this value to calculate the linear address in frame buffer for a given point.
    */
    POKE_32(DE_WINDOW_WIDTH,
        FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, (dPitch/bytePerPixel)) |
        FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE,      0 )); /* Source window width is don't care */

    POKE_32(DE_SOURCE,
        FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE) |
        FIELD_VALUE(0, DE_SOURCE, X_K1, 0)   |
        FIELD_VALUE(0, DE_SOURCE, Y_K2, 0));    /* Source data starts at location (0, 0) */

    POKE_32(DE_DESTINATION,
        FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
        FIELD_VALUE(0, DE_DESTINATION, X,    dx)  |
        FIELD_VALUE(0, DE_DESTINATION, Y,    dy));

    POKE_32(DE_DIMENSION,
        FIELD_VALUE(0, DE_DIMENSION, X,    width) |
        FIELD_VALUE(0, DE_DIMENSION, Y_ET, height));

    POKE_32(DE_FOREGROUND, fColor);
    POKE_32(DE_BACKGROUND, bColor);
    POKE_32(DE_COLOR_COMPARE, bColor);
    
    /* Set the pixel format of the destination */
    deSetPixelFormat(bpp);

    de_ctrl = 
        FIELD_VALUE(0, DE_CONTROL, ROP, rop2) |
        FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2) |
        FIELD_SET(0, DE_CONTROL, COMMAND, FONT) |
        FIELD_SET(0, DE_CONTROL, HOST, MONO) |
        FIELD_VALUE(0, DE_CONTROL, MONO_DATA, packed) |
        FIELD_SET(0, DE_CONTROL, STATUS, START);

    POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());

    return 0;
}

/*
 * Video memory to Video memory monochrome expansion.
 * 
 * Difference between this function and deVideoMem2VideoMemMonoBlt():
 * 1) Input is base address of the whole font table.
 * 2) An extra input about which character in the font table to display.
 * 3) This function demos how to use registers DE_SOURCE and
 *    DE_WINDOW_WIDTH, where they are set to 0 in deVideoMem2VideoMemMonoBlt().
 */
long deFontCacheTblMonoBlt(
unsigned long fontTblBase,/* Base address of monochrome font table in frame buffer */
unsigned long fontNumber, /* Which character in the font table, starting from 0 */
unsigned long dBase,      /* Base address of destination in frame buffer */
unsigned long dPitch,     /* Pitch value of destination surface in BYTE */
unsigned long bpp,        /* Color depth of destination surface */
unsigned long dx,
unsigned long dy,         /* Starting coordinate of destination surface */
unsigned long width,      /* width of each monochrome font in pixel value */
unsigned long height,     /* height of each monochrome font in pixel value */
unsigned long fColor,     /* Foreground color (corresponding to a 1 in the monochrome data */
unsigned long bColor,     /* Background color (corresponding to a 0 in the monochrome data */
unsigned long rop2)       /* ROP value */
{
    unsigned long bytePerPixel, distBetweenFont, de_ctrl, packed;

    bytePerPixel = bpp/8;

    /* Distance between fonts in pixel value */
    distBetweenFont = width * height;

    /* SM50x hardware requires each font character be a minimum of 128 
       pixel value apart.
       For 8x8 fonts, each font character is only 64 pixel apart.
       However, if application uses deCacheFontToFrameBuffer()
       to cache the font in video memory, each font character will be 
       stored as 128 pixels apart.
    */
    if (distBetweenFont < 128)
        distBetweenFont = 128;

    switch ( width )
    {
        case 8:
            packed = DE_CONTROL_MONO_DATA_8_PACKED;
            break;
        case 16:
            packed = DE_CONTROL_MONO_DATA_16_PACKED;
            break;
        case 32:
            packed = DE_CONTROL_MONO_DATA_32_PACKED;
            break;
        default:
            packed = DE_CONTROL_MONO_DATA_NOT_PACKED;
    }

    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
    }

    /* Note:
       DE_COLOR_COMPARE and DE_COLOR_COMPARE_MAKS are set by set deSetTransparency().
    */

    /* Base address of font table in frame buffer */
    POKE_32(DE_WINDOW_SOURCE_BASE, fontTblBase);

    /* 2D Destination Base.
       It is an address offset (128 bit aligned) from the beginning of frame buffer.
    */
    POKE_32(DE_WINDOW_DESTINATION_BASE, dBase);

    POKE_32(DE_PITCH,
        FIELD_VALUE(0, DE_PITCH, DESTINATION, (dPitch/bytePerPixel)) |
        FIELD_VALUE(0, DE_PITCH, SOURCE,      0));      /* Source pitch is don't care */

    /* Pay attention how DE_WINDOW_WIDTH:SOURCE is different from
       deVideoMem2VideoMemMonoBlt()
    */
    POKE_32(DE_WINDOW_WIDTH,
        FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, (dPitch/bytePerPixel)) |
        FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE,      distBetweenFont));

    /* Pay attention how DE_SOURCE:Y_K2 is different from 
       deVideoMem2VideoMemMonoBlt()
    */
    POKE_32(DE_SOURCE,
        FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE) |
        FIELD_VALUE(0, DE_SOURCE, X_K1, 0)   |
        FIELD_VALUE(0, DE_SOURCE, Y_K2, fontNumber));

    POKE_32(DE_DESTINATION,
        FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
        FIELD_VALUE(0, DE_DESTINATION, X,    dx)  |
        FIELD_VALUE(0, DE_DESTINATION, Y,    dy));

    POKE_32(DE_DIMENSION,
        FIELD_VALUE(0, DE_DIMENSION, X,    width) |
        FIELD_VALUE(0, DE_DIMENSION, Y_ET, height));

    POKE_32(DE_FOREGROUND, fColor);
    POKE_32(DE_BACKGROUND, bColor);
    POKE_32(DE_COLOR_COMPARE, bColor);
    
    /* Set the pixel format of the destination */
    deSetPixelFormat(bpp);

    de_ctrl = 
        FIELD_VALUE(0, DE_CONTROL, ROP, rop2) |
        FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2) |
        FIELD_SET(0, DE_CONTROL, COMMAND, FONT) |
        FIELD_SET(0, DE_CONTROL, HOST, MONO) |
        FIELD_VALUE(0, DE_CONTROL, MONO_DATA, packed) |
        FIELD_SET(0, DE_CONTROL, STATUS, START);

    POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());

    return 0;
}


/*
 * Rotation helper function.
 * 
 * This function sets the source coordinate, destination coordinate, window
 * dimension, and also the control register. This function is only used statically
 * to simplify the deRotateBlt function.
 *
 */
void deRotate(
    unsigned long sx,               /* X Coordinate of the source */
    unsigned long sy,               /* Y Coordinate of the source */
    unsigned long dx,               /* X Coordinate of the destination */
    unsigned long dy,               /* Y Coordinate of the destination */
    unsigned long width,            /* Width of the window */
    unsigned long height,           /* Height of the window */
    unsigned long de_ctrl           /* DE_CONTROL Control Value */
)
{
    /* Wait until the engine is not busy */
    deWaitForNotBusy();
                
    /* Set the source coordinate */
    POKE_32(DE_SOURCE,
        FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE) |
        FIELD_VALUE(0, DE_SOURCE, X_K1, sx) | 
        FIELD_VALUE(0, DE_SOURCE, Y_K2, sy));
        
    /* Set the destination coordinate */    
    POKE_32(DE_DESTINATION,
        FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
        FIELD_VALUE(0, DE_DESTINATION, X, dx) | 
        FIELD_VALUE(0, DE_DESTINATION, Y, dy));

    /* Set the source width and height dimension */
    POKE_32(DE_DIMENSION,
        FIELD_VALUE(0, DE_DIMENSION, X, width) | 
        FIELD_VALUE(0, DE_DIMENSION, Y_ET, height));
        
    /* Start the DE Control */
    POKE_32(DE_CONTROL, de_ctrl);
}

/*
 * Rotation Blt.
 * 
 * This function rotates an image to the screen based on the given rotation direction
 * (0, 90, 180, or 270 degree).
 * 
 * NOTE:
 *      1. The function takes care of any coordinates that pass the top and left
 *         boundary, but it does not take care of right and bottom boundary since
 *         more information is needed in order to do so. However, it seems that
 *         SM50x will take care the right and bottom by using wrap around.
 *         Depending on the implementation, this function might be modified further.
 *      2. There are 3 hardware bugs found on the rotation:
 *         a. The rotated image sometimes is not correct when using some width number
 *            due to FIFO bug. Therefore, the image is divided into segments and
 *            rotated based on segment by segment using 32/byte per pixel as the width.
 *            This value (32/byte per pixel) seems to be consistent in producing a good 
 *            rotated image.
 *         b. Rotating 0 degree using the actual Rotation BLT will have the same result
 *            as rotation 270 degree.
 *         c. Rotating 180 degree on some x destination coordinate will result in 
 *            incorrect image. To workaround this problem, two 90 degree rotations are
 *            used.
 *      3. The rop parameter might not be necessary if only one ROP operation is used.
 *         This might be deleted in the future as necessary.
 */
long deVideoMem2VideoMemRotateBlt(
    unsigned long sBase,            /* Source Base Address */
    unsigned long sPitch,           /* Source pitch */
    unsigned long sx,               /* X Coordinate of the source */
    unsigned long sy,               /* Y Coordinate of the source */
    unsigned long dBase,            /* Destination Base Address */
    unsigned long dPitch,           /* Destination pitch */
    unsigned long bpp,              /* Color depth of destination surface */
    unsigned long dx,               /* X Coordinate of the destination */ 
    unsigned long dy,               /* Y Coordinate of the destination */
    unsigned long width,            /* Width  of un-rotated image in pixel value */
    unsigned long height,           /* Height of un-rotated image in pixel value */
    rotate_dir_t rotateDirection,   /* Direction of the rotation */
    unsigned long repeatEnable,     /* Enable repeat rotation control where the
                                       drawing engine is started again every vsync */
    unsigned long rop2              /* ROP control */
)
{
    unsigned long de_ctrl = 0; 
    unsigned long tempWidth, dxTemp, dyTemp;
    unsigned long maxRotationWidth;
    
    /* Maximum rotation width BLT */
    maxRotationWidth = 32 / BYTE_PER_PIXEL(bpp);

    /* Wait for the engine to be idle */
    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */
        return -1;

        /* or */
        /* deReset(); */
    } 

    /* Return error if either the width or height is zero */ 
    if ((width == 0) || (height == 0))
        return -1;

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
        FIELD_VALUE(0, DE_PITCH, DESTINATION, dPitch / BYTE_PER_PIXEL(bpp)) |
        FIELD_VALUE(0, DE_PITCH, SOURCE, sPitch / BYTE_PER_PIXEL(bpp)));

    /* Screen Window width in Pixels.
       2D engine uses this value to calculate the linear address in frame buffer for a given point.
    */
    POKE_32(DE_WINDOW_WIDTH,
        FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, dPitch / BYTE_PER_PIXEL(bpp)) |
        FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE,      sPitch / BYTE_PER_PIXEL(bpp)));
        
    /* Set the pixel format of the destination */
    deSetPixelFormat(bpp);
       
    /* Setup Control Register */
    de_ctrl = FIELD_SET(0, DE_CONTROL, STATUS, START)    |
              FIELD_SET(0, DE_CONTROL, COMMAND, ROTATE)  |
              FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2) |
              FIELD_VALUE(0, DE_CONTROL, ROP, rop2)      |
              ((repeatEnable == 1) ? 
                    FIELD_SET(0, DE_CONTROL, REPEAT_ROTATE, ENABLE) :
                    FIELD_SET(0, DE_CONTROL, REPEAT_ROTATE, DISABLE)) |
              deGetTransparency();
       
    /* 501 Hardware cannot handle rotblits > 32 bytes. Therefore the rotation 
       should be done part by part. Note on each rotation case. */
    switch (rotateDirection)
    {
        case ROTATE_NORMAL:
#if 0   /* Enable this on if the hardware 0 degree rotation bug has been fixed */

            /* Setup rotation direction to 0 degree */
            de_ctrl |= (FIELD_SET(0, DE_CONTROL, STEP_X, POSITIVE) |
                        FIELD_SET(0, DE_CONTROL, STEP_Y, POSITIVE));

            deRotate(sx, sy, dx, dy, width, height, de_ctrl);                        
#else
            /* 
             * Due to the hardware bug on the SM750, rotate normal is simply done
             * by calling the normal bit BLT. Calling the rotation with 0 degree
             * will cause the hardware to rotate it 270 degree. 
             */
            return(ddk750_deVideoMem2VideoMemBlt(
                    sBase,
                    sPitch,
                    sx,
                    sy,
                    dBase,
                    dPitch,
                    bpp,
                    dx,
                    dy,
                    width, 
                    height,
                    rop2));
#endif
            break;

        case ROTATE_180_DEGREE:
            /* The 180 degree rotation that has problem */

            return -1; /* Don't do anything until there is a HW fix */
            
            de_ctrl |= (FIELD_SET(0, DE_CONTROL, STEP_X, NEGATIVE) |
                        FIELD_SET(0, DE_CONTROL, STEP_Y, NEGATIVE));

            deRotate(sx, sy, dx, dy, width, height, de_ctrl);
            break;

        case ROTATE_90_DEGREE:
            /* 90 degrees rotation.  Calculate destination
			   coordinates:

				*---+
				|   |       +-----+
				|   |       |     |
				|   |  -->  |     |
				|   |       |     |
				|   |       *-----+
				+---+
			*/
            /* Update the new width */
            if (dy < width)
                width = dy+1;
                
            /* Set up the rotation direction to 90 degree */                
            de_ctrl |= (FIELD_SET(0, DE_CONTROL, STEP_X, NEGATIVE) |
                        FIELD_SET(0, DE_CONTROL, STEP_Y, POSITIVE));

#if 1   /* Disable this one if the hardware bug has been fixed */

            /* Do rotation part by part based on the maxRotationWidth */
            while (width > maxRotationWidth)
	        {
                deRotate(sx, sy, dx, dy, maxRotationWidth, height, de_ctrl);

                width -= maxRotationWidth;
                sx    += maxRotationWidth;
                dy    -= maxRotationWidth;
            }
#endif
            /* Rotate the rest of the segment */
            if (width > 0)
                deRotate(sx, sy, dx, dy, width, height, de_ctrl);
                
            break;
        
        case ROTATE_270_DEGREE:
            /* 270 degrees (90 degree CW) rotation.  Calculate destination
			   coordinates:

				*---+
				|   |       +-----*
				|   |       |     |  
				|   |  -->  |     | 
				|   |       |     |
				|   |       +-----+
				+---+
			*/
            de_ctrl |= (FIELD_SET(0, DE_CONTROL, STEP_X, POSITIVE) |
                        FIELD_SET(0, DE_CONTROL, STEP_Y, NEGATIVE));
 
#if 1   /* Disable this one if the hardware bug has been fixed */

            /* Do rotation part by part based on the maxRotationWidth */            
            while (width > maxRotationWidth)
	        {
                deRotate(sx, sy, dx, dy, maxRotationWidth, height, de_ctrl);
               
                width -= maxRotationWidth;
                sx    += maxRotationWidth;
                dy    += maxRotationWidth;
            }
#endif            
            /* Update the rest of the segment */
            if (width > 0)
                deRotate(sx, sy, dx, dy, width, height, de_ctrl);
            break;
    }

    return 0;
}

/* 
 * Function to draw a vertical line.
 *
 * Note:
 *      This function is using Short Stroke line
 */
long deVerticalLine(
    unsigned long dBase,    /* Base address of destination surface counted from beginning of video frame buffer */
    unsigned long dPitch,   /* Pitch value of destination surface in BYTES */
    unsigned long bpp,      /* Color depth of destination surface: 8, 16 or 32 */
    unsigned long x,        /* Starting X Coordinate */
    unsigned long y,        /* Starting Y Coordinate */
    unsigned long length,   /* Length of the line */
    unsigned long color,    /* Color of the line */
    unsigned long rop2      /* ROP value */
)
{
    unsigned long de_ctrl;
    
    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
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

    /* Set the destination coordinate. */
    POKE_32(DE_DESTINATION,
        FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
        FIELD_VALUE(0, DE_DESTINATION, X,    x)       |
        FIELD_VALUE(0, DE_DESTINATION, Y,    y));

    /* Set the line length and width */
    POKE_32(DE_DIMENSION,
        FIELD_VALUE(0, DE_DIMENSION, X,    1) |
        FIELD_VALUE(0, DE_DIMENSION, Y_ET, length));
        
    /* Set the pixel format of the destination */
    deSetPixelFormat(bpp);

    /* Set the control register. For the Vertical line Short Stroke, the Direction Control
       should be set to 0 (which is defined as Left to Right). */
    de_ctrl = 
        FIELD_SET  (0, DE_CONTROL,  STATUS,     START)         |
        FIELD_SET  (0, DE_CONTROL,  DIRECTION,  LEFT_TO_RIGHT) |
        FIELD_SET  (0, DE_CONTROL,  MAJOR,      Y)             |
        FIELD_SET  (0, DE_CONTROL,  STEP_X,     NEGATIVE)      |
        FIELD_SET  (0, DE_CONTROL,  STEP_Y,     NEGATIVE)      |
        FIELD_SET  (0, DE_CONTROL,  LAST_PIXEL, OFF)           |
        FIELD_SET  (0, DE_CONTROL,  COMMAND,    SHORT_STROKE)  |
        FIELD_SET  (0, DE_CONTROL,  ROP_SELECT, ROP2)          |
        FIELD_VALUE(0, DE_CONTROL,  ROP,        rop2);

    POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());

    return 0;
}

/* 
 * Function to draw a horizontal line.
 *
 * Note:
 *      This function is using Short Stroke line
 */
long deHorizontalLine(
    unsigned long dBase,    /* Base address of destination surface counted from beginning of video frame buffer */
    unsigned long dPitch,   /* Pitch value of destination surface in BYTES */
    unsigned long bpp,      /* Color depth of destination surface: 8, 16 or 32 */
    unsigned long x,        /* Starting X Coordinate */
    unsigned long y,        /* Starting Y Coordinate */
    unsigned long length,   /* Length of the line */
    unsigned long color,    /* Color of the line */
    unsigned long rop2      /* ROP value */
)
{
    unsigned long de_ctrl;
    
    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
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

    /* Set the line length and width */
    POKE_32(DE_DIMENSION,
        FIELD_VALUE(0, DE_DIMENSION, X,    length) |
        FIELD_VALUE(0, DE_DIMENSION, Y_ET, 1));
        
    /* Set the pixel format of the destination */
    deSetPixelFormat(bpp);

    /* Set the control register. For the Horizontal line Short Stroke, the Direction Control
       should be set to 1 (which is defined as Right to Left). */
    de_ctrl = 
        FIELD_SET  (0, DE_CONTROL,  STATUS,     START)         |
        FIELD_SET  (0, DE_CONTROL,  DIRECTION,  RIGHT_TO_LEFT) |
        FIELD_SET  (0, DE_CONTROL,  MAJOR,      X)             |
        FIELD_SET  (0, DE_CONTROL,  STEP_X,     NEGATIVE)      |
        FIELD_SET  (0, DE_CONTROL,  STEP_Y,     NEGATIVE)      |
        FIELD_SET  (0, DE_CONTROL,  LAST_PIXEL, OFF)           |
        FIELD_SET  (0, DE_CONTROL,  COMMAND,    SHORT_STROKE)  |
        FIELD_SET  (0, DE_CONTROL,  ROP_SELECT, ROP2)          |
        FIELD_VALUE(0, DE_CONTROL,  ROP,        rop2);

    POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());
    
    return 0;
}

/* 
 * Function to draw a line.
 *
 * Note:
 *      This function is using Short Stroke Command for Vertical, Horizontal, and 
 *      Diagonal line. Other line are drawn using the Line Draw Command.
 */
long deLine(
    unsigned long dBase,    /* Base address of destination surface counted from beginning of video frame buffer */
    unsigned long dPitch,   /* Pitch value of destination surface in BYTES */
    unsigned long bpp,      /* Color depth of destination surface: 8, 16 or 32 */
    long x0,                /* Starting X Coordinate */
    long y0,                /* Starting Y Coordinate */
    long x1,                /* Ending X Coordinate */
    long y1,                /* Ending Y Coordinate */
    unsigned long color,    /* Color of the line */
    unsigned long rop2      /* ROP value */
)
{
    unsigned long de_ctrl =
        FIELD_SET  (0, DE_CONTROL, STATUS,     START)         |
        FIELD_SET  (0, DE_CONTROL, DIRECTION,  LEFT_TO_RIGHT) |
        FIELD_SET  (0, DE_CONTROL, MAJOR,      X)             |
        FIELD_SET  (0, DE_CONTROL, STEP_X,     POSITIVE)      |
        FIELD_SET  (0, DE_CONTROL, STEP_Y,     POSITIVE)      |
        FIELD_SET  (0, DE_CONTROL, LAST_PIXEL, ON)            |
        FIELD_SET  (0, DE_CONTROL, ROP_SELECT, ROP2)          |
        FIELD_VALUE(0, DE_CONTROL, ROP,        rop2);

    unsigned long dx, dy;
    
    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
    }
    
    //DDKDEBUGPRINT((DE_LEVEL, "x0=%d(%x), y0=%d(%0x)\n", x0, x0, y0, y0));
    //DDKDEBUGPRINT((DE_LEVEL, "x1=%d(%x), y1=%d(%0x)\n", x1, x1, y1, y1));
    
    /* Return error if x0 and/or y0 are negative numbers. The hardware does not take
       any negative values on these two origin line coordinate.
       When drawing with a negative x0, the line might be drawn incorrectly.
       When drawing with a negative y0, the system might reboot or hang.
     */
    if ((x0 < 0) || (y0 < 0))
    {
        //DDKDEBUGPRINT((ERROR_LEVEL, "Negative origin coordinates are not allowed (x0, y0) = (%d, %d).\n", x0, y0));
        return (-1);
    }

    /* Calculate delta X */
    if (x0 <= x1)
    {
        dx = x1 - x0;
        de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_X, NEGATIVE);
    }
    else
        dx = x0 - x1;

    /* Calculate delta Y */
    if (y0 <= y1)
    {
        dy = y1 - y0;
        de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_Y, NEGATIVE);
    }
    else
        dy = y0 - y1;

    /* Determine the major axis */
    if (dx < dy)
        de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, MAJOR, Y);        

    /*****************************************************
     * Draw the line based on the calculated information *
     *****************************************************/
     
    /* Vertical line? */
    if (x0 == x1)
        deVerticalLine(dBase, dPitch, bpp, x0, (y0 < y1) ? y0 : y1, dy + 1, color, rop2);

    /* Horizontal line? */
    else if (y0 == y1)
        deHorizontalLine(dBase, dPitch, bpp, (x0 < x1) ? x0 : x1, y0, dx + 1, color, rop2);

    else 
    {
        /****************************
         * Set the common registers *
         ****************************/
         
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
            FIELD_VALUE(0, DE_DESTINATION, X,    x0)      |
            FIELD_VALUE(0, DE_DESTINATION, Y,    y0));
        
        /* Set the pixel format of the destination */
        deSetPixelFormat(bpp);
    
        /* Diagonal line? */
        if (dx == dy)
        {
            /* Set the line length and width */
            POKE_32(DE_DIMENSION,
                FIELD_VALUE(0, DE_DIMENSION, X,    1) |
                FIELD_VALUE(0, DE_DIMENSION, Y_ET, dx + 1));

            /* Set the command register. */
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, COMMAND, SHORT_STROKE);

            POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());            
        }

        /* Generic line */
        else
        {
            long k1, k2, et, w;
            
            if (dx < dy)
            {
                k1 = 2 * dx;
                et = k1 - dy;
                k2 = et - dy;
                w  = dy + 1;
            } 
            else 
            {
                k1 = 2 * dy;
                et = k1 - dx;
                k2 = et - dx;
                w  = dx + 1;
            }
            
            //DDKDEBUGPRINT((DE_LEVEL, "k1=%d(%x), k2=%d(%x)\n", k1, k1, k2, k2));
            
            /* Return error if the value of K1 and/or K2 is/are overflowed which is 
               determined using the following condition:
                        0 < k1 < (2^13 - 1)
                    -2^13 < k2 < 0
               Bit-14 is the sign bit.
               
               Failing to follow this guidance will result in incorrect line drawing.
               On failure, please use software to draw the correct line.
             */             
            if ((k1 > 8191) || (k2 < (0 - 8192)))
            {
                //DDKDEBUGPRINT((ERROR_LEVEL, "K1=%d(0x%04x) and/or K2=%d(0x%04x) is/are overflowed.\n", k1, k1, k2, k2));
                return (-1);
            }
        
            /* Set the k1 and k2 */
            POKE_32(DE_SOURCE,
                FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE) |
                FIELD_VALUE(0, DE_SOURCE, X_K1_LINE, k1) |
                FIELD_VALUE(0, DE_SOURCE, Y_K2_LINE, k2));

            /* Set the line length and width */
            POKE_32(DE_DIMENSION,
                FIELD_VALUE(0, DE_DIMENSION, X,    w) |
                FIELD_VALUE(0, DE_DIMENSION, Y_ET, et));

            /* Set the control register. */
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, COMMAND, LINE_DRAW);

            POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());
        }
    }
    
    return 0;
}

#if 0   /* These RLE functions are not working. */
/*
 *  The following functions, that uses to draw line using RLE Strip command, are not working,
 *  as described in the following:
 *      1. Line strips can not be used to draw line vertically.
 *      2. Origin Y automatic update is not work. Only Origin X automatic update is working.
 *      3. Line counter to accelerate drawing multiple strip of the same characteristics does
 *         not work.
 *  For more information about RLE Line Strips, please refer to "Programmer's Guide to the EGA,
 *  VGA, and Super VGA Cards", third edition, page 727-730.
 */
 
/* 
 * Function to draw a line using RLE line strip method.
 */
long deStartLineStrip(
    unsigned long dBase,        /* Base address of destination surface counted from beginning of video frame buffer */
    unsigned long dPitch,       /* Pitch value of destination surface in BYTES */
    unsigned long bpp,          /* Color depth of destination surface: 8, 16 or 32 */
    unsigned long x,            /* X Coordinate */
    unsigned long y,            /* Y Coordinate */
    unsigned long length,       /* Length of the line */
    unsigned long lineCounter,  /* Line Counter */
    unsigned long direction,    /* Direction:   0 - Left to Right
                                                1 - Right to Left
                                                2 - Top to Bottom
                                                3 - Bottom to Top
                                */
    unsigned long color,        /* Color of the line */
    unsigned long rop2          /* ROP value */
)
{
    unsigned long dx, dy;
    unsigned long de_ctrl =
        FIELD_SET  (0, DE_CONTROL, STATUS,      START)      |
        FIELD_SET  (0, DE_CONTROL, QUICK_START, ENABLE)     |
        FIELD_SET  (0, DE_CONTROL, LAST_PIXEL,  OFF)        |
        FIELD_SET  (0, DE_CONTROL, COMMAND,     RLE_STRIP)  |
        FIELD_SET  (0, DE_CONTROL, ROP_SELECT,  ROP2)       |
        FIELD_VALUE(0, DE_CONTROL, ROP,         rop2);

    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
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
    deSetPixelFormat(bpp);

#if 1   /* Testing if any of the configuration combination of the following have
           any differences. */
    switch (direction)
    {
        case 0:
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, DIRECTION, LEFT_TO_RIGHT);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, MAJOR, X);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_X, POSITIVE);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_Y, POSITIVE);
            break;
        case 1:
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, DIRECTION, LEFT_TO_RIGHT);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, MAJOR, X);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_X, POSITIVE);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_Y, NEGATIVE);
            break;
        case 2:
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, DIRECTION,  LEFT_TO_RIGHT);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, MAJOR, X);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_X, NEGATIVE);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_Y, POSITIVE);
            break;
        case 3:
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, DIRECTION,  LEFT_TO_RIGHT);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, MAJOR, X);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_X, NEGATIVE);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_Y, NEGATIVE);
            break;
        case 4:
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, DIRECTION, LEFT_TO_RIGHT);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, MAJOR, Y);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_X, POSITIVE);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_Y, POSITIVE);
            break;
        case 5:
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, DIRECTION, LEFT_TO_RIGHT);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, MAJOR, Y);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_X, POSITIVE);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_Y, NEGATIVE);
            break;
        case 6:
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, DIRECTION,  LEFT_TO_RIGHT);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, MAJOR, Y);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_X, NEGATIVE);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_Y, POSITIVE);
            break;
        case 7:
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, DIRECTION,  LEFT_TO_RIGHT);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, MAJOR, Y);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_X, NEGATIVE);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_Y, NEGATIVE);
            break;
        case 8:
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, DIRECTION, RIGHT_TO_LEFT);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, MAJOR, X);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_X, POSITIVE);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_Y, POSITIVE);
            break;
        case 9:
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, DIRECTION, RIGHT_TO_LEFT);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, MAJOR, X);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_X, POSITIVE);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_Y, NEGATIVE);
            break;
        case 10:
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, DIRECTION,  RIGHT_TO_LEFT);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, MAJOR, X);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_X, NEGATIVE);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_Y, POSITIVE);
            break;
        case 11:
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, DIRECTION,  RIGHT_TO_LEFT);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, MAJOR, X);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_X, NEGATIVE);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_Y, NEGATIVE);
            break;
        case 12:
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, DIRECTION, RIGHT_TO_LEFT);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, MAJOR, Y);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_X, POSITIVE);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_Y, POSITIVE);
            break;
        case 13:
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, DIRECTION, RIGHT_TO_LEFT);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, MAJOR, Y);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_X, POSITIVE);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_Y, NEGATIVE);
            break;
        case 14:
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, DIRECTION,  RIGHT_TO_LEFT);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, MAJOR, Y);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_X, NEGATIVE);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_Y, POSITIVE);
            break;
        case 15:
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, DIRECTION,  RIGHT_TO_LEFT);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, MAJOR, Y);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_X, NEGATIVE);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_Y, NEGATIVE);
            break;
    }
    
    /* Set the line length and width */
    POKE_32(DE_DIMENSION,
        FIELD_VALUE(0, DE_DIMENSION, X,    lineCounter) |
        FIELD_VALUE(0, DE_DIMENSION, Y_ET, length));
#else
    /* Determine the x & y steps direction control and major axis */
    switch (direction)
    {
        case 0:
            /* Left to Right */
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, DIRECTION, LEFT_TO_RIGHT);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, MAJOR, X);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_X, POSITIVE);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_Y, POSITIVE);
            
            /* Set the line length and width */
            POKE_32(DE_DIMENSION,
                FIELD_VALUE(0, DE_DIMENSION, X,    length) |
                FIELD_VALUE(0, DE_DIMENSION, Y_ET, lineCounter));
            break;
            
        case 1:
            /* Right to Left */
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, DIRECTION, RIGHT_TO_LEFT);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, MAJOR, X);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_X, NEGATIVE);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_Y, POSITIVE);
            
            /* Set the line length and width */
            POKE_32(DE_DIMENSION,
                FIELD_VALUE(0, DE_DIMENSION, X,    length) |
                FIELD_VALUE(0, DE_DIMENSION, Y_ET, lineCounter));
            break;
            
        case 2:
            /* Top to Bottom */
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, DIRECTION,  LEFT_TO_RIGHT);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, MAJOR, Y);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_X, POSITIVE);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_Y, POSITIVE);
            
            /* Set the line length and width */
            POKE_32(DE_DIMENSION,
                FIELD_VALUE(0, DE_DIMENSION, X,    lineCounter) |
                FIELD_VALUE(0, DE_DIMENSION, Y_ET, length));
            break;
            
        case 3:
            /* Bottom to Top */
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, DIRECTION,  RIGHT_TO_LEFT);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, MAJOR, Y);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_X, POSITIVE);
            de_ctrl = FIELD_SET(de_ctrl, DE_CONTROL, STEP_Y, NEGATIVE);
            
            /* Set the line length and width */
            POKE_32(DE_DIMENSION,
                FIELD_VALUE(0, DE_DIMENSION, X,    lineCounter) |
                FIELD_VALUE(0, DE_DIMENSION, Y_ET, length));
            break;
    }
#endif

    /* Enable the 2D Engine. */
    POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());
    
    return 0;
}

/* 
 * Function to continue drawing a line using RLE line strip method.
 */
long deNextLineStrip(
    unsigned long length,       /* Line strip length */
    unsigned long lineCounter   /* Line Counter */
)
{
    unsigned long de_ctrl;
    
    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
    }
    
    de_ctrl = PEEK_32(DE_CONTROL);
    
    if (FIELD_GET(de_ctrl, DE_CONTROL, MAJOR) == DE_CONTROL_MAJOR_X)
    {
        /* Set the line length and width */
        POKE_32(DE_DIMENSION,
            FIELD_VALUE(0, DE_DIMENSION, X,    length) |
            FIELD_VALUE(0, DE_DIMENSION, Y_ET, lineCounter));
    }
    else
    {
        /* Set the line length and width */
        POKE_32(DE_DIMENSION,
            FIELD_VALUE(0, DE_DIMENSION, X,    lineCounter) |
            FIELD_VALUE(0, DE_DIMENSION, Y_ET, length));
    }
    
    return 0;
}

/* 
 * Function to stop the RLE line strip drawing.
 * This function has to be called to end the Line Strip drawing.
 * Otherwise, the next 2D function might still use this function.
 */
long deStopLineStrip()
{
    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
    }
    
    POKE_32(DE_CONTROL, FIELD_SET(PEEK_32(DE_CONTROL), DE_CONTROL, QUICK_START, DISABLE));
    
    return 0;
}
#endif

/* Define this definition to block transparency in Alpha Blend blt since
   transparency does not work with alpha blend. */
//#define ALPHA_BLEND_BLOCK_TRANSPARENCY

/*
 * Alpha Blend Blt.
 * 
 * This function blends the source with the destination image.
 */
long deVideoMem2VideoMemAlphaBlendBlt(
    unsigned long sBase,            /* Source Base Address */
    unsigned long sPitch,           /* Source pitch */
    unsigned long sx,               /* X Coordinate of the source */
    unsigned long sy,               /* Y Coordinate of the source */
    unsigned long sWidth,           /* Source Width */
    unsigned long sHeight,          /* Source Height */
    unsigned long dBase,            /* Destination Base Address */
    unsigned long dPitch,           /* Destination pitch */
    unsigned long bpp,              /* Color depth of destination surface */
    unsigned long dx,               /* X Coordinate of the destination */ 
    unsigned long dy,               /* Y Coordinate of the destination */
    unsigned long dHeight,          /* Destination Height (only height stretch is supported) */
    unsigned long alphaValue,       /* Alpha value for Alpha Blend*/
    unsigned long rop2              /* ROP control */
)
{
    unsigned long de_ctrl = 0, transparency, bytePerPixel;
    
    bytePerPixel = bpp/8; 
    
    /* Wait for the engine to be idle */
    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */
        return -1;

        /* or */
        /* deReset(); */
    }

    /* Return error if either the width or height is zero */ 
    if ((sWidth == 0) || (sHeight == 0) || (dHeight == 0))
        return -1;
    
#ifdef ALPHA_BLEND_BLOCK_TRANSPARENCY
    /* Get the transparency settings. The transparency does not work with Alpha Blend,
       therefore, save the transparency setting, disable, and restore it back later. */
    transparency = deGetTransparency();
#endif

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
       pixel values. Need Byte to pixel conversion.
    */
    POKE_32(DE_PITCH,
        FIELD_VALUE(0, DE_PITCH, DESTINATION, dPitch / BYTE_PER_PIXEL(bpp)) |
        FIELD_VALUE(0, DE_PITCH, SOURCE, sPitch / BYTE_PER_PIXEL(bpp)));

    /* Screen Window width in Pixels.
       2D engine uses this value to calculate the linear address in frame buffer for a given point.
    */
    POKE_32(DE_WINDOW_WIDTH,
        FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, dPitch / BYTE_PER_PIXEL(bpp)) |
        FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE,      sPitch / BYTE_PER_PIXEL(bpp)));
        
    /* Set the Source Height */
    POKE_32(DE_STRETCH_FORMAT, 
        FIELD_VALUE(PEEK_32(DE_STRETCH_FORMAT), DE_STRETCH_FORMAT, SOURCE_HEIGHT, sHeight));
        
    /* Set the alpha blend value */
    POKE_32(DE_ALPHA,
        FIELD_VALUE(0, DE_ALPHA, VALUE, alphaValue));

    /* Set the pixel format of the destination */
    deSetPixelFormat(bpp);

#ifdef ENABLE_192_BYTES_PATCH
    /* Workaround for 192 byte requirement */
    if ((sWidth * bytePerPixel) > 192)
    {
        /* Perform the ROP2 operation in chunks of (xWidth * nHeight) */
        unsigned long xChunk = 192 / bytePerPixel; /* chunk width is in pixels */
        
        //DDKDEBUGPRINT((DE_LEVEL, "width * bytePerPixel = %x (> 192 bytes)\n", sWidth * bytePerPixel));

        while (1)
        {
            deWaitForNotBusy();
            
            /* Set the source coordinate */    
            POKE_32(DE_SOURCE,
                FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE) |
                FIELD_VALUE(0, DE_SOURCE, X_K1, sx)   |
                FIELD_VALUE(0, DE_SOURCE, Y_K2, sy));

            /* Set the destination coordinate */        
            POKE_32(DE_DESTINATION,
                FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
                FIELD_VALUE(0, DE_DESTINATION, X,    dx)  |
                FIELD_VALUE(0, DE_DESTINATION, Y,    dy));

            /* Set the Destination Dimension */    
            POKE_32(DE_DIMENSION,
                FIELD_VALUE(0, DE_DIMENSION, X,    xChunk) |
                FIELD_VALUE(0, DE_DIMENSION, Y_ET, dHeight));

            /* Setup Control Register */
            de_ctrl = FIELD_SET(0, DE_CONTROL, STATUS, START)    |
                      FIELD_SET(0, DE_CONTROL, COMMAND, ALPHA_BLEND)  |
                      FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2) |
                      FIELD_VALUE(0, DE_CONTROL, ROP, rop2)      |
                      ((sHeight < dHeight) ?
                            FIELD_SET(0, DE_CONTROL, STRETCH, ENABLE) :
                            FIELD_SET(0, DE_CONTROL, STRETCH, DISABLE));

            POKE_32(DE_CONTROL, de_ctrl);

            if (xChunk == sWidth)
                break;

            sx += xChunk;
            dx += xChunk;
            sWidth -= xChunk;

            if (xChunk > sWidth)
            {
                /* This is the last chunk. */
                xChunk = sWidth;
            }
        }
    }
    else
#endif
    {
        deWaitForNotBusy();
            
        /* Set the source coordinate */    
        POKE_32(DE_SOURCE,
            FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE) |
            FIELD_VALUE(0, DE_SOURCE, X_K1, sx)   |
            FIELD_VALUE(0, DE_SOURCE, Y_K2, sy));

        /* Set the destination coordinate */        
        POKE_32(DE_DESTINATION,
            FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
            FIELD_VALUE(0, DE_DESTINATION, X,    dx)  |
            FIELD_VALUE(0, DE_DESTINATION, Y,    dy));

        /* Set the Destination Dimension */    
        POKE_32(DE_DIMENSION,
            FIELD_VALUE(0, DE_DIMENSION, X,    sWidth) |
            FIELD_VALUE(0, DE_DIMENSION, Y_ET, dHeight));

        /* Setup Control Register */
        de_ctrl = FIELD_SET(0, DE_CONTROL, STATUS, START)    |
                  FIELD_SET(0, DE_CONTROL, COMMAND, ALPHA_BLEND)  |
                  FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2) |
                  FIELD_VALUE(0, DE_CONTROL, ROP, rop2)      |
                  ((sHeight < dHeight) ?
                        FIELD_SET(0, DE_CONTROL, STRETCH, ENABLE) :
                        FIELD_SET(0, DE_CONTROL, STRETCH, DISABLE));

        POKE_32(DE_CONTROL, de_ctrl);
    }

#ifdef ALPHA_BLEND_BLOCK_TRANSPARENCY
    /* Restore back the transparency */
    deWaitForNotBusy();
    de_ctrl = PEEK_32(de_ctrl) | transparency;
    POKE_32(DE_CONTROL, de_ctrl);
#endif

    return 0;
}

/*
 * This function sets the monochrome pattern on the pattern registers.
 */
void deSetPattern(
    unsigned long monoPatternLow,
    unsigned long monoPatternHigh
)
{
    POKE_32(DE_MONO_PATTERN_LOW, monoPatternLow);
    POKE_32(DE_MONO_PATTERN_HIGH, monoPatternHigh);
}

/*
 * This function uses 2D engine to fill a rectangular area with a specific pattern.
 * The filled area includes the starting points.
 */
long deRectPatternFill(
    unsigned long dBase,  /* Base address of destination surface counted from beginning of video frame buffer */
    unsigned long dPitch, /* Pitch value of destination surface in BYTES */
    unsigned long bpp,    /* Color depth of destination surface: 8, 16 or 32 */
    unsigned long x,
    unsigned long y,      /* Upper left corner (X, Y) of rectangle in pixel value */
    unsigned long width, 
    unsigned long height, /* width and height of rectange in pixel value */
    unsigned long color,  /* Color to be filled */
    unsigned long rop3)   /* ROP value */
{
    unsigned long de_ctrl, bytePerPixel;

    bytePerPixel = bpp/8;
    
    if (deWaitForNotBusy() != 0)
    {
        /* The 2D engine is always busy for some unknown reason.
           Application can choose to return ERROR, or reset it and
           continue the operation.
        */

        return -1;

        /* or */
        /* deReset(); */
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
    deSetPixelFormat(bpp);

#ifdef ENABLE_192_BYTES_PATCH
    /* Workaround for 192 byte requirement when ROP is not COPY */
    if (((rop3 != ROP2_COPY) || (rop3 != ROP2_Sn) || (rop3 != ROP2_Dn) || 
         (rop3 != ROP2_D) || (rop3 != ROP2_BLACK) || (rop3 != ROP2_WHITE)) && 
        ((width * bytePerPixel) > 192))
    {
        /* Perform the ROP2 operation in chunks of (xWidth * nHeight) */
        unsigned long xChunk = 192 / bytePerPixel; /* chunk width is in pixels */
        
        //DDKDEBUGPRINT((DE_LEVEL, "ROP != ROP_COPY, width * bytePerPixel = %x (> 192 bytes)\n", width * bytePerPixel));

        while (1)
        {
            deWaitForNotBusy();
            
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
                FIELD_VALUE(0, DE_CONTROL,  ROP,        rop3);

            POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());

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
            FIELD_VALUE(0, DE_CONTROL,  ROP,        rop3);

        POKE_32(DE_CONTROL, de_ctrl | deGetTransparency());
    }
    
    return 0;
}
