/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  Video.C --- Falcon SDK 
*  This file contains the definitions for the Video functions.
* 
*******************************************************************/
#include "ddk768_help.h"
#include "ddk768_chip.h"
#include "ddk768_mode.h"
#include "ddk768_video.h"
#include "ddk768_reg.h"


/* New video function */

#define SCALE_CONSTANT                      (1 << 12)

/* Offset Adjustment for the window */
static short gWidthAdjustment = 0;
static short gHeightAdjustment = 0;

/* Source Video Width and Height */
static unsigned long gSrcVideoWidth = 0;
static unsigned long gSrcVideoHeight = 0;

/*
 *  videoSetWindowAdjustment
 *      This function sets the video window adjustment. There are usually
 *      some garbage lines or pixels at the bottom and right of the video
 *      window. These function will adjust the video window accordingly.
 *
 *  Input:
 *      widthAdjustment     - Width adjustments in pixel
 *      heightAdjustment    - Height adjustment in line        
 */
void videoSetWindowAdjustment(
	unsigned dispCtrl,
    short widthAdjustment,
    short heightAdjustment
)
{
    unsigned long width, height;
    videoGetWindowSize(dispCtrl, &width, &height);
    
    gWidthAdjustment = widthAdjustment;
    gHeightAdjustment = heightAdjustment;
    
    videoSetWindowSize(dispCtrl, width, height);
}

/*
 *  videoGetWindowAdjustment
 *      This function gets the video window adjustment.
 *
 *  Input:
 *      widthAdjustment     - Width adjustments in pixel
 *      heightAdjustment    - Height adjustment in line 
 */
void videoGetWindowAdjustment(
    short *pWidthAdjustment,
    short *pHeightAdjustment
)
{
    if (pWidthAdjustment != ((short *)0))
        *pWidthAdjustment = gWidthAdjustment;
    
    if (pHeightAdjustment != ((short *)0))
        *pHeightAdjustment = gHeightAdjustment;
}

/*
 * videoGetBufferStatus
 *      This function gets the status of the video buffer, either the buffer
 *      has been used or not.
 *
 *  Input:
 *      bufferIndex     - The index of the buffer which size to be retrieved
 *
 *  Output:
 *      0 - No flip pending
 *      1 - Flip pending
 */
unsigned long videoGetBufferStatus(
    unsigned long bufferIndex
)
{
        return (FIELD_GET(peekRegisterDWord(VIDEO_FB_ADDRESS), VIDEO_FB_ADDRESS, STATUS));
}

/*
 * videoGetPitch
 *      This function gets the video plane pitch
 *
 * Output:
 *      pitch   - Number of bytes per line of the video plane 
 *                specified in 128-bit aligned bytes.
 */
unsigned short videoGetPitch()
{
    return (FIELD_GET(peekRegisterDWord(VIDEO_FB_WIDTH), VIDEO_FB_WIDTH, WIDTH));
}

/*
 * videoGetLineOffset
 *      This function gets the video plane line offset
 *
 * Output:
 *      lineOffset  - Number of 128-bit aligned bytes per line 
 *                    of the video plane.
 */
unsigned short videoGetLineOffset()
{
    return (FIELD_GET(peekRegisterDWord(VIDEO_FB_WIDTH), VIDEO_FB_WIDTH, OFFSET));
}

/*
 * videoGetBufferSize
 *      This function gets the buffer size
 *
 *  Input:
 *      bufferIndex - The index of the buffer which size to be retrieved
 */
unsigned long videoGetBufferSize(
    unsigned long bufferIndex
)
{
    unsigned long value;
    
    if (bufferIndex == 0)
    {
        value = (unsigned long)
            FIELD_GET(peekRegisterDWord(VIDEO_FB_ADDRESS), VIDEO_FB_ADDRESS, ADDRESS);
    }
    
    return value;
}


/*
 * videoGetBuffer
 *      This function gets the video buffer
 *
 *  Input:
 *      bufferIndex - The index of the buffer to get
 *
 *  Output:
 *      The video buffer of the requested index.
 */
unsigned long videoGetBuffer(
    unsigned char bufferIndex
)
{
        return (FIELD_GET(peekRegisterDWord(VIDEO_FB_ADDRESS), VIDEO_FB_ADDRESS, ADDRESS));
}

/*
 * videoSetBufferLastAddress
 *      This function sets the video buffer last address.
 *      The value can be calculated by subtracting one line offset 
 *      from the buffer size (Total number of line offset * 
 *      source video height).
 *
 *  Input:
 *      bufferIndex         - The index of the buffer to be set
 *      bufferSize          - Size of the video buffer.
 */
