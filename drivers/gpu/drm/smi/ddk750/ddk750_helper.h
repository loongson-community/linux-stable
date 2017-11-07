/*******************************************************************
* 
*         Copyright (c) 2009 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  helper.h --- SMI DDK 
*  This file contains the helper functions those are used throughout
*  the library
* 
*******************************************************************/
#ifndef _HELPER_H_
#define _HELPER_H_

/* Perform a rounded division. 
 * For example, if the result is 4.5, this function returns 5.
 * If the result is 4.4, this function returns 4.
 */
unsigned long roundedDiv(unsigned long num, unsigned long denom);

/* Absolute differece between two numbers */
unsigned long absDiff(unsigned long a, unsigned long b);

/* This function calculates 2 to the power of x 
   Input is the power number.
 */
unsigned long twoToPowerOfx(unsigned long x);

#endif  /* _HELPER_H_ */
