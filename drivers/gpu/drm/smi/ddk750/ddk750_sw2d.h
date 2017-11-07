/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  sw2d.h --- SM750/SM718 DDK 
*  This file contains the function prototype for 2D Function
*  implementation.
* 
*******************************************************************/
#ifndef _SW2D_H_
#define _SW2D_H_

/* Some color definitions */
#define BPP32_RED    0x00ff0000
#define BPP32_GREEN  0x0000ff00
#define BPP32_BLUE   0x000000ff
#define BPP32_WHITE  0x00ffffff
#define BPP32_GRAY   0x00808080
#define BPP32_YELLOW 0x00ffff00
#define BPP32_CYAN   0x0000ffff
#define BPP32_PINK   0x00ff00ff
#define BPP32_BLACK  0x00000000

#define BPP16_RED    0x0000f800
#define BPP16_GREEN  0x000007e0
#define BPP16_BLUE   0x0000001f
#define BPP16_WHITE  0x0000ffff
#define BPP16_GRAY   0x00008410
#define BPP16_YELLOW 0x0000ffe0
#define BPP16_CYAN   0x000007ff
#define BPP16_PINK   0x0000f81f
#define BPP16_BLACK  0x00000000

#define BPP8_RED     0x000000b4
#define BPP8_GREEN   0x0000001e
#define BPP8_BLUE    0x00000005
#define BPP8_WHITE   0x000000ff
#define BPP8_GRAY    0x000000ec
#define BPP8_YELLOW  0x000000d2
#define BPP8_CYAN    0x00000023
#define BPP8_PINK    0x000000b9
#define BPP8_BLACK   0x00000000

/* Raster Op 2 functions */
#define ROP2_XOR        0x06
#define ROP2_AND        0x08
#define ROP2_COPY       0x0C
#define ROP2_OR         0x0E

#define ROP2_BLACK      0x00
#define ROP2_DSon       0x01
#define ROP2_DSna       0x02
#define ROP2_Sn         0x03
#define ROP2_SDna       0x04
#define ROP2_Dn         0x05
#define ROP2_DSx        0x06
#define ROP2_SDan       0x07
#define ROP2_DSa        0x08
#define ROP2_SDnx       0x09
#define ROP2_D          0x0A
#define ROP2_DSno       0x0B
#define ROP2_S          0x0C
#define ROP2_SDno       0x0D
#define ROP2_DSo        0x0E
#define ROP2_WHITE      0x0F

/* Blt Direction definitions */
#define TOP_TO_BOTTOM 0
#define LEFT_TO_RIGHT 0
#define BOTTOM_TO_TOP 1
#define RIGHT_TO_LEFT 1

unsigned long swRasterOp2(unsigned long S, unsigned long D, unsigned long rop2);

/*
 * This function set up a pixel value in the frame buffer.
 *
 * Note:
 * 1) It can only set pixel within the frame buffer.
 * 2) This function is NOT for drawing surface created in system memory.
 *
 */
void swSetPixel(
unsigned long destBase, /* Base address of destination surface counted from beginning of video frame buffer */
unsigned long pitch,    /* Pitch value of destination surface in BYTES */
unsigned long bpp,      /* Color depth of destination surface: 8, 16 or 32 */
unsigned long x, 
unsigned long y,        /* Position (X, Y) to set in pixel value */
unsigned long color,    /* Color */
unsigned long rop2);     /* ROP value */

/*
 * This function gets a pixel value in the frame buffer.
 *
 * Note:
 * 1) It can only get pixel within the frame buffer.
 * 2) This function is NOT for drawing surface created in system memory.
 * 3) This function always return a 32 bit pixel value disregard bpp = 8, 16, or 32.
 *    The calling funtion has to type cast the return value into Byte, word or 
 *    DWord according to BPP.
 *
 */
unsigned long swGetPixel(
unsigned long destBase, /* Base address of destination surface counted from beginning of video frame buffer */
unsigned long pitch,    /* Pitch value of destination surface in BYTES */
unsigned long bpp,      /* Color depth of destination surface: 8, 16 or 32 */
unsigned long x, 
unsigned long y);        /* Position (X, Y) to set in pixel value */