void videoSetBufferLastAddress(
    unsigned char bufferIndex,          /* The index of the buffer to be set. */
    unsigned long bufferStart,          /* Buffer start */
    unsigned long bufferSize            /* Size of the video buffer */
)
{
#if 0

	if (getChipType() == SM750)    
	{
	    /* Substract with one line offset to get the last address value when added
	       with the bufferStart. Somehow, this is only happen in SM750 chip */    
	    bufferSize -= (unsigned long) videoGetLineOffset();
	}

    if (bufferIndex == 0)
    {
        /* Set Video Buffer 0 Last Address */
        pokeRegisterDWord(VIDEO_FB_0_LAST_ADDRESS,
            FIELD_VALUE(0, VIDEO_FB_0_LAST_ADDRESS, ADDRESS, bufferStart + bufferSize));
    }
    else
    {   
        /* Set Video Buffer 1 Last Address */ 
        pokeRegisterDWord(VIDEO_FB_1_LAST_ADDRESS,
            FIELD_VALUE(0, VIDEO_FB_1_LAST_ADDRESS, ADDRESS, bufferStart + bufferSize));
    }
#endif
}

/*
 * videoGetBufferLastAddress
 *      This function gets the video buffer last address.
 *
 *  Input:
 *      bufferIndex         - The index of the buffer last address to be retrieved
 */
unsigned long videoGetBufferLastAddress(
    unsigned char bufferIndex           /* The index of the buffer last address to be retrieved. */
)
{
#if 0
    if (bufferIndex == 0)
    {
        /* Get Video Buffer 0 Last Address */
        return (unsigned long) (FIELD_GET(peekRegisterDWord(VIDEO_FB_0_LAST_ADDRESS), 
                                          VIDEO_FB_0_LAST_ADDRESS, ADDRESS));
    }
    else
    {   
        /* Get Video Buffer 1 Last Address */ 
        return (unsigned long) (FIELD_GET(peekRegisterDWord(VIDEO_FB_1_LAST_ADDRESS), 
                                          VIDEO_FB_1_LAST_ADDRESS, ADDRESS));
    }
#endif
	return 0;
}

/*
 * videoSetBuffer
 *      This function sets the video buffer
 *
 *  Input:
 *      bufferIndex         - The index of the buffer to be set
 *      bufferStartAddress  - The starting address of the buffer
 */
void videoSetBuffer(
	unsigned dispCtrl,
    unsigned char bufferIndex,          /* The index of the buffer to be set. */
    unsigned long bufferStartAddress    /* Video buffer with 128-bit alignment */
)
{
    unsigned long bufferSize, lastAddress;
	unsigned long regFB;

    /* Get the buffer size first */
    bufferSize = videoGetBufferSize(bufferIndex);
    
    lastAddress = videoGetBufferLastAddress(bufferIndex);
#if 0    
	if (getChipType() == SM750)    
	{
	    if (lastAddress <= (bufferStartAddress + bufferSize - videoGetLineOffset()))
	        videoSetBufferLastAddress(bufferIndex, bufferStartAddress, bufferSize);
    }
    else
#endif
    {
		if (lastAddress <= (bufferStartAddress + bufferSize))
	        videoSetBufferLastAddress(bufferIndex, bufferStartAddress, bufferSize);
    }

    if (bufferIndex == 0)
    {
		regFB = (dispCtrl == CHANNEL0_CTRL)? VIDEO_FB_ADDRESS : (VIDEO_FB_ADDRESS+CHANNEL_OFFSET);
	    pokeRegisterDWord(regFB,
	            FIELD_SET(0, VIDEO_FB_ADDRESS, STATUS, PENDING) |
	            FIELD_VALUE(0, VIDEO_FB_ADDRESS, ADDRESS, bufferStartAddress));
    }
}
/*
 * videoSetUVBuffer
 *      This function sets the video buffer
 *
 *  Input:
 *      bufferIndex         - The index of the buffer to be set
 *      bufferStartAddress  - The starting address of the buffer
 */
void videoSetUVBuffer(
	unsigned dispCtrl,
    unsigned long bufferStartUAddress,    /* Video buffer with 128-bit alignment */
    unsigned long bufferStartVAddress    /* Video buffer with 128-bit alignment */
)
{
    unsigned long bufferSize;
	unsigned long regU, regV;

	regU = (dispCtrl == CHANNEL0_CTRL)? VIDEO_FB_ADDRESS_U : (VIDEO_FB_ADDRESS_U+CHANNEL_OFFSET);
	regV = (dispCtrl == CHANNEL0_CTRL)? VIDEO_FB_ADDRESS_V : (VIDEO_FB_ADDRESS_V+CHANNEL_OFFSET);
    pokeRegisterDWord(regU,
        FIELD_VALUE(0, VIDEO_FB_ADDRESS_U, ADDRESS, bufferStartUAddress));
    pokeRegisterDWord(regV,
        FIELD_VALUE(0, VIDEO_FB_ADDRESS_V, ADDRESS, bufferStartVAddress));
}

/*
 * videoSetPitchOffset
 *      This function sets the video plane pitch and offset
 *
 *  Input:
 *      pitch           - Number of bytes per line of the video plane 
 *                        specified in 128-bit aligned bytes.
 *      lineOffset      - Number of 128-bit aligned bytes per line 
 *                        of the video plane.
 */
