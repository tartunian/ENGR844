/*
 * print.h
 *
 *  Created on: Nov 30, 2021
 *      Author: tartunian
 */

#ifndef PRINT_H_
#define PRINT_H_

#include <stdint.h>
#include <drivers/enc28j60/enc28j60.h>

void printARPTable(ARPEntry*, uint8_t);
void printFrame(uint8_t*, uint8_t);
void printFrameDecoded(uint8_t*, uint8_t);
void printEthernet(EthernetHeader*);
void printARPPacket(ARPHeader*);
void printPingRequest(IPHeader*, ICMPHeader*);
void printPingResponse(IPHeader*, ICMPHeader*);
void printUDPPacketUnicast(IPHeader*, UDPHeader*);
void printUDPPacket(IPHeader*, UDPHeader*);

#endif /* PRINT_H_ */
