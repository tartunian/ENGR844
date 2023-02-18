/*
 * timers.c
 *
 *  Created on: Nov 30, 2021
 *      Author: tartunian
 */

#include <stdint.h>
#include <stdbool.h>

#include <inc/hw_memmap.h>
#include <inc/tm4c123gh6pm.h>

#include <driverlib/interrupt.h>
#include <driverlib/timer.h>
#include <driverlib/rom_map.h>

#include "periph_timers.h"

// Configure the periodic timer
void periphResetTimer0(uint32_t period) {
    MAP_TimerDisable(TIMER0_BASE, TIMER_A);                 // Disable TIMER0 while configuring
    MAP_TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC);  // Set TIMER0 as periodic
    MAP_TimerLoadSet(TIMER0_BASE, TIMER_A, period - 1);     // Set the period
    MAP_IntPendClear(INT_TIMER0A);                          // Acknowledge/clear any pending calls? might not be necessary
}

void periphEnableTimers()
{
    TimerEnable(TIMER0_BASE, TIMER_A);                          // Start TIMER0A
}
