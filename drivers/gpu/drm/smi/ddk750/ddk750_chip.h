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
#ifndef _CHIP_H_
#define _CHIP_H_

/* This is all the chips recognized by this library */
typedef enum _logical_chip_type_t
{
    SM_UNKNOWN = 0,
    SM718 = 1,
    SM750 = 2,
    SM750LE = 3,
	/* There will 3 versions of SM750HS because of different interfaces connected to the Graphic IP.
	   The major difference between them are the methods to set up the PLL for pixel clock. */
	SM750HS = 4,	/* SMI FPGA verification version */
	SM750HS_F = 5,  /* HiS FPGA verification version */
	SM750HS_A = 6	/* HiS ASIC version */
}
logical_chip_type_t;

/* input struct to initChipParam() function */
typedef struct _initchip_param_t
{
    unsigned short powerMode;    /* Use power mode 0 or 1 */
    unsigned short chipClock;    /* Speed of main chip clock in MHz unit
                                    0 = keep the current clock setting
                                    Others = the new main chip clock
                                  */
    unsigned short memClock;     /* Speed of memory clock in MHz unit
                                    0 = keep the current clock setting
                                    Others = the new memory clock
                                  */
    unsigned short masterClock;  /* Speed of master clock in MHz unit 
                                    0 = keep the current clock setting
                                    Others = the new master clock
                                  */
    unsigned short setAllEngOff; /* 0 = leave all engine state untouched.
                                    1 = make sure they are off: 2D, Overlay,
                                    video alpha, alpha, hardware cursors
                                 */
    unsigned char resetMemory;   /* 0 = Do not reset the memory controller
                                    1 = Reset the memory controller
                                  */

    /* More initialization parameter can be added if needed */
}
initchip_param_t;

/*
 *  Get the default memory clock value used by this DDK
 */
unsigned long ddk750_getDefaultMemoryClock(void);

/*
 * This function returns frame buffer memory size in Byte units.
 */
unsigned long ddk750_getFrameBufSize(void);

/*
 * This function gets the Frame buffer location.
 */
unsigned char ddk750_getFrameBufLocation(void);

/*
 * This function returns the logical chip type defined in chip.h
 * It is one of the following: SM501, SM502, SM107, SM718, SM 750 or
 * SM_UNKNOWN.
 */
logical_chip_type_t ddk750_getChipType(void);

/*
 * Return a char string name of the current chip.
 * It's convenient for application need to display the chip name.
 */
char * ddk750_getChipTypeString(void);

/*
 *  resetFrameBufferMemory
 *      This function resets the Frame Buffer Memory
 */
void ddk750_resetFrameBufferMemory(void);

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
long ddk750_initChipParamEx(initchip_param_t * pInitParam);

/*
 * Initialize every chip and environment according to input parameters. 
 * (Obsolete)
 *
 * Input: initchip_param_t structure.
 *
 * Return: 0 (or NO_ERROR) if successful.
 *        -1 if fail.
 */
long ddk750_initChipParam(initchip_param_t * pInitParam);

/*
 * Initialize a single chip with default parameters.
 *
 * Input: none.
 *
 * Return: 0 (or NO_ERROR) if successful.
 *        -1 if fail.
 *
 * Note:
 *      Caller needs to call the detectDevice and setCurrentDevice
 *      to set the device before calling this initChipEx.
 */
long ddk750_initChipEx(void);

/*
 * Initialize the chip with default parameters.
 *
 * Input: none.
 *
 * Return: 0 (or NO_ERROR) if successful.
 *        -1 if fail.
 */
long ddk750_initChip(void);

/*******************************************************************
 * Scratch Data implementation (To be used by DDK library only)
 *******************************************************************/
 
/*
 * Set Scratch Data
 */
void setScratchData(
    unsigned long dataFlag,
    unsigned long data
);

/*
 * Get Scratch Data
 */
unsigned long getScratchData(
    unsigned long dataFlag
);


#define MB(x) (x*0x100000) /* Don't use this macro if x is fraction number */

/* Memory Clock Default Values */
#define DEFAULT_SM750_MEMORY_CLK        290
#define DEFAULT_SM718_MEMORY_CLK        290

/* Cheok_0509: Define some fixed values for SM750LE */
#define SM750LE_REVISION_ID 0XFE
#define SM750LE_MEMORY_CLK  333    /* In MHz */
#define SM750LE_MASTER_CLK  130    /* in MHz */
#define SM750LE_MEM_SIZE    MB(16) /* SP605 has 64M, but Huawei's actual system has only 16M */
#define SM750LE_I2C_SCL     0      /* GPIO 0 is used as I2C clk */
#define SM750LE_I2C_SDA     1      /* GPIO 1 is used as I2C data */
#define CH7301_I2C_ADDRESS  0xEC /* I2C address of CH7301 in Xilinx SP605 */

/* Cheok_2012_1211: Define some fixed values for SM750HS */
/* The following values are fixed in FPGA and cannot be adjusted */
#define SM750HS_REVISION_ID 0XE0
#define SM750HS_MEMORY_CLK  333    /* In MHz */
#define SM750HS_MASTER_CLK  100    /* in MHz */
#define SM750HS_MEM_SIZE    MB(16) /* Actual size unknown at the moment */

#define SM750HS_F_REVISION_ID 0X00
#define SM750HS_F_MEMORY_CLK  333    /* In MHz */
#define SM750HS_F_MASTER_CLK  100    /* in MHz */
#define SM750HS_F_MEM_SIZE    MB(16) /* Actual size unknown at the moment */

#define SM750HS_A_REVISION_ID 0X10
#define SM750HS_A_MEMORY_CLK  333    /* In MHz */
#define SM750HS_A_MASTER_CLK  100    /* in MHz */
#define SM750HS_A_MEM_SIZE    MB(16) /* Actual size unknown at the moment */

#endif /* _CHIP_H_ */

