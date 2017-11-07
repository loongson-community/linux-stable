/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  ddkdebug.c --- DDK Debug Tool 
*  This file contains the source code for the SMI DDK Debugging.
* 
*******************************************************************/
#ifdef DDKDEBUG /* Don't enable debug flag in ARM yet */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "ddkdebug.h"
#include "os.h"

/* COM Port index that is used for the debugging */
#define DEBUG_COM_PORT_INDEX                0

/* Buffer length */
#define BUFFER_LENGTH                       1024

static ddk_debug_output_t gDebugOutput = DEBUG_OUTPUT_SCREEN;
static unsigned long gDebugLevelMask = 0;
static FILE *gFileHandle = (FILE *)0;
static short gCOMInit = 0;
static unsigned char gEnableDebugMessage;

/*
 * This function initializes the debug print out system.
 *  
 *  Input:
 *      debugOutput - Output where to print out the debug. It could be 
 *                    screen, file, or serial port.
 *      debugLevel  - Debugging level      
 */
void ddkDebugPrintInit(ddk_debug_output_t debugOutput, unsigned long debugLevelMask)
{
    gDebugOutput = debugOutput;
    gDebugLevelMask = debugLevelMask;
    gEnableDebugMessage = 1;
    
    /* Initialize the output media as necessary */
    switch (gDebugOutput)
    {
        default:
        case DEBUG_OUTPUT_SCREEN:
            /* Do nothing */
            break;
        case DEBUG_OUTPUT_FILE:
            /* Close the previous log file if opened */
            if (gFileHandle != (FILE *)0)
                fclose(gFileHandle);
            
            /* Create a LOG file */    
            gFileHandle = fopen("ddkdebug.log", "w");
            if (gFileHandle == NULL)
                ddkDebugPrint(0, "Can not open log file\n");
            break;
        case DEBUG_OUTPUT_SERIAL:
            /* Open COM Port */
            if (comInit(DEBUG_COM_PORT_INDEX,
                        COM_9600, 
                        DATA_SIZE_8, 
                        PARITY_NONE, 
                        STOP_BIT_1,
                        FLOW_CONTROL_NONE) == 0)
                gCOMInit = 1;
            else
                ddkDebugPrint(0, "Can not open COM Port\n");            
            break;
    }
}

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
void ddkDebugEnable(unsigned char enableDebugMessage)
{
    gEnableDebugMessage = enableDebugMessage;
}

/*
 * This function prints out the formatted string.
 *  
 *  Input:
 *      debugLevel  - The level of the debug of which the message is intended for.
 *      pszFormat   - Format of the printed message      
 */
void ddkDebugPrint(unsigned long debugLevel, const char* pszFormat, ...)
{
	static char pszPrintBuffer[BUFFER_LENGTH];
	unsigned long nWritten;

    /* Do not print any messages when this variable is flagged. */
    if (gEnableDebugMessage == 0)
        return;
        
    /* Only process any ddkDebugPrint with the debugLevel less or equal the preset
       debug Level during the init */
    if (((debugLevel & gDebugLevelMask) != 0) || (debugLevel == 0))
    {
	    /* Format the string */
	    va_list arg_ptr;
	    va_start(arg_ptr, pszFormat);
	    nWritten = (unsigned long) vsnprintf(pszPrintBuffer, BUFFER_LENGTH - 1, pszFormat, arg_ptr);
	    va_end(arg_ptr);
        
        /* Check for buffer overflow */
	    if (nWritten == (unsigned long)(-1))
	    {
		    ddkDebugPrint(0, "ddkDebugPrint(): BUFFER OVERFLOW DETECTED!!!\r\n" \
			    "MAX STRING LENGTH = %d\n", BUFFER_LENGTH);
		    return;
	    }

        /* Print out the data */
        switch (gDebugOutput)
        {
            default:
            case DEBUG_OUTPUT_SCREEN:
                printf(pszPrintBuffer);
                
                /* 
                 * Flush the stdout before the getch function. Otherwise the
                 * previous printf will not be displayed correctly sometimes.
                 */
                fflush(stdout);
                break;
            case DEBUG_OUTPUT_FILE:
                /* Print out the message to the log file */
                if (gFileHandle != NULL)
                {
                    fprintf(gFileHandle, pszPrintBuffer);
                    fflush(gFileHandle);
                }
                break;
            case DEBUG_OUTPUT_SERIAL:
                /* Send the data out to the COM Port */
                if (gCOMInit)
                {
#if 1                
                    char *pString1, *pString2;
                    unsigned long length;
                    char linefeed = '\r';

                    length = 0;
                    pString1 = pszPrintBuffer; 
                    while(nWritten)
                    {
                        /* Search for all '\n' and add '\r' so that the serial port can display correctly. */
                        pString2 = strchr(pString1, '\n');
                        if (pString2 != (char *)0)
                        {
                            length = pString2 - pString1 + 1;
                            
                            /* Check the previous character and the next character */
                            if ((*(pString2 - 1) != '\r') && (*(pString2 + 1) != '\r'))
                            {
                                /* Write the buffer with the '\r' */
                                comWrite(pString1, length);
                                comWrite(&linefeed, 1); 
                            }
                            else
                                comWrite(pString1, length);
                                
                            /* Adjust the nWritten */
                            nWritten -= length;
                            
                            /* Adjust the new string pointer */
                            pString1 = pString2 + 1;
                        }
                        else
                        {
                            comWrite(pString1, nWritten);
                            nWritten = 0;
                        }
                    }
#else
                    comWrite(pszPrintBuffer, nWritten);
#endif
                }
                break;                
        }
    }
}

/*
 * This function cleans up (such as closing the debug file, etc...) when
 * exiting the debug module.      
 */
void ddkDebugPrintExit()
{
    /* Clean up the debug print module */
    switch (gDebugOutput)
    {
        default:
        case DEBUG_OUTPUT_SCREEN:
            /* Do nothing */
            break;
        case DEBUG_OUTPUT_FILE:
            /* Close the log file */
            if (gFileHandle != (FILE *)0)
                fclose(gFileHandle);
            break;
        case DEBUG_OUTPUT_SERIAL:
            comClose();
            break;
    } 
}

#endif