/*
 *  This function uses software only method to fill a rectangular area with a specific color.
 * Input: See comment of code below.
 * 
 */
void swRectFill(
unsigned long destBase, /* Base address of destination surface counted from beginning of video frame buffer */
unsigned long pitch,    /* Pitch value of destination surface in BYTES */
unsigned long bpp,      /* Color depth of destination surface: 8, 16 or 32 */
unsigned long x,
unsigned long y,        /* Upper left corner (X, Y) of rectangle in pixel value */
unsigned long width, 
unsigned long height,   /* width and height of rectange in pixel value */
unsigned long color,    /* Fill color */
unsigned long rop2);    /* ROP value */

/*
 * This function draws a hollow rectangle, no fills.
 */
void swRect(
unsigned long destBase, /* Base address of destination surface counted from beginning of video frame buffer */
unsigned long pitch,    /* Pitch value of destination surface in BYTES */
unsigned long bpp,      /* Color depth of destination surface: 8, 16 or 32 */
unsigned long x,
unsigned long y,        /* Upper left corner (X, Y) of rectangle in pixel value */
unsigned long width, 
unsigned long height,   /* width and height of rectange in pixel value */
unsigned long color,    /* border color */
unsigned long rop2);     /* ROP value */

void swHorizontalLine(
unsigned long destBase, /* Base address of destination surface counted from beginning of video frame buffer */
unsigned long pitch,    /* Pitch value of destination surface in BYTES */
unsigned long bpp,      /* Color depth of destination surface: 8, 16 or 32 */
unsigned long x,
unsigned long y,        /* Starting point (X, Y) of line */
unsigned long length,   /* Length of line */
unsigned long color,    /* Color */
unsigned long rop2);    /* ROP value */

void swVerticalLine(
unsigned long destBase, /* Base address of destination surface counted from beginning of video frame buffer */
unsigned long pitch,    /* Pitch value of destination surface in BYTES */
unsigned long bpp,      /* Color depth of destination surface: 8, 16 or 32 */
unsigned long x,
unsigned long y,        /* Starting point (X, Y) of line */
unsigned long length,   /* Length of line */
unsigned long color,    /* Color */
unsigned long rop2);    /* ROP value */

/* 
 * Function to draw a line.
 */
long swLine(
    unsigned long destBase, /* Base address of destination surface counted from beginning of video frame buffer */
    unsigned long pitch,    /* Pitch value of destination surface in BYTES */
    unsigned long bpp,      /* Color depth of destination surface: 8, 16 or 32 */
    unsigned long x0,       /* Starting X Coordinate */
    unsigned long y0,       /* Starting Y Coordinate */
    unsigned long x1,       /* Ending X Coordinate */
    unsigned long y1,       /* Ending Y Coordinate */
    unsigned long color,    /* Color of the line */
    unsigned long rop2      /* ROP value */
);

/*
 * Video Memory to Video Memroy data transfer.
 *
 * Note: 
 * 1) All addresses are offset from the beginning for frame buffer.
 * 2) Both source and destination have to be same bpp (color depth).
 * 
 */
void swVideoMem2VideoMemBlt(
unsigned long sBase,  /* Address of source: offset in frame buffer */
unsigned long sPitch, /* Pitch value of source surface in BYTE */
unsigned long sx,
unsigned long sy,     /* Starting coordinate of source surface */
unsigned long dBase,  /* Address of destination: offset in frame buffer */
unsigned long dPitch, /* Pitch value of destination surface in BYTE */
unsigned long bpp,   /* color depth of destiination, source must have same bpp */
unsigned long dx,
unsigned long dy,     /* Starting coordinate of destination surface */
unsigned long width, 
unsigned long height, /* width and height of rectange in pixel value */
unsigned long rop2);  /* ROP value */

#endif /* _SW2D_H_ */ 
