/*
 * print.c
 *
 *  Created on: Nov 30, 2021
 *      Author: tartunian
 */

#include <print.h>
#include <driverlib/uart.h>
#include <inc/hw_memmap.h>
#include <utils/uartstdio.h>

void printFrame(uint8_t *data, uint8_t length)
{
    UARTprintf("----------Packet----------\n");
    for (uint8_t i = 0; i < length; i++)
    {
        UARTprintf("%02x ", data[i]);
        if ((i + 1) % 16 == 0)
        {
            UARTCharPut(UART0_BASE, '\n');
        }
    }
    UARTCharPut(UART0_BASE, '\n');
    UARTprintf("--------------------------\n");
}

void printFrameDecoded(uint8_t *data, uint8_t length)
{
    uint8_t i = 0;
    uint8_t f = 0;
    UARTprintf("----------Packet----------\n");
    for (; i < length - 4; i++)
    {
        if (!f)
        {
            UARTprintf("%02x ", data[i + 4]);
            if ((i + 1) % 16 == 0)
            {
                UARTCharPut(UART0_BASE, ' ');
                f = 1;
                i -= 16;
            }
        }
        else
        {
            UARTprintf("%c", data[i + 4]);
            if ((i + 1) % 16 == 0)
            {
                UARTCharPut(UART0_BASE, '\n');
                f = 0;
            }
        }
    }
    UARTCharPut(UART0_BASE, '\n');
    UARTprintf("--------------------------\n");
}

void printEthernet(EthernetHeader *ethernetHeader)
{
    UARTprintf(
            "Ethernet, Src: %02x:%02x:%02x:%02x:%02x:%02x, Dst: %02x:%02x:%02x:%02x:%02x:%02x\n",
            ethernetHeader->sourceAddress[0], ethernetHeader->sourceAddress[1],
            ethernetHeader->sourceAddress[2], ethernetHeader->sourceAddress[3],
            ethernetHeader->sourceAddress[4], ethernetHeader->sourceAddress[5],
            ethernetHeader->destAddress[0], ethernetHeader->destAddress[1],
            ethernetHeader->destAddress[2], ethernetHeader->destAddress[3],
            ethernetHeader->destAddress[4], ethernetHeader->destAddress[5]);
}

void printARPPacket(ARPHeader *arpHeader)
{
    UARTprintf("Received ARP (%s) packet for %d.%d.%d.%d from %d.%d.%d.%d\n",
    ntohs(arpHeader->op) == 1 ? "Request" : "Reply",
               arpHeader->targetIp[0], arpHeader->targetIp[1],
               arpHeader->targetIp[2], arpHeader->targetIp[3],
               arpHeader->senderIp[0], arpHeader->senderIp[1],
               arpHeader->senderIp[2], arpHeader->senderIp[3]);
}

void printPingRequest(IPHeader *ipHeader, ICMPHeader *icmpHeader)
{
    UARTprintf("Received Ping request from: %d.%d.%d.%d. Seq#: %d\n",
               ipHeader->sourceIp[0], ipHeader->sourceIp[1],
               ipHeader->sourceIp[2], ipHeader->sourceIp[3],
               icmpHeader->seq_no);
}

void printPingResponse(IPHeader *ipHeader, ICMPHeader *icmpHeader)
{
    UARTprintf("Received Ping response from: %d.%d.%d.%d. Seq#: %d\n",
               ipHeader->sourceIp[0], ipHeader->sourceIp[1],
               ipHeader->sourceIp[2], ipHeader->sourceIp[3],
               icmpHeader->seq_no);
}

void printUDPPacketUnicast(IPHeader *ipHeader, UDPHeader *udpHeader)
{
    UARTprintf("Received UDP packet from: %d.%d.%d.%d:%d to %d.%d.%d.%d:%d\n",
               ipHeader->sourceIp[0], ipHeader->sourceIp[1],
               ipHeader->sourceIp[2], ipHeader->sourceIp[3],
               udpHeader->sourcePort, ipHeader->destIp[0], ipHeader->destIp[1],
               ipHeader->destIp[2], ipHeader->destIp[3], udpHeader->destPort);
}

void printUDPPacket(IPHeader *ipHeader, UDPHeader *udpHeader)
{
    UARTprintf("Received UDP packet from: %d.%d.%d.%d:%d to %d.%d.%d.%d:%d\n",
               ipHeader->sourceIp[0], ipHeader->sourceIp[1],
               ipHeader->sourceIp[2], ipHeader->sourceIp[3],
               htons(udpHeader->sourcePort), // htons is used to convert host byte order to network byte order
               ipHeader->destIp[0], ipHeader->destIp[1], ipHeader->destIp[2],
               ipHeader->destIp[3], htons(udpHeader->destPort)); // htons is used to convert host byte order to network byte order
}

void printARPTable(ARPEntry* arpTable, uint8_t size) {
    for(uint8_t i=0; i<size; i++) {
        UARTprintf("IP: %d.%d.%d.%d\t\tMAC: %2x:%2x:%2x:%2x:%2x:%2x\n",
                   arpTable[i].ip[0], arpTable[i].ip[1], arpTable[i].ip[2],
                   arpTable[i].ip[3], arpTable[i].mac[0], arpTable[i].mac[1],
                   arpTable[i].mac[2], arpTable[i].mac[3], arpTable[i].mac[4],
                   arpTable[i].mac[5]);
    }
}
