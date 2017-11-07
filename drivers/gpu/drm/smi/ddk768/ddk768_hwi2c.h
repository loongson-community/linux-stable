/*******************************************************************
* 
*         Copyright (c) 2008 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  hwi2c.H --- SMI DDK 
*  This file contains the definitions for Hardware I2C.
* 
*******************************************************************/
#ifndef _DDK768_HWI2C_H_
#define _DDK768_HWI2C_H_

#define MAX_HWI2C_FIFO 16
#define HWI2C_WAIT_TIMEOUT 0x7FF

/*
 *  This function initializes the hardware i2c
 *
 *  Return Value:
 *       0   - Success
 *      -1   - Fail to initialize i2c
 */
long ddk768_hwI2CInit(
    unsigned char i2cNumber //I2C0 or I2C1
);

/* 
 * This function close the hardware i2c 
 */
void ddk768_hwI2CClose(
    unsigned char i2cNumber //I2C0 or I2C1
);

/* 
 * This function read the i2c device register value
 *
 * Input:   deviceAddress   - I2C Device Address
 *          registerIndex   - Register index to be read
 *
 * Output:
 *          The value of the register being read.
 */
unsigned char ddk768_hwI2CReadReg(
    unsigned char i2cNumber, //I2C0 or I2C1
    unsigned char deviceAddress, 
    unsigned char registerIndex
);

/* 
 * This function writes a value to the i2c device register.
 *
 * Input:   deviceAddress   - I2C Device Address
 *          registerIndex   - Register index to be written to
 *          data            - Data to be written to
 *
 * Output:
 *          0   - Success
 *         -1   - Fail
 */
long ddk768_hwI2CWriteReg(
    unsigned char i2cNumber, //I2C0 or I2C1
    unsigned char deviceAddress, 
    unsigned char registerIndex, 
    unsigned char data
);

#endif  /* _HWI2C_H_ */
