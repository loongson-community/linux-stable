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
#include "ddk750_defs.h"
#include "ddk750_regdc.h"
#include "ddk750_helper.h"
#include "ddk750_power.h"
#include "ddk750_clock.h"
#include "ddk750_chip.h"
#include "ddk750_help.h"

/*
 *  Get the default memory clock value used by this DDK
 */
unsigned long ddk750_getDefaultMemoryClock(void)
{
    switch(ddk750_getChipType())
    {
        case SM718:
            return DEFAULT_SM718_MEMORY_CLK;
            break;
        case SM750:
        	return DEFAULT_SM750_MEMORY_CLK;
            break;
		case SM750LE:
        	return SM750LE_MEMORY_CLK;
			break;
		case SM750HS_F:
			return SM750HS_F_MEMORY_CLK;
			break;
		case SM750HS_A:
			return SM750HS_A_MEMORY_CLK;
			break;
		case SM750HS:
        default:
			return SM750HS_MEMORY_CLK;
			break;
    }
}

/*
 * This function returns frame buffer memory size in Byte units.
 */
unsigned long ddk750_getFrameBufSize(void)
{
    unsigned long sizeSymbol, memSize;

	sizeSymbol = FIELD_GET(peekRegisterDWord(MISC_CTRL), MISC_CTRL, LOCALMEM_SIZE);
	switch(sizeSymbol)
	{
    	case MISC_CTRL_LOCALMEM_SIZE_8M:  memSize = MB(8);  break; /* 8  Mega byte */
    	case MISC_CTRL_LOCALMEM_SIZE_16M: memSize = MB(16); break; /* 16 Mega byte */
    	case MISC_CTRL_LOCALMEM_SIZE_32M: memSize = MB(32); break; /* 32 Mega byte */
    	case MISC_CTRL_LOCALMEM_SIZE_64M: memSize = MB(64); break; /* 64 Mega byte */
    	default:                          memSize = MB(0);  break; /* 0  Mege byte */
	}

    return memSize;
}


/*
 * This function returns the logical chip type defined in chip.h
 * It is one of the following: SM501, SM502, SM107, SM718, SM 750 or
 * SM_UNKNOWN.
 */
logical_chip_type_t ddk750_getChipType()
{
    return SM750;
}

/*
 *  ddk750_resetFrameBufferMemory
 *      This function resets the Frame Buffer Memory
 */
void ddk750_resetFrameBufferMemory()
{
    unsigned long ulReg;
	logical_chip_type_t chipType = ddk750_getChipType();
    
	/* Only SM718 and SM750 has register to reset video memory */
	if (chipType == SM718 || chipType == SM750)
	{        
    	ulReg = peekRegisterDWord(MISC_CTRL);
    	ulReg = FIELD_SET(ulReg, MISC_CTRL, LOCALMEM_RESET, RESET);
    	pokeRegisterDWord(MISC_CTRL, ulReg);

    	ulReg = FIELD_SET(ulReg, MISC_CTRL, LOCALMEM_RESET, NORMAL);
    	pokeRegisterDWord(MISC_CTRL, ulReg);
	}
}

/*
 * Initialize a single chip and environment according to input parameters.
 *
 * Input: initchip_param_t structure.
 *
 * Return: 0 (or NO_ERROR) if successful.
 *        -1 if fail.
 *
 * Note:
 *      Caller needs to call the detectDevice and setCurrentDevice
 *      to set the device before calling this initChipParamEx.
 */
