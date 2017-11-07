/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  power.c --- Voyager GX SDK 
*  This file contains the source code for the power functions.
* 
*******************************************************************/
#include "ddk768_reg.h"

#include "ddk768_chip.h"
#include "ddk768_power.h"

#include "ddk768_help.h"



/*
 *  Enable/disable jpeg decoder 1.
 */
void ddk768_enableJPU1(unsigned long enable)
{
    unsigned long regValue;

    regValue = peekRegisterDWord(CLOCK_ENABLE);
    if (enable)
        regValue = FIELD_SET(regValue, CLOCK_ENABLE, JPU1, ON);
    else
        regValue = FIELD_SET(regValue, CLOCK_ENABLE, JPU1, OFF);

    pokeRegisterDWord(CLOCK_ENABLE, regValue);
}

/* 
 * This function enable/disable the 2D engine.
 */
void ddk768_enable2DEngine(unsigned long enable)
{
	unsigned long regValue;
	
	regValue = peekRegisterDWord(CLOCK_ENABLE);
	   if (enable)
		   regValue = FIELD_SET(regValue, CLOCK_ENABLE, DE, ON);
	   else
		   regValue = FIELD_SET(regValue, CLOCK_ENABLE, DE, OFF);
	
	pokeRegisterDWord(CLOCK_ENABLE, regValue);

}

/* 
 * This function enable/disable the ZV Port.
 */
void ddk768_enableZVPort(unsigned long enable)
{
    unsigned long regValue;

    regValue = peekRegisterDWord(CLOCK_ENABLE);
	if (enable)
		regValue = FIELD_SET(regValue, CLOCK_ENABLE, ZV, ON);
	else
		regValue = FIELD_SET(regValue, CLOCK_ENABLE, ZV, OFF);

	pokeRegisterDWord(CLOCK_ENABLE, regValue);

}

/* 
 * This function enable/disable the SSP.
 */
void ddk768_enableSSP(unsigned long enable)
{
    unsigned long regValue;

    regValue = peekRegisterDWord(CLOCK_ENABLE);
	if (enable)
		regValue = FIELD_SET(regValue, CLOCK_ENABLE, SSP, ON);
	else
		regValue = FIELD_SET(regValue, CLOCK_ENABLE, SSP, OFF);

	pokeRegisterDWord(CLOCK_ENABLE, regValue);

}

/* 
 * This function enable/disable the DMA Engine
 */
void ddk768_enableDMA(unsigned long enable)
{
    unsigned long regValue;

    regValue = peekRegisterDWord(CLOCK_ENABLE);
	if (enable)
		regValue = FIELD_SET(regValue, CLOCK_ENABLE, DMA, ON);
	else
		regValue = FIELD_SET(regValue, CLOCK_ENABLE, DMA, OFF);

	pokeRegisterDWord(CLOCK_ENABLE, regValue);

}



/*
 *  This function enable/disable HDMI
 */
void ddk768_enableHDMI(unsigned long enable)
{
    unsigned long regValue;

    regValue = peekRegisterDWord(CLOCK_ENABLE);
	if (enable)
		regValue = FIELD_SET(regValue, CLOCK_ENABLE, HDMI, ON);
	else
		regValue = FIELD_SET(regValue, CLOCK_ENABLE, HDMI, OFF);

	pokeRegisterDWord(CLOCK_ENABLE, regValue);
}

/*
 *  Enable/disable USB 2 Host.
 */
void ddk768_enableUsbHost(unsigned long enable)
{
    unsigned long regValue;

    regValue = peekRegisterDWord(CLOCK_ENABLE);
	if (enable)
		regValue = FIELD_SET(regValue, CLOCK_ENABLE, USBH, ON);
	else
		regValue = FIELD_SET(regValue, CLOCK_ENABLE, USBH, OFF);

	pokeRegisterDWord(CLOCK_ENABLE, regValue);
}

/*
 *  Enable/disable USB 3 device
 */
