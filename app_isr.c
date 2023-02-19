/*
 * isr.c
 *
 *  Created on: Nov 30, 2021
 *      Author: tartunian
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include <inc/hw_memmap.h>
#include <inc/tm4c123gh6pm.h>

#include <driverlib/gpio.h>
#include <driverlib/rom_map.h>
#include <driverlib/ssi.h>
#include <driverlib/timer.h>
#include <driverlib/uart.h>
#include <driverlib/sysctl.h>

#include <utils/uartstdio.h>

// Driver headers
#include "embedded_cli.h"
#include "embedded_cli_port.h"

#include "ether.h"


void UART0_Handler(void) {
    char c = UARTCharGet(UART0_BASE);
    embeddedCliReceiveChar(cliObject, c);
}


void TIMER0_Handler(void) {
    TimerIntClear(TIMER0_BASE, TIMER_A);
    UARTprintf("TIMER0\n");
    checkEthernet(TimerValueGet(TIMER0_BASE, TIMER_A));
}

void SSI2_Handler(void) {
    UARTprintf("New SSI2 data\n");
    SSIIntClear(SSI2_BASE, 0xff);
}
