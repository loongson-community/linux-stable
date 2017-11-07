/*******************************************************************
* 
*         Copyright (c) 2009 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  vdif.h --- SMI DDK 
*  This file contains the video display information format structure
* 
*******************************************************************/
#ifndef _VDIF_H_
#define _VDIF_H_

/* Sync polarity */
typedef enum _vdif_sync_polarity_t
{
    VDIF_SYNC_NEGATIVE = 0,
    VDIF_SYNC_POSITIVE
} vdif_sync_polarity_t; 

/* Scan type */
typedef enum _vdif_scan_type_t
{
    VDIF_NONINTERLACED = 0,
    VDIF_INTERLACED
} vdif_scan_type_t;

/* Monitor Timing Information */
typedef struct _video_display_information_format_t
{
    unsigned long pixelClock;
    unsigned long characterWidth;
    vdif_scan_type_t scanType; 
    
    unsigned long horizontalFrequency;
    vdif_sync_polarity_t horizontalSyncPolarity;
    unsigned long horizontalTotal;
    unsigned long horizontalActive;
    unsigned long horizontalBlankStart;
    unsigned long horizontalBlankTime;
    unsigned long horizontalSyncStart;
    unsigned long horizontalRightBorder;
    unsigned long horizontalFrontPorch;
    unsigned long horizontalSyncWidth;
    unsigned long horizontalBackPorch;
    unsigned long horizontalLeftBorder;
    
    unsigned long verticalFrequency;
    vdif_sync_polarity_t verticalSyncPolarity; 
    unsigned long verticalTotal;
    unsigned long verticalActive;
    unsigned long verticalBlankStart;
    unsigned long verticalBlankTime;
    unsigned long verticalSyncStart;
    unsigned long verticalBottomBorder;
    unsigned long verticalFrontPorch;
    unsigned long verticalSyncHeight;
    unsigned long verticalBackPorch;
    unsigned long verticalTopBorder;
} vdif_t;

#endif  /* _VDIF_H_ */
