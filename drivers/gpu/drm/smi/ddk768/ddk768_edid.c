/*******************************************************************
* 
*         Copyright (c) 2009 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  edid.c --- SM750/SM718 DDK 
*  This file contains functions to interpret the EDID structure.
* 
*******************************************************************/

#include "ddk768_help.h"
#include "ddk768_helper.h"
#include "ddk768_hwi2c.h"
#include "ddk768_swi2c.h"
#include "ddk768_edid.h"

#include "ddkdebug.h"


/* Enable this one to print the VDIF timing when debug is enabled. */
//#define ENABLE_DEBUG_PRINT_VDIF


#define HEADER_EDID_REGISTERS               8

typedef struct _est_timing_mode_t
{
    unsigned long x;        /* Mode Width */
    unsigned long y;        /* Mode Height */
    unsigned long hz;       /* Refresh Rate */
    unsigned char source;   /* Source:  0 - VESA
                                        1 - IBM
                                        2 - Apple
                             */
}
est_timing_mode_t;

/* These values only applies to EDID Version 1 */
static est_timing_mode_t establishTiming[3][8] =
{
    /* Established Timing 1 */
    {   
        { 800,  600, 60, 0},
        { 800,  600, 56, 0},
        { 640,  480, 75, 0},
        { 640,  480, 72, 0},
        { 640,  480, 67, 2},
        { 640,  480, 60, 1},
        { 720,  400, 88, 1},
        { 720,  400, 70, 1},
    },
    {
        {1280, 1024, 75, 0},
        {1024,  768, 75, 0},
        {1024,  768, 70, 0},
        {1024,  768, 60, 0},
        {1024,  768, 87, 1},
        { 832,  624, 75, 0},
        { 800,  600, 75, 0},
        { 800,  600, 72, 0},
    },
    {
        {   0,    0,  0, 0},
        {   0,    0,  0, 0},
        {   0,    0,  0, 0},
        {   0,    0,  0, 0},
        {   0,    0,  0, 0},
        {   0,    0,  0, 0},
        {   0,    0,  0, 0},
        {1152,  870, 75, 2},
    }
};

static void printVdif(
    vdif_t *pVDIF
)
{
#ifdef DDKDEBUG

#ifndef ENABLE_DEBUG_PRINT_VDIF
    DDKDEBUGENABLE(0);
#endif
    
    DDKDEBUGPRINT((DISPLAY_LEVEL, "pixelClock = %d\n", pVDIF->pixelClock));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "characterWidth = %d\n", pVDIF->characterWidth));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "scanType = %s\n", (pVDIF->scanType == VDIF_INTERLACED) ? "Interlaced" : "Progressive"));
    
    DDKDEBUGPRINT((DISPLAY_LEVEL, "horizontalFrequency = %d\n", pVDIF->horizontalFrequency));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "horizontalTotal = %d\n", pVDIF->horizontalTotal));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "horizontalActive = %d\n", pVDIF->horizontalActive));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "horizontalBlankStart = %d\n", pVDIF->horizontalBlankStart));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "horizontalBlankTime = %d\n", pVDIF->horizontalBlankTime));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "horizontalSyncStart = %d\n", pVDIF->horizontalSyncStart));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "horizontalRightBorder = %d\n", pVDIF->horizontalRightBorder));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "horizontalFrontPorch = %d\n", pVDIF->horizontalFrontPorch));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "horizontalSyncWidth = %d\n", pVDIF->horizontalSyncWidth));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "horizontalBackPorch = %d\n", pVDIF->horizontalBackPorch));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "horizontalLeftBorder = %d\n", pVDIF->horizontalLeftBorder));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "horizontalSyncPolarity = %s\n", 
        (pVDIF->horizontalSyncPolarity == VDIF_SYNC_NEGATIVE) ? "Negative" : "Positive"));
    
    DDKDEBUGPRINT((DISPLAY_LEVEL, "verticalFrequency = %d\n", pVDIF->verticalFrequency));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "verticalTotal = %d\n", pVDIF->verticalTotal));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "verticalActive = %d\n", pVDIF->verticalActive));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "verticalBlankStart = %d\n", pVDIF->verticalBlankStart));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "verticalBlankTime = %d\n", pVDIF->verticalBlankTime));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "verticalSyncStart = %d\n", pVDIF->verticalSyncStart));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "verticalBottomBorder = %d\n", pVDIF->verticalBottomBorder));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "verticalFrontPorch = %d\n", pVDIF->verticalFrontPorch));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "verticalSyncHeight = %d\n", pVDIF->verticalSyncHeight));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "verticalBackPorch = %d\n", pVDIF->verticalBackPorch));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "verticalTopBorder = %d\n", pVDIF->verticalTopBorder));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "verticalSyncPolarity = %s\n", 
        (pVDIF->verticalSyncPolarity == VDIF_SYNC_NEGATIVE) ? "Negative" : "Positive"));

#ifndef ENABLE_DEBUG_PRINT_VDIF
    DDKDEBUGENABLE(1);
#endif
#endif
}

/*
 *  edidGetHeader
 *      This function gets the EDID Header
 *
 *  Input:
 *      pEDIDBuffer - Buffer that contains the EDID structure of the monitor
 *
 *  Output:
 *      0 get header success; -1 fail.
 */
unsigned char edidGetHeader(
    unsigned char *pEDIDBuffer
)
{
    if (pEDIDBuffer != (unsigned char *)0)
    {
        /* Check the header */
        if ((pEDIDBuffer[0] == 0x00) && (pEDIDBuffer[1] == 0xFF) && (pEDIDBuffer[2] == 0xFF) &&
            (pEDIDBuffer[3] == 0xFF) && (pEDIDBuffer[4] == 0xFF) && (pEDIDBuffer[5] == 0xFF) &&
            (pEDIDBuffer[6] == 0xFF) && (pEDIDBuffer[7] == 0x00))
        {
            return 0;
        }
        else
            return -1;
    }

   // DDKDEBUGPRINT((DISPLAY_LEVEL, "Invalid EDID bufffer\n"));
    return -1;
}

/*
 *  edidGetVersion
 *      This function gets the EDID version
 *
 *  Input:
 *      pEDIDBuffer - Buffer that contains the EDID structure of the monitor
 *      pRevision   - Revision of the EDIE (if exist)
 *
 *  Output:
 *      Revision number of the given EDID buffer.
 */
unsigned char ddk768_edidGetVersion(
    unsigned char *pEDIDBuffer,
    unsigned char *pRevision
)
{
    unsigned char version;
    
    if (pEDIDBuffer != (unsigned char *)0)
    {
        /* Check the header */
        if ((pEDIDBuffer[0] == 0x00) && (pEDIDBuffer[1] == 0xFF) && (pEDIDBuffer[2] == 0xFF) &&
            (pEDIDBuffer[3] == 0xFF) && (pEDIDBuffer[4] == 0xFF) && (pEDIDBuffer[5] == 0xFF) &&
            (pEDIDBuffer[6] == 0xFF) && (pEDIDBuffer[7] == 0x00))
        {
            /* 
             * EDID Structure Version 1.
             */
        
            /* Read the version field from the buffer. It should be 1 */
            version  = pEDIDBuffer[18];
        
            if (version == 1)
            {
                /* Copy the revision first */
                if (pRevision != (unsigned char *)0)
                    *pRevision = pEDIDBuffer[19];
                    
                return version;
            }
        }
        else
        {
            /* 
             * EDID Structure Version 2 
             */
             
            /* Read the version and revision field from the buffer. */
            version = pEDIDBuffer[0];
        
            if ((version >> 4) == 2)
            {
                /* Copy the revision */
                if (pRevision != (unsigned char *)0)
                    *pRevision = version & 0x0F;
                
                return (version >> 4);
            }
        }
    }
    
    DDKDEBUGPRINT((DISPLAY_LEVEL, "Invalid EDID Structure\n"));    
    return 0;    
}

