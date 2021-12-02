/*
 * led.c
 *
 *  Created on: Nov 30, 2021
 *      Author: tartunian
 */

#include <stdbool.h>
#include <led.h>
#include <config.h>
#include <timers.h>
#include <driverlib/gpio.h>
#include <driverlib/timer.h>
#include <inc/hw_memmap.h>
#include <inc/tm4c123gh6pm.h>

void flashLED(uint8_t red, uint8_t green, uint8_t blue, uint32_t duration)
{
    GPIO_PORTF_DATA_R |= ((red ? RED_LED_PIN : 0) | (green ? GREEN_LED_PIN : 0)
            | (blue ? BLUE_LED_PIN : 0));
    ResetTimer2(duration);
    TimerEnable(TIMER2_BASE, TIMER_A);
}