void videoSetPitchOffset(
	unsigned dispCtrl,
    unsigned short pitch,
    unsigned short lineOffset
)
{
	unsigned long regWidth;

    /* Set Video Buffer Offset (pitch) */
	regWidth = (dispCtrl == CHANNEL0_CTRL)? VIDEO_FB_WIDTH : (VIDEO_FB_WIDTH+CHANNEL_OFFSET);
	pokeRegisterDWord(regWidth,
	    FIELD_VALUE(0, VIDEO_FB_WIDTH, WIDTH, pitch) |
	    FIELD_VALUE(0, VIDEO_FB_WIDTH, OFFSET, lineOffset));
}
/*
 * videoSetUVPitchOffset
 *      This function sets the video plane pitch and offset of U and V
 *
 *  Input:
 *      pitch           - Number of bytes per line of the video plane 
 *                        specified in 128-bit aligned bytes.
 *      lineOffset      - Number of 128-bit aligned bytes per line 
 *                        of the video plane.
 */
void videoSetUVPitchOffset(
	unsigned dispCtrl,
    unsigned short pitch,
    unsigned short lineOffset
)
{
	unsigned long regWidthU, regWidthV;
	regWidthU = (dispCtrl == CHANNEL0_CTRL)? VIDEO_FB_WIDTH_U : (VIDEO_FB_WIDTH_U+CHANNEL_OFFSET);
	regWidthV =	(dispCtrl == CHANNEL0_CTRL)? VIDEO_FB_WIDTH_V : (VIDEO_FB_WIDTH_V+CHANNEL_OFFSET);
	pokeRegisterDWord(regWidthU,
	    FIELD_VALUE(0, VIDEO_FB_WIDTH_U, WIDTH, pitch) |
	    FIELD_VALUE(0, VIDEO_FB_WIDTH_U, OFFSET, lineOffset));
	pokeRegisterDWord(regWidthV,
	    FIELD_VALUE(0, VIDEO_FB_WIDTH_V, WIDTH, pitch) |
	    FIELD_VALUE(0, VIDEO_FB_WIDTH_V, OFFSET, lineOffset));
}
/*
 *  videoSetLast 
 *      This function sets the video source last lines and width.
 *
 *  Input:
 *
 *      width      - Video source width
 *      height      - Video source height
 */
void videoSetLast(
	unsigned dispCtrl,
    unsigned long width,
    unsigned long height
)
{
#if 0 // SM768 don't have this register. Leave empty function here.
    if (dispCtrl == CHANNEL0_CTRL)
	{
	    pokeRegisterDWord(CHANNEL0_VIDEO_LAST, 
	        FIELD_VALUE(0, CHANNEL0_VIDEO_LAST, COLUMN, width) |
	        FIELD_VALUE(0, CHANNEL0_VIDEO_LAST, ROW, height)); 
	}
	else
	{
	    pokeRegisterDWord(CHANNEL1_VIDEO_LAST, 
	        FIELD_VALUE(0, CHANNEL1_VIDEO_LAST, COLUMN, width) |
	        FIELD_VALUE(0, CHANNEL1_VIDEO_LAST, ROW, height)); 
	}													   
#endif
}
/*
 *  videoSetWindowSize
 *      This function sets the video window size.
 *
 *  Input:
 *      width       - Video Window width
 *      height      - Video Window height
 */
void videoSetWindowSize(
	unsigned dispCtrl,
    unsigned long width,
    unsigned long height
)
{
    unsigned long value, startX, startY;
	unsigned long regTL, regBR;
	regTL = (dispCtrl == CHANNEL0_CTRL)? VIDEO_PLANE_TL : (VIDEO_PLANE_TL+CHANNEL_OFFSET);
	regBR = (dispCtrl == CHANNEL0_CTRL)? VIDEO_PLANE_BR : (VIDEO_PLANE_BR+CHANNEL_OFFSET);

	value = peekRegisterDWord(regTL);
	startX = FIELD_GET(value, VIDEO_PLANE_TL, LEFT);
	startY = FIELD_GET(value, VIDEO_PLANE_TL, TOP);

	/* Set bottom and right position */
	pokeRegisterDWord(regBR,
	    FIELD_VALUE(0, VIDEO_PLANE_BR, BOTTOM, startY + height - 1 - gHeightAdjustment) |
	    FIELD_VALUE(0, VIDEO_PLANE_BR, RIGHT, startX + width - 1 - gWidthAdjustment)); 
}

/*
 *  videoGetWindowSize
 *      This function gets the video window size.
 *
 *  Output:
 *      width       - Video Window width
 *      height      - Video Window height
 */