/*
 *  ddk768_edidGetProductInfo
 *      This function gets the vendor and product information.
 *
 *  Input:
 *      pEDIDBuffer         - Buffer that contains the EDID structure of the monitor [in]
 *      pManufacturerName   - Pointer to a 3 byte length variable to store the manufacturer name [out]
 *      pProductCode        - Pointer to a variable to store the product code [out]
 *      pSerialNumber       - Pointer to a variable to store the serial number [out]
 *      pWeekOfManufacture  - Pointer to a variable to store the week of manufacture [out]
 *      pYearOfManufacture  - Pointer to a variable to store the year of manufacture 
 *                            or model year (if WeekOfManufacture is 0xff) [out]
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
long ddk768_edidGetProductInfo(
    unsigned char *pEDIDBuffer,
    char *pManufacturerName,
    unsigned short *pProductCode,
    unsigned long *pSerialNumber,
    unsigned char *pWeekOfManufacture,
    unsigned short *pYearOfManufacture
)
{
    unsigned char version, revision;
    unsigned short manufactureID;
    
    /* Get EDID Version and revision */
    version = ddk768_edidGetVersion(pEDIDBuffer, &revision);
    
    if (version == 1)
    {
        edid_version_1_t *pEDIDStructure;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        
        if (pManufacturerName != (char *)0)
        {
            /* Swap the byte */
            manufactureID = (pEDIDStructure->manufacturerID >> 8) + (pEDIDStructure->manufacturerID << 8);
            pManufacturerName[0] = ((manufactureID >> 10) & 0x001F) + 'A' - 1;
            pManufacturerName[1] = ((manufactureID >> 5) & 0x001F) + 'A' - 1;
            pManufacturerName[2] = (manufactureID & 0x001F) + 'A' - 1;
            pManufacturerName[3] = '\0';
        }
        
        if (pProductCode != (unsigned short *)0)
            *pProductCode = pEDIDStructure->productCode;
            
        /* Only EDID structure version 1.1 and 1.2 supports this. EDID 1.3 uses
           detail timing descriptor to store the serial number in ASCII. */
        if (pSerialNumber != (unsigned long *)0)
            *pSerialNumber = pEDIDStructure->serialNumber;
        
        /*
         * Rev 1.3: - A value of 0 means that week of manufacture is not specified
         *          - A value in the range of 1 to 54 (0x01 - 0x36) means the week of manufacture
         *          - Any values greater than 54 is invalid.
         *
         * Rev 1.4: - A value of 0 means that week of manufacture is not specified
         *          - A value in the range of 1 to 54 (0x01 - 0x36) means the week of manufacture
         *          - A value of 0xFF means that Year of Manufacture contains the model year
         *            instead of year of Manufacture.
         *          - Other values means invalid
         */
        if (pWeekOfManufacture != (unsigned char *)0)
            *pWeekOfManufacture = pEDIDStructure->weekOfManufacture;
            
        /* The value must be greater than 3 and less than or equal to the current
           year minus 1990.
           A value of 3 or less would indicated that the display was manufactured 
           before the EDID standard was defined.
           A value greater than (current year - 1990) would indicate that the display
           has not yet been manufactured.
         */
        if (pYearOfManufacture != (unsigned short *)0)
            *pYearOfManufacture = (unsigned short) pEDIDStructure->yearOfManufacture + 1990;
        
        return 0;
    }

    return (-1);
}

/*
 *  ddk768_edidCheckMonitorInputSignal
 *      This function checks whether the monitor is expected analog/digital 
 *      input signal.
 *
 *  Input:
 *      pEDIDBuffer     - Buffer that contains the EDID structure of the monitor
 *
 *  Output:
 *      0   - Analog
 *      1   - Digital
 */
unsigned char ddk768_edidCheckMonitorInputSignal(
    unsigned char *pEDIDBuffer
)
{
    unsigned char version;

    
    /* Get EDID Version and revision */
    version = ddk768_edidGetVersion(pEDIDBuffer, (unsigned char *)0);
    
    if (version == 1)
        return (unsigned char)((edid_version_1_t *)pEDIDBuffer)->videoInputDefinition.analogSignal.inputSignal;
    
    return 0;
}

/*
 *  ddk768_edidGetAnalogSignalInfo
 *      This function gets the analog video input signal information
 *
 *  Input:
 *      pEDIDBuffer             - Buffer that contains the EDID structure of the monitor
 *      pRefWhiteAboveBlank     - Pointer to a variable to store the reference white above blank
 *                                value. The value is in milliVolt.
 *      pSyncLevelBelowBlank    - Pointer to a variable to store the Sync tip level below blank
 *                                The value is also in milliVolt
 *      pBlank2BlackSetup       - Pointer to a variable to store the Blank to black setup or
 *                                pedestal per appropriate Signal Level Standard flag. 
 *                                1 means that the display expect the setup.
 *      pSeparateSyncSupport    - Pointer to a variable to store the flag to indicate that the
 *                                monitor supports separate sync.
 *      pCompositeSyncSupport   - Pointer to a variable to store a flag to indicate that the
 *                                monitor supports composite sync.
 *      pSyncOnGreenSupport     - Pointer to a variable to store a flag to indicate that
 *                                the monitor supports sync on green video.
 *      pVSyncSerrationRequired - Pointer to a variable to store a flag to indicate that serration
 *                                of the VSync pulse is required when composite sync or
 *                                sync-on-green video is used.
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
long ddk768_edidGetAnalogSignalInfo(
    unsigned char *pEDIDBuffer,
    unsigned short *pRefWhiteAboveBlank,
    unsigned short *pSyncLevelBelowBlank,
    unsigned char *pBlank2BlackSetup,
    unsigned char *pSeparateSyncSupport,
    unsigned char *pCompositeSyncSupport,
    unsigned char *pSyncOnGreenSupport,
    unsigned char *pVSyncSerrationRequired
)
{
    unsigned char version, revision;
    unsigned short whiteReference, syncLevel;
    
    /* Get EDID Version and revision */
    version = ddk768_edidGetVersion(pEDIDBuffer, &revision);
    
    if (version == 1)
    {
        edid_version_1_t *pEDIDStructure;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        
        /* Check if the input signal is analog */
        if (pEDIDStructure->videoInputDefinition.analogSignal.inputSignal != 0)
            return (-1);
        
        switch (pEDIDStructure->videoInputDefinition.analogSignal.signalLevelStd)
        {
            case 0:
                whiteReference = 700;
                syncLevel = 300;
                break;
            case 1:
                whiteReference = 714;
                syncLevel = 286;
                break;
            case 2:
                whiteReference = 1000;
                syncLevel = 400;
                break;
            case 3:
                whiteReference = 700;
                syncLevel = 0;
                break;
        }
        
        if (pRefWhiteAboveBlank != (unsigned short *)0)
            *pRefWhiteAboveBlank = whiteReference; 
        
        if (pSyncLevelBelowBlank != (unsigned short *)0)
            *pSyncLevelBelowBlank = syncLevel;
        
        if (pBlank2BlackSetup != (unsigned char *)0)
            *pBlank2BlackSetup = (unsigned char)
                                  pEDIDStructure->videoInputDefinition.analogSignal.blank2Black;
        
        if (pSeparateSyncSupport != (unsigned char *)0)
            *pSeparateSyncSupport = (unsigned char)
                                     pEDIDStructure->videoInputDefinition.analogSignal.separateSyncSupport;
        
        if (pCompositeSyncSupport != (unsigned char *)0)
            *pCompositeSyncSupport = (unsigned char)
                                      pEDIDStructure->videoInputDefinition.analogSignal.compositeSyncSupport;
        
        if (pSyncOnGreenSupport != (unsigned char *)0)
            *pSyncOnGreenSupport = (unsigned char)
                                    pEDIDStructure->videoInputDefinition.analogSignal.syncOnGreenSupport;
        
        if (pVSyncSerrationRequired != (unsigned char *)0)
            *pVSyncSerrationRequired = (unsigned char)
                                        pEDIDStructure->videoInputDefinition.analogSignal.vsyncSerration;
                                        
        return 0;
    }
    else
    {
        /* EDID Structure 2 */
    }
    
    return (-1);
}

/*
 *  ddk768_edidGetDigitalSignalInfo
 *      This function gets the digital video input signal information.
 *      Only applies to EDID 1.3 and above.
 *
 *  Input:
 *      pEDIDBuffer     - Buffer that contains the EDID structure of the monitor
 *      pDFP1xSupport   - Pointer to a variable to store the flag to indicate that
 *                        the mointor interface is signal compatible with VESA
 *                        DFP 1.x TMDS CRGB, 1 pixel/clock, up to 8 bits / color
 *                        MSB aligned, DE active high
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
long ddk768_edidGetDigitalSignalInfo(
    unsigned char *pEDIDBuffer,
    unsigned char *pDFP1xSupport
)
{
    unsigned char version, revision;
    
    /* Get EDID Version and revision */
    version = ddk768_edidGetVersion(pEDIDBuffer, &revision);
    
    if ((version == 1) && (revision == 3))
    {
        edid_version_1_t *pEDIDStructure;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        
        /* Check if the input signal is digital */
        if (pEDIDStructure->videoInputDefinition.digitalSignal.inputSignal != 1)
            return (-1);
            
        if (pDFP1xSupport != (unsigned char *)0)
            *pDFP1xSupport = pEDIDStructure->videoInputDefinition.digitalSignal.dfp1Support;
        
        return 0;
    }
    
    return (-1);
}

/*
 *  ddk768_edidGetDisplaySize
 *      This function gets the display sizes in cm.
 *
 *  Input:
 *      pEDIDBuffer         - Buffer that contains the EDID structure of the monitor
 *      pMaxHorzImageSize   - Pointer to a variable to store the maximum horizontal 
 *                            image size to the nearest centimeter. A value of 0
 *                            indicates that the size is indeterminate size.
 *      pMaxVertImageSize   - Pointer to a variable to store the maximum vertical
 *                            image size to the nearest centimeter. A value of 0
 *                            indicates that the size is indeterminate size.
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
long ddk768_edidGetDisplaySize(
    unsigned char *pEDIDBuffer,
    unsigned char *pMaxHorzImageSize,
    unsigned char *pMaxVertImageSize
)
{
    unsigned char version, revision;
    
    /* Get EDID Version and revision */
    version = ddk768_edidGetVersion(pEDIDBuffer, &revision);
    
    if (version == 1)
    {
        edid_version_1_t *pEDIDStructure;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        
        if (pMaxHorzImageSize != (unsigned char *)0)
            *pMaxHorzImageSize = pEDIDStructure->maxHorzImageSize;
        
        if (pMaxVertImageSize != (unsigned char *)0)
            *pMaxVertImageSize = pEDIDStructure->maxVertImageSize;
        
        return 0;
    }
    
    return (-1);
}

