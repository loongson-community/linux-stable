/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  Video.H --- SM768 DDK 
*  This file contains the definitions for the Video functions.
* 
*******************************************************************/


/****************************************************************************
   Structure and data type definition 
 ****************************************************************************/

/* video format: 
   - 16-bit RGB 5:6:5 mode
   - 16-bit YUYV mode
  
   Note: The 8-bit index and RGB 8:8:8 formats are not supported
 */
typedef enum _video_format_t
{
    FORMAT_RGB565 = 0,
    FORMAT_YUV420,
    FORMAT_YUYV,
	FORMAT_RGB888
}
video_format_t;

/* YUV Data Byte Swap */
typedef enum _video_byteswap_t
{
    NORMAL = 0,
    SWAP_BYTE
}
video_byteswap_t;

/* Turn on/off video */
typedef enum _video_sync_source_t
{
    NORMAL_BUFFER = 0,
    CAPTURE_BUFFER
}
video_sync_source_t;

/* FIFO Request Level */
typedef enum _video_fifo_t
{
    FIFO_LEVEL_1 = 0,
    FIFO_LEVEL_3,
    FIFO_LEVEL_7,
    FIFO_LEVEL_11
}
video_fifo_t;

/* Turn on/off video */
typedef enum _video_ctrl_t
{
    VIDEO_OFF = 0,
    VIDEO_ON
}
video_ctrl_t;


/****************************************************************************
   Function prototype 
 ****************************************************************************/

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
);

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
);

/*
 * videoGetCurrentBufferDisplay
 *      This function gets the current buffer used by SM50x to display on the screen
 *
 *  Return:
 *      0   - Buffer 0
 *      1   - Buffer 1 
 */
// unsigned char videoGetCurrentBufferDisplay();

/*
 * videoEnableDoubleBuffer
 *      This function enables/disables the double buffer usage
 *
 *  Input:
 *      enable  - Flag to enable/disable the double buffer. 
 */
#if 0
void videoEnableDoubleBuffer(
    unsigned long enable
);
#endif

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
);

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
);

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
);
/*
 * videoSetUVBuffer
 *      This function sets the video buffer of U and V
 *
 *  Input:
 *      bufferIndex         - The index of the buffer to be set
 *      bufferStartAddress  - The starting address of the buffer
 */
void videoSetUVBuffer(
	unsigned dispCtrl,
    unsigned long bufferStartUAddress,    /* Video buffer with 128-bit alignment */
    unsigned long bufferStartVAddress    /* Video buffer with 128-bit alignment */
);

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
);

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
);

/*
 * videoGetPitch
 *      This function gets the video plane pitch
 *
 * Output:
 *      pitch   - Number of bytes per line of the video plane 
 *                specified in 128-bit aligned bytes.
 */
unsigned short videoGetPitch(void);

/*
 * videoGetLineOffset
 *      This function gets the video plane line offset
 *
 * Output:
 *      lineOffset  - Number of 128-bit aligned bytes per line 
 *                    of the video plane.
 */
unsigned short videoGetLineOffset(void);
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
);

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
);

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
    unsigned long *pWidth,
    unsigned long *pHeight
);

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
);

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
);

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
);

/*
 *  videoSetFIFOLevel
 *      This function sets the video FIFO Request Level.
 *
 *  Input:
 *      videoSource - Buffer source selection
 */
#if 0
void videoSetFIFOLevel(
    video_fifo_t videoFIFO
);
#endif
/*
 *  videoSetSourceBuffer
 *      This function sets the video to use the capture buffer as the source.
 *
 *  Input:
 *      videoSource - Buffer source selection
 */
#if 0
void videoSetSourceBuffer(
    video_sync_source_t videoSource
);
#endif

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
);

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
);

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
);

/*
 *  videoSetStartPanningPixel
 *      This function sets the starting pixel number for smooth pixel panning.
 *
 *  Input:
 *      startPixel  - Starting pixel number for smooth pixel panning
 */
void videoSetStartPanningPixel(
    unsigned char startPixel
);

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
);

/*
 *  isVideoEnable
 *      This function check whether the video plane is already enabled or not.
 *
 *  Output:
 *      0   - Disable
 *      1   - Enable
 */
unsigned char isVideoEnable(void);

/*
 *  videoSetEdgeDetection
 *      This function enable/disable the edge detection and fill out the edge detection
 *      value as well. This function only works in SM718. SM750 does not support this
 *      feature.
 *
 *  Input:
 *      enableEdgeDetect    - Enable/Disable Edge Detection
 *      edgeDetectValue     - The Edge Detection value. This is the difference (delta)
 *                            of the pixel colors to be considered as an edge.
 *
 *  Note:
 *      This edge correction only works in up-scale video.
 */
void videoSetEdgeDetection(
    unsigned long enableEdgeDetect,
    unsigned long edgeDetectValue
);

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
);

/*
 *  videoSetupEx
 *      This function setups the video. It only applies in SM718
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
 *      edgeDetect      - Edge Detection enable flag
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
);

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
);

/*
 *  startVideo
 *      This function starts the video.
 */
void startVideo(unsigned dispCtrl);

/*
 *  stopVideo
 *      This function stops the video.
 */
void stopVideo(unsigned dispCtrl);


