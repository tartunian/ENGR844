/*
 * ether.c
 *
 *  Created on: Feb 18, 2023
 *      Author: tartunian
 */

// Standard C headers
#include <stdint.h>
#include <stdbool.h>


#include <driverlib/sysctl.h>
#include <driverlib/timer.h>

#include "drivers/enc28j60/enc28j60.h"

#include "ether.h"



PacketType_t packetType;
volatile uint8_t packetReady = 0;
uint32_t printTimerPeriod;
uint32_t ethernetCheckTimerPeriod;
volatile uint8_t displayRx = 1;
volatile uint8_t displayRaw = 0;


void checkEthernet(uint64_t ts)
{

    // Check if an Ethernet frame has been received
    if (etherKbhit())
    {

        // Increment the internal packet count
        etherIncrementPacketCount();

        if (etherGetPacketCount() == 1)
        {
            etherStartTimestamp = ts;
        }

        // Check if there was an overflow problem and wait for it to clear
        if (etherIsOverflow())
        {
//            flashLED(1, 1, 1, SysCtlClockGet() / 10);
        }

        // Get the new frame/packet from the buffer with maximum size of 128 bytes
        uint8_t packetSize = etherGetPacket(rxData, ETHER_BUF_SIZE);

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
        packetReady = ETHER_PACKET_READY;
    }
    else
    {
        packetReady = ETHER_PACKET_NOTREADY;
    }
}