#if 0   /* Use the ddk768_edidGetWhitePoint to get the Gamma */
/*
 *  edidGetGamma
 *      This function gets the Display Transfer Characteristic (Gamma).
 *
 *  Input:
 *      pEDIDBuffer         - Buffer that contains the EDID structure of the monitor
 *
 *  Output:
 *      Gamma value multiplied by 100. A value of 0xFFFF (-1) indicates that
 *      the gamma value is not defined.
 */
unsigned short edidGetGamma(
    unsigned char *pEDIDBuffer
)
{
    unsigned char version;
    
    /* Get EDID Version and revision */
    version = ddk768_edidGetVersion(pEDIDBuffer, (unsigned char *)0);
    
    if (version == 1)
        return (unsigned short)(((edid_version_1_t *)pEDIDBuffer)->displayTransferChar + 100);
    
    return (-1);
}
#endif

/*
 *  ddk768_edidGetPowerManagementSupport
 *      This function gets the monitor's power management support.
 *
 *  Input:
 *      pEDIDBuffer     - Buffer that contains the EDID structure of the monitor
 *      pStandBy        - Pointer to a variable to store the flag to indicate that
 *                        standby power mode is supported.
 *      pSuspend        - Pointer to a variable to store the flag to indicate that
 *                        suspend power mode is supported.
 *      pLowPower       - Pointer to a variable to store the flag to indicate that
 *                        the display consumes low power when it receives a timing
 *                        signal that is outside its declared active operating range.
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
long ddk768_edidGetPowerManagementSupport(
    unsigned char *pEDIDBuffer,
    unsigned char *pStandBy,
    unsigned char *pSuspend,
    unsigned char *pLowPower
)
{
    unsigned char version;
    
    /* Get EDID Version and revision */
    version = ddk768_edidGetVersion(pEDIDBuffer, (unsigned char *)0);
    
    if (version == 1)
    {
        edid_version_1_t *pEDIDStructure;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        
        if (pStandBy != (unsigned char *)0)
            *pStandBy = (unsigned char) pEDIDStructure->featureSupport.standbySupport;
            
        if (pSuspend != (unsigned char *)0)
            *pSuspend = (unsigned char) pEDIDStructure->featureSupport.suspendSupport;
            
        if (pLowPower != (unsigned char *)0)
            *pLowPower = (unsigned char) pEDIDStructure->featureSupport.lowPowerSupport;
        
        return 0;
    }
    
    return (-1);
}

/*
 *  ddk768_edidGetDisplayType
 *      This function gets the display type.
 *
 *  Input:
 *      pEDIDBuffer     - Buffer that contains the EDID structure of the monitor
 *
 *  Output:
 *      0   - Monochrome / grayscale display
 *      1   - RGB Color Display
 *      2   - Non-RGB multicolor display, e.g. R/G/Y
 *      3   - Undefined
 */
unsigned char ddk768_edidGetDisplayType(
    unsigned char *pEDIDBuffer
)
{
    unsigned char version;
    
    /* Get EDID Version and revision */
    version = ddk768_edidGetVersion(pEDIDBuffer, (unsigned char *)0);
    
    if (version == 1)
        return (unsigned char)((edid_version_1_t *)pEDIDBuffer)->featureSupport.displayType;
    
    return (3);
}

/*
 *  ddk768_edidChecksRGBUsage
 *      This function checks if the display is using the sRGB standard default
 *      color space as its primary color space. 
 *
 *  Input:
 *      pEDIDBuffer     - Buffer that contains the EDID structure of the monitor
 *
 *  Output:
 *      0   - Does not use sRGB as its primary color space
 *      1   - Use sRGB as its primary color space
 */
unsigned char ddk768_edidChecksRGBUsage(
    unsigned char *pEDIDBuffer
)
{
    unsigned char version;
    
    /* Get EDID Version and revision */
    version = ddk768_edidGetVersion(pEDIDBuffer, (unsigned char *)0);
    
    if (version == 1)
        return (unsigned char)((edid_version_1_t *)pEDIDBuffer)->featureSupport.sRGBSupport;
    
    return (0);
}

/*
 *  ddk768_edidIsPreferredTimingAvailable
 *      This function checks whether the preffered timing mode is available.
 *      Use of preferred timing mode is required by EDID structure version 1
 *      Revision 3 and higher. 
 *
 *  Input:
 *      pEDIDBuffer     - Buffer that contains the EDID structure of the monitor
 *
 *  Output:
 *      0   - Preferred Timing is not available
 *      1   - Preferred Timing is available
 */
unsigned char ddk768_edidIsPreferredTimingAvailable(
    unsigned char *pEDIDBuffer
)
{
    unsigned char version;
    
    if (pEDIDBuffer != (unsigned char *)0)
    {
        /* Get EDID Version and revision */
        version = ddk768_edidGetVersion(pEDIDBuffer, (unsigned char *)0);
    
        if (version == 1)
            return (unsigned char)((edid_version_1_t *)pEDIDBuffer)->featureSupport.preferredTiming;
    }
        
    return (0);
}

/*
 *  ddk768_edidIsDefaultGTFSupported
 *      This function checks whether the display supports timings based on the
 *      GTF standard using default GTF parameter values. 
 *
 *  Input:
 *      pEDIDBuffer     - Buffer that contains the EDID structure of the monitor
 *
 *  Output:
 *      0   - Default GTF is not supported
 *      1   - Default GTF is supported
 */
unsigned char ddk768_edidIsDefaultGTFSupported(
    unsigned char *pEDIDBuffer
)
{
    unsigned char version, revision;
    
    /* Get EDID Version and revision */
    version = ddk768_edidGetVersion(pEDIDBuffer, (unsigned char *)0);
    
    if (version == 1)
        return (unsigned char)((edid_version_1_t *)pEDIDBuffer)->featureSupport.defaultGTFSupport;
    
    return (0);
}

/*
 *  edidCalculateChromaticValue
 *      This function calculates the chromatic value. 
 *
 *  Input:
 *      colorBinaryValue    - Color Characteristic Binary Representation Value 
 *                            to be computed
 *
 *  Output:
 *      The chromatic value times a 1000.
 */
static unsigned short edidCalculateChromaticValue(
    unsigned short colorBinaryValue
)
{
    unsigned long index;
    unsigned long result;
    
    result = 0;
    for (index = 10; index > 0; index--)
    {
        /* Times 1000000 to make it accurate to the micro value. */
        result += ddk768_roundedDiv((colorBinaryValue & 0x0001) * 1000000, ddk768_twoToPowerOfx(index));
        colorBinaryValue >>= 1;
    }
    
    /* Make it accurate to 1000 place */
    return ((unsigned short)ddk768_roundedDiv(result, 1000));
}

/*
 *  ddk768_edidGetColorCharacteristic
 *      This function gets the chromaticity and white point values expressed as
 *      an integer value which represents the actual value times 1000.
 *
 *  Input:
 *      pEDIDBuffer - Buffer that contains the EDID structure of the monitor
 *      pRedX   - Pointer to a variable to store the Red X values
 *      pRedY   - Pointer to a variable to store the Red Y values
 *      pGreenX - Pointer to a variable to store the Green X values
 *      pGreenY - Pointer to a variable to store the Green Y values
 *      pBlueX  - Pointer to a variable to store the Blue X values
 *      pBlueY  - Pointer to a variable to store the Blue Y values
 *
 *  Note:
 *      To get the White color characteristic, use the ddk768_edidGetWhitePoint
 */
void ddk768_edidGetColorCharacteristic(
    unsigned char *pEDIDBuffer,
    unsigned short *pRedX,
    unsigned short *pRedY,
    unsigned short *pGreenX,
    unsigned short *pGreenY,
    unsigned short *pBlueX,
    unsigned short *pBlueY
)
{
    unsigned char version, revision;
    
    /* Get EDID Version and revision */
    version = ddk768_edidGetVersion(pEDIDBuffer, (unsigned char *)0);
    
    if (version == 1)
    {
        edid_version_1_t *pEDIDStructure;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        
        if (pRedX != (unsigned short *)0)
        {
            *pRedX = edidCalculateChromaticValue(((unsigned short)pEDIDStructure->redX << 2) + 
                                                  (unsigned short)pEDIDStructure->redGreenLowBits.redXLowBits);
        }
            
        if (pRedY != (unsigned short *)0)
        {
            *pRedY = edidCalculateChromaticValue(((unsigned short)pEDIDStructure->redY << 2) + 
                                                  (unsigned short)pEDIDStructure->redGreenLowBits.redYLowBits);
        }
            
        if (pGreenX != (unsigned short *)0)
        {
            *pGreenX = edidCalculateChromaticValue(((unsigned short)pEDIDStructure->greenX << 2) + 
                                                    (unsigned short)pEDIDStructure->redGreenLowBits.greenXLowBits);
        }
            
        if (pGreenY != (unsigned short *)0)
        {
            *pGreenY = edidCalculateChromaticValue(((unsigned short)pEDIDStructure->greenY << 2) + 
                                                    (unsigned short)pEDIDStructure->redGreenLowBits.greenYLowBits);
        }
            
        if (pBlueX != (unsigned short *)0)
        {
            *pBlueX = edidCalculateChromaticValue(((unsigned short)pEDIDStructure->blueX << 2) +
                                                   (unsigned short)pEDIDStructure->blueWhiteLowBits.blueXLowBits);
        }
            
        if (pBlueY != (unsigned short *)0)
        {
            *pBlueY = edidCalculateChromaticValue(((unsigned short)pEDIDStructure->blueY << 2) +
                                                   (unsigned short)pEDIDStructure->blueWhiteLowBits.blueYLowBits);
        }
    }
}

