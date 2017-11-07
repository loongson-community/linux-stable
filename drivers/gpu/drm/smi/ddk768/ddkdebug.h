/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  ddkdebug.h --- DDK Debug module 
*  This file contains the definitions for the SMI DDK debugging.
* 
*******************************************************************/
#ifndef _DDKDEBUG_H_
#define _DDKDEBUG_H_

#ifdef DDKDEBUG

/*********************
 * Definition  
 *********************/
 
/* Debug Print Level definitions */
/* Bit 16 ~ 31 are used by the library. Bit 0 ~ 15 can be used by application */
#define ERROR_LEVEL                 0x00010000
#define WARNING_LEVEL               0x00020000
#define INIT_LEVEL                  0x00040000
#define DISPLAY_LEVEL               0x00080000
#define DMA_LEVEL                   0x00100000
#define DE_LEVEL                    0x00200000
#define CAPTURE_LEVEL               0x00400000
#define SSP_LEVEL                   0x00800000
#define RESERVED8_LEVEL             0x01000000
#define RESERVED9_LEVEL             0x02000000
#define RESERVED10_LEVEL            0x04000000
#define RESERVED11_LEVEL            0x08000000
#define RESERVED12_LEVEL            0x10000000
#define RESERVED13_LEVEL            0x20000000
#define RESERVED14_LEVEL            0x40000000
#define RESERVED15_LEVEL            0x80000000

#define SYSTEM_LEVEL_MASK           0xFFFF0000
#define APPLICATION_LEVEL_MASK      0x0000FFFF

/*********************
 * Structure 
 *********************/
typedef enum _ddk_debug_output_t
{
    DEBUG_OUTPUT_SCREEN = 0,
    DEBUG_OUTPUT_FILE,
    DEBUG_OUTPUT_SERIAL
} 
ddk_debug_output_t;

/*********************
 * MACROS 
 *********************/
 
/* This function has to be called before calling other DEBUGPIRNT functions. */
#define DDKDEBUGPRINTINIT(debugOutput, debugLevelMask) \
    ddkDebugPrintInit(debugOutput, debugLevelMask)

/* This function enable or disable the debug message. 
 *  Note:
 *      This function can be used to enable/disable the debug message
 *      at certain point of software, so that the debug messages, that
 *      are printed, are only the important ones.
 */
#define DDKDEBUGENABLE(arg)                           \
    ddkDebugEnable(arg)

/* Calling the DDKDEBUGPRINT needs to have the arg to be enclosed with
   two of open and close brackets.
   Example:
            DDKDEBUGPRINT(("Hello World: %s\n", pszString));
 */
#define DDKDEBUGPRINT(arg)                             \
    ddkDebugPrint arg
    
/* This function has to be called when exiting the application.
   It is necessary to clean up the debug module. */
#define DDKDEBUGPRINTEXIT()                            \
    ddkDebugPrintExit()

/*********************
 * Function prototype 
 *********************/
 
/*
 * This function initializes the debug print out system.
 *  
 *  Input:
 *      debugOutput - Output where to print out the debug. It could be 
 *                    screen, file, or serial port.
 *      debugLevel  - Debugging level      
 */
void ddkDebugPrintInit(ddk_debug_output_t debugOutput, unsigned long debugLevelMask);

/*
 *  This function enable or disable the debug message.
 *  
 *  Input:
 *      enableDebugMessage  - Enable/disable the debug message
 *                            0 - Disable Debug Message
 *                            1 - Enable Debug Message
 *
 *  Note:
 *      This function can be used to enable/disable the debug message
 *      at certain point of software, so that the debug messages, that
 *      are printed, are only the important ones.     
 */
void ddkDebugEnable(unsigned char enableDebugMessage);

/*
 * This function prints out the formatted string.
 *  
 *  Input:
 *      debugLevel  - The level of the debug of which the message is intended for.
 *      pszFormat   - Format of the printed message      
 */
void ddkDebugPrint(unsigned long debugLevel, const char* pszFormat, ...);

/*
 * This function cleans up (such as closing the debug file, etc...) when
 * exiting the debug module.      
 */
void ddkDebugPrintExit();

#else

/* 
 * If there is no DEBUG definition, then treat the macro as an empty macro.
 * Therefore all the debug print will be stripped out. 
 */
#define DDKDEBUGPRINTINIT(debugOutput, debugLevelMask)
#define DDKDEBUGENABLE(arg)
#define DDKDEBUGPRINT(arg)
#define DDKDEBUGPRINTEXIT()

#endif

#endif /* _DDKDEBUG_H_ */
