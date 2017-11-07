/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  CLOCK.C --- Voyager GX DDK 
*  This file contains source code for the Voyager Family PLL's
* 
*******************************************************************/
#include "ddk750_defs.h"
//#include "ddk750_hardware.h"
#include "ddk750_helper.h"
#include "ddk750_power.h"
#include "ddk750_clock.h"
#include "ddk750_chip.h"
#include "ddk750_helper.h"
#include "ddk750_help.h"

//#include "ddkdebug.h"

static unsigned char g_ucMemoryClockDivider[] = { 1, 2, 3, 4 };
static unsigned char g_ucMasterClockDivider[] = { 3, 4, 6, 8 };

/*
 * A local function to calculate the clock value of the given PLL.
 *
 * Input:
 *      Pointer to a PLL structure to be calculated based on the
 *      following formula:
 *      inputFreq * M / N / (2 to the power of OD) / (2 to the power of POD)
 */
unsigned long calcPLL(pll_value_t *pPLL)
{
    return (pPLL->inputFreq * pPLL->M / pPLL->N / twoToPowerOfx(pPLL->OD) / twoToPowerOfx(pPLL->POD));
}

/*
 * Given a requested clock frequency, this function calculates the 
 * best M, N & OD values for the PLL.
 * 
 * Input: Requested pixel clock in Hz unit.
 *        The followiing fields of PLL has to be set up properly:
 *        pPLL->clockType, pPLL->inputFreq.
 *
 * Output: Update the PLL structure with the proper M, N and OD values
 * Return: The actual clock in Hz that the PLL is able to set up.
 *
 * The PLL uses this formula to operate: 
 * requestClk = inputFreq * M / N / (2 to the power of OD) / (2 to the power of POD)
 *
 * The PLL specification mention the following restrictions:
 *      1 MHz <= inputFrequency / N <= 25 MHz
 *      200 MHz <= outputFrequency <= 1000 MHz --> However, it seems that it can 
 *                                                 be set to lower than 200 MHz.
 */
unsigned long calcPllValue(
unsigned long ulRequestClk, /* Required pixel clock in Hz unit */
pll_value_t *pPLL           /* Structure to hold the value to be set in PLL */
)
{
    unsigned long M, N, OD, POD = 0, diff, pllClk, odPower, podPower, tempRequestClk;
    unsigned long bestDiff = 0xffffffff; /* biggest 32 bit unsigned number */

    /* Init PLL structure to know states */
    pPLL->M = 0;
    pPLL->N = 0;
    pPLL->OD = 0;
    pPLL->POD = 0;

	//if (getChipType() == SM750LE)
	if (ddk750_getChipType() >= SM750LE)
    {
        /* SM750LE don't have prgrammable PLL and M/N values to work on.
           Just return the requested clock. */
        return ulRequestClk;
    }

    /* Convert everything in Khz range in order to avoid calculation overflow */
    pPLL->inputFreq /= 1000;
    tempRequestClk = ulRequestClk / 1000;

    /* If the requested clock is higher than 1 GHz, then set it to the maximum, which is
       1 GHz. */
    if (tempRequestClk > MHz(1))
        tempRequestClk = MHz(1);

    /* The maximum of post divider is 8. */
    for (POD=0; POD<=3; POD++)
    {
        /* MXCLK_PLL does not have post divider. */
        if ((POD > 0) && (pPLL->clockType == MXCLK_PLL))
            break;
    
        /* Work out 2 to the power of POD */
        podPower = twoToPowerOfx(POD);
        
        /* OD has only 2 bits [15:14] and its value must between 0 to 3 */
        for (OD=0; OD<=3; OD++)
        {
            /* Work out 2 to the power of OD */
            odPower = twoToPowerOfx(OD);            

            /* N has 4 bits [11:8] and its value must between 2 and 15. 
               The N == 1 will behave differently --> Result is not correct. */
            for (N=2; N<=15; N++)
            {
                /* The formula for PLL is ulRequestClk = inputFreq * M / N / (2^OD)
                   In the following steps, we try to work out a best M value given the others are known.
                   To avoid decimal calculation, we use 1000 as multiplier for up to 3 decimal places of accuracy.
                */
                M = tempRequestClk * N * odPower * podPower / pPLL->inputFreq;

                /* M field has only 8 bits, reject value bigger than 8 bits */
                if (M < 256)
                {
                    /* Calculate the actual clock for a given M & N */        
                    pllClk = pPLL->inputFreq * M / N / odPower / podPower;

                    /* How much are we different from the requirement */
                    diff = absDiff(pllClk, tempRequestClk);
        
                    if (diff < bestDiff)
                    {
                        bestDiff = diff;

                        /* Store M and N values */
                        pPLL->M  = M;
                        pPLL->N  = N;
                        pPLL->OD = OD;
                        pPLL->POD = POD;
                    }
                }
            }
        }
    }
    
    /* Restore input frequency from Khz to hz unit */
    pPLL->inputFreq *= 1000;

    /* Output debug information */
//    DDKDEBUGPRINT((DISPLAY_LEVEL, "calcPllValue: Requested Frequency = %d\n", ulRequestClk));
//    DDKDEBUGPRINT((DISPLAY_LEVEL, "calcPllValue: Input CLK = %dHz, M=%d, N=%d, OD=%d, POD=%d\n", pPLL->inputFreq, pPLL->M, pPLL->N, pPLL->OD, pPLL->POD));

    /* Return actual frequency that the PLL can set */
    return calcPLL(pPLL);
}