void ddk768_enableUsbDevice(unsigned long enable)
{
    unsigned long regValue;

    regValue = peekRegisterDWord(CLOCK_ENABLE);
	if (enable)
		regValue = FIELD_SET(regValue, CLOCK_ENABLE, USBS, ON);
	else
		regValue = FIELD_SET(regValue, CLOCK_ENABLE, USBS, OFF);

	pokeRegisterDWord(CLOCK_ENABLE, regValue);
}

/*
 *  Enable/disable jpeg decoder.
 */
void ddk768_enableJPU(unsigned long enable)
{
    unsigned long regValue;

    regValue = peekRegisterDWord(CLOCK_ENABLE);
	if (enable)
		regValue = FIELD_SET(regValue, CLOCK_ENABLE, JPU, ON);
	else
		regValue = FIELD_SET(regValue, CLOCK_ENABLE, JPU, OFF);

	pokeRegisterDWord(CLOCK_ENABLE, regValue);
}

/*
 *	Enable/disable H264 video decoder.
 */ 
void ddk768_enableVPU(unsigned long enable)
{
    unsigned long regValue;

    regValue = peekRegisterDWord(CLOCK_ENABLE);
	if (enable)
		regValue = FIELD_SET(regValue, CLOCK_ENABLE, VPU, ON);
	else
		regValue = FIELD_SET(regValue, CLOCK_ENABLE, VPU, OFF);

	pokeRegisterDWord(CLOCK_ENABLE, regValue);
}


/*
 *	Enable/disable UART
 */ 
void ddk768_enableUART(unsigned long enable)
{
    unsigned long regValue;

    regValue = peekRegisterDWord(CLOCK_ENABLE);
	if (enable)
		regValue = FIELD_SET(regValue, CLOCK_ENABLE, UART, ON);
	else
		regValue = FIELD_SET(regValue, CLOCK_ENABLE, UART, OFF);

	pokeRegisterDWord(CLOCK_ENABLE, regValue);
}

/*
 *	Enable/disable I2S
 */ 
void ddk768_enableI2S(unsigned long enable)
{
    unsigned long regValue;

    regValue = peekRegisterDWord(CLOCK_ENABLE);
	if (enable)
		regValue = FIELD_SET(regValue, CLOCK_ENABLE, I2S, ON);
	else
		regValue = FIELD_SET(regValue, CLOCK_ENABLE, I2S, OFF);

	pokeRegisterDWord(CLOCK_ENABLE, regValue);
}



/*
 *	Enable/disable ARM
 */ 
void ddk768_enableARM(unsigned long enable)
{
    unsigned long regValue;

    regValue = peekRegisterDWord(CLOCK_ENABLE);
	if (enable)
		regValue = FIELD_SET(regValue, CLOCK_ENABLE, ARM, ON);
	else
		regValue = FIELD_SET(regValue, CLOCK_ENABLE, ARM, OFF);

	pokeRegisterDWord(CLOCK_ENABLE, regValue);
}

/*
 *	Enable/disable display control 0
 */ 
void ddk768_enableDC0(unsigned long enable)
{
    unsigned long regValue;

    regValue = peekRegisterDWord(CLOCK_ENABLE);
	if (enable)
		regValue = FIELD_SET(regValue, CLOCK_ENABLE, DC0, ON);
	else
		regValue = FIELD_SET(regValue, CLOCK_ENABLE, DC0, OFF);

	pokeRegisterDWord(CLOCK_ENABLE, regValue);
}

/*
 *	Enable/disable display control 1
 */ 
void ddk768_enableDC1(unsigned long enable)
{
    unsigned long regValue;

    regValue = peekRegisterDWord(CLOCK_ENABLE);
	if (enable)
		regValue = FIELD_SET(regValue, CLOCK_ENABLE, DC1, ON);
	else
		regValue = FIELD_SET(regValue, CLOCK_ENABLE, DC1, OFF);

	pokeRegisterDWord(CLOCK_ENABLE, regValue);
}

