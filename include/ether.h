/*
 * ether.h
 *
 *  Created on: Feb 18, 2023
 *      Author: tartunian
 */

#ifndef INCLUDE_ETHER_H_
#define INCLUDE_ETHER_H_

#define ETHER_PACKET_NOTREADY 0
#define ETHER_PACKET_READY 1

// Standard C headers
#include <stdint.h>
#include <stdbool.h>


#include "drivers/enc28j60/enc28j60.h"

uint8_t *udpData;

#define ETHER_BUF_SIZE        128
uint8_t rxData[ETHER_BUF_SIZE];
uint8_t txData[ETHER_BUF_SIZE];

extern PacketType_t packetType;
extern volatile uint8_t packetReady;
extern uint32_t printTimerPeriod;
extern uint32_t ethernetCheckTimerPeriod;
extern uint8_t *udpData;
extern uint8_t rxData[];
extern volatile uint8_t displayRx;
extern volatile uint8_t displayRaw;

void checkEthernet(uint64_t);

#endif /* INCLUDE_ETHER_H_ */
