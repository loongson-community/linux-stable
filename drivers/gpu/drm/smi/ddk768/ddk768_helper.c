/*******************************************************************
* 
*         Copyright (c) 2009 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  helper.c --- SM750 DDK 
*  This file contains helper functions those are used throughout
*  the DDK library.
* 
*******************************************************************/
#include "ddkdebug.h"

/* A test counter to be shared by all modules */
static unsigned long gTestCounter;

/* Functions to manipulate a test counter. */
unsigned long getTestCounter(void)
{
    return gTestCounter;
}

void setTestCounter(unsigned long value)
{
    gTestCounter = value;
}

void incTestCounter(void)
{
    gTestCounter++;
}

/* Perform a rounded division with signed number. 
 * For example, if the result is 4.5, this function returns 5.
 * If the result is 4.4, this function returns 4.
 */
long lRoundedDiv(long num, long denom)
{
    /* n / d + 1 / 2 = (2n + d) / 2d */
    return (2 * num + denom) / (2 * denom);
}

/* Perform a rounded division. 
 * For example, if the result is 4.5, this function returns 5.
 * If the result is 4.4, this function returns 4.
 */
unsigned long ddk768_roundedDiv(unsigned long num, unsigned long denom)
{
    /* n / d + 1 / 2 = (2n + d) / 2d */
    return (2 * num + denom) / (2 * denom);
}

/* Perform a rounded division with unsigned number. 
 * For example, if the result is 4.5, this function returns 5.
 * If the result is 4.4, this function returns 4.
 */
unsigned long ulRoundedDiv(unsigned long num, unsigned long denom)
{
    return ddk768_roundedDiv(num, denom);
}

/* Absolute differece between two numbers */
unsigned long ddk768_absDiff(unsigned long a, unsigned long b)
{
    if ( a >= b )
        return(a - b);
    else
        return(b - a);
}

/* This function calculates 2 to the power of x 
   Input is the power number.
 */
unsigned long ddk768_twoToPowerOfx(unsigned long x)
{
    unsigned long i;
    unsigned long result = 1;

    for (i=1; i<=x; i++)
        result *= 2;

    return result;
}
