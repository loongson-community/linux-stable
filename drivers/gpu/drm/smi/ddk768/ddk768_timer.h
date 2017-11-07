/*******************************************************************
*
*         Copyright (c) 2014 by Silicon Motion, Inc. (SMI)
*
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
*
*  This file contains the definitions for the timer functions.
*
*******************************************************************/
#ifndef _TIMER_H_
#define _TIMER_H_



typedef enum _timer_number_t
{
    TIMER0 = 0,
    TIMER1 = 1,
    TIMER2 = 2,
    TIMER3 = 3,
}
timer_number_t;

/*
 * Calculate a value for timer counter according to input time in micro-second.
 * Calculation is based on 168MHz master clock, and the counter decrements at every 16 ticks.
 */
unsigned long calcTimerCounter(
    unsigned long microSeconds
);

/*
 * This function start the timer with raw interrupt enabled.
 * When the timer decrements to 0, timer raw interrupt will be generated.
 *
 * Raw interrupt of the timers can be used in one of 2 ways:
 * 1. In pulling mode, detection of raw interrupt pending means timer is decremented to 0.
 * 2. In interrupt mode, unlock the timer interrupt mask will generate a interrput to system.
 *
 */
void timerStart(
    timer_number_t timer,         /* which timer: 0 to 3 */
    unsigned long timerCounter,      /* Timer counter: use calcTimerCounter() to work out a counter for a specific period. */
    unsigned long div16Enable     /* Enable the 16 divisor, time out will be increased by 16 */
);

/*
 * This function checks if a timer's raw interrupt has been pending.
 * When raw interrupt is detected with pending status, it indicate the
 * countdown of timerStart() has been completed.
 *
 * Return:
 *        1 = Raw interrupt status is pending.
 *        0 = Raw int is NOT pending.
 */
unsigned long timerRawIntPending(
    timer_number_t timer         /* which timer: 0 to 3 */
);

/*
 * This function clears the RAW interrupt status of the timer.
 *
 * When a timer completes countdown, the raw interrupt bit will be set.
 * It has to be cleared, in order to distinguish between different sessions of countdown.
 *
 */
void timerClearRawInt(
    timer_number_t timer         /* which timer: 0 to 3 */
);

/*
 * This function stop the timer.
 *
 */
void timerStop(
    timer_number_t timer         /* which timer: 0 to 3 */
);

/*
 * This function read the current value in the timer counter.
 *
 * Note: When timer is disable, always read back 0.
 */
unsigned long timerGetCounter(
    timer_number_t timer         /* which timer: 0 to 3 */
);

/*
 * This function gets the countdown setting stored in timer.
 * Function timerGetCounter() can only get the current counter value.
 * It cannot get the original countdown setting of timer.
 *
 * Note: When timer is disable, always read back 0.
 */
unsigned long timerGetCounterSetting(
    timer_number_t timer         /* which timer: 0 to 3 */
);

/* 
 * This funciton uses the timer to wait a specific amount of time
 * in micro-second.
 */
void timerWait(
    timer_number_t timer,
    unsigned long microSeconds
);

/* 
 * This funciton uses the timer to wait a specific ticks of master clock
 *
 */
void timerWaitTicks(
    timer_number_t timer, /* Use timer 0, 1, 2 or 3 */
    unsigned long ticks
);

/* 
 * This function returns the INT mask for a specific timer.
 *
 */
unsigned long timerIntMask(
    timer_number_t timer        /* Which timer: 0 to 3 */
);

unsigned long getTestCounter(void);

void setTestCounter(unsigned long value);

/*
 * This is a reference sample showing how to implement ISR for timers.
 * It works together with libsrc\intr.c module.
 * 
 * Refer to Apps\timer\tstimer.c on how to hook up this function with system
 * interrupt under WATCOM DOS extender.
 * 
 */
void timerIsrTemplate(unsigned long status);

void timerWaitMsec(
    unsigned long milliSeconds
);

void timerWaitUsec(
    unsigned long USeconds
);
#define sb_OS_WAIT_MSEC_POLL(ms) timerWaitMsec(ms)
#define sb_OS_WAIT_USEC_POLL(us) timerWaitUsec(us)
#endif /* _TIMER_H_ */

