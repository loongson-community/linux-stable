/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  power.h --- Voyager GX SDK 
*  This file contains the definitions for the power functions.
* 
*******************************************************************/
#ifndef _DDK768_POWER_H_
#define _DDK768_POWER_H_

/*
 *  Enable/disable jpeg decoder 1.
 */
void ddk768_enableJPU1(unsigned long enable);


/* 
 * This function enable/disable the 2D engine.
 */
void ddk768_enable2DEngine(unsigned long enable);

/* 
 * This function enable/disable the ZV Port 
 */
void ddk768_enableZVPort(unsigned long enable);

/* 
 * This function enable/disable the DMA Engine
 */
void ddk768_enableDMA(unsigned long enable);



/* 
 * This function enable/disable the PWM Engine
 */
void ddk768_enablePWM(unsigned long enable);

/* 
 * This function enable/disable the SSP.
 */
void ddk768_enableSSP(unsigned long enable);

/*
 * This function enable/disable the HDMI Clock. 
 */
void ddk768_enableHDMI(unsigned long enable);


void ddk768_enableI2S(unsigned long enable);



#endif /* _POWER_H_ */