/*
 * Set up the corresponding bit field of the programmable PLL register.
 *
 * Input: Pointer to PLL structure with type and values set up properly.
 *        Usually, calcPllValue() function will be called before this to calculate the values first.
 *
 */
unsigned long formatPllReg(pll_value_t *pPLL)
{
    unsigned long ulPllReg = 0;

    /* Note that all PLL's have the same format. Here, we just use Panel PLL parameter
       to work out the bit fields in the register.
       On returning a 32 bit number, the value can be applied to any PLL in the calling function.
    */
    ulPllReg =
        FIELD_SET(  0, PRIMARY_PLL_CTRL, BYPASS, OFF)
      | FIELD_SET(  0, PRIMARY_PLL_CTRL, POWER,  ON)
      | FIELD_SET(  0, PRIMARY_PLL_CTRL, INPUT,  OSC)
      | FIELD_VALUE(0, PRIMARY_PLL_CTRL, POD,    pPLL->POD)      
      | FIELD_VALUE(0, PRIMARY_PLL_CTRL, OD,     pPLL->OD)
      | FIELD_VALUE(0, PRIMARY_PLL_CTRL, N,      pPLL->N)
      | FIELD_VALUE(0, PRIMARY_PLL_CTRL, M,      pPLL->M);

 //   DDKDEBUGPRINT((DISPLAY_LEVEL, "formatPllReg: PLL register value = 0x%08x\n", ulPllReg));

    return(ulPllReg);
}

/*
 * Get the programmable PLL register value.
 *
 * Input:
 *      clockType   - The clock Type that the PLL is associated with.
 *      pPLL        - Pointer to a PLL structure to be filled with the
 *                    PLL value read from the register.
 * Output:
 *      The actual clock value calculated, together with the values of
 *      PLL register stored in the pPLL pointer.
 */
unsigned long getPllValue(clock_type_t clockType, pll_value_t *pPLL)
{
    unsigned long ulPllReg = 0;
    
    pPLL->inputFreq = DEFAULT_INPUT_CLOCK;
    pPLL->clockType = clockType;
    
    switch (clockType)
    {
        case MXCLK_PLL:
            ulPllReg = peekRegisterDWord(MXCLK_PLL_CTRL);
            break;
        case PRIMARY_PLL:
            ulPllReg = peekRegisterDWord(PRIMARY_PLL_CTRL);
            break;
        case SECONDARY_PLL:
            ulPllReg = peekRegisterDWord(SECONDARY_PLL_CTRL);
            break;
        case VGA0_PLL:
            ulPllReg = peekRegisterDWord(VGA_PLL0_CTRL);
            break;
        case VGA1_PLL:
            ulPllReg = peekRegisterDWord(VGA_PLL1_CTRL);
            break;
    }
    
    pPLL->M = FIELD_GET(ulPllReg, PRIMARY_PLL_CTRL, M);
    pPLL->N = FIELD_GET(ulPllReg, PRIMARY_PLL_CTRL, N);
    pPLL->OD = FIELD_GET(ulPllReg, PRIMARY_PLL_CTRL, OD);
    pPLL->POD = FIELD_GET(ulPllReg, PRIMARY_PLL_CTRL, POD);    

    return calcPLL(pPLL);
}

/*
 * This function set up the main chip clock.
 *
 * Input: Frequency to be set.
 */
