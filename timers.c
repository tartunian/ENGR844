/*
 * timers.c
 *
 *  Created on: Nov 30, 2021
 *      Author: tartunian
 */

#include <timers.h>
#include <stdbool.h>
#include <driverlib/interrupt.h>
#include <driverlib/timer.h>
#include <inc/hw_memmap.h>
#include <inc/tm4c123gh6pm.h>

// Configure the periodic timer
void ResetTimer0(uint32_t period)
{
    TimerDisable(TIMER0_BASE, TIMER_A);      // Disable TIMER0 while configuring
    TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC); // Set TIMER0 as periodic
    TimerLoadSet(TIMER0_BASE, TIMER_A, period - 1);            // Set the period
    IntPendClear(INT_TIMER0A); // Acknowledge/clear any pending calls? might not be necessary
}

// Configure the periodic timer
void ResetTimer1(uint32_t period)
{
    TimerDisable(TIMER1_BASE, TIMER_A);      // Disable TIMER1 while configuring
    TimerConfigure(TIMER1_BASE, TIMER_CFG_A_PERIODIC); // Set TIMER1 as periodic
    TimerLoadSet(TIMER1_BASE, TIMER_A, period - 1);            // Set the period
    IntPendClear(INT_TIMER1A); // Acknowledge/clear any pending calls? might not be necessary
}

// Configure the LED timer
void ResetTimer2(uint32_t period)
{
    TimerDisable(TIMER2_BASE, TIMER_A);      // Disable TIMER2 while configuring
    TimerConfigure(TIMER2_BASE, TIMER_CFG_A_ONE_SHOT); // Set TIMER2 as one-shot
    TimerLoadSet(TIMER2_BASE, TIMER_A, period - 1);            // Set the period
    IntPendClear(INT_TIMER2A); // Acknowledge/clear any pending calls? might not be necessary
}

// Configure the EEPROM save timer
void ResetTimer3(uint32_t period)
{
    TimerDisable(TIMER3_BASE, TIMER_A);      // Disable TIMER3 while configuring
    TimerConfigure(TIMER3_BASE, TIMER_CFG_A_PERIODIC); // Set TIMER3 as one-shot
    TimerLoadSet(TIMER3_BASE, TIMER_A, period - 1);            // Set the period
    IntPendClear(INT_TIMER3A); // Acknowledge/clear any pending calls? might not be necessary
}