long ddk750_initChipParamEx(initchip_param_t * pInitParam)
{
    unsigned long ulReg;


    /* Set power mode.
       Check parameter validity first.
       If calling function didn't set it up properly or set to some
       weird value, always default it to 0.
     */
    if (pInitParam->powerMode > 1) 
        pInitParam->powerMode = 0;
    setPowerMode(pInitParam->powerMode);
    
    /* Set the Main Chip Clock */
    setChipClock(MHz(pInitParam->chipClock));

    /* Set up memory clock. */
    setMemoryClock(MHz(pInitParam->memClock));

    /* Set up master clock */
    setMasterClock(MHz(pInitParam->masterClock));    
    
    /* Reset the memory controller. If the memory controller is not reset in SM750, 
       the system might hang when sw accesses the memory. 
       The memory should be resetted after changing the MXCLK.
     */
    if (pInitParam->resetMemory == 1)
        ddk750_resetFrameBufferMemory();    
    
    if (pInitParam->setAllEngOff == 1)
    {
        enable2DEngine(0);

        /* Disable Overlay, if a former application left it on */
        ulReg = peekRegisterDWord(VIDEO_DISPLAY_CTRL);
        ulReg = FIELD_SET(ulReg, VIDEO_DISPLAY_CTRL, PLANE, DISABLE); 
        pokeRegisterDWord(VIDEO_DISPLAY_CTRL, ulReg);

        /* Disable video alpha, if a former application left it on */
        ulReg = peekRegisterDWord(VIDEO_ALPHA_DISPLAY_CTRL);
        ulReg = FIELD_SET(ulReg, VIDEO_ALPHA_DISPLAY_CTRL, PLANE, DISABLE); 
        pokeRegisterDWord(VIDEO_ALPHA_DISPLAY_CTRL, ulReg);

        /* Disable alpha plane, if a former application left it on */
        ulReg = peekRegisterDWord(ALPHA_DISPLAY_CTRL);
        ulReg = FIELD_SET(ulReg, ALPHA_DISPLAY_CTRL, PLANE, DISABLE); 
        pokeRegisterDWord(ALPHA_DISPLAY_CTRL, ulReg);

        /* Disable Primary hardware cursor, if a former application left it on */
        ulReg = peekRegisterDWord(PRIMARY_HWC_ADDRESS);
        ulReg = FIELD_SET(ulReg, PRIMARY_HWC_ADDRESS, ENABLE, DISABLE); 
        pokeRegisterDWord(PRIMARY_HWC_ADDRESS, ulReg);

        /* Disable Secondary hardware cursor, if a former application left it on */
        ulReg = peekRegisterDWord(SECONDARY_HWC_ADDRESS);
        ulReg = FIELD_SET(ulReg, SECONDARY_HWC_ADDRESS, ENABLE, DISABLE); 
        pokeRegisterDWord(SECONDARY_HWC_ADDRESS, ulReg);
#if 0
        /* Disable ZV Port 0, if a former application left it on */
        ulReg = peekRegisterDWord(ZV0_CAPTURE_CTRL);
        ulReg = FIELD_SET(ulReg, ZV0_CAPTURE_CTRL, CAP, DISABLE); 
        pokeRegisterDWord(ZV0_CAPTURE_CTRL, ulReg);

        /* Disable ZV Port 1, if a former application left it on */
        ulReg = peekRegisterDWord(ZV1_CAPTURE_CTRL);
        ulReg = FIELD_SET(ulReg, ZV1_CAPTURE_CTRL, CAP, DISABLE); 
        pokeRegisterDWord(ZV1_CAPTURE_CTRL, ulReg);
       
        /* Disable ZV Port Power, if a former application left it on */
        enableZVPort(0);
        
        /* Disable i2c */
        enableI2C(0);
        
        /* Disable DMA Channel, if a former application left it on */
        ulReg = peekRegisterDWord(DMA_ABORT_INTERRUPT);
        ulReg = FIELD_SET(ulReg, DMA_ABORT_INTERRUPT, ABORT_1, ABORT);
        pokeRegisterDWord(DMA_ABORT_INTERRUPT, ulReg);
        
        /* Disable DMA Power, if a former application left it on */
        enableDMA(0);
#endif 		
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
 *
 * Note:
 *      This function initialize with a default set of parameters.
 *      Use initChipParam() if you don't want default parameters.
 */
long ddk750_initChip()
{
    initchip_param_t initParam;
    
    /* Check if any SMI VGX family chip exist and alive */
//	if (detectDevices() == 0)
//        return (-1);

    /* Initialize the chip with some default parameters */
    initParam.powerMode = 0;
    initParam.memClock = ddk750_getDefaultMemoryClock();
    initParam.chipClock = initParam.memClock;
    initParam.masterClock = initParam.chipClock/3;

    initParam.setAllEngOff = 1;
    initParam.resetMemory = 1;
    
    return(ddk750_initChipParamEx(&initParam));
}

