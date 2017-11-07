/*******************************************************************
* 
*         Copyright (c) 2016 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  CLOCK.H --- Falcon DDK 
* 
*******************************************************************/
#ifndef _DDK768_CLOCK_H_
#define _DDK768_CLOCK_H_

#define DEFAULT_INPUT_CLOCK 12000000 /* Default reference clock */
#define MHz(x) (x*1000000) /* Don't use this macro if x is fraction number */

#define DEFAULT_INPUT_CLOCK MHz(12) /* Default reference clock */

#define GHz(x) (x*1000000000)   //1000000000HZ  = 1(Ghz)
#define FRAC_MAX    32768

typedef struct pll_value_t
{
    unsigned long inputFreq; /* Input clock frequency to the PLL */
    unsigned long INT;
    unsigned long FRAC;
    unsigned long VCO;
    unsigned long BS;
}
pll_value_t;

/*
 * Given a requested clock frequency, this function calculates the 
 * best INT, FRAC, VCO and BS values for the PLL.
 * 
 * Input: Requested pixel clock in Hz unit.
 *        The followiing fields of PLL has to be set up properly:
 *        pPLL->inputFreq.
 *
 * Output: Update the PLL structure with the proper values
 * Return: The actual clock in Hz that the PLL is able to set up.
 *
 */
unsigned long ddk768_calcPllValue(
unsigned long ulRequestClk, /* Required pixel clock in Hz unit */
pll_value_t *pPLL           /* Structure to hold the value to be set in PLL */
);

/*
 * Set up the corresponding bit field of the programmable PLL register.
 *
 * Input: Pointer to PLL structure with all values set up properly.
 *
 */
unsigned long ddk768_formatPllReg(pll_value_t *pPLL);

/*
    This funtion sets up pixel clock for Falcon FPGA.
    Final product will have another PLL clock.
*/

long ddk768_setVclock(unsigned dispCtrl, unsigned long pixelClock);





#endif /*_CLOCK_H_*/
