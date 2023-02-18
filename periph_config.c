/*
 * init_hw.c
 *
 *  Created on: Nov 30, 2021
 *      Author: tartunian
 */

#include <stdbool.h>
#include <stdint.h>

#include <driverlib/eeprom.h>
#include <driverlib/gpio.h>
#include <driverlib/interrupt.h>
#include <driverlib/pin_map.h>
#include <driverlib/timer.h>
#include <driverlib/uart.h>
#include <driverlib/sysctl.h>
#include <driverlib/rom_map.h>
#include <utils/uartstdio.h>

#include <inc/hw_memmap.h>
#include <inc/tm4c123gh6pm.h>
#include <periph_config.h>

void periphConfigEEPROM0(void) {
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_EEPROM0));
    MAP_EEPROMInit();
}

void periphConfigTimers(void) {
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);  // Enable the clock to TIMER0
}

void periphConfigTimerInterrupts(void)
{
    // Configure periodic timer
    MAP_IntPrioritySet(INT_TIMER0A, 0x00); // Set highest priority for the periodic timer
    MAP_IntEnable(INT_TIMER0A);
    MAP_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT); // Don't know why this and IntEnable(INT_TIMER0A) are needed
    MAP_IntPendClear(INT_TIMER0A);
}

void periphConfigUSBUART0(void)
{

    //
    // Enable Peripheral Clocks
    //
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable pin PA0 for UART0 U0RX
    //
    MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0);

    //
    // Enable pin PA1 for UART0 U0TX
    //
    MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_1);

    MAP_UARTFIFOLevelSet(UART0_BASE, UART_FIFO_TX1_8, UART_FIFO_RX1_8); // Set the FIFO size to 1 so that interrupts are generated for each character



    //
    // Use the internal 16MHz oscillator as the UART0 clock source.
    //
    MAP_UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
}

void periphConfigUSBUART0Interrupt(void)
{
    MAP_IntEnable(INT_UART0);                         // Enable interrupts for UART0
    MAP_UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT); // Enable specific interrupts for UART0 receive and receive timeout
}
