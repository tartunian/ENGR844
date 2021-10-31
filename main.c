// Ethernet Example
// Credits : Dr. Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL w/ ENC28J60
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// ENC28J60 Ethernet controller
//   MOSI (SSI2Tx) on PB7
//   MISO (SSI2Rx) on PB6
//   SCLK (SSI2Clk) on PB4
//   ~CS connected to PB1

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom_map.h"
#include "driverlib/uart.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

#include "utils/uartstdio.h"
#include "drivers/enc28j60/enc28j60.h"
#include "drivers/enc28j60/initHw.h"
#include "drivers/enc28j60/led.h"
#include "wait.h"

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void ConfigureUSBUART0(void)
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

    //
    // Use the internal 16MHz oscillator as the UART0 clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
}

void printPacket(uint8_t* data, uint8_t length)
{
    uint8_t i = 0;
    UARTprintf("----------Packet----------\n");
    for (; i < length-4; i++)
    {
        UARTprintf("%02x ", data[i+4]);
        if ((i+1) % 16 == 0)
            UARTCharPut(UART0_BASE, '\n');
    }
    UARTCharPut(UART0_BASE, '\n');
    UARTprintf("--------------------------\n");
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

void main(void)
{

    ConfigureUSBUART0();
    UARTprintf("UART0 configured!\n");

    uint8_t *udpData;
    uint8_t data[128];

    // init controller
    initHw();

    // init ethernet interface
    etherInit(ETHER_UNICAST | ETHER_BROADCAST | ETHER_HALFDUPLEX);
    etherSetIpAddress(192, 168, 1, 100);    //

    // flash phy leds
    etherWritePhy(PHLCON, 0x0880);
    RED_LED = 1;
    waitMicrosecond(500000);
    etherWritePhy(PHLCON, 0x0990);
    RED_LED = 0;
    waitMicrosecond(500000);

    // message loop
    while (1)
    {
        if (etherKbhit())
        {
            if (etherIsOverflow())
            {
                RED_LED = 1;
                waitMicrosecond(100000);
                RED_LED = 0;
            }
            // get packet
            uint8_t packetSize = etherGetPacket(data, 128);
            printPacket(data, packetSize);

            // handle arp request
            if (etherIsArp(data))
            {
                etherSendArpResp(data);
                RED_LED = 1;
                GREEN_LED = 1;
                waitMicrosecond(50000);
                RED_LED = 0;
                GREEN_LED = 0;
            }
            // handle ip datagram
            if (etherIsIp(data))
            {
                if (etherIsIpUnicast(data))
                {
                    // handle icmp ping request
                    if (etherIsPingReq(data))
                    {
                        etherSendPingResp(data);
                        RED_LED = 1;
                        BLUE_LED = 1;
                        waitMicrosecond(50000);
                        RED_LED = 0;
                        BLUE_LED = 0;
                    }
                    // handle udp datagram
                    if (etherIsUdp(data))
                    {
                        udpData = etherGetUdpData(data);
                        if (udpData[0] == '1')
                            GREEN_LED = 1;
                        if (udpData[0] == '0')
                            GREEN_LED = 0;
                        etherSendUdpData(data, (uint8_t*) "Received", 9);
                        BLUE_LED = 1;
                        waitMicrosecond(100000);
                        BLUE_LED = 0;
                    }
                }
            }
        }
    }

}