/*
 *  ddk768_edidGetWhitePoint
 *      This function gets the white point.
 *      To get the default white point, set the index to 0. For multiple white point,
 *      call this function multiple times to check if more than 1 white point is supported.
 *
 *  Input:
 *      pEDIDBuffer - Buffer that contains the EDID structure of the monitor
 *      pWhitePointIndex    - Pointer to a variable that contains the white point index 
 *                            to be retrieved.
 *      pWhiteX             - Pointer to a variable to store the White X value
 *      pWhiteY             - Pointer to a variable to store the White Y value
 *      pWhiteGamma         - Pointer to a variable to store the White Gamma value
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
long ddk768_edidGetWhitePoint(
    unsigned char *pEDIDBuffer,
    unsigned char *pWhitePointIndex,
    unsigned short *pWhiteX,
    unsigned short *pWhiteY,
    unsigned short *pWhiteGamma
)
{
    unsigned char version, revision, index, tableIndex;
    
    if (pWhitePointIndex == (unsigned char *)0)
        return (-1);
    
    /* Get EDID Version and revision */
    version = ddk768_edidGetVersion(pEDIDBuffer, (unsigned char *)0);
    
    if (version == 1)
    {
        edid_version_1_t *pEDIDStructure;
        monitor_desc_t *pMonitorDescriptor;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        
        /* Get the index to a temporary variable and increment the index for the
           next loop. */
        index = *pWhitePointIndex;
        (*pWhitePointIndex)++;
        
        if (index == 0)
        {
            if (pWhiteX != (unsigned short *)0)
            {
                *pWhiteX = edidCalculateChromaticValue(((unsigned short)pEDIDStructure->whiteX << 2) +
                                                        (unsigned short)pEDIDStructure->blueWhiteLowBits.whiteXLowBits);
            }
            
            if (pWhiteY != (unsigned short *)0)
            {
                *pWhiteY = edidCalculateChromaticValue(((unsigned short)pEDIDStructure->whiteY << 2) +
                                                        (unsigned short)pEDIDStructure->blueWhiteLowBits.whiteYLowBits);
            }
            
            if (pWhiteGamma != (unsigned short *)0)
                *pWhiteGamma = pEDIDStructure->displayTransferChar + 100;
                
            return 0;
        }
        else
        {
            for (tableIndex = 0; tableIndex < 4; tableIndex++)
            {
                pMonitorDescriptor = &pEDIDStructure->miscInformation.monitorDesc[tableIndex];
                if ((pMonitorDescriptor->flag1 == 0) && (pMonitorDescriptor->flag2 == 0) &&
                    (pMonitorDescriptor->dataTypeTag == 0xFB) && 
                    (pMonitorDescriptor->descriptor.colorPoint.white[index-1].whitePointIndex != 0))
                {
                    if (pWhiteX != (unsigned short *)0)
                    {
                        *pWhiteX = edidCalculateChromaticValue(((unsigned short)pMonitorDescriptor->descriptor.colorPoint.white[index-1].whiteX << 2) +
                                                                (unsigned short)pMonitorDescriptor->descriptor.colorPoint.white[index-1].whiteLowBits.whiteXLowBits);
                    }
                    
                    if (pWhiteY != (unsigned short *)0)
                    {
                        *pWhiteY = edidCalculateChromaticValue(((unsigned short)pMonitorDescriptor->descriptor.colorPoint.white[index-1].whiteY << 2) +
                                                                (unsigned short)pMonitorDescriptor->descriptor.colorPoint.white[index-1].whiteLowBits.whiteYLowBits);
                    }
                    
                    if (pWhiteGamma != (unsigned short *)0)
                        *pWhiteGamma = pMonitorDescriptor->descriptor.colorPoint.white[index-1].gamma + 100;
                    
                    return 0;
                }
            }
        }
    }
    
    return (-1);
}

/*
 *  edidCalculateChecksum
 *      This function adds all one-byte value of the EDID buffer. 
 *      The total should be equal to 0x00
 *
 *  Input:
 *      pEDIDBuffer     - Buffer that contains the EDID structure of the monitor
 *
 *  Output:
 *      Total of one-byte values. It should equal to 0x00. A value other than
 *      0x00 indicates the EDID buffer is not valid.
 */
static unsigned char edidCalculateChecksum(
    unsigned char *pEDIDBuffer
)
{
    unsigned char version, revision, checksum;
    unsigned short index;
    
    /* Get EDID Version and revision */
    version = ddk768_edidGetVersion(pEDIDBuffer, &revision);
    
    checksum = 0;
    if (version == 1)
    {
        for (index = 0; index < 128; index++)
            checksum += pEDIDBuffer[index];
    }
    
    return checksum;
}

/*
 *  ddk768_edidGetExtension
 *      This function gets the number of (optional) EDID extension blocks to follow
 *      the given EDID buffer.
 *
 *  Input:
 *      pEDIDBuffer     - Buffer that contains the EDID structure of the monitor
 *
 *  Output:
 *      Total number of EDID Extension to follow the given EDID buffer.
 */
unsigned char ddk768_edidGetExtension(
    unsigned char *pEDIDBuffer
)
{
    unsigned char version, revision;
    
    /* Get EDID Version and revision */
    version = ddk768_edidGetVersion(pEDIDBuffer, &revision);
    
    if (version == 1)
        return ((edid_version_1_t *)pEDIDBuffer)->extFlag;
    
    return 0;
}

#define EDID_TOTAL_RETRY_COUNTER            4
/*
 *  ddk768_edidReadMonitor
 *      This function reads the EDID structure from the attached monitor
 *
 *  Input:
 *      displayPath - Display device which EDID to be read from.
 *      pEDIDBuffer - Buffer that contains the EDID structure of the monitor
 *      bufferSize  - The EDID Buffer size index (usually 128-bytes)
 *      edidExtNo   - Extension Index of the EDID Structure to be read
 *      sclGpio     - GPIO pin used as the I2C Clock (SCL)
 *      sdaGpio     - GPIO pin used as the I2C Data (SDA)
 *
 *  Output:
 *      0   - Fail
 *      edidSize   - Success and return the edid's size
 */
long ddk768_edidReadMonitorEx(
    unsigned char *pEDIDBuffer,
    unsigned long bufferSize,
    unsigned char edidExtNo,
    unsigned char sclGpio,
    unsigned char sdaGpio
)
{
    unsigned char value, retry, edidVersion, edidRevision;
    unsigned char edidBuffer[TOTAL_EDID_REGISTERS_256];
    unsigned long offset;
    long edidSize = TOTAL_EDID_REGISTERS_128;

    /* Initialize the i2c bus */
    ddk768_swI2CInit(sclGpio, sdaGpio);

    for (retry = 0; retry < EDID_TOTAL_RETRY_COUNTER; retry++)
    {
       // DDKDEBUGPRINT((DISPLAY_LEVEL, "retry: %d\n", retry));
            
        /* Read the EDID from the monitor. */
        for (offset = 0; offset < TOTAL_EDID_REGISTERS_128; offset++)
            edidBuffer[offset] = ddk768_swI2CReadReg(EDID_DEVICE_I2C_ADDRESS, (unsigned char)offset);
         	if(edidBuffer[EDID_EXTEND_BLOCK])
       	 	{
            	for (offset = TOTAL_EDID_REGISTERS_128; offset < TOTAL_EDID_REGISTERS_256; offset++)
                edidBuffer[offset] = ddk768_swI2CReadReg(EDID_DEVICE_I2C_ADDRESS, (unsigned char)offset);
            	edidSize = TOTAL_EDID_REGISTERS_256;
         	}	

        /* Check if the EDID is valid. */
        edidVersion = ddk768_edidGetVersion((unsigned char *)&edidBuffer, (unsigned char *)&edidRevision);
        //DDKDEBUGPRINT((DISPLAY_LEVEL, "EDID Structure Version: %d.%d\n", edidVersion, edidRevision)); 
        if (edidVersion != 0)
            break;
    }
    
    /* 
     *  The monitor might not be DDC2B compliance. Therefore, need to use DDC1 protocol,
     *  which uses the Vertical Sync to clock in the EDID data.
     *  Currently this function return error. DDC1 protocol can be added later.
     */
    if (retry == EDID_TOTAL_RETRY_COUNTER)
    {
        /* DDC1 uses the SDA line to transmit 9 bit data per byte. The last bit is
         * only an acknowledge flag, which could be high or low. However, SCL line
         * is not used. Instead the data is clock-in using vertical sync.
         */
        return 0;
    }

    /* Copy the data to the given buffer */
    if (pEDIDBuffer != (unsigned char *)0)
    {
        for (offset = 0; offset < edidSize; offset++)
            pEDIDBuffer[offset] = edidBuffer[offset];
    }

    return edidSize;
}