void videoGetWindowSize(
	unsigned dispCtrl,
    unsigned long *pVideoWidth,
    unsigned long *pVideoHeight
)
{
    unsigned long positionTopLeft, positionRightBottom;
    unsigned long videoWidth, videoHeight;
	unsigned long regTL, regBR;
	regTL = (dispCtrl == CHANNEL0_CTRL)? VIDEO_PLANE_TL : (VIDEO_PLANE_TL+CHANNEL_OFFSET);
	regBR = (dispCtrl == CHANNEL0_CTRL)? VIDEO_PLANE_BR : (VIDEO_PLANE_BR+CHANNEL_OFFSET);

	positionTopLeft = peekRegisterDWord(regTL);
	positionRightBottom = peekRegisterDWord(regBR);
	videoWidth  = FIELD_GET(positionRightBottom, VIDEO_PLANE_BR, RIGHT) - 
	              FIELD_GET(positionTopLeft, VIDEO_PLANE_TL, LEFT) + 1 +
	              gWidthAdjustment;
	videoHeight = FIELD_GET(positionRightBottom, VIDEO_PLANE_BR, BOTTOM) - 
	              FIELD_GET(positionTopLeft, VIDEO_PLANE_TL, TOP) + 1 +
	              gHeightAdjustment;

    if (pVideoWidth != ((unsigned long *)0))
        *pVideoWidth = videoWidth;
    
    if (pVideoHeight != ((unsigned long *)0))
        *pVideoHeight = videoHeight;
}

/*
 *  videoSetPosition
 *      This function sets the video starting coordinate position.
 *
 *  Input:
 *      startX      - X Coordinate of the video window starting position
 *      startY      - Y Coordinate of the video window starting position
 */
void videoSetPosition(
	unsigned dispCtrl,
    unsigned long startX,
    unsigned long startY
)
{
    unsigned long videoWidth, videoHeight;
	unsigned long regTL;

	regTL = (dispCtrl == CHANNEL0_CTRL)? VIDEO_PLANE_TL : (VIDEO_PLANE_TL+CHANNEL_OFFSET);

	/* Get the video window width and height */
    videoGetWindowSize(dispCtrl, &videoWidth, &videoHeight);

	pokeRegisterDWord(regTL,
	    FIELD_VALUE(0, VIDEO_PLANE_TL, TOP, startY) |
	    FIELD_VALUE(0, VIDEO_PLANE_TL, LEFT, startX));

    /* Set bottom and right position */    
    videoSetWindowSize(dispCtrl, videoWidth, videoHeight);

}

/*
 *  videoSetConstants
 *      This function sets the video constants. The actual component will be
 *      added by this constant to get the expected component value.
 *
 *  Input:
 *      yConstant       - Y Constant Value
 *      redConstant     - Red Constant Value
 *      greenConstant   - Green Constant Value
 *      blueConstant    - Blue Constant Value
 */
void videoSetConstants(
    unsigned dispCtrl,
    unsigned char  yConstant,               /* Y Adjustment */
    unsigned char  redConstant,             /* Red Conversion constant */
    unsigned char  greenConstant,           /* Green Conversion constant */
    unsigned char  blueConstant             /* Blue Conversion constant */
)
{
	unsigned long regYUV;

	regYUV = (dispCtrl == CHANNEL0_CTRL)? VIDEO_YUV_CONSTANTS : (VIDEO_YUV_CONSTANTS+CHANNEL_OFFSET);
	pokeRegisterDWord(regYUV,
	    FIELD_VALUE(0, VIDEO_YUV_CONSTANTS, Y, yConstant) |
	    FIELD_VALUE(0, VIDEO_YUV_CONSTANTS, R, redConstant) |
	    FIELD_VALUE(0, VIDEO_YUV_CONSTANTS, G, greenConstant) |
	    FIELD_VALUE(0, VIDEO_YUV_CONSTANTS, B, blueConstant));
}

/*
 *  videoSetInitialScale
 *      This function sets the video buffer initial vertical scale.
 *
 *  Input:
 *      bufferIndex         - Index of the buffer which vertical scale value
 *                            to be set.
 *      bufferInitScale     - Buffer Initial vertical scale value
 */
void videoSetInitialScale(
	unsigned dispCtrl,
    unsigned short InitScaleHorizontal,
    unsigned short InitScaleVertical
)
{
    unsigned long value;
	unsigned long regScale;

	regScale = (dispCtrl == CHANNEL0_CTRL)? VIDEO_INITIAL_SCALE : (VIDEO_INITIAL_SCALE+CHANNEL_OFFSET);

    value = peekRegisterDWord(regScale);
    value = FIELD_VALUE(value, VIDEO_INITIAL_SCALE, VERTICAL, InitScaleVertical);
    value = FIELD_VALUE(value, VIDEO_INITIAL_SCALE, HORIZONTAL, InitScaleHorizontal);
    pokeRegisterDWord(regScale, value);
}

/*
 *  videoGetInitialScale
 *      This function gets the video buffer initial vertical scale.
 *
 *  Input:
 *      pbuffer0InitScale   - Pointer to variable to store buffer 0 initial vertical scale
 *      pbuffer1InitScale   - Pointer to variable to store buffer 1 initial vertical scale
 */
