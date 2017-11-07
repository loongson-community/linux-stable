/*******************************************************************
* 
*         Copyright (c) 2008 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  intr.h --- SM750/SM718 DDK 
*  This file contains the source code for the interrupt mechanism 
* 
*******************************************************************/
#ifndef _INTR_H_
#define _INTR_H_


/*******************************************************************
 * Interrupt implementation
 * 
 * This implementation is used for handling the interrupt.
 *******************************************************************/

typedef void (*PFNINTRHANDLER)(unsigned long);

/*
 * Register an interrupt handler function to an interrupt status
 */ 
short registerHandler(
    void (*handler)(unsigned long status), 
    unsigned long mask
);

/*
 * Un-register a registered interrupt handler
 */
short unregisterHandler(
    void (*handler)(unsigned long status)
);

/* 
 * Hook a ISR function to the interrupt.
 *
 * This is a low level function providing one to one mapping of a single ISR to the interrupt line.
 * It is a quick method to verify if a specific interrupt is working.
 * However, it does not provide the structure of chaining several ISR together (use registerHandler() to chain ISR, instead).
 *
 * Note: 
 *   1. If this function is used to hook up interrupt, unhookInterrupt() must be use to release the INT.
 *      Don't use unregisterHandler() to release interrupt set up by this function.
 *
 * Output:
 *      0   - Success
 *      -1  - Out of memory
 * 
 */
short hookInterrupt(
    void (*handler)(void),      /* Interrupt function to be registered */
    unsigned long mask          /* interrupt mask */
);

/* 
 * Unregister a registered interrupt handler
 * The matching call for singleIsrRegisterHandler()
 *
 * Output:
 *      0   - Success
 *      -1  - Handler is not found 
 */
short unhookInterrupt(
    unsigned long mask          /* interrupt mask */
);

/*
 * When hookInterrupt() is used to set up interrupt.
 * ISR should call this function before exit.
 *
 */
void notifyEndOfISR(void);


void sb_IRQMask(int irq_num);
void sb_IRQUnmask(int irq_num);

#define SB_IRQ_VAL_TC3     	31
#define SB_IRQ_VAL_TC2     	30
#define SB_IRQ_VAL_TC1     	29
#define SB_IRQ_VAL_TC0     	28
#define SB_IRQ_VAL_USBH		25
#define SB_IRQ_VAL_USBS		24
#define SB_IRQ_VAL_I2S		23
#define SB_IRQ_VAL_USART1  22
#define SB_IRQ_VAL_USART0  21
#define SB_IRQ_VAL_SSP1  	20
#define SB_IRQ_VAL_SSP0		19
#define SB_IRQ_VAL_I2C1		18
#define SB_IRQ_VAL_I2C0		17	
#define SB_IRQ_VAL_PWM		16
#define SB_IRQ_VAL_GPIO6	15
#define SB_IRQ_VAL_GPIO5	14
#define SB_IRQ_VAL_GPIO4	13
#define SB_IRQ_VAL_GPIO3	12
#define SB_IRQ_VAL_GPIO2	11
#define SB_IRQ_VAL_GPIO1	10
#define SB_IRQ_VAL_GPIO0	9
#define SB_IRQ_VAL_DMA		8
#define SB_IRQ_VAL_CPU		7
#define SB_IRQ_VAL_CSC		5
#define SB_IRQ_VAL_DE			4
#define SB_IRQ_VAL_C			3
#define SB_IRQ_VAL_DC1		2
#define SB_IRQ_VAL_DC0		1
#define SB_IRQ_VAL_VGA		0




#endif /* _INTR_H_ */
