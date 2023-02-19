// Standard C headers
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// TI peripheral drivers
#include <driverlib/interrupt.h>
#include "driverlib/sysctl.h"
#include <driverlib/timer.h>
#include <driverlib/uart.h>
#include <driverlib/rom_map.h>

// TI utilities
#include <utils/uartstdio.h>

// Driver headers
#include "embedded_cli.h"
#include "embedded_cli_port.h"
#include "drivers/enc28j60/initHw.h"
#include "drivers/enc28j60/enc28j60.h"

// Application headers
#include "app_config.h"
#include "periph_config.h"
#include "periph_timers.h"
#include "ether.h"



void main() {

    // Configure HW to work with 16 MHz XTAL, PLL enabled, system clock of 80 MHz
    MAP_SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

    periphConfigUSBUART0();
    periphConfigUSBUART0Interrupt();
#ifdef APP_DEBUG
    UARTprintf("UART0 configured!\n");
#endif

    periphConfigTimers();
    periphResetTimer0(5 * SysCtlClockGet());
    periphConfigTimerInterrupts();
#ifdef APP_DEBUG
    UARTprintf("Timers configured!\n");
#endif

    etherInitHW();
    etherInit(ETHER_UNICAST | ETHER_BROADCAST | ETHER_HALFDUPLEX, rxData, txData, ETHER_BUF_SIZE);
        UARTprintf("Ethernet initialized!\n");

    uint32_t cliCreated = cliInit();
    if(cliCreated == CLI_CREATE_FAIL)
        exit(0);

    // Start all timers
    periphEnableTimers();
    UARTprintf("Timers started!\n");

    MAP_IntMasterEnable();
#ifdef APP_DEBUG
    UARTprintf("Interrupts enabled!\n");
#endif

    while(1) {
        embeddedCliProcess(cliObject);
    }

}
