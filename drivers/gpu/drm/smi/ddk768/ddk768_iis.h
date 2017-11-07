/*******************************************************************
* 
*         Copyright (c) 2014 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  This file contains the definitions for the IIS functions.
* 
*******************************************************************/
#ifndef _IIS_H_
#define _IIS_H_


#define IIS_REF_CLOCK 48000000


/*
 * Set up I2S and GPIO registers to transmit/receive data.
 */
void iisOpen(
   unsigned long wordLength, //Number of bits in IIS data: 16 bit, 24 bit, 32 bit
   unsigned long sampleRate  //Sampling rate.
);

/*
 *    Turn off I2S and close GPIO 
 */
void iisClose(void);

/*
 *  This function set up I2S to DMA data from SRAM.
 *
 *  SRAM area has max size of 2048 bytes (or 512 DWords).
 *  Max size of each I2S DMA session is 256 DWords.
 *
 *  Inputs: 
 *        offset address in SRAM to start DMA (DWord aligned)
 *        Number of bytes to DMA (DWord aligned)
 */
void iisTxDmaSetup(
    unsigned long offset, /* Offset from start of SRAM area */
    unsigned long len     /* Number of bytes to DMA */
    );

/*
 * Return current IIS DMA position.
 */
unsigned long iisDmaPointer(void);

/*
 * This function start IIS without enabling Tx line.
 * It can be used to flush left over SRAM data without
 * sending them to Codec.
 */
void iisStartNoTx(void);

/*
 * This function is needed only when I2S is intended to operate in master mode.
 *
 * For slave mode, just use iisOpen() is enough, because I2S will start
 * functioning as soon as an external clock is detected after iisOpen().
 *
 */
void iisStart(void);

/*
 * This function is useful only when I2S is operating in master mode.
 *
 * For slave mode, clock is external and cannot be stopped by IIS
 * control register.
 *
 */
void iisStop(void);

/*
 * Set values for left Tx and right Tx register.
 */
void iisSetTx(
    unsigned long left, //Data for left channel Tx
    unsigned long right //Data for right channel Tx
    );

/*
 * This function clears the RAW interrupt status of I2S.
 * 
 * When I2S completes sending data, the raw interrupt bit will be set.
 * It has to be cleared, in order to distinguish between different sessions of countdown.
 * 
 */
void iisClearRawInt(void);

/* 
 * This function returns the INT mask for IIS.
 *
 */
unsigned long iisIntMask(void);

/*
 * This is a reference sample showing how to implement ISR for I2S.
 * It works wiht libsrc\intr.c together.
 * 
 * Refer to Apps\iis\tstiis.c on how to hook up this function with system
 * interrupt under WATCOM DOS extender.
 * 
 */


#endif /* _IIS_H_ */