void setChipClock(unsigned long frequency)
{
    pll_value_t pll;
    unsigned long ulActualMxClk;

#if 1
	/* Cheok_0509: For SM750LE, the chip clock is fixed. Nothing to set. */
	//if (getChipType() == SM750LE)
	if (ddk750_getChipType() >= SM750LE)
		return;
#endif 

    if (frequency != 0)
    {
        /*
         * Set up PLL, a structure to hold the value to be set in clocks.
         */
        pll.inputFreq = DEFAULT_INPUT_CLOCK; /* Defined in CLOCK.H */
        pll.clockType = MXCLK_PLL;

        /* 
         * Call calcPllValue() to fill up the other fields for PLL structure.
         * Sometime, the chip cannot set up the exact clock required by User.
         * Return value from calcPllValue() gives the actual possible clock.
         */
 //       DDKDEBUGPRINT((INIT_LEVEL, "setMemoryClock: frequency = 0x%08x\n", frequency));
        ulActualMxClk = calcPllValue(frequency, &pll);
 //       DDKDEBUGPRINT((INIT_LEVEL, "setMemoryClock: Current Clock = 0x%08x\n", ulActualMxClk));

        /* Master Clock Control: MXCLK_PLL */
        pokeRegisterDWord(MXCLK_PLL_CTRL, formatPllReg(&pll));
    }
}

/*
 * This function gets the Main Chip Clock value.
 *
 * Output:
 *      The Actual Main Chip clock value.
 */
unsigned long getChipClock()
{
    pll_value_t pll;

#if 1
	/* Cheok_0509: For SM750LE, the chip clock is fixed */
	//if (getChipType() == SM750LE)
	if (ddk750_getChipType() >= SM750LE)
		return (MHz(SM750LE_MASTER_CLK));
#endif 

    return getPllValue(MXCLK_PLL, &pll);
}

/*
 * This function set up the memory clock.
 *
 * Input: Frequency to be set.
 *
 * NOTE:
 *      The maximum frequency that the DDR Memory clock can be set is 336MHz.
 */
void setMemoryClock(unsigned long frequency)
{
    unsigned long ulReg, divisor;

#if 1
	/* Cheok_0509: For SM750LE, the memory clock is fixed. Nothing to set. */
	//if (getChipType() == SM750LE)
	if (ddk750_getChipType() >= SM750LE)
		return;
#endif 
    
    if (frequency != 0)        
    {
        /* Set the frequency to the maximum frequency that the DDR Memory can take
           which is 336MHz. */
        if (frequency > MHz(336))
            frequency = MHz(336);
        
        /* Calculate the divisor */
        divisor = (unsigned long) roundedDiv(getChipClock(), frequency);
        
        /* Set the corresponding divisor in the register. */
        ulReg = peekRegisterDWord(CURRENT_GATE);
        switch(divisor)
        {   
            default:
            case 1:
                ulReg = FIELD_SET(ulReg, CURRENT_GATE, M2XCLK, DIV_1);
                break;
            case 2:
                ulReg = FIELD_SET(ulReg, CURRENT_GATE, M2XCLK, DIV_2);
                break;
            case 3:
                ulReg = FIELD_SET(ulReg, CURRENT_GATE, M2XCLK, DIV_3);
                break;
            case 4:
                ulReg = FIELD_SET(ulReg, CURRENT_GATE, M2XCLK, DIV_4);
                break;
        }
        
        setCurrentGate(ulReg);
    }
}

/*
 * This function set up the master clock (MCLK).
 *
 * Input: Frequency to be set.
 *
 * NOTE:
 *      The maximum frequency the engine can run is 168MHz.
 */
void setMasterClock(unsigned long frequency)
{
    unsigned long ulReg, divisor;

#if 1
	/* Cheok_0509: For SM750LE, the master clock is fixed. Nothing to set. */
	//if (getChipType() == SM750LE)
	if (ddk750_getChipType() >= SM750LE)
		return;
#endif 
    
    if (frequency != 0)
    {
        /* Set the frequency to the maximum frequency that the SM750 engine can
           run, which is about 190 MHz. */
        if (frequency > MHz(190))
            frequency = MHz(190);
               
        /* Calculate the divisor */
        divisor = (unsigned long) roundedDiv(getChipClock(), frequency);
        
        /* Set the corresponding divisor in the register. */
        ulReg = peekRegisterDWord(CURRENT_GATE);
        switch(divisor)
        {
            default:
            case 3:
                ulReg = FIELD_SET(ulReg, CURRENT_GATE, MCLK, DIV_3);
                break;
            case 4:
                ulReg = FIELD_SET(ulReg, CURRENT_GATE, MCLK, DIV_4);
                break;
            case 6:
                ulReg = FIELD_SET(ulReg, CURRENT_GATE, MCLK, DIV_6);
                break;
            case 8:
                ulReg = FIELD_SET(ulReg, CURRENT_GATE, MCLK, DIV_8);
                break;
        }
        
        setCurrentGate(ulReg);
    }
}

/*
 * This function get the Primary Display Control Pixel Clock value.
 *
 * Output:
 *      The Primary Display Control Pixel Clock value in whole number.
 */
unsigned long getPrimaryDispCtrlClock()
{
    pll_value_t pll;
    return getPllValue(PRIMARY_PLL, &pll);
}

/*
 * This function get the Secondary Display Control Pixel Clock value.
 *
 * Output:
 *      The Secondary Display Control Pixel Clock value in whole number.
 */
