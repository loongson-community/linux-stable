/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  CHIP.C --- SMI DDK 
*  This file contains the source code for the mode table.
* 
*******************************************************************/
#include "ddk768_reg.h"
#include "ddk768_chip.h"
#include "ddk768_power.h"
#include "ddk768_clock.h"

#include "ddk768_help.h"


/* Size of SM768 MMIO and memory */
#define SMI_MMIO_SIZE_SM768       (2<<20)    /* 2M of MMIO space */
#define SMI_MEMORY_SIZE_SM768     (128<<20)  
/*
 * This function returns frame buffer memory size in Byte units.
 */
unsigned long ddk768_getFrameBufSize()
{
	return SMI_MEMORY_SIZE_SM768;
    unsigned long strapPin, ddrController, rValue;

    strapPin = FIELD_GET(peekRegisterDWord(STRAP_PINS), STRAP_PINS, MEM_SIZE);
    ddrController = peekRegisterDWord(DDR_CONTROL);

    switch(strapPin)
    {
        case STRAP_PINS_MEM_SIZE_512M:
        pokeRegisterDWord(DDR_CONTROL, ddrController | FIELD_SET(0, DDR_CONTROL, SIZE, 512M));
        rValue = MB(512);
        break;

        case STRAP_PINS_MEM_SIZE_1024M:
        pokeRegisterDWord(DDR_CONTROL, ddrController | FIELD_SET(0, DDR_CONTROL, SIZE, 1024M));
        rValue = MB(1024);
        break;

        default: /* default size of 256M. Don't need to do anything */
        rValue = MB(256);
        break;
    }
    return(rValue);
}

/*
 * This function returns the logical chip type defined in chip.h
 * It is one of the following: SM501, SM502, SM107, SM718, SM 750 or
 * SM_UNKNOWN.
 */
logical_chip_type_t ddk768_getChipType()
{

    logical_chip_type_t chip;


    chip = SM768;


    return chip;
}

/*
 * Return a char string name of the current chip.
 * It's convenient for application need to display the chip name.
 */
char *ddk768_getChipTypeString()
{
    char * chipName;

    switch(ddk768_getChipType())
    {
        case SM768:
            chipName = "SM768";
            break;
        default:
            chipName = "Unknown";
            break;
    }

    return chipName;
}

/*
 * Initialize a single chip and environment according to input parameters.
 *
 * Input: initchip_param_t structure.
 *
 * Return: 0 (or NO_ERROR) if successful.
 *        -1 if fail.
 *
 */
long ddk768_initChipParamEx(initchip_param_t * pInitParam)
{
    unsigned long ulReg;

    /* Check if we know this chip */
    if (ddk768_getChipType() == SM_UNKNOWN)
        return -1;

    if (pInitParam->setAllEngOff == 1)
    {
        ulReg = peekRegisterDWord(VIDEO_DISPLAY_CTRL);
        ulReg = FIELD_SET(ulReg, VIDEO_DISPLAY_CTRL, PLANE, DISABLE); 
        pokeRegisterDWord(VIDEO_DISPLAY_CTRL, ulReg); /* Channel 0 */
        pokeRegisterDWord(VIDEO_DISPLAY_CTRL+CHANNEL_OFFSET, ulReg); /* Channel 1 */

        /* Disable alpha plane, if a former application left it on */
        ulReg = peekRegisterDWord(ALPHA_DISPLAY_CTRL);
        ulReg = FIELD_SET(ulReg, ALPHA_DISPLAY_CTRL, PLANE, DISABLE); 
        pokeRegisterDWord(ALPHA_DISPLAY_CTRL, ulReg); /* Channel 0 */
        pokeRegisterDWord(ALPHA_DISPLAY_CTRL+CHANNEL_OFFSET, ulReg); /* Channel 1 */

        /* Disable hardware cursor, if a former application left it on */
        ulReg = peekRegisterDWord(HWC_CONTROL);
        ulReg = FIELD_SET(ulReg, HWC_CONTROL, MODE, DISABLE); 
        pokeRegisterDWord(HWC_CONTROL, ulReg); /* Channel 0 */
        pokeRegisterDWord(HWC_CONTROL+CHANNEL_OFFSET, ulReg); /* Channel 1 */
    }

    /* We can add more initialization as needed. */

        
    return 0;
}

/*
 * Initialize chip with default parameters.
 *
 * Input: none.
 *
 * Return: 0 (or NO_ERROR) if successful.
 *        -1 if fail.
 */
long ddk768_initChip()
{
    initchip_param_t initParam;
    
    /* Initialize the chip with some default parameters */

    initParam.setAllEngOff = 1;
    
    
    return(ddk768_initChipParamEx(&initParam));
}

#if 0
#define PHY_STATUS(A, B)\
    peekRegisterDWord(A) & B
/*
    Program DDR PHY register PIR to do DDR training.
    Other parameters for training are assumed well set before calling here.
*/
void ddrTraining()
{
    unsigned long ulTmp;

    ulTmp = peekRegisterDWord(CLOCK_ENABLE);

    //Shut off all active components, espically ARM
    pokeRegisterDWord(CLOCK_ENABLE, 0);

    pokeRegisterDWord(PIR, 0);
    pokeRegisterDWord(PIR, 0x1ff);

    // May take few cycles for PGSR.IDONE = 0 after kick off DDR training.
    while((PHY_STATUS(PGSR, 0x00000001))){}

    // Wait until DDR training completed
    while(!(PHY_STATUS(PGSR, 0x00000001))){}

    //Restore clocks
    pokeRegisterDWord(CLOCK_ENABLE, ulTmp);
}
#endif