/*
 *  This function is same as editReadMonitorEx(), but using HW I2C.
 *
 *  Input:
 *      pEDIDBuffer - Buffer that contains the EDID structure of the monitor
 *      bufferSize  - The EDID Buffer size index (usually 128-bytes)
 *      edidExtNo   - Extension Index of the EDID Structure to be read
 *      i2cNumber   - 0 = I2C0 and 1 = I2C1
 *
 *  Output:
 *      0   - Fail
 *     edidSize   - Success and return the edid's size.
 */
long ddk768_edidReadMonitorExHwI2C(
    unsigned char *pEDIDBuffer,
    unsigned long bufferSize,
    unsigned char edidExtNo,
    unsigned char i2cNumber
)
{
    unsigned char  retry, edidVersion, edidRevision;
    unsigned long value;
    unsigned char edidBuffer[TOTAL_EDID_REGISTERS_256];
    unsigned long offset;
    long edidSize = 0;
    
    /* Initialize the i2c bus */
    ddk768_hwI2CInit(i2cNumber);
#if 0
    for (retry = 0; retry < EDID_TOTAL_RETRY_COUNTER; retry++)
    {
       // DDKDEBUGPRINT((DISPLAY_LEVEL, "retry: %d\n", retry));
            
        /* Read the EDID from the monitor. */
        for (offset = 0; offset < TOTAL_EDID_REGISTERS; offset++)
            edidBuffer[offset] = ddk768_hwI2CReadReg(i2cNumber, EDID_DEVICE_I2C_ADDRESS, (unsigned char)offset);
            
        /* Check if the EDID is valid. */
        edidVersion = ddk768_edidGetVersion((unsigned char *)&edidBuffer, (unsigned char *)&edidRevision);
      //  DDKDEBUGPRINT((DISPLAY_LEVEL, "EDID Structure Version: %d.%d\n", edidVersion, edidRevision)); 
        if (edidVersion != 0)
            break;
    }
#else
	
    for (retry = 0; retry < EDID_TOTAL_RETRY_COUNTER; retry++)
    {
        //DDKDEBUGPRINT((DISPLAY_LEVEL, "retry: %d\n", retry));
		edidSize = 0;
        /* Read the EDID from the monitor. */
        for (offset = 0; offset < TOTAL_EDID_REGISTERS_128; offset++)
        {
            value = ddk768_hwI2CReadReg(i2cNumber,EDID_DEVICE_I2C_ADDRESS, (unsigned char)offset);
            if(0xFFFFFFFF == value)
                break;
            edidBuffer[offset] = (0xFF & value);
        }
        if(0xFFFFFFFF != value)
        {
          	edidSize = TOTAL_EDID_REGISTERS_128;
            if(edidBuffer[EDID_EXTEND_BLOCK])
            {
                for (offset = TOTAL_EDID_REGISTERS_128; offset < TOTAL_EDID_REGISTERS_256; offset++)
                {
                    value = ddk768_hwI2CReadReg(i2cNumber,EDID_DEVICE_I2C_ADDRESS, (unsigned char)offset);

                    if(0xFFFFFFFF == value)
                        break;
                    edidBuffer[offset] = (0xFF & value);
                }

                if(0xFFFFFFFF != value)
                	edidSize = TOTAL_EDID_REGISTERS_256;
            }
            if(0xFFFFFFFF != value)
            {
                /* Check if the EDID is valid. */
                edidVersion = ddk768_edidGetVersion((unsigned char *)&edidBuffer, (unsigned char *)&edidRevision);
           //     DDKDEBUGPRINT((DISPLAY_LEVEL, "EDID Structure Version: %d.%d\n", edidVersion, edidRevision));
                if (edidVersion != 0)
                    break;
            }
        }
    }
#endif

    /* Finish using HW I2C, we can close the device. */
    ddk768_hwI2CClose(i2cNumber);
    
     /*
     *  The monitor might not be DDC2B compliance. Therefore, need to use DDC1 protocol,
     *  which uses the Vertical Sync to clock in the EDID data.
     *  Currently this function return error. DDC1 protocol can be added later.
     */
    if (retry == EDID_TOTAL_RETRY_COUNTER)
    {
        /* DDC1 uses the SDA line to transmit 9 bit data per byte. The last bit is
         * only an acknowledge flag, which could be high or low. However, SCL line
         * is not used. Instead the data is clock-in using vertical sync.
         */
        return 0;
    }

    /* Copy the data to the given buffer */
    if (pEDIDBuffer != (unsigned char *)0)
    {
        for (offset = 0; offset < edidSize; offset++)
            pEDIDBuffer[offset] = edidBuffer[offset];
    }

    return edidSize;
}



/*
 *  ddk768_edidReadMonitor
 *      This function reads the EDID structure from the attached monitor
 *
 *  Input:
 *      pEDIDBuffer - Buffer that contains the EDID structure of the monitor
 *      bufferSize  - The EDID Buffer size index (usually 128-bytes)
 *      edidExtNo   - Extension Index of the EDID Structure to be read
 *      i2cNumber   - 0 = I2c0 and 1 = 12c1
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
long ddk768_edidReadMonitor(
    unsigned char *pEDIDBuffer,
    unsigned long bufferSize,
    unsigned char edidExtNo,
    unsigned char i2cNumber
)
{

	unsigned char i2cSCL, i2cSDA;

    if (i2cNumber == 0)
    {
        i2cSCL = DEFAULT_I2C0_SCL;
        i2cSDA = DEFAULT_I2C0_SDA;
    }
    else
    {
        i2cSCL = DEFAULT_I2C1_SCL;
        i2cSDA = DEFAULT_I2C1_SDA;
    }

    return ddk768_edidReadMonitorEx(pEDIDBuffer, bufferSize, edidExtNo, i2cSCL, i2cSDA);
}


/*
 *  edidHeaderReadMonitor
 *      This function reads the EDID header from the attached monitor
 *
 *  Input:
 *      sclGpio     - GPIO pin used as the I2C Clock (SCL)
 *      sdaGpio     - GPIO pin used as the I2C Data (SDA)
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
long ddk768_edidHeaderReadMonitorEx(
    unsigned char sclGpio,
    unsigned char sdaGpio
)
{
    unsigned char retry;//value,
    unsigned char edidBuffer[10];
    unsigned long offset;

    /* Initialize the i2c bus */
    ddk768_swI2CInit(sclGpio, sdaGpio);

    for (retry = 0; retry < EDID_TOTAL_RETRY_COUNTER; retry++)
    {
        DDKDEBUGPRINT((DISPLAY_LEVEL, "retry: %d\n", retry));

        /* Read the EDID from the monitor. */
        for (offset = 0; offset < HEADER_EDID_REGISTERS; offset++)
            edidBuffer[offset] = ddk768_swI2CReadReg(EDID_DEVICE_I2C_ADDRESS, (unsigned char)offset);

        /* Check if the EDID header is valid. */
        if (!edidGetHeader((unsigned char *)&edidBuffer))
            break;
    }

    /*
     *  The monitor might not be DDC2B compliance. Therefore, need to use DDC1 protocol,
     *  which uses the Vertical Sync to clock in the EDID data.
     *  Currently this function return error. DDC1 protocol can be added later.
     */
    if (retry == EDID_TOTAL_RETRY_COUNTER)
    {
        /* DDC1 uses the SDA line to transmit 9 bit data per byte. The last bit is
         * only an acknowledge flag, which could be high or low. However, SCL line
         * is not used. Instead the data is clock-in using vertical sync.
         */
        return (-1);
    }

    return 0;
}

long ddk768_edidHeaderReadMonitorExHwI2C(
    unsigned char i2cNumber
)
{
    unsigned char retry;//value,
    unsigned char edidBuffer[10];
    unsigned long offset;

    /* Initialize the i2c bus */
    ddk768_hwI2CInit(i2cNumber);

    for (retry = 0; retry < EDID_TOTAL_RETRY_COUNTER; retry++)
    {
        DDKDEBUGPRINT((DISPLAY_LEVEL, "retry: %d\n", retry));

        /* Read the EDID from the monitor. */
        for (offset = 0; offset < HEADER_EDID_REGISTERS; offset++)
            edidBuffer[offset] = ddk768_hwI2CReadReg(i2cNumber,EDID_DEVICE_I2C_ADDRESS, (unsigned char)offset);

        /* Check if the EDID header is valid. */
        if (!edidGetHeader((unsigned char *)&edidBuffer))
            break;
    }

	/* Finish using HW I2C, we can close the device. */
    ddk768_hwI2CClose(i2cNumber);

    /*
     *  The monitor might not be DDC2B compliance. Therefore, need to use DDC1 protocol,
     *  which uses the Vertical Sync to clock in the EDID data.
     *  Currently this function return error. DDC1 protocol can be added later.
     */
    if (retry == EDID_TOTAL_RETRY_COUNTER)
    {
        /* DDC1 uses the SDA line to transmit 9 bit data per byte. The last bit is
         * only an acknowledge flag, which could be high or low. However, SCL line
         * is not used. Instead the data is clock-in using vertical sync.
         */
        return (-1);
    }

    return 0;
}



