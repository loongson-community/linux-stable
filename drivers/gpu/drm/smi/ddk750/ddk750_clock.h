/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  CLOCK.H --- Voyager GX DDK 
* 
*******************************************************************/
#ifndef _CLOCK_H_
#define _CLOCK_H_

#define DEFAULT_INPUT_CLOCK 14318181 /* Default reference clock */
#define MHz(x) (x*1000000) /* Don't use this macro if x is fraction number */

/* The following maximum value is not the optimum value. */
/* Maximum Master Clock is about 190MHz */
#define MAXIMUM_MASTER_CLOCK        MHz(190)

/* Maximum Chip Clock (MXCLK) is 1 GHz */
#define MAXIMUM_CHIP_CLOCK          MHz(1000)

typedef enum _clock_type_t
{
    MXCLK_PLL,      /* Programmable Master clock */
    PRIMARY_PLL,    /* Programmable Primary pixel clock */
    SECONDARY_PLL,  /* Programmable Secondary pixel clock */
    VGA0_PLL,
    VGA1_PLL,
}
clock_type_t;

typedef struct pll_value_t
{
    clock_type_t clockType;
    unsigned long inputFreq; /* Input clock frequency to the PLL */
    unsigned long M;
    unsigned long N;
    unsigned long OD;
    unsigned long POD;
}
pll_value_t;

/*
 * Given a requested clock frequency, this function calculates the 
 * best M, N & OD values for the PLL.
 * 
 * Input: Requested pixel clock in Hz unit.
 *        The followiing fields of PLL has to be set up properly:
 *        pPLL->clockType, pPLL->inputFreq.
 *
 * Output: Update the PLL structure with the proper N, N and OD values
 * Return: The actual clock in Hz that the PLL is able to set up.
 *
 * The PLL uses this formula to operate: 
 * requestClk = inputFreq * M / N / (2 to the power of OD)
 *
 */
unsigned long calcPllValue(
unsigned long ulRequestClk, /* Required pixel clock in Hz unit */
pll_value_t *pPLL           /* Structure to hold the value to be set in PLL */
);

/*
 * Set up the corresponding bit field of the programmable PLL register.
 *
 * Input: Pointer to PLL structure with type and values set up properly.
 *        Usually, calcPllValue() function will be called before this to calculate the values first.
 *
 */
unsigned long formatPllReg(pll_value_t *pPLL);

/*
 * This function set up the main chip clock.
 *
 * Input: Frequency to be set.
 */
void setChipClock(unsigned long frequency);

/*
 * This function gets the Main Chip Clock value.
 *
 * Output:
 *      The Actual Main Chip clock value.
 */
unsigned long getChipClock(void);

/*
 * This function set up the memory clock.
 *
 * Input: Frequency to be set.
 */
void setMemoryClock(unsigned long frequency);

/*
 *  getMemoryClock
 *      This function gets the Memory Clock value.
 *
 *  Output:
 *      The Memory Clock value in whole number.
 */
unsigned long getMemoryClock(void);

/*
 * This function set up the master clock (MCLK).
 *
 * Input: Frequency to be set.
 */
void setMasterClock(unsigned long frequency);

/*
 *  getMasterClock
 *      This function gets the Master Clock value.
 *
 *  Output:
 *      The Master Clock value in whole number.
 */
unsigned long getMasterClock(void);

/*
 * This function get the Primary display control Pixel Clock value.
 *
 * Output:
 *      The Primary display control Pixel Clock value in whole number.
 */
unsigned long getPrimaryDispCtrlClock(void);

/*
 * This function get the Secondary display control Pixel Clock value.
 *
 * Output:
 *      The Secondary display control Pixel Clock value in whole number.
 */
unsigned long getSecondaryDispCtrlClock(void);

/*
 * This function gets the Master Clock Divider Values List.
 *
 * Output:
 *      The list of Master Clock divider values.
 */
unsigned char *getMasterClockDivider(void);

/*
 * This function gets the total number of Master Clock Divider Values.
 */
unsigned long getTotalMasterClockDivider(void);

/*
 * This function gets the Memory Clock Divider Values List.
 *
 * Output:
 *      The list of Memory Clock divider values.
 */
unsigned char *getMemoryClockDivider(void);

/*
 * This function gets the total number of Memory Clock Divider Values.
 */
unsigned long getTotalMemoryClockDivider(void);

/*
 * This function uses the Master Clock PLL clock counter to provide some delay in ms.
 */
void waitMasterClock(
    unsigned long miliseconds
);

/*
  * This function provides new logic to program the PLL registers for HiSilicon ASIC. 
  *
  * Input: 
  *	The value of HiSilicon PLL register. 
  */
void setVclock_HiSilicon(unsigned long pll);

#endif /*_CLOCK_H_*/
