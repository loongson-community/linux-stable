/*******************************************************************
* 
*         Copyright (c) 2016 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  CLOCK.C --- Falcon  DDK 
*  This file contains source code for the Falcon  PLL's
* 
*******************************************************************/
#include "ddk768_reg.h"

#include "ddk768_chip.h"
#include "ddk768_power.h"
#include "ddk768_clock.h"
#include "ddk768_mode.h"


#include "ddk768_help.h"
#include "ddk768_helper.h"

/*
 * A local function to calculate the output frequency of a given PLL structure.
 *
 */
unsigned long ddk768_calcPLL(pll_value_t *pPLL)
{
    unsigned long pllClk, vcoPower;
    unsigned long fifteenPower = ddk768_twoToPowerOfx(15); /* 2^15 */

    /* Convert everything in Khz range in order to avoid calculation overflow */
    pPLL->inputFreq /= 1000;
    
    /* Work out 2 to the power of (VCO + 1) */
    vcoPower = ddk768_twoToPowerOfx(pPLL->VCO + 1);
    
    pllClk = (pPLL->inputFreq * pPLL->INT + pPLL->inputFreq * pPLL->FRAC / fifteenPower) / vcoPower;
    
    /* Restore input frequency from Khz to hz unit */
    pPLL->inputFreq *= 1000;
    pllClk *= 1000;

    return pllClk;
}

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
)
{

		unsigned long INTEGER, FRAC, VCO,  diff, pllClk, vcoPower, tempRequestClk;
		unsigned long bestDiff = 0xffffffff; /* biggest 32 bit unsigned number */
		unsigned long fifteenPower = ddk768_twoToPowerOfx(15); /* 2^15 */
	
		/* Init PLL structure to known states */
		pPLL->INT = 0;
		pPLL->FRAC = 0;
		pPLL->VCO = 0;
		pPLL->BS = 0;
	
		/* Convert everything in Khz range in order to avoid calculation overflow */
		pPLL->inputFreq /= 1000;
		tempRequestClk = ulRequestClk / 1000;
	
		/* If the requested clock is higher than 1 GHz, then set it to the maximum, which is
		   1 GHz. */
		if (tempRequestClk > MHz(1))
			tempRequestClk = MHz(1);
	
		/* The maximum of VCO is 5. */
		for (VCO=0; VCO<=5; VCO++)
		{
			/* Work out 2 to the power of (VCO + 1) */
			vcoPower = ddk768_twoToPowerOfx(VCO + 1);
	
			INTEGER = tempRequestClk * vcoPower / pPLL->inputFreq;
	
			/* 28 <= INT <= 56 */
			if ((INTEGER >= 28) && (INTEGER <=56))
			{
				/* FRAC = (requestClk * vcoPower / inputFreq - INT ) * (2^15)
				  Use formula as following to avoid decimal calculation:
				  FRAC = (requestClk * vcoPower - INT * inputFreq) * (2^15) / inputFreq
				*/
				FRAC = ((tempRequestClk * vcoPower) - (INTEGER * pPLL->inputFreq)) * fifteenPower / pPLL->inputFreq;
	
				/* FRAC field has 15 bits, reject value bigger than 15 bits */
				if (FRAC < FRAC_MAX)
				{
					/* Calculate the actual clock for a given INT & FRAC */
					pllClk = (pPLL->inputFreq * INTEGER + pPLL->inputFreq * FRAC / fifteenPower) / vcoPower;
					
					/* How much are we different from the requirement */
					diff = ddk768_absDiff(pllClk, tempRequestClk);
	
					if (diff < bestDiff)
					{
						bestDiff = diff;
	
						/* Store INT and FRAC values */
						pPLL->INT  = INTEGER;
						pPLL->FRAC = FRAC;
						pPLL->VCO = VCO;
					}
				}
			}
		}
	
		/* Calculate BS value */
		if ((pPLL->INT >= 28) && (pPLL->INT < 35))
		{
			pPLL->BS = 0;
		}
		else if ((pPLL->INT >=35) && (pPLL->INT < 42))
		{
			pPLL->BS = 1;
		}
		else if ((pPLL->INT >= 42) && (pPLL->INT < 49))
		{
			pPLL->BS = 2;
		}
		else if ((pPLL->INT >= 49) && (pPLL->INT <= 56))
		{
			pPLL->BS = 3;
		}
		
		/* Restore input frequency from Khz to hz unit */
		pPLL->inputFreq *= 1000;
#if 0	
		/* Output debug information */
		DDKDEBUGPRINT((DISPLAY_LEVEL, "calcPllValue: Requested Frequency = %d\n", ulRequestClk));
		DDKDEBUGPRINT((DISPLAY_LEVEL, "calcPllValue: Input CLK = %dHz, INT=%d, FRAC=%d, VCO=%d, BS=%d\n", 
							pPLL->inputFreq, pPLL->INT, pPLL->FRAC, pPLL->VCO, pPLL->BS));
#endif	
		/* Return actual frequency that the PLL can set */
		return ddk768_calcPLL(pPLL);

    
}

/*
 * Set up the corresponding bit field of the programmable PLL register.
 *
 * Input: Pointer to PLL structure with all values set up properly.
 *
 */
unsigned long ddk768_formatPllReg(pll_value_t *pPLL)
{

     unsigned long ulPllReg = 0;

    /* Note that all PLL's have the same format. Here, we just use Panel PLL parameter
       to work out the bit fields in the register.
       On returning a 32 bit number, the value can be applied to any PLL in the calling function.
    */
    ulPllReg =
        FIELD_SET(0, VCLK_PLL, FN, FRAC_MODE)
      | FIELD_VALUE(0, VCLK_PLL, BS, pPLL->BS)      
      | FIELD_VALUE(0, VCLK_PLL, VCO, pPLL->VCO)
      | FIELD_VALUE(0, VCLK_PLL, INT, pPLL->INT)
      | FIELD_VALUE(0, VCLK_PLL, FRAC, pPLL->FRAC);

    return(ulPllReg);

}