/*
 *  ddk768_edidGetEstablishedTiming
 *      This function gets the established timing list from the given EDID buffer,
 *      table, and timing index.
 *
 *  Input:
 *      pEDIDBuffer     - Buffer that contains the EDID structure of the monitor (In)
 *      pEstTableIndex  - Pointer to the Established Timing Table index  (In/Out)
 *      pIndex          - Pointer to the Establihsed Timing Index (In/Out)
 *      pWidth          - Pointer to a variable that to store the horizontal active / width
 *                        value of the retrieved timing (Out)
 *      pHeight         - Pointer to a variable to store the vertical active / height
 *                        value of the retrieved timing (Out)
 *      pRefreshRate    - Pointer to a variable to store the vertical frequency value
 *                        of the retrieved timing (out)
 *      pSource         - Pointer to a variable to store the standard timing source:
 *                          0 - VESA
 *                          1 - IBM
 *                          2 - Apple
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
long ddk768_edidGetEstablishedTiming(
    unsigned char *pEDIDBuffer,
    /*unsigned char *pEstTableIndex,*/
    unsigned char *pIndex,
    unsigned long *pWidth,
    unsigned long *pHeight,
    unsigned long *pRefreshRate,
    unsigned char *pSource
)
{
    unsigned char version, revision;
    unsigned char tableIndex, index;
    
    /* Get EDID Version and revision */
    version = ddk768_edidGetVersion(pEDIDBuffer, &revision);
    
    if (version == 1)
    {
        edid_version_1_t *pEDIDStructure;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        
        while (1)
        {
            /* Get index */
            index = *pIndex;
            
            if (index > 16)
                break;
            
            /* Search Established Table index 0 when the index is less than 8 */
            tableIndex = index / 8;
            
            /* Exit the function when it has reached the last table. */
            if (tableIndex > 2)
                break;
            
            /* Increment the index value and update the index accordingly */
            (*pIndex)++;
            index %= 8;
            
            /* Check */
            if ((pEDIDStructure->estTiming[tableIndex] & (1 << index)) != 0)
            {
                if (pWidth != (unsigned long *)0)
                    *pWidth = establishTiming[tableIndex][index].x;
        
                if (pHeight != (unsigned long *)0)
                    *pHeight = establishTiming[tableIndex][index].y;
    
                if (pRefreshRate != (unsigned long *)0)
                    *pRefreshRate = establishTiming[tableIndex][index].hz;
    
                if (pSource != (unsigned char *)0)
                    *pSource = establishTiming[tableIndex][index].source;
                    
                /* Return success */
                return 0;
            }
        }
    }
    else
    {
        /* EDID Structure Version 2.0. */
    }
    
    return (-1);
}

/*
 *  edidCalculateStdTiming
 *      This function calculates the width, height, and vertical frequency values
 *      from the given Standard Timing structure. This function only applies to
 *      EDID structure version 1. It will give the wrong result when used with
 *      EDID version 2.
 *
 *  Input:
 *      pStdTiming      - Pointer to a standard timing structure that contains the
 *                        standard timing value to be calculated (In)
 *      edid1Revision   - Revision of the EDID 1 (In)
 *      pWidth          - Pointer to a variable that to store the horizontal active / width
 *                        value of the retrieved timing (Out)
 *      pHeight         - Pointer to a variable to store the vertical active / height
 *                        value of the retrieved timing (Out)
 *      pRefreshRate    - Pointer to a variable to store the vertical frequency value
 *                        of the retrieved timing (out)
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
static long edidCalculateStdTiming(
    standard_timing_t *pStdTiming,
    unsigned char edid1Revision,
    unsigned long *pWidth,
    unsigned long *pHeight,
    unsigned long *pRefreshRate
)
{
    unsigned long x, y;
     
    /* Calculate the standard timing into x and y mode dimension */
    if (pStdTiming->horzActive != 0x01)
    {
        /* Calculate the X and Y */
        x = (pStdTiming->horzActive + 31) * 8;
        switch (pStdTiming->stdTimingInfo.aspectRatio)
        {
            case 0:
                if (edid1Revision != 3)
                    y = x;                  /* 1:1 aspect ratio (prior revision 1.3) */
                else
                    y = x * 10 / 16;        /* 16:10 aspect ratio (revision 1.3) */
                break;
            case 1:
                y = x * 3 / 4;              /* 4:3 aspect ratio */
                break;
            case 2:
                y = x * 4 / 5;              /* 5:4 aspect ratio */
                break;
            case 3:
                y = x * 9 / 16;             /* 16:9 aspect ratio */
                break;
        }

        if (pWidth != (unsigned long *)0)
            *pWidth = x;

        if (pHeight != (unsigned long *)0)
            *pHeight = y;

        if (pRefreshRate != (unsigned long *)0)
            *pRefreshRate = pStdTiming->stdTimingInfo.refreshRate + 60;
    
        return 0;
    }
    
    return (-1);
}

