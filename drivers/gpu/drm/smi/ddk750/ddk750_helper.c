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
#include "ddk750_helper.h"

/* Perform a rounded division. 
 * For example, if the result is 4.5, this function returns 5.
 * If the result is 4.4, this function returns 4.
 */
unsigned long roundedDiv(unsigned long num, unsigned long denom)
{
    /* n / d + 1 / 2 = (2n + d) / 2d */
    return (2 * num + denom) / (2 * denom);
}


/* Absolute differece between two numbers */
unsigned long absDiff(unsigned long a, unsigned long b)
{
    if ( a >= b )
        return(a - b);
    else
        return(b - a);
}

/* This function calculates 2 to the power of x 
   Input is the power number.
 */
unsigned long twoToPowerOfx(unsigned long x)
{
    unsigned long i;
    unsigned long result = 1;

    for (i=1; i<=x; i++)
        result *= 2;

    return result;
}
