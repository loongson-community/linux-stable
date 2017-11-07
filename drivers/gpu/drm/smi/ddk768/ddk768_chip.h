/*******************************************************************
* 
*         Copyright (c) 2008 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  CHIP.H --- SMI DDK 
*  This file contains the source code for the SM750/SM718 chip.
* 
*******************************************************************/
#ifndef _DDK768_CHIP_H_
#define _DDK768_CHIP_H_

/* This is all the chips recognized by this library */
typedef enum _logical_chip_type_t
{
    SM_UNKNOWN = 0,
    SM768 = 1
}
logical_chip_type_t;

/* input struct to initChipParam() function */
typedef struct _initchip_param_t
{

    unsigned short setAllEngOff; /* 0 = leave all engine state untouched.
                                    1 = make sure they are off: 2D, Overlay,
                                    video alpha, alpha, hardware cursors
                                 */

    /* More initialization parameter can be added if needed */
}
initchip_param_t;


/*
 * This function returns frame buffer memory size in Byte units.
 */
unsigned long ddk768_getFrameBufSize(void);



/*
 * This function returns the logical chip type defined in chip.h
 * It is one of the following: SM501, SM502, SM107, SM718, SM 750 or
 * SM_UNKNOWN.
 */
logical_chip_type_t ddk768_getChipType(void);

/*
 * Return a char string name of the current chip.
 * It's convenient for application need to display the chip name.
 */
char * ddk768_getChipTypeString(void);

/*
 * Initialize a single chip and environment according to input parameters.
 *
 * Input: initchip_param_t structure.
 *
 * Return: 0 (or NO_ERROR) if successful.
 *        -1 if fail.
 *
 */
long ddk768_initChipParamEx(initchip_param_t * pInitParam);

/*
 * Initialize the chip with default parameters.
 *
 * Input: none.
 *
 * Return: 0 (or NO_ERROR) if successful.
 *        -1 if fail.
 */
long ddk768_initChip(void);

#define MB(x) (x<<20) /* Macro for Mega Bytes */




#endif /* _CHIP_H_ */