/*
 *  ddk768_edidGetStandardTiming
 *      This function gets the standard timing from the given EDID buffer and
 *      calculates the width, height, and vertical frequency from that timing.
 *
 *  Input:
 *      pEDIDBuffer     - Buffer that contains the EDID structure of the monitor
 *      pStdTimingIndex - Pointer to a standard timing index to be retrieved
 *      pWidth          - Pointer to a variable that to store the horizontal active / width
 *                        value of the retrieved timing (Out)
 *      pHeight         - Pointer to a variable to store the vertical active / height
 *                        value of the retrieved timing (Out)
 *      pRefreshRate    - Pointer to a variable to store the vertical frequency value
 *                        of the retrieved timing (out)
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
long ddk768_edidGetStandardTiming(
    unsigned char *pEDIDBuffer,
    unsigned char *pStdTimingIndex,
    unsigned long *pWidth,
    unsigned long *pHeight,
    unsigned long *pRefreshRate
)
{
    unsigned char version, revision, timingIndex, tableIndex;
  
    
    /* Get EDID Version and revision */
    version = ddk768_edidGetVersion(pEDIDBuffer, &revision);
    
    if (version == 1)
    {
        edid_version_1_t *pEDIDStructure;
        monitor_desc_t *pMonitorDescriptor;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        
        while (1)
        {
            /* There are only 8 standard timing entries */
            if (*pStdTimingIndex > 7)
                break;
            
            /* Get the table index first before incrementing the index. */
            timingIndex = *pStdTimingIndex;
            
            /* Increment the standard timing index */
            (*pStdTimingIndex)++;
            
            if (timingIndex < 8)
            {
                /*
                 *  Search the first Standard Timing Identifier table
                 */
                 
                /* Calculate the standard timing into x and y mode dimension */
                if (edidCalculateStdTiming(&pEDIDStructure->stdTiming[timingIndex], 
                                       revision, pWidth, pHeight, pRefreshRate) == 0)
                {
                    return 0;
                }
            }
            else
            {
                /*
                 *  Search Standard Timing Identifier Table in the detailed Timing block. 
                 */
                
                /* 
                 * Each Detailed Timing Identifier can contains 6 entries of Standard Timing
                 * Identifier. Based on this value, we can get the Detailed Timing Table Index
                 * that contains the requested standard timing.
                 */
                timingIndex = timingIndex - 8;
                for (tableIndex = 0; tableIndex < 4; tableIndex++)
                {
                    /* Get detailed info */
                    pMonitorDescriptor = &pEDIDStructure->miscInformation.monitorDesc[tableIndex];
                    if ((pMonitorDescriptor->flag1 == 0) && (pMonitorDescriptor->flag2 == 0) &&
                        (pMonitorDescriptor->dataTypeTag == 0xFA))
                    {
                        if (timingIndex >= 6)
                        {
                            timingIndex-=6;
                            continue;
                        }
                        else
                        {
                            if (edidCalculateStdTiming(&pMonitorDescriptor->descriptor.stdTimingExt.stdTiming[timingIndex], 
                                                   revision, pWidth, pHeight, pRefreshRate) == 0)
                            {
                                return 0;
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        /* EDID Structure version 2 */
    }
    
    return (-1);
}

/*
 *  ddk768_edidGetDetailedTiming
 *      This function gets the detailed timing from the given EDID buffer.
 *
 *  Input:
 *      pEDIDBuffer             - Buffer that contains the EDID structure of the monitor
 *      pDetailedTimingIndex    - Pointer to a detailed timing index to be retrieved
 *      pModeParameter          - Pointer to a mode_parameter_t structure that will be
 *                                filled with the detailed timing.
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
long ddk768_edidGetDetailedTiming(
    unsigned char *pEDIDBuffer,
    unsigned char *pDetailedTimingIndex,
    vdif_t *pVDIF
)
{
    unsigned char version, revision, tableIndex;
    unsigned long x, y, aspectRatio;
    
    /* Get EDID Version and revision */
    version = ddk768_edidGetVersion(pEDIDBuffer, &revision);
    
    if (version == 1)
    {
        edid_version_1_t *pEDIDStructure;
        detailed_timing_t *pDetailedTiming;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        
        while (1)
        {
            if (*pDetailedTimingIndex > 3)
                break;
            
            /* Get the Detail Timing entry index */
            tableIndex = *pDetailedTimingIndex;
            
            /* Increment the index */
            (*pDetailedTimingIndex)++;
                
            /* Get detailed info */
            pDetailedTiming = &pEDIDStructure->miscInformation.detailTiming[tableIndex];
            if ((pDetailedTiming->pixelClock != 0) && (pVDIF != (vdif_t *)0))
            {
                /* Translate the Detail timing to VDIF format. */
                pVDIF->pixelClock = (unsigned long)pDetailedTiming->pixelClock * 10000;
                pVDIF->characterWidth = 8;
                pVDIF->scanType = (pDetailedTiming->flags.interlaced == 0) ? VDIF_NONINTERLACED : VDIF_INTERLACED;

                pVDIF->horizontalActive = 
                    ((unsigned long)pDetailedTiming->horzActiveBlanking.horzActiveMSB << 8) +
                     (unsigned long)pDetailedTiming->horzActive;
                pVDIF->horizontalBlankStart = pVDIF->horizontalActive;
                pVDIF->horizontalBlankTime = 
                    ((unsigned long)pDetailedTiming->horzActiveBlanking.horzBlankingMSB << 8) +
                     (unsigned long)pDetailedTiming->horzBlanking;
                pVDIF->horizontalTotal = pVDIF->horizontalActive + pVDIF->horizontalBlankTime;
                pVDIF->horizontalFrontPorch = 
                    ((unsigned long)pDetailedTiming->syncAuxInfo.horzSyncOffset << 8) + 
                     (unsigned long)pDetailedTiming->horzSyncOffset;
                pVDIF->horizontalSyncStart = pVDIF->horizontalBlankStart + pVDIF->horizontalFrontPorch;
                pVDIF->horizontalSyncWidth = 
                    ((unsigned long)pDetailedTiming->syncAuxInfo.horzSyncWidth << 8) +
                     (unsigned long)pDetailedTiming->horzSyncPulseWidth;                     
                pVDIF->horizontalBackPorch = 
                    pVDIF->horizontalBlankTime - (pVDIF->horizontalFrontPorch + pVDIF->horizontalSyncWidth);
                pVDIF->horizontalFrequency = ddk768_roundedDiv(pVDIF->pixelClock, pVDIF->horizontalTotal);
                pVDIF->horizontalLeftBorder = 0;
                pVDIF->horizontalRightBorder = 0;
        
                pVDIF->verticalActive = 
                    ((unsigned long)pDetailedTiming->vertActiveBlanking.vertActiveMSB << 8) +
                     (unsigned long)pDetailedTiming->vertActive;
                pVDIF->verticalBlankStart = pVDIF->verticalActive;
                pVDIF->verticalBlankTime =
                    ((unsigned long)pDetailedTiming->vertActiveBlanking.vertBlankingMSB << 8) +
                     (unsigned long)pDetailedTiming->vertBlanking;
                pVDIF->verticalTotal = pVDIF->verticalActive + pVDIF->verticalBlankTime;
                pVDIF->verticalFrontPorch = 
                    ((unsigned long)pDetailedTiming->syncAuxInfo.vertSyncOffset << 8) +
                     (unsigned long)pDetailedTiming->verticalSyncInfo.syncOffset;
                pVDIF->verticalSyncStart = pVDIF->verticalBlankStart + pVDIF->verticalFrontPorch;
                pVDIF->verticalSyncHeight =
                    ((unsigned long)pDetailedTiming->syncAuxInfo.vertSyncWidth  << 8) +
                     (unsigned long)pDetailedTiming->verticalSyncInfo.syncWidth;
                pVDIF->verticalBackPorch =
                    pVDIF->verticalBlankTime - (pVDIF->verticalFrontPorch + pVDIF->verticalSyncHeight);
                pVDIF->verticalFrequency =
                    ddk768_roundedDiv(pVDIF->pixelClock, (pVDIF->horizontalTotal * pVDIF->verticalTotal));
                pVDIF->verticalTopBorder = 0;
                pVDIF->verticalBottomBorder = 0;
                
                if (pDetailedTiming->flags.connectionType == 3)
                {
                    pVDIF->verticalSyncPolarity = 
                        (pDetailedTiming->flags.vertSyncFlag == 1) ? VDIF_SYNC_POSITIVE : VDIF_SYNC_NEGATIVE;
                    pVDIF->horizontalSyncPolarity = 
                        (pDetailedTiming->flags.horzSyncFlag == 1) ? VDIF_SYNC_POSITIVE : VDIF_SYNC_NEGATIVE;
                }
                else
                {
                    pVDIF->verticalSyncPolarity = VDIF_SYNC_NEGATIVE;
                    pVDIF->horizontalSyncPolarity = VDIF_SYNC_NEGATIVE;
                }
                
                /* For debugging purpose. */
                printVdif(pVDIF);
                
                return 0;
            }
        }
    }
    
    return (-1);
}

/*
 *  ddk768_edidGetMonitorSerialNumber
 *      This function gets the monitor serial number from the EDID structure.
 *      Only EDID version 1 and revision 1 or above supports this feature.
 *
 *  Input:
 *      pEDIDBuffer             - Buffer that contains the EDID structure of the monitor
 *      pMonitorSerialNumber    - Pointer to a buffer to store the serial number 
 *                                retrieved from the EDID
 *      bufferSize              - The size of the buffer to store the serial number.
 *                                The maximum size required is 13 bytes.
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
long ddk768_edidGetMonitorSerialNumber(
    unsigned char *pEDIDBuffer,
    char *pMonitorSerialNumber,
    unsigned char bufferSize
)
{
    unsigned char version, revision, tableIndex, charIndex;
    
    /* If no pointer is given or the buffer size is set to 0, then return fail. */
    if ((pMonitorSerialNumber == (char *)0) || (bufferSize == 0))
        return (-1);
    
    /* Get EDID Version and revision */
    version = ddk768_edidGetVersion(pEDIDBuffer, &revision);
    
    if ((version == 1) && (revision > 0))
    {
        edid_version_1_t *pEDIDStructure;
        monitor_desc_t *pMonitorDescriptor;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        for (tableIndex = 0; tableIndex < 4; tableIndex++)
        {
            pMonitorDescriptor = &pEDIDStructure->miscInformation.monitorDesc[tableIndex];
            if ((pMonitorDescriptor->flag1 == 0) && (pMonitorDescriptor->flag2 == 0) &&
                (pMonitorDescriptor->dataTypeTag == 0xFF))
            {
                bufferSize = (bufferSize > 13) ? 13 : bufferSize;
                for (charIndex = 0; charIndex < 13; charIndex++)
                {
                    if (pMonitorDescriptor->descriptor.serialNo[charIndex] == 0x0A)
                    {
                        pMonitorSerialNumber[charIndex] = '\0';
                        break;
                    }
                        
                    pMonitorSerialNumber[charIndex] = pMonitorDescriptor->descriptor.serialNo[charIndex];
                }
                
                return 0;
            }
        }
    }
    
    /* Serial Number is not found. */
    return (-1);
}

/*
 *  ddk768_edidGetDataString
 *      This function gets the data string from the EDID 
 *      Only EDID version 1 and revision 1 or above supports this feature.
 *
 *  Input:
 *      pEDIDBuffer             - Buffer that contains the EDID structure of the monitor
 *      pMonitorSerialNumber    - Pointer to a buffer to store the data string 
 *                                retrieved from the EDID
 *      bufferSize              - The size of the buffer to store the data string
 *                                The maximum size required is 13 bytes.
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
long ddk768_edidGetDataString(
    unsigned char *pEDIDBuffer,
    char *pDataString,
    unsigned char bufferSize
)
{
    unsigned char version, revision, tableIndex, charIndex;
    
    /* If no pointer is given or the buffer size is set to 0, then return fail. */
    if ((pDataString == (char *)0) || (bufferSize == 0))
        return (-1);
    
    /* Get EDID Version and revision */
    version = ddk768_edidGetVersion(pEDIDBuffer, &revision);
    
    if ((version == 1) && (revision > 0))
    {
        edid_version_1_t *pEDIDStructure;
        monitor_desc_t *pMonitorDescriptor;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        for (tableIndex = 0; tableIndex < 4; tableIndex++)
        {
            pMonitorDescriptor = &pEDIDStructure->miscInformation.monitorDesc[tableIndex];
            if ((pMonitorDescriptor->flag1 == 0) && (pMonitorDescriptor->flag2 == 0) &&
                (pMonitorDescriptor->dataTypeTag == 0xFE))
            {
                bufferSize = (bufferSize > 13) ? 13 : bufferSize;
                for (charIndex = 0; charIndex < 13; charIndex++)
                {
                    if (pMonitorDescriptor->descriptor.dataString[charIndex] == 0x0A)
                    {
                        pDataString[charIndex] = '\0';
                        break;
                    }
                        
                    pDataString[charIndex] = pMonitorDescriptor->descriptor.dataString[charIndex];
                }
                
                return 0;
            }
        }
    }
    
    /* Data String is not found. */
    return (-1);
}

/*
 *  ddk768_edidGetMonitorRangeLimit
 *      This function gets the monitor range limits from the EDID structure.
 *      Only EDID version 1 revision 1 or above supports this feature.
 *      This is a required field in EDID Version 1.3
 *
 *  Input:
 *      pEDIDBuffer         - Buffer that contains the EDID structure of the monitor
 *      pMinVerticalRate    - Pointer to a variable to store the Minimum Vertical Rate (Hz)
 *      pMaxVerticalRate    - Pointer to a variable to store the Maximum Vertical Rate (Hz)
 *      pMinHorzFreq        - Pointer to a variable to store the Minimum Horz. Freq (kHz)
 *      pMaxHorzFreq        - Pointer to a variable to store the Maximum Horz. Freq (kHz)
 *      pMaxPixelClock      - Pointer to a variable to store the Maximum Pixel Clock (Hz)
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
long ddk768_edidGetMonitorRangeLimit(
    unsigned char *pEDIDBuffer,
    unsigned char *pMinVerticalRate,
    unsigned char *pMaxVerticalRate,
    unsigned char *pMinHorzFreq,
    unsigned char *pMaxHorzFreq,
    unsigned long *pMaxPixelClock
)
{
    unsigned char version, revision, tableIndex, charIndex;
    
    if (pEDIDBuffer == (unsigned char *)0)
        return (-1);
            
    /* Get EDID Version and revision */
    version = ddk768_edidGetVersion(pEDIDBuffer, &revision);
    
    if ((version == 1) && (revision > 0))
    {
        edid_version_1_t *pEDIDStructure;
        monitor_desc_t *pMonitorDescriptor;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        
        for (tableIndex = 0; tableIndex < 4; tableIndex++)
        {
            pMonitorDescriptor = &pEDIDStructure->miscInformation.monitorDesc[tableIndex];
            if ((pMonitorDescriptor->flag1 == 0) && (pMonitorDescriptor->flag2 == 0) &&
                (pMonitorDescriptor->dataTypeTag == 0xFD) && (pMonitorDescriptor->flag3 == 0))
            {
                if (pMinVerticalRate != (unsigned char *)0)
                    *pMinVerticalRate = pMonitorDescriptor->descriptor.monitorRange.minVertRate;

                if (pMaxVerticalRate != (unsigned char *)0)
                    *pMaxVerticalRate = pMonitorDescriptor->descriptor.monitorRange.maxVertRate;
                    
                if (pMinHorzFreq != (unsigned char *)0)
                    *pMinHorzFreq = pMonitorDescriptor->descriptor.monitorRange.minHorzFrequency;
                    
                if (pMaxHorzFreq != (unsigned char *)0)
                    *pMaxHorzFreq = pMonitorDescriptor->descriptor.monitorRange.maxHorzFrequency;
                    
                if (pMaxPixelClock != (unsigned long *)0)
                    *pMaxPixelClock = (unsigned long) pMonitorDescriptor->descriptor.monitorRange.maxPixelClock * 10 * 1000000;
                    
                return 0;
            }
        }
    }
    
    /* Data String is not found. */
    return (-1);
}

/*
 *  edidGetChannel1TimingSupport
 *      This function gets the secondary GTF timing support.
 *      Only EDID version 1 and revision 1 or above supports this feature.
 *
 *  Input:
 *      pEDIDBuffer             - Buffer that contains the EDID structure of the monitor
 *      pStartFrequency         - Pointer to a variable to store the start frequency of 
 *                                the secondary GTF
 *      pOffset                 - Pointer to a variable to store the Offset (C) value of
 *                                the secondary GTF
 *      pGradient               - Pointer to a variable to store the Gradient (M) value of
 *                                the secondary GTF
 *      pScalingFactor          - Pointer to a variable to store the Scaling Factor (K)
 *                                value of the secondary GTF
 *      pScalingFactorWeight    - Pointer to a variable to store the Scaling Factore Weight (J)
 *                                value of the secondary GTF
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
long edidGetChannel1TimingSupport(
    unsigned char *pEDIDBuffer,
    unsigned short *pStartFrequency,
    unsigned char *pOffset,
    unsigned short *pGradient,
    unsigned char *pScalingFactor,
    unsigned char *pScalingFactorWeight
)
{
    unsigned char version, revision, tableIndex, charIndex;
            
    /* Get EDID Version and revision */
    version = ddk768_edidGetVersion(pEDIDBuffer, &revision);
    
    if ((version == 1) && (revision > 0) && (pEDIDBuffer != (unsigned char *)0))
    {
        edid_version_1_t *pEDIDStructure;
        monitor_desc_t *pMonitorDescriptor;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        for (tableIndex = 0; tableIndex < 4; tableIndex++)
        {
            pMonitorDescriptor = &pEDIDStructure->miscInformation.monitorDesc[tableIndex];
            if ((pMonitorDescriptor->flag1 == 0) && (pMonitorDescriptor->flag2 == 0) &&
                (pMonitorDescriptor->dataTypeTag == 0xFD) && 
                (pMonitorDescriptor->descriptor.monitorRange.secondaryTimingFlag == 0x02))
            {
                if (pStartFrequency != (unsigned short *)0)
                    *pStartFrequency = (unsigned short)
                        pMonitorDescriptor->descriptor.monitorRange.secondaryTimingInfo.cmkjParam.startFrequency * 2 * 1000;

                if (pOffset != (unsigned char *)0)
                    *pOffset = pMonitorDescriptor->descriptor.monitorRange.secondaryTimingInfo.cmkjParam.cParam/2;
                    
                if (pGradient != (unsigned short *)0)
                    *pGradient = pMonitorDescriptor->descriptor.monitorRange.secondaryTimingInfo.cmkjParam.mParam;
                    
                if (pScalingFactor != (unsigned char *)0)
                    *pScalingFactor = pMonitorDescriptor->descriptor.monitorRange.secondaryTimingInfo.cmkjParam.kParam;
                    
                if (pScalingFactorWeight != (unsigned char *)0)
                    *pScalingFactorWeight = pMonitorDescriptor->descriptor.monitorRange.secondaryTimingInfo.cmkjParam.jParam / 2;
                    
                return 0;
            }
        }
    }
    
    /* Data String is not found. */
    return (-1);
}