void videoGetInitialScale(
	unsigned dispCtrl,
    unsigned short *pBufferVInitScale,
    unsigned short *pBufferHInitScale
)
{
	unsigned long regScale;

	regScale = (dispCtrl == CHANNEL0_CTRL)? VIDEO_INITIAL_SCALE : (VIDEO_INITIAL_SCALE+CHANNEL_OFFSET);

    *pBufferHInitScale = (unsigned short)
        FIELD_GET(peekRegisterDWord(regScale), VIDEO_INITIAL_SCALE, HORIZONTAL);
    *pBufferVInitScale = (unsigned short)
        FIELD_GET(peekRegisterDWord(regScale), VIDEO_INITIAL_SCALE, VERTICAL);
}

/*
 *  videoScale
 *      This function scales the video.
 *
 *  Input:
 *      srcWidth     - The source video width
 *      srcHeight    - The source video height
 *      dstWidth     - The destination video width 
 *      dstHeight    - The destination video height
 */
void videoScale(
	unsigned dispCtrl,
    unsigned long srcWidth,
    unsigned long srcHeight,
    unsigned long dstWidth,
    unsigned long dstHeight
)
{
    unsigned long value = 0;
    unsigned long scaleFactor;
	unsigned long regScale;

	regScale = (dispCtrl == CHANNEL0_CTRL)? VIDEO_SCALE : (VIDEO_SCALE+CHANNEL_OFFSET);

	if (dstHeight >= srcHeight)
	{
	    /* Calculate the factor */
	    scaleFactor = (srcHeight-1) * SCALE_CONSTANT / dstHeight;
	    value = FIELD_VALUE(value, VIDEO_SCALE , VERTICAL_SCALE, scaleFactor);
	}
	
	/* Scale the horizontal size */
	if (dstWidth >= srcWidth)
	{
	    /* Calculate the factor */
	    scaleFactor = (srcWidth-1) * SCALE_CONSTANT / dstWidth;
	    value = FIELD_VALUE(value, VIDEO_SCALE, HORIZONTAL_SCALE, scaleFactor);
	}
	
	pokeRegisterDWord(regScale, value);
}


/*
 *  videoSwapYUVByte
 *      This function swaps the YUV data byte.
 *
 *  Input:
 *      byteSwap    - Flag to enable/disable YUV data byte swap.
 */
void videoSwapYUVByte(
	unsigned dispCtrl,
   	video_byteswap_t byteSwap  
)
{
    unsigned long value;
	unsigned long regCtrl;

	regCtrl = (dispCtrl == CHANNEL0_CTRL)? VIDEO_DISPLAY_CTRL : (VIDEO_DISPLAY_CTRL+CHANNEL_OFFSET);

	value = peekRegisterDWord(regCtrl);
	if (byteSwap == SWAP_BYTE)
	    value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, BYTE_SWAP, ENABLE);
	else
	    value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, BYTE_SWAP, DISABLE);
	
	pokeRegisterDWord(regCtrl, value);
}

/*
 *  videoSetInterpolation
 *      This function enables/disables the horizontal and vertical interpolation.
 *
 *  Input:
 *      enableHorzInterpolation   - Flag to enable/disable Horizontal interpolation
 *      enableVertInterpolation   - Flag to enable/disable Vertical interpolation
 */
void videoSetInterpolation(
	unsigned dispCtrl,
    unsigned long enableHorzInterpolation,
    unsigned long enableVertInterpolation
)
{
    unsigned long value;
	unsigned long regCtrl;

	regCtrl = (dispCtrl == CHANNEL0_CTRL)? VIDEO_DISPLAY_CTRL : (VIDEO_DISPLAY_CTRL+CHANNEL_OFFSET);

	value = peekRegisterDWord(regCtrl);
	
	if (enableHorzInterpolation)
	    value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, HORIZONTAL_MODE, INTERPOLATE);
	else
	    value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, HORIZONTAL_MODE, REPLICATE);
	    
	if (enableVertInterpolation)
	    value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, VERTICAL_MODE, INTERPOLATE);
	else
	    value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, VERTICAL_MODE, REPLICATE);
	    
	pokeRegisterDWord(regCtrl, value);
}

/*
 *  videoGetInterpolation
 *      This function gets the horizontal and vertical interpolation enable status.
 *
 *  Input:
 *      pHorzInterpolationStatus	- Pointer to store the horizontal interpolation status
 *      pVertInterpolationStatus	- Pointer to store the vertical interpolation status
 */
