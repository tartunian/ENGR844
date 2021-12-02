/*
 * init_hw.c
 *
 *  Created on: Nov 30, 2021
 *      Author: tartunian
 */

#include <init_hw.h>

#include <stdbool.h>
#include <stdint.h>

#include <driverlib/eeprom.h>
#include <driverlib/gpio.h>
#include <driverlib/interrupt.h>
#include <driverlib/pin_map.h>
#include <driverlib/timer.h>
#include <driverlib/uart.h>
#include <driverlib/sysctl.h>
#include <utils/uartstdio.h>

#include <inc/hw_memmap.h>
#include <inc/tm4c123gh6pm.h>



void ConfigureEEPROM(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_EEPROM0));
    EEPROMInit();
}

void ConfigureTimers(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);  // Enable the clock to TIMER0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);  // Enable the clock to TIMER1
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);  // Enable the clock to TIMER2
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);  // Enable the clock to TIMER3
}

void ConfigureTimerInterrupts(void)
{

    // Configure periodic timer
    IntPrioritySet(INT_TIMER0A, 0x00); // Set highest priority for the periodic timer
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT); // Don't know why this and IntEnable(INT_TIMER0A) are needed
    IntPendClear(INT_TIMER0A);

    // Configure periodic timer
    IntPrioritySet(INT_TIMER1A, 0x01); // Set highest priority for the periodic timer
    IntEnable(INT_TIMER1A);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT); // Don't know why this and IntEnable(INT_TIMER1A) are needed
    IntPendClear(INT_TIMER1A);

    // Configure debounce timer
    IntPrioritySet(INT_TIMER2A, 0x07); // Set lowest priority for the debouncing timer
    IntEnable(INT_TIMER2A);
    TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT); // Don't know why this and IntEnable(INT_TIMER2A) are needed
    IntPendClear(INT_TIMER2A);

    IntPrioritySet(INT_TIMER3A, 0x07);
    IntEnable(INT_TIMER3A);
    TimerIntEnable(TIMER3_BASE, TIMER_TIMA_TIMEOUT);
    IntPendClear(INT_TIMER3A);
}

void ConfigureUSBUART0(void)
{

    //
    // Enable Peripheral Clocks
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable pin PA0 for UART0 U0RX
    //
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0);

    //
    // Enable pin PA1 for UART0 U0TX
    //
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_1);

    UARTFIFOLevelSet(UART0_BASE, UART_FIFO_TX1_8, UART_FIFO_RX1_8); // Set the FIFO size to 1 so that interrupts are generated for each character

    //
    // Use the internal 16MHz oscillator as the UART0 clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
}

void ConfigureUSBUART0Interrupt(void)
{
    IntEnable(INT_UART0);                         // Enable interrupts for UART0
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT); // Enable specific interrupts for UART0 receive and receive timeout
}
