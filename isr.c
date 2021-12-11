/*
 * isr.c
 *
 *  Created on: Nov 30, 2021
 *      Author: tartunian
 */

#include <isr.h>
#include <config.h>
#include <led.h>
#include <print.h>
#include <shell.h>
#include <timers.h>
#include <stdbool.h>
#include <driverlib/eeprom.h>
#include <driverlib/gpio.h>
#include <driverlib/timer.h>
#include <driverlib/uart.h>
#include <driverlib/sysctl.h>
#include <inc/hw_memmap.h>
#include <inc/tm4c123gh6pm.h>
#include <utils/uartstdio.h>
#include <drivers/enc28j60/enc28j60.h>

extern PacketType_t packetType;
extern volatile uint8_t packetReady;
extern uint32_t printTimerPeriod;
extern uint32_t ethernetCheckTimerPeriod;
extern uint8_t *udpData;
extern uint8_t rxData[];
extern volatile uint8_t displayRx;
extern volatile uint8_t displayRaw;
extern char commandBuffer[];
extern uint8_t commandBufferSize;
extern uint8_t commandBufferIndex;

uint8_t checkEthernet(void)
{

    // Check if an Ethernet frame has been received
    if (etherKbhit())
    {

        // Increment the internal packet count
        etherIncrementPacketCount();

        if (etherGetPacketCount() == 1)
        {
            etherStartTimestamp = TimerValueGet64(WTIMER5_BASE);
        }

        // Check if there was an overflow problem and wait for it to clear
        if (etherIsOverflow())
        {
            flashLED(1, 1, 1, SysCtlClockGet() / 10);
        }

        // Get the new frame/packet from the buffer with maximum size of 128 bytes
        uint8_t packetSize = etherGetPacket(rxData, BUF_SIZE);

        packetType = getPacketType();

        // Check if the frame is ARP
        if (packetType & PKT_ARP)
        {

            ARPHeader *arpHeader = (ARPHeader*) (rxData + sizeof(ENCHeader)
                    + sizeof(EthernetHeader));

            // Check if the packet is an ARP request
            if (packetType & PKT_ARP_REQ)
            {
                etherSendArpResp();

            }
            // Check if the packet is an ARP response
            else if (packetType & PKT_ARP_RES)
            {
                ARPEntry entry;
                memcpy(&entry.ip, arpHeader->senderIp, 4);
                memcpy(&entry.mac, arpHeader->senderAddress, 6);
                addARPEntry(entry);
            }
        }

        // Check if the frame is an IP Packet
        if (packetType & PKT_IP)
        {

            // Check if the packet is a ping request
            if (packetType & PKT_ICMP_REQ)
            {
                etherSendPingResp();
            }

            // Check if the packet is a ping response
            if (packetType & PKT_ICMP_RES)
            {

            }

            // Check if the frame is a UDP packet
            if (packetType & PKT_UDP)
            {
                udpData = etherGetUdpData();
                etherSendUdpData((uint8_t*) "Received", 9);
            }
        }
        return 1;
    }
    else
    {
        return 0;
    }
}

// Handler for the print timer
void TIMER0_Handler(void)
{
    TIMER0_ICR_R |= 0x01;                               // Clear the TIMER0 flag
    TimerLoadSet(TIMER0_BASE, TIMER_A, printTimerPeriod - 1);

    if (packetReady && displayRx)
    {

        if (displayRaw)
        {
            printFrame(rxData + sizeof(ENCHeader), 128);
        }
        else if (packetType & PKT_ARP)
        {
            ARPHeader *arpHeader = (ARPHeader*) (rxData + sizeof(ENCHeader)
                    + sizeof(EthernetHeader));
            flashLED(1, 0, 0, SysCtlClockGet() / 10);
            printARPPacket(arpHeader);
        }
        else if (packetType & PKT_ICMP_REQ)
        {
            IPHeader *ipHeader = (IPHeader*) (rxData + sizeof(ENCHeader)
                    + sizeof(EthernetHeader));
            ICMPHeader *icmpHeader = (ICMPHeader*) ((uint8_t*) ipHeader
                    + ((ipHeader->rev_size & 0xF) * 4));
            flashLED(0, 0, 1, SysCtlClockGet() / 10);
            printPingRequest(ipHeader, icmpHeader);
        }
        else if (packetType & PKT_ICMP_RES)
        {
            IPHeader *ipHeader = (IPHeader*) (rxData + sizeof(ENCHeader)
                    + sizeof(EthernetHeader));
            ICMPHeader *icmpHeader = (ICMPHeader*) ((uint8_t*) ipHeader
                    + ((ipHeader->rev_size & 0xF) * 4));
            flashLED(0, 0, 1, SysCtlClockGet() / 10);
            printPingResponse(ipHeader, icmpHeader);
        }
        else if (packetType & PKT_UDP)
        {
            IPHeader *ipHeader = (IPHeader*) (rxData + sizeof(ENCHeader)
                    + sizeof(EthernetHeader));
            UDPHeader *udpHeader = (UDPHeader*) (rxData + sizeof(ENCHeader)
                    + sizeof(EthernetHeader) + sizeof(IPHeader));
            flashLED(1, 1, 0, SysCtlClockGet() / 10);
            printUDPPacket(ipHeader, udpHeader);
        }
        packetReady = 0;
    }
}