void videoGetInterpolation(
    unsigned long *pHorzInterpolationStatus,
    unsigned long *pVertInterpolationStatus
)
{
    unsigned long value;
    
    value = peekRegisterDWord(VIDEO_DISPLAY_CTRL);
    if (pHorzInterpolationStatus != (unsigned long *)0)
	{
		if (FIELD_GET(value, VIDEO_DISPLAY_CTRL, HORIZONTAL_MODE) == VIDEO_DISPLAY_CTRL_HORIZONTAL_MODE_INTERPOLATE)
        	*pHorzInterpolationStatus = 1;
		else
			*pHorzInterpolationStatus = 0;
	}
        
    if (pHorzInterpolationStatus != (unsigned long *)0)
	{
		if (FIELD_GET(value, VIDEO_DISPLAY_CTRL, VERTICAL_MODE) == VIDEO_DISPLAY_CTRL_VERTICAL_MODE_INTERPOLATE)
			*pVertInterpolationStatus = 1;
		else
			*pVertInterpolationStatus = 0;
	}
}

/*
 *  videoSetStartPanningPixel
 *      This function sets the starting pixel number for smooth pixel panning.
 *
 *  Input:
 *      startPixel  - Starting pixel number for smooth pixel panning
 */
void videoSetStartPanningPixel(
    unsigned char startPixel
)
{
    pokeRegisterDWord(VIDEO_DISPLAY_CTRL, 
                      peekRegisterDWord(VIDEO_DISPLAY_CTRL) | 
                      FIELD_VALUE(0, VIDEO_DISPLAY_CTRL, PIXEL, startPixel));    
}

/*
 *  videoSetGamma
 *      This function enables/disables gamma control.
 *
 *  Input:
 *      enableGammaCtrl - The gamma enable control
 *
 *  NOTE:
 *      The gamma can only be enabled in RGB565 and RGB888. Enable this gamma
 *      without proper format will have no effect.
 */
void videoSetGammaCtrl(
    unsigned dispCtrl,
    unsigned long enableGammaCtrl
)
{
    unsigned long value;
	unsigned long regCtrl;

	regCtrl = (dispCtrl == CHANNEL0_CTRL)? VIDEO_DISPLAY_CTRL : (VIDEO_DISPLAY_CTRL+CHANNEL_OFFSET);

	value = peekRegisterDWord(regCtrl);
	
	if (enableGammaCtrl)
	    value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, GAMMA, ENABLE);
	else
	    value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, GAMMA, DISABLE);
	    
	pokeRegisterDWord(regCtrl, value);    
}

/*
 *  isVideoEnable
 *      This function check whether the video plane is already enabled or not.
 *
 *  Output:
 *      0   - Disable
 *      1   - Enable
 */
unsigned char isVideoEnable()
{
    unsigned long value;
    
    value = peekRegisterDWord(VIDEO_DISPLAY_CTRL);
    
    return ((FIELD_GET(value, VIDEO_DISPLAY_CTRL, PLANE) == VIDEO_DISPLAY_CTRL_PLANE_ENABLE) ? 1 : 0);
}

/*
 *  videoSetCtrl
 *      This function enable/disable the video plane.
 *
 *  Input:
 *      videoCtrl   - Enable/Disable video
 */
static void videoSetCtrl(
    disp_control_t dispCtrl,
    video_ctrl_t videoCtrl
)
{
    unsigned long value;
	unsigned long regCtrl;

	regCtrl = (dispCtrl == CHANNEL0_CTRL)? VIDEO_DISPLAY_CTRL : (VIDEO_DISPLAY_CTRL+CHANNEL_OFFSET);

	value = peekRegisterDWord(regCtrl);
	
	if (videoCtrl == VIDEO_ON)
	    value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, PLANE, ENABLE);
	else
	    value = FIELD_SET(value, VIDEO_DISPLAY_CTRL, PLANE, DISABLE);
	            
	pokeRegisterDWord(regCtrl, value); 
}

/*
 *  videoSetFormat
 *      This function sets the video format.
 *
 *  Input:
 *      videoFormat - The video content format
 *                    * FORMAT_RGB565 - 16-bit RGB 5:6:5 mode
 *                    * FORMAT_YUYV - 16-bit YUYV mode
 */
static void videoSetFormat(
    unsigned dispCtrl,
    video_format_t  videoFormat
)
{
    unsigned long value;
	unsigned long regCtrl;

	regCtrl = (dispCtrl == CHANNEL0_CTRL)? VIDEO_DISPLAY_CTRL : (VIDEO_DISPLAY_CTRL+CHANNEL_OFFSET);

	value = peekRegisterDWord(regCtrl);
	switch (videoFormat)
	{
	    default:
	    case FORMAT_RGB565:
	        value = FIELD_SET(value,  VIDEO_DISPLAY_CTRL , FORMAT, 16);
	        break;
	    case FORMAT_RGB888:
	        value = FIELD_SET(value,  VIDEO_DISPLAY_CTRL , FORMAT, 32);
	        break;
	    case FORMAT_YUYV:
	        value = FIELD_SET(value,  VIDEO_DISPLAY_CTRL , FORMAT, YUV422);
	        break;
	    case FORMAT_YUV420:
	        value = FIELD_SET(value,  VIDEO_DISPLAY_CTRL , FORMAT, YUV420);
	        break;
	}
	
	pokeRegisterDWord(regCtrl, value);
}