unsigned long getSecondaryDispCtrlClock()
{
    pll_value_t pll;
    return getPllValue(SECONDARY_PLL, &pll);
}

/*
 * This function gets the Master Clock value.
 *
 * Output:
 *      The Master Clock value in whole number.
 */
unsigned long getMasterClock()
{
    unsigned long value, divisor;

#if 1
	/* Cheok_0509: For SM750LE, the chip clock is fixed */
	//if (getChipType() == SM750LE)
	if (ddk750_getChipType() >= SM750LE)
		return(MHz(SM750LE_MASTER_CLK));
#endif 
    
    /* Get the divisor */
    value = peekRegisterDWord(CURRENT_GATE);
    switch (FIELD_GET(value, CURRENT_GATE, MCLK))
    {
        case CURRENT_GATE_MCLK_DIV_3:
            divisor = 3;
            break;
        case CURRENT_GATE_MCLK_DIV_4:
            divisor = 4;
            break;
        case CURRENT_GATE_MCLK_DIV_6:
            divisor = 6;
            break;
        case CURRENT_GATE_MCLK_DIV_8:
            divisor = 8;
            break;
    }

    return (getChipClock() / divisor);
}

/*
 * This function gets the Memory Clock value.
 *
 * Output:
 *      The Memory Clock value in whole number.
 */
unsigned long getMemoryClock()
{
    unsigned long value, divisor;

#if 1
	/* Cheok_0509: For SM750LE, the memory clock is fixed. */
	//if (getChipType() == SM750LE)
	if (ddk750_getChipType() >= SM750LE)
		return (MHz(SM750LE_MEMORY_CLK));
#endif 
    
    /* Get the divisor */
    value = peekRegisterDWord(CURRENT_GATE);
    switch (FIELD_GET(value, CURRENT_GATE, M2XCLK))
    {
        case CURRENT_GATE_M2XCLK_DIV_1:
            divisor = 1;
            break;
        case CURRENT_GATE_M2XCLK_DIV_2:
            divisor = 2;
            break;
        case CURRENT_GATE_M2XCLK_DIV_3:
            divisor = 3;
            break;
        case CURRENT_GATE_M2XCLK_DIV_4:
            divisor = 4;
            break;
    }

    return (getChipClock() / divisor);
}

/*
 * This function gets the Master Clock Divider Values List.
 *
 * Output:
 *      The list of Master Clock divider values.
 */
unsigned char *getMasterClockDivider()
{
    return g_ucMasterClockDivider;
}

/*
 * This function gets the total number of Master Clock Divider Values.
 */
unsigned long getTotalMasterClockDivider()
{
    return (sizeof(g_ucMasterClockDivider)/sizeof(unsigned char));
}

/*
 * This function gets the Memory Clock Divider Values List.
 *
 * Output:
 *      The list of Memory Clock divider values.
 */
unsigned char *getMemoryClockDivider()
{
    return g_ucMemoryClockDivider;
}

/*
 * This function gets the total number of Memory Clock Divider Values.
 */
unsigned long getTotalMemoryClockDivider()
{
    return (sizeof(g_ucMemoryClockDivider)/sizeof(unsigned char));
}

/*
 * This function uses the Master Clock PLL clock counter to provide some delay in ms.
 */
void waitMasterClock(
    unsigned long miliseconds
)
{
    unsigned long totalClockCount, startCount, endCount, diff, value;
    
    /* Calculate the clock counter needed for the delay */
    totalClockCount = roundedDiv(getMasterClock(), 1000) * miliseconds;
    
    /* Enable PLL Clock Count*/
    pokeRegisterDWord(MISC_CTRL, FIELD_SET(peekRegisterDWord(MISC_CTRL), MISC_CTRL, PLL_CLK_COUNT, ON));
    
    /* Start the counter */
    endCount = 0;
    while (totalClockCount > 0)
    {
        startCount = endCount;
        value = FIELD_GET(peekRegisterDWord(PLL_CLK_COUNT), PLL_CLK_COUNT, COUNTER);
        endCount = value;
        
        /* Check if the counter has overflown */
        if (startCount > endCount)
            endCount += 0x10000;
        
        /* Find the counter difference from last to start counter. */
        diff = endCount - startCount;
        
        /* Decrement the total clock counter with the difference until it reaches 0. */
        if (totalClockCount > diff)
            totalClockCount -= diff;
        else
            totalClockCount = 0;
    }
    
    /* Disable the PLL Clock Count to reset the counter the next time this function is called. */
    pokeRegisterDWord(MISC_CTRL, FIELD_SET(peekRegisterDWord(MISC_CTRL), MISC_CTRL, PLL_CLK_COUNT, OFF));
}
