/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  2d.h --- SM750/SM718 DDK 
*  This file contains the definitions for the 2D Engine interrupt.
* 
*******************************************************************/
#ifndef _2D_H_
#define _2D_H_

//#include "csc.h"

#define POKE_8(address, value)          pokeRegisterByte(address, value)
#define POKE_16(address, value)         pokeRegisterWord(address, value)
#define POKE_32(address, value)         pokeRegisterDWord(address, value)
#define PEEK_8(address)                 peekRegisterByte(address)
#define PEEK_16(address)                peekRegisterWord(address)
#define PEEK_32(address)                peekRegisterDWord(address)

/* Rotation Direction */
typedef enum _rotate_dir_t
{
    ROTATE_NORMAL = 0,
    ROTATE_90_DEGREE,
    ROTATE_180_DEGREE,
    ROTATE_270_DEGREE
}
rotate_dir_t;

/*
 * 2D Engine Initialization.
 * This function must be called before other 2D functions.
 * Assumption: A specific vidoe mode has been properly set up.
 */
void ddk750_deInit(void);

/*
 * Reset 2D engine by 
 * 1) Aborting the current 2D operation.
 * 2) Re-enable 2D engine to normal state.
 */
void deReset(void);
 
/*
 * Wait until 2D engine is not busy.
 * All 2D operations are recommand to check 2D engine idle before start.
 *
 * Return: 0 = return because engine is idle and normal.
 *        -1 = return because time out (2D engine may have problem).
 */
long deWaitForNotBusy(void);

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
long deWaitIdle(unsigned long i);

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
unsigned long y2);    /* Note that the region will not include x2 and y2 */

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
unsigned long ulColor);   /* Color to compare. */

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
unsigned long rop2);  /* ROP value */

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
unsigned long height, /* width and height of rectange in pixel value */
unsigned long rop2);  /* ROP value */

/*
 * System Memory to Video Memory data transfer.
 * Note: 
 *        It works whether the Video Memroy is off-screeen or on-screen.
 *        This function is a one to one transfer without stretching or 
 *        mono expansion.
 */
long deSystemMem2VideoMemBusMasterBlt(
    unsigned char *pSBase,  /* Address of source: offset in frame buffer */
    unsigned long sPitch,   /* Pitch value of source surface in BYTE */
    unsigned long sx,
    unsigned long sy,       /* Starting coordinate of source surface */
    unsigned long dBase,    /* Address of destination: offset in frame buffer */
    unsigned long dPitch,   /* Pitch value of destination surface in BYTE */
    unsigned long bpp,      /* Color depth of destination surface */
    unsigned long dx,
    unsigned long dy,       /* Starting coordinate of destination surface */
    unsigned long width, 
    unsigned long height,   /* width and height of rectangle in pixel value */
    unsigned long rop2      /* ROP value */
);

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
unsigned long rop2);    /* ROP value */

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
unsigned long rop2);    /* ROP value */

#if 0
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
);
#endif

/*
 * This function cache font table into frame buffer.
 *
 * Inputs:
 *      Pointer to font table in system memory.
 *      Charcter size in byte: How many bytes are used to store the font for one characer.
 *      As an example: For 8x8 mono font, the size is 8 bytes; 
 *                     For 8x16 mono font, the size is 16 bytes.
 *                     For 16x32 mono font, the size is 64 bytes.
 *      Number of characters in the font table.
 *      Pointer to location of frame buffer to store the font: This is an offset from the beginning of frame buffer.
 *  
 * Rules for storing fonts in off-screen.
 *     1) Base address of font table must be 16 byte (or 128 bit) aligned.
 *     2) Each font character must be stored in a 16 byte (128 bit) aligned
 *        location.
 *
 */
long deCacheFontToFrameBuffer(
unsigned char *fontTable,   /* Pointer to font table in system memory */
unsigned long sizeOfChar,   /* How many bytes for one monochrome character */
unsigned long numberOfChar, /* Number of characters in the font table */
unsigned long fbAddr);      /* Destination in Video memory to store the font */

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
unsigned long rop2);  /* ROP value */

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
unsigned long rop2);      /* ROP value */

/*
 * Stretch Blt.
 * 
 * The stretch blt is done by using the CSC engine.
 */
long deStretchBlt(
    unsigned long sBase,    /* Source Base address */
    unsigned long sPitch,   /* Source pitch value in bytes */
    unsigned long sbpp,     /* Source bits per pixel */
    unsigned long sx,       /* Source x coordinate */            
    unsigned long sy,       /* Source y coordinate */
    unsigned long sWidth,   /* Width of source in pixel */
    unsigned long sHeight,  /* Height of source in lines */
    unsigned long dBase,    /* Destination base address */
    unsigned long dPitch,   /* Destination pitch value in bytes */
    unsigned long dbpp,     /* Destination bits per pixel */
    unsigned long dx,       /* Destination x coordinate */
    unsigned long dy,       /* Destination y coordinate */
    unsigned long dWidth,   /* Width of the destination display */
    unsigned long dHeight   /* Height of the destination display */
);

/*
 * Rotation Blt.
 * 
 * This function rotates an image to the screen based on the given rotation direction
 * (0, 90, 180, or 270 degree).
 * 
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
);

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
);

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
);

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
);

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
);

#if 0   /* These RLE functions are not working. */
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
);

/* 
 * Function to continue drawing a line using RLE line strip method.
 */
long deNextLineStrip(
    unsigned long length,       /* Line strip length */
    unsigned long lineCounter   /* Line Counter */
);

/* 
 * Function to stop the RLE line strip drawing.
 * This function has to be called to end the Line Strip drawing.
 * Otherwise, the next 2D function might still use this function.
 */
long deStopLineStrip();
#endif


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
    unsigned long rop2);

/* 
 * Function to continue drawing a line using Trapezoid Fill method.
 */
long deNextTrapezoidFill(
    unsigned long x,            /* Starting X location. */
    unsigned long length        /* Line length */
);

/* 
 * Function to stop the Trapezoid Fill drawing.
 * This function has to be called to end the Trapezoid Fill drawing.
 * Otherwise, the next 2D function might still use this function.
 */
long deStopTrapezoidFill(void);

#endif /* _2D_H_ */