/*
 *  videoSetEdgeDetection
 *      This function enable/disable the edge detection and fill out the edge detection
 *      value as well. This function only works in SM718. SM750 does not support this
 *      feature.
 *
 *  Input:
 *      enableEdgeDetect    - Enable/Disable Edge Detection (0 - disable, 1 = enable)
 *      edgeDetectValue     - The Edge Detection value. This is the difference (delta)
 *                            of the pixel colors to be considered as an edge.
 *
 *  Note:
 *      This edge correction only works in up-scale video.
 */
void videoSetEdgeDetection(
    unsigned long enableEdgeDetect,
    unsigned long edgeDetectValue
)
{
#if 0 //SM768 don't have this register. Just leave an empty function here.
    unsigned long value;
    
    if (getChipType() == SM718)
    {
        if (enableEdgeDetect == 1)
        {
            value = FIELD_SET(0, VIDEO_EDGE_DETECTION, DETECT, ENABLE) |
                    FIELD_VALUE(value, VIDEO_EDGE_DETECTION, VALUE, edgeDetectValue);
            pokeRegisterDWord(VIDEO_EDGE_DETECTION, value); 
        }
        else
        {
            value = FIELD_SET(peekRegisterDWord(VIDEO_EDGE_DETECTION), VIDEO_EDGE_DETECTION, DETECT, DISABLE);
            pokeRegisterDWord(VIDEO_EDGE_DETECTION, value);
        }
    }
#endif
}

/*
 *  videoGetEdgeDetection
 *      This function gets the information whether the edge detection is enabled or not.
 *      It also outputs the edge detection value if required.
 *      This function only works in SM718. SM750 does not support this feature.
 *
 *  Input:
 *      pEdgeDetectValue    - Pointer to a buffer to store the edge detection value.
 *
 *  Note:
 *      0   - Edge Detection is disabled
 *      1   - Edge Detection is enabled
 */
unsigned long videoGetEdgeDetection(
    unsigned long *pEdgeDetectValue
)
{
#if 0 //Not for SM768
    unsigned long value;
    
    if (getChipType() == SM718)
    {
        value = peekRegisterDWord(VIDEO_EDGE_DETECTION);
        
        if (pEdgeDetectValue != (unsigned long *)0)
            *pEdgeDetectValue = (unsigned long) FIELD_GET(value, VIDEO_EDGE_DETECTION, VALUE);
            
        if (FIELD_GET(value, VIDEO_EDGE_DETECTION, DETECT) == VIDEO_EDGE_DETECTION_DETECT_ENABLE)
            return 1;
        else
            return 0; 
    }
#endif    
    return 0;
}

/*
 *  videoSetup
 *      This function setups the video. This function only works in SM718. 
 *      SM750 does not support edge detection feature. If calling this function
 *      in SM750, set edgeDetect flag to 0
 *
 *  Input:
 *      x               - X Coordinate of the video window
 *      y               - Y Coordinate of the video window
 *      srcWidth        - The source video width
 *      srcHeight       - The source video height
 *      dstWidth        - The destination video width
 *      dstHeight       - The destination video height
 *      doubleBuffer    - Double Buffer enable flag
 *      srcAddress0     - The source of the video buffer 0 to display
 *      srcAddress1     - The source of the video buffer 1 to display
 *                        (only for double buffering).
 *      srcPitch        - The source video plane pitch in bytes
 *      srcLineOffset   - The source video plane line offset in bytes.
 *                        In normal usage, set it the same as the srcBufferPitch
 *      videoFormat     - Source video format
 *      edgeDetect      - Edge Detection enable flag (can only works with vertical upscaling)
 *                              0 - Disable
 *                              1 - Always Enable (alwasy enabled regardless horizontal scaling condition)
 *                              2 - Auto Enable (only enabled when no horizontal shrink)
 *      edgeDetectValue - Edge Detection value
 *
 *  Output:
 *      0   - Success
 *     -1  - Fail
 */
