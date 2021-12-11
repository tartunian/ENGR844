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
#include <stdio.h>
#include <driverlib/eeprom.h>
#include <driverlib/interrupt.h>
#include <driverlib/pin_map.h>
#include <driverlib/rom_map.h>
#include <driverlib/uart.h>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include <driverlib/timer.h>
#include <inc/hw_memmap.h>
#include <inc/tm4c123gh6pm.h>
#include <utils/uartstdio.h>
#include <utils/ustdlib.h>
#include <drivers/enc28j60/enc28j60.h>
#include <drivers/enc28j60/initHw.h>
#include <config.h>
#include <init_hw.h>
#include <isr.h>
#include <led.h>
#include <print.h>
#include <shell.h>
#include <timers.h>
#include <wait.h>

uint8_t *udpData;
uint8_t rxData[128];
uint8_t txData[128];
PacketType_t packetType;
volatile uint8_t packetReady = 0;
uint32_t printTimerPeriod;
uint32_t ethernetCheckTimerPeriod;
volatile uint8_t displayRx = 1;
volatile uint8_t displayRaw = 0;

char commandBuffer[64];
uint8_t commandBufferSize = 64;
uint8_t commandBufferIndex = 0;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

uint8_t command_ipconfig(uint8_t argc, char **argv) {
    uint8_t* ip = etherGetIpAddress();
    uint8_t* sub = etherGetSubnetMask();
    uint8_t* gw = etherGetGatewayIpAddress();
    UARTprintf("IP:\t\t\t%d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
    UARTprintf("Subnet Mask:\t\t%d.%d.%d.%d\n", sub[0], sub[1], sub[2], sub[3]);
    UARTprintf("Gateway:\t\t%d.%d.%d.%d\n", gw[0], gw[1], gw[2], gw[3]);
}

uint8_t command_setIP(uint8_t argc, char **argv) {
    uint8_t ip[4];
    uint8_t validIP = parseIPv4(*argv, ip);
    if(validIP) {
        etherSetIpAddress(ip[0], ip[1], ip[2], ip[3]);
        UARTprintf("IP set!\n");
    } else {
        UARTprintf("Invalid IP!\n");
    }
}

uint8_t command_setSubnetMask(uint8_t argc, char **argv) {
    uint8_t sub[4];
    uint8_t validSubnetMask = parseIPv4(*argv, sub);
    if(validSubnetMask) {
        etherSetSubnetMask(sub[0], sub[1], sub[2], sub[3]);
        UARTprintf("Subnet mask set!\n");
    } else {
        UARTprintf("Invalid subnet mask!\n");
    }
}

uint8_t command_setGateway(uint8_t argc, char **argv) {
    uint8_t ip[4];
    uint8_t validIP = parseIPv4(*argv, ip);
    if(validIP) {
        etherSetGateway(ip[0], ip[1], ip[2], ip[3]);
        UARTprintf("Gateway set!\n");
    } else {
        UARTprintf("Invalid IP!\n");
    }
}

uint8_t command_ping(uint8_t argc, char **argv)
{
    uint8_t ip[4];
    parseIPv4(*argv, ip);
    ARPEntry *arpEntry = getARPEntry(ip);

    uint8_t *selfIP = etherGetIpAddress();
    uint8_t sameSubnet = etherIsSameSubnet(selfIP, ip);
//    UARTprintf("%d.%d.%d.%d %s in same subnet as %d.%d.%d.%d\n", ip[0], ip[1],
//               ip[2], ip[3], sameSubnet ? "is" : "is not", selfIP[0], selfIP[1],
//               selfIP[2], selfIP[3]);

    if (arpEntry)
    {
        etherSendPingReq(arpEntry->mac, ip);
    }
    else
    {

        if (!sameSubnet)
        {
            ARPEntry *gatewayARPEntry = getARPEntry(etherGetGatewayIpAddress());

            if (gatewayARPEntry)
            {
                etherSendPingReq(gatewayARPEntry->mac, ip);
            }
            else
            {
                UARTprintf("Cannot find network gateway!\n");
            }
        }

        etherSendArpReq(ip);
    }
    return 0;
}

uint8_t command_arp(uint8_t argc, char **argv)
{

    // If there is an argument (IP) included
    if (*argv[0])
    {
        uint8_t ip[4];
        uint8_t validIP = parseIPv4(*argv, ip);

        if (validIP)
        {
            ARPEntry *arpEntry = getARPEntry(ip);
            if (arpEntry)
            {
                UARTprintf("IP: %d.%d.%d.%d\t\tMAC: %2x:%2x:%2x:%2x:%2x:%2x\n",
                           arpEntry->ip[0], arpEntry->ip[1], arpEntry->ip[2],
                           arpEntry->ip[3], arpEntry->mac[0], arpEntry->mac[1],
                           arpEntry->mac[2], arpEntry->mac[3], arpEntry->mac[4],
                           arpEntry->mac[5]);
            }
            else
            {
                UARTprintf("IP: %d.%d.%d.%d\t\tMAC: Unknown\n", ip[0], ip[1],
                           ip[2], ip[3]);
                etherSendArpReq(ip);
            }
        } else {
            UARTprintf("Invalid IP!\n");
        }
    }

    // If there is no argument included
    else
    {
        printARPTable(arpTable, getARPTableCount());
    }
    return 0;
}

uint8_t command_raw(uint8_t argc, char **argv) {
    displayRaw = !displayRaw;
}

uint8_t command_help(uint8_t argc, char **argv) {
    UARTprintf("Press CTRL+S to open the shell\n");
    UARTprintf("Press CTRL+C to terminate output.\n");
    UARTprintf("Press CTRL+B to restart output.\n");
    for(int i=0; i<getCommandCount(); i++) {
        command_t cmd = command_table[i];
        UARTprintf("%16s - %s\n", cmd.command_str, cmd.command_desc);
    }
}

uint8_t command_uptime(uint8_t argc, char **argv) {
    if(etherStartTimestamp) {
        uint32_t uptime = (uint32_t)((TimerValueGet64(WTIMER5_BASE) - etherStartTimestamp)/SysCtlClockGet());
        UARTprintf("Uptime: %u sec\n", uptime);
    } else {
        UARTprintf("No network activity yet!\n");
    }
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

void main(void)
{

    // Configure HW to work with 16 MHz XTAL, PLL enabled, system clock of 40 MHz
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN
            | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);

    etherInitHW();

    ConfigureUSBUART0();
    ConfigureUSBUART0Interrupt();
    UARTprintf("UART0 configured!\n");

    ConfigureEEPROM();
    EEPROMRead((uint32_t*)etherGetIpAddress(), 0x00, 4);
    EEPROMRead((uint32_t*)etherGetSubnetMask(), 0x10, 4);
    EEPROMRead((uint32_t*)etherGetGatewayIpAddress(), 0x20, 4);
    UARTprintf("Loaded IP configuration from EEPROM!\n");

    EEPROMRead((uint32_t*)arpTable, 0x30, ARP_TBL_SIZE * sizeof(ARPEntry));
    EEPROMRead((uint32_t*)&arpTableCount, 0x40, sizeof(uint32_t));
    UARTprintf("Loaded ARP table from EEPROM!\n");

    printTimerPeriod = SysCtlClockGet() / 100;
    ethernetCheckTimerPeriod = SysCtlClockGet() / 100;

    ConfigureTimers();
    ResetTimer0(printTimerPeriod);
    ResetTimer1(ethernetCheckTimerPeriod);
    ResetTimer2(0xFFFFFFFF);
    ResetTimer3(6*SysCtlClockGet());
    ResetWTimer5();
    ConfigureTimerInterrupts();
    UARTprintf("Timers configured!\n");

    // Initialize the Ethernet interface
    etherInit(ETHER_UNICAST | ETHER_BROADCAST | ETHER_HALFDUPLEX, rxData,
              txData, BUF_SIZE);
    UARTprintf("Ethernet initialized!\n");

//    etherSetIpAddress(192, 168, 1, 100);
//    etherSetGateway(192, 168, 1, 1);
//    etherSetSubnetMask(255, 255, 255, 0);
//    UARTprintf("IP Set!\n");

    command_t ping_cmd = { "ping", command_ping, "Pings an IPv4 address." };
    command_t arp_cmd = { "arp", command_arp,
                          "Looks up the IPv4 address of a MAC address." };
    command_t raw_cmd = { "raw", command_raw, "Toggles raw printing of Ethernet frames." };
    command_t ipconfig_cmd = { "ipconfig", command_ipconfig, "Displays IPv4 configuration." };
    command_t setip_cmd = { "setip", command_setIP, "Sets IPv4 address." };
    command_t setsub_cmd = { "setsub", command_setSubnetMask, "Sets IPv4 subnet." };
    command_t setgw_cmd = { "setgw", command_setGateway, "Sets IPv4 gateway." };
    command_t help_cmd = { "help", command_help, "Prints available commands." };
    command_t uptime_cmd = { "uptime", command_uptime, "Reports time that Tiva has been on the network." };

    addCommand(ping_cmd);
    addCommand(arp_cmd);
    addCommand(raw_cmd);
    addCommand(ipconfig_cmd);
    addCommand(setip_cmd);
    addCommand(setsub_cmd);
    addCommand(setgw_cmd);
    addCommand(help_cmd);
    addCommand(uptime_cmd);

    IntMasterEnable();
    UARTprintf("Interrupts enabled!\n");

    TimerEnable(TIMER0_BASE, TIMER_A);                          // Start TIMER0A
    TimerEnable(TIMER1_BASE, TIMER_A); // Start the timer which handles packet checking
    TimerEnable(TIMER3_BASE, TIMER_A);
    TimerEnable(WTIMER5_BASE, TIMER_A);
    UARTprintf("System running...\n");
    UARTprintf("Press CTRL+S to open the shell\n");
    UARTprintf("Press CTRL+C to terminate output.\n");
    UARTprintf("Press CTRL+B to restart output.\n");
    UARTprintf("Type 'help' for list of commands\n");
//    command_help(0, 0);

    while (1)
        ;

}