/*
 *  ddk768_edidGetMonitorName
 *      This function gets the monitor name from the EDID structure.
 *      This is a required field in EDID Version 1.3
 *
 *  Input:
 *      pEDIDBuffer             - Buffer that contains the EDID structure of the monitor
 *      pMonitorName            - Pointer to a buffer to store the monitor name 
 *                                retrieved from the EDID
 *      bufferSize              - The size of the buffer to store the monitor name
 *                                The maximum size required is 13 bytes.
 * 
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
long ddk768_edidGetMonitorName(
    unsigned char *pEDIDBuffer,
    char *pMonitorName,
    unsigned char bufferSize
)
{
    unsigned char version, revision, tableIndex, charIndex;
    
    /* If no pointer is given or the buffer size is set to 0, then return fail. */
    if ((pMonitorName == (char *)0) || (bufferSize == 0))
        return (-1);
        
    /* Get EDID Version and revision */
    version = ddk768_edidGetVersion(pEDIDBuffer, &revision);
    
    if (version == 1)
    {
        edid_version_1_t *pEDIDStructure;
        monitor_desc_t *pMonitorDescriptor;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        for (tableIndex = 0; tableIndex < 4; tableIndex++)
        {
            pMonitorDescriptor = &pEDIDStructure->miscInformation.monitorDesc[tableIndex];
            if ((pMonitorDescriptor->flag1 == 0) && (pMonitorDescriptor->flag2 == 0) &&
                (pMonitorDescriptor->dataTypeTag == 0xFC) && (pMonitorDescriptor->flag3 == 0))
            {
                bufferSize = (bufferSize > 13) ? 13 : bufferSize;
                for (charIndex = 0; charIndex < 13; charIndex++)
                {
                    if (pMonitorDescriptor->descriptor.monitorName[charIndex] == 0x0A)
                    {
                        pMonitorName[charIndex] = '\0';
                        break;
                    }
                        
                    pMonitorName[charIndex] = pMonitorDescriptor->descriptor.monitorName[charIndex];
                }
                
                return 0;
            }
        }
    }
    
    /* Data String is not found. */
    return (-1);
}

/*
 *  ddk768_edidGetPreferredTiming
 *      This function gets the preferred/native timing of the monitor
 *
 *  Input:
 *      pEDIDBuffer         - Buffer that contains the EDID structure of the monitor
 *      pWidth              - Pointer to an unsigned long buffer to store the width 
 *                            of the preferred (native) timing.
 *      pHeight             - Pointer to an unsigned long buffer to store the height
 *                            of the preferred (native) timing.
 *      pVerticalFrequency  - Pointer to an unsigned long buffer to store the refresh
 *                            rate of the preferred (native) timing.
 * 
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
long ddk768_edidGetPreferredTiming(
    unsigned char *pEDIDBuffer,
    unsigned long *pWidth,
    unsigned long *pHeight,
    unsigned long *pVerticalFrequency
)
{
    unsigned char index = 0;
    vdif_t vdifBuffer;


    {
        /* The preferred (native) timing is available, so get the timing. It is located
           at the first index of detailed timing.
         */
        if (ddk768_edidGetDetailedTiming(pEDIDBuffer, &index, &vdifBuffer) == 0)
        {
            if (pWidth != (unsigned long *)0)
                *pWidth = vdifBuffer.horizontalActive;
                
            if (pHeight != (unsigned long *)0)
                *pHeight = vdifBuffer.verticalActive;
                
            if (pVerticalFrequency != (unsigned long *)0)
                *pVerticalFrequency = vdifBuffer.verticalFrequency;
                
            return 0;
        }
    }
    
    return (-1);
}