unsigned char videoSetupEx(
	unsigned dispCtrl,
    unsigned long x,                /* X Coordinate of the video window */
    unsigned long y,                /* Y Coordinate of the video window */
    unsigned long srcWidth,         /* The source video width */
    unsigned long srcHeight,        /* The source video height */
    unsigned long dstWidth,         /* The destination video width */
    unsigned long dstHeight,        /* The destination video height */
    unsigned long doubleBuffer,     /* Double Buffer enable flag */
    unsigned long srcAddress0,      /* The source of the video buffer 0 to display */
    unsigned long sUAddress,            /* U Source Base Address (not used in RGB Space) */
    unsigned long sVAddress,            /* V Source Base Address (not used in RGB Space) */
    unsigned long sUVPitch,             /* UV plane pitch value in bytes (not used in */ 
    unsigned long srcPitch,         /* The source video plane pitch in bytes */
    unsigned long srcLineOffset,    /* The source video plane line offset in bytes.
                                       Set it the same as srcPitch in normal
                                       usage. */
    video_format_t videoFormat,      /* Source video format */
    unsigned long edgeDetect,       /* Edge Detection enable flag */
    unsigned long edgeDetectValue   /* Edge Detection value. SM718 only use bit 9 to 0 */
)
{
    unsigned long enableEdgeDetect;
    /* Save the source video width and height */
    gSrcVideoWidth = srcWidth;
    gSrcVideoHeight = srcHeight;
    /* Disable the video plane first */
    videoSetCtrl(dispCtrl, VIDEO_OFF);
    
    /* Set the video position */
    videoSetPosition(dispCtrl, x, y);
    
    /* Set the scale factor */
    videoScale(dispCtrl, srcWidth, srcHeight, dstWidth, dstHeight);
    
    /* Set the video format */
    videoSetFormat(dispCtrl, videoFormat);
    
    /* Set the buffer pitch */
    videoSetPitchOffset(dispCtrl, srcPitch, srcLineOffset);
    /* Set the UV buffer pitch */
    videoSetUVPitchOffset(dispCtrl, sUVPitch, sUVPitch);
    
    /* Enable double buffer */
//    videoEnableDoubleBuffer(doubleBuffer);
    
    /* Set the video buffer 0 and 1 */
    videoSetBuffer(dispCtrl, 0, srcAddress0);
//    videoSetBuffer(dispCtrl, 1, srcAddress1);
   
    /* Set the video buffer U and V */
    videoSetUVBuffer(dispCtrl, sUAddress, sVAddress);
        
    /* Set the destination video window */
    videoSetWindowSize(dispCtrl, dstWidth, dstHeight);

    /* Set the last line */
    videoSetLast(dispCtrl, srcWidth, srcHeight);
    
    /* Set the edge detection enable bit and its value (if applicable) */
    if (edgeDetect == 0)
        enableEdgeDetect = 0;
    else if (edgeDetect == 1)
        enableEdgeDetect = 1;
    else
    {
        /* Only enable the edgeDetection when scaling up vertically and no 
           shrinking on the horizontal. */
        if ((dstHeight > srcHeight) && (dstWidth >= srcWidth))
            enableEdgeDetect = 1;
        else
            enableEdgeDetect = 0;
    }
    videoSetEdgeDetection(enableEdgeDetect, edgeDetectValue);
    
    return 0;
}

/*
 *  videoSetup
 *      This function setups the video.
 *
 *  Input:
 *      x               - X Coordinate of the video window
 *      y               - Y Coordinate of the video window
 *      srcWidth        - The source video width
 *      srcHeight       - The source video height
 *      dstWidth        - The destination video width
 *      dstHeight       - The destination video height
 *      doubleBuffer    - Double Buffer enable flag
 *      srcAddress0     - The source of the video buffer 0 to display
 *      srcAddress1     - The source of the video buffer 1 to display
 *                        (only for double buffering).
 *      srcPitch        - The source video plane pitch in bytes
 *      srcLineOffset   - The source video plane line offset in bytes.
 *                        In normal usage, set it the same as the srcBufferPitch
 *      videoFormat     - Source video format
 *
 *  Output:
 *      0   - Success
 *     -1  - Fail
 */
unsigned char videoSetup(
    disp_control_t dispCtrl,
    unsigned long x,                /* X Coordinate of the video window */
    unsigned long y,                /* Y Coordinate of the video window */
    unsigned long srcWidth,         /* The source video width */
    unsigned long srcHeight,        /* The source video height */
    unsigned long dstWidth,         /* The destination video width */
    unsigned long dstHeight,        /* The destination video height */
    unsigned long doubleBuffer,     /* Double Buffer enable flag */
    unsigned long srcAddress0,      /* The source of the video buffer 0 to display */
    unsigned long srcAddress1,      /* The source of the video buffer 1 to display
                                       (only for double buffering).
                                     */
    unsigned long srcPitch,         /* The source video plane pitch in bytes */
    unsigned long srcLineOffset,    /* The source video plane line offset in bytes.
                                       Set it the same as srcPitch in normal
                                       usage. */
    video_format_t videoFormat      /* Source video format */
)
{
    return videoSetupEx(dispCtrl, x, y, srcWidth, srcHeight, dstWidth, dstHeight, doubleBuffer, 
                        srcAddress0, 0, 0, 0,srcPitch, srcLineOffset, videoFormat,
                        0, 0);
    
}

/*
 *  startVideo
 *      This function starts the video.
 */
void startVideo( 
unsigned dispCtrl
)
{
    /* Enable the video plane */
    videoSetCtrl(dispCtrl, VIDEO_ON);
}

/*
 *  stopVideo
 *      This function stops the video.
 */
void stopVideo(unsigned dispCtrl)
{
    /* Just disable the video plane */
    videoSetCtrl(dispCtrl, VIDEO_OFF);
}