// Handler for the packet RX timer
void TIMER1_Handler(void)
{
    TIMER1_ICR_R |= 0x01;                               // Clear the TIMER1 flag
    TimerDisable(TIMER1_BASE, TIMER_A);         // Disable TIMER1 while checking
    packetReady = checkEthernet() ? 1 : packetReady;
    TimerEnable(TIMER1_BASE, TIMER_A);                        // Reenable TIMER1
}

// Handler for the LEDduration timer
void TIMER2_Handler(void)
{
    TIMER2_ICR_R |= 0x01;                               // Clear the TIMER2 flag
    TimerDisable(TIMER2_BASE, TIMER_A);
    GPIO_PORTF_DATA_R &= ~(RED_LED_PIN | GREEN_LED_PIN | BLUE_LED_PIN);
}

// Handler for the EEPROM save timer
void TIMER3_Handler(void)
{
    TIMER3_ICR_R |= 0x01;                               // Clear the TIMER3 flag

    if (etherConfigHasChanged)
    {

        uint32_t writeStatus = 0;
        writeStatus = EEPROMProgram((uint32_t*) etherGetIpAddress(), 0x00,
                                    sizeof(uint32_t));
        writeStatus = EEPROMProgram((uint32_t*) etherGetSubnetMask(), 0x10,
                                    sizeof(uint32_t));
        writeStatus = EEPROMProgram((uint32_t*) etherGetGatewayIpAddress(),
                                    0x20, sizeof(uint32_t));
        writeStatus = EEPROMProgram((uint32_t*) arpTable, 0x30,
                                    ARP_TBL_SIZE * sizeof(ARPEntry));
        writeStatus = EEPROMProgram((uint32_t*) &arpTableCount, 0x40,
                                    sizeof(uint32_t));
        etherConfigHasChanged = 0;
        flashLED(0, 1, 0, SysCtlClockGet() / 10);
        UARTprintf("Configuration changes saved to EEPROM.\n");

    }
}

void UART0_Handler(void)
{
    uint32_t UART0Status = UARTIntStatus(UART0_BASE, true); // Get the UART status

    char c = UARTCharGetNonBlocking(UART0_BASE);

    // CTRL+B, CTRL+C and Enter should have immediate effect with no delay
    // All other key presses should delay momentarily
    if (c != 2 && c != 3 && c != 13)
    {
        ResetTimer0(2 * SysCtlClockGet()); // Reset the periodic timer to delay for five seconds while user is typing
        TimerEnable(TIMER0_BASE, TIMER_A);
    }

    // CTRL+B
    if (c == 2)
    {
        displayRx = 1;
    }

    // CTRL+C
    else if (c == 3)
    {
        displayRx = 0;
    }

    // If buffer is full or Enter was pressed
    else if (commandBufferIndex == commandBufferSize - 1 || c == 13)
    {

        UARTCharPut(UART0_BASE, '\n');

        /* Do the command */
        command_t cmd;
        char *args;
        uint8_t res = parseCommand(commandBuffer, &cmd, &args);

        if (res == 0)
        {
            cmd.func(1, &args);
//            UARTprintf("\n\tCommand: %s\n\tDescription: %s\n\tArgument: %s\n",
//                       cmd.command_str, cmd.command_desc, args);
        }
        else
        {
            UARTprintf("Invalid command!\n");
        }

        commandBufferIndex = 0;
        commandBuffer[commandBufferIndex] = '\0';
//        UARTprintf("\n");
    }
    else if (c == 8 && commandBufferIndex > 0)
    {
        commandBufferIndex--;
        commandBuffer[commandBufferIndex] = '\0';
    }
    else
    {
        commandBuffer[commandBufferIndex++] = c;
        commandBuffer[commandBufferIndex] = '\0';
    }

    if (c != 13)
    {
        UARTprintf("\r                                ");
        UARTprintf("\rshell> %s", commandBuffer);
    }

}
