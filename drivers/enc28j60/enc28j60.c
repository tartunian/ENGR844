// ENC28J60 Driver


//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/tm4c123gh6pm.h"
#include "enc28j60.h"
#include "wait.h"

// ------------------------------------------------------------------------------
//  Globals
// ------------------------------------------------------------------------------

uint8_t nextPacketLsb = 0x00;
uint8_t nextPacketMsb = 0x00;
uint8_t sequenceId = 1;
uint32_t sum;
volatile uint8_t macAddress[6] = {0xcc,0xcc,0xcc,0xcc,0xcc,0xcc}; // {0x54,0x49,0x56,0x41,0x2d,0x43};
volatile uint8_t ipv4Address[4];
volatile uint8_t gatewayIPv4Address[4];
volatile uint8_t subnetMask[4];

uint8_t* rxBuffer;
uint8_t* txBuffer;
uint32_t bufferSize;

Packet rxPacket, txPacket;

uint32_t packetCount = 0;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void clearTx(void) {
    memset(txBuffer, 0, bufferSize);
}

void spiWrite(uint8_t data)
{
    SSI2_DR_R = data;
    while (SSI2_SR_R & SSI_SR_BSY);
}

uint8_t spiRead()
{
    return SSI2_DR_R;
}

void etherCsOn()
{
    PIN_ETHER_CS = 0;
//    GPIO_PORTB_DATA_R &= ~0x1;
    __asm (" NOP");                    // allow line to settle
    __asm (" NOP");
    __asm (" NOP");
    __asm (" NOP");
}

void etherCsOff()
{
//    GPIO_PORTB_DATA_R |= 0x1;
    PIN_ETHER_CS = 1;
}

void etherWriteReg(uint8_t reg, uint8_t data)
{
    etherCsOn();
    spiWrite(0x40 | (reg & 0x1F));
    spiRead();
    spiWrite(data);
    spiRead();
    etherCsOff();
}

uint8_t etherReadReg(uint8_t reg)
{
    uint8_t data;
    etherCsOn();
    spiWrite(0x00 | (reg & 0x1F));
    spiRead();
    spiWrite(0);
    data = spiRead();
    etherCsOff();
    return data;
}

void etherSetReg(uint8_t reg, uint8_t mask)
{
    etherCsOn();
    spiWrite(0x80 | (reg & 0x1F));
    spiRead();
    spiWrite(mask);
    spiRead();
    etherCsOff();
}

void etherClearReg(uint8_t reg, uint8_t mask)
{
    etherCsOn();
    spiWrite(0xA0 | (reg & 0x1F));
    spiRead();
    spiWrite(mask);
    spiRead();
    etherCsOff();
}

void etherSetBank(uint8_t reg)
{
    etherClearReg(ECON1, 0x03);
    etherSetReg(ECON1, reg >> 5);
}

void etherWritePhy(uint8_t reg, uint16_t data)
{
    etherSetBank(MIREGADR);
    etherWriteReg(MIREGADR, reg);
    etherWriteReg(MIWRL, data & 0xFF);
    etherWriteReg(MIWRH, (data >> 8) & 0xFF);
}

uint16_t etherReadPhy(uint8_t reg)
{
    uint16_t data, data2;
    etherSetBank(MIREGADR);
    etherWriteReg(MIREGADR, reg);
    etherWriteReg(MICMD, etherReadReg(MICMD) | MIIRD);
    waitMicrosecond(50);
    while ((etherReadReg(MISTAT) | MIBUSY) != 0);
    etherWriteReg(MICMD, etherReadReg(MICMD) & ~MIIRD);
    data = etherReadReg(MIRDL);
    data2 = etherReadReg(MIRDH);
    data |= (data2 << 8);
    return data;
}

void etherWriteMemStart()
{
    etherCsOn();
    spiWrite(0x7A);
    spiRead();
}

void etherWriteMem(uint8_t data)
{
    spiWrite(data);
    spiRead();
}

void etherWriteMemStop()
{
    etherCsOff();
}

void etherReadMemStart()
{
    etherCsOn();
    spiWrite(0x3A);
    spiRead();
}

uint8_t etherReadMem()
{
    spiWrite(0);
    return spiRead();
}

void etherReadMemStop()
{
    etherCsOff();
}

uint32_t etherGetPacketCount() {
    return packetCount;
}

void etherIncrementPacketCount(void) {
    packetCount++;
}

// Initializes Ethernet device
// Uses order suggested in Chapter 6 of datasheet except 6.4 OST which is first here
void etherInit(uint8_t mode, uint8_t* rxBuf, uint8_t* txBuf, uint32_t bufSize)
{
    rxBuffer = rxBuf;
    txBuffer = txBuf;
    bufferSize = bufSize;

    uint8_t stat;
    // make sure that oscillator start-up timer has expired
    while ((stat = etherReadReg(ESTAT) & CLKRDY) == 0);

    // disable transmission and reception of packets
    etherClearReg(ECON1, RXEN);
    etherClearReg(ECON1, TXRTS);

    // initialize receive buffer space
    etherSetBank(ERXSTL);
    etherWriteReg(ERXSTL, LOBYTE(0x0000));
    etherWriteReg(ERXSTH, HIBYTE(0x0000));
    etherWriteReg(ERXNDL, LOBYTE(0x1A09));
    etherWriteReg(ERXNDH, HIBYTE(0x1A09));

    // initialize receiver write and read ptrs
    // at startup, will write from 0 to 1A08 only and will not overwrite rd ptr
    etherWriteReg(ERXWRPTL, LOBYTE(0x0000));
    etherWriteReg(ERXWRPTH, HIBYTE(0x0000));
    etherWriteReg(ERXRDPTL, LOBYTE(0x1A09));
    etherWriteReg(ERXRDPTH, HIBYTE(0x1A09));
    etherWriteReg(ERDPTL, LOBYTE(0x0000));
    etherWriteReg(ERDPTH, HIBYTE(0x0000));

    // setup receive filter
    // always check CRC, use OR mode
    etherSetBank(ERXFCON);
    etherWriteReg(ERXFCON, (mode | 0x20) & 0xBF);
    // bring mac out of reset
    etherSetBank(MACON2);
    etherWriteReg(MACON2, 0);

    // enable mac rx, enable pause control for full duplex
    etherWriteReg(MACON1, TXPAUS | RXPAUS | MARXEN);

    // enable padding to 60 bytes (no runt packets)
    // add crc to tx packets, set full or half duplex
    if ((mode & ETHER_FULLDUPLEX) != 0)
        etherWriteReg(MACON3, FULDPX | FRMLNEN | TXCRCEN | PAD60);
    else
        etherWriteReg(MACON3, FRMLNEN | TXCRCEN | PAD60);

    // leave MACON4 as reset

    // set maximum rx packet size
    etherWriteReg(MAMXFLL, LOBYTE(1518));
    etherWriteReg(MAMXFLH, HIBYTE(1518));

    // set back-to-back uint8_ter-packet gap to 9.6us
    if ((mode & ETHER_FULLDUPLEX) != 0)
        etherWriteReg(MABBIPG, 0x15);
    else
        etherWriteReg(MABBIPG, 0x12);

    // set non-back-to-back uint8_ter-packet gap registers
    etherWriteReg(MAIPGL, 0x12);
    etherWriteReg(MAIPGH, 0x0C);

    // leave collision window MACLCON2 as reset

    // setup mac address
    etherSetBank(MAADR0);
    etherWriteReg(MAADR5, macAddress[0]);
    etherWriteReg(MAADR4, macAddress[1]);
    etherWriteReg(MAADR3, macAddress[2]);
    etherWriteReg(MAADR2, macAddress[3]);
    etherWriteReg(MAADR1, macAddress[4]);
    etherWriteReg(MAADR0, macAddress[5]);

    // initialize phy duplex
    if ((mode & ETHER_FULLDUPLEX) != 0)
        etherWritePhy(PHCON1, PDPXMD);
    else
        etherWritePhy(PHCON1, 0);

    // disable phy loopback if in half-duplex mode
    etherWritePhy(PHCON2, HDLDIS);

    // set LEDA (link status) and LEDB (tx/rx activity)
    // stretch LED on to 40ms (default)
    etherWritePhy(PHLCON, 0x0472);

    // enable reception
    etherSetReg(ECON1, RXEN);
}

// Returns TRUE if packet received
uint8_t etherKbhit()
{
    return ((etherReadReg(EIR) & PKTIF) != 0);
}

// Returns up to max_size characters in data buffer
// Returns number of bytes copied to buffer
// Contents written are 16-bit size, 16-bit status, payload excl crc
uint16_t etherGetPacket(uint8_t data[], uint16_t max_size)
{
    uint16_t i = 0, size, tmp;

    // enable read from FIFO buffers
    etherReadMemStart();

    // get next pckt information
    nextPacketLsb = etherReadMem();
    nextPacketMsb = etherReadMem();

    // calc size
    // don't return crc, instead return size + status, so size is correct
    size = etherReadMem();
    data[i++] = size;
    tmp = etherReadMem();
    data[i++] = tmp;
    size |= (tmp << 8);

    // copy status + data
    if (size > max_size)
        size = max_size;
    while (i < size)
        data[i++] = etherReadMem();

    // end read from FIFO buffers
    etherReadMemStop();

    // advance read ptr
    etherSetBank(ERXRDPTL);
    etherWriteReg(ERXRDPTL, nextPacketLsb); // hw ptr
    etherWriteReg(ERXRDPTH, nextPacketMsb);
    etherWriteReg(ERDPTL, nextPacketLsb); // dma rd ptr
    etherWriteReg(ERDPTH, nextPacketMsb);

    // decrement packet counter so that PKTIF is maintained correctly
    etherSetReg(ECON2, PKTDEC);

//    encHeader = (ENCHeader*)data;
//    ethernetHeader = (EthernetHeader*)&encHeader->data;

    return size;
}

// Returns TRUE is rx buffer overflowed after correcting the problem
uint8_t etherIsOverflow()
{
    uint8_t err;
    err = (etherReadReg(EIR) & RXERIF) != 0;
    if (err)
        etherClearReg(EIR, RXERIF);
    return err;
}

// Writes a packet
bool etherPutPacket(uint8_t data[], uint16_t size)
{
    uint16_t i;

    // clear out any tx errors
    if ((etherReadReg(EIR) & TXERIF) != 0)
    {
        etherClearReg(EIR, TXERIF);
        etherSetReg(ECON1, TXRTS);
        etherClearReg(ECON1, TXRTS);
    }

    // set DMA start address
    etherSetBank(EWRPTL);
    etherWriteReg(EWRPTL, LOBYTE(0x1A0A));
    etherWriteReg(EWRPTH, HIBYTE(0x1A0A));

    // start FIFO buffer write
    etherWriteMemStart();

    // write control byte
    etherWriteMem(0);

    // write data
    for (i = 0; i < size; i++)
        etherWriteMem(data[i]);

    // stop write
    etherWriteMemStop();

    // request transmit
    etherWriteReg(ETXSTL, LOBYTE(0x1A0A));
    etherWriteReg(ETXSTH, HIBYTE(0x1A0A));
    etherWriteReg(ETXNDL, LOBYTE(0x1A0A+size));
    etherWriteReg(ETXNDH, HIBYTE(0x1A0A+size));
    etherClearReg(EIR, TXIF);
    etherSetReg(ECON1, TXRTS);

    // wait for completion
    while ((etherReadReg(ECON1) & TXRTS) != 0);

    // determine success
    return ((etherReadReg(ESTAT) & TXABORT) == 0);
}

// Calculate sum of words
// Must use getEtherChecksum to complete 1's compliment addition
void etherSumWords(void* data, uint16_t size_in_bytes)
{
    uint8_t* pData = (uint8_t*)data;
    uint16_t i;
    uint8_t phase = 0;
    uint16_t data_temp;
    for (i = 0; i < size_in_bytes; i++)
    {
        if (phase)
        {
            data_temp = *pData;
            sum += data_temp << 8;
        }
        else
          sum += *pData;
        phase = 1 - phase;
        pData++;
    }
}

// Completes 1's compliment addition by folding carries back uint8_to field
uint16_t getEtherChecksum()
{
    uint16_t result;
    // this is based on rfc1071
    while ((sum >> 16) > 0)
      sum = (sum & 0xFFFF) + (sum >> 16);
    result = sum & 0xFFFF;
    return ~result;
}

// Converts from host to network order and vice versa
uint16_t htons(uint16_t value)
{
    return ((value & 0xFF00) >> 8) + ((value & 0x00FF) << 8);
}

void castEthernet(uint8_t* data, Packet* headers) {
    headers->encHeader = (ENCHeader*)data;
    headers->ethernetHeader = (EthernetHeader*)(data + sizeof(ENCHeader));
}

void castIP(uint8_t* data, Packet* headers) {
    castEthernet(data, headers);
    headers->ipHeader = (IPHeader*)(data + sizeof(ENCHeader) + sizeof(EthernetHeader));
}

void castICMP(uint8_t* data, Packet* headers) {
    castEthernet(data, headers);
    castIP(data, headers);
    headers->icmpHeader = (ICMPHeader*)(data + sizeof(ENCHeader) + sizeof(EthernetHeader) + sizeof(IPHeader));
}

void castARP(uint8_t* data, Packet* headers) {
    castEthernet(data, headers);
    headers->arpHeader = (ARPHeader*)(data + sizeof(ENCHeader) + sizeof(EthernetHeader));
}

void castUDP(uint8_t* data, Packet* headers) {
    castEthernet(data, headers);
    castIP(data, headers);
    headers->udpHeader = (UDPHeader*)((uint8_t*)headers->ipHeader + ((headers->ipHeader->rev_size & 0xF) * 4));
}

#define ntohs htons

// Determines whether packet is IP datagram
uint8_t etherIsIp()
{
    castIP(rxBuffer, &rxPacket);
    if (rxPacket.ethernetHeader->frameType == 0x0008)
    {
        sum = 0;
        etherSumWords(&rxPacket.ipHeader->rev_size, (rxPacket.ipHeader->rev_size & 0xF) * 4);
        return getEtherChecksum() == 0;
    }
    return false;
}

// Determines whether packet is unicast to this ip
// Must be an IP packet
bool etherIsIpUnicast()
{
    uint8_t i = 0;
    castIP(rxBuffer, &rxPacket);
    while (i < 4)
    {
        if(!(rxPacket.ipHeader->destIp[i] == ipv4Address[i])) return false;
        i++;
    }
    return true;
}

// Determines whether packet is ping request
// Must be an IP packet
uint8_t etherIsPingReq()
{
    castICMP(rxBuffer, &rxPacket);
    return (rxPacket.ipHeader->protocol == 0x01 && rxPacket.icmpHeader->type == 8);
}

// Determines whether packet is ping response
// Must be an IP packet
uint8_t etherIsPingRes()
{
    castICMP(rxBuffer, &rxPacket);
    return (rxPacket.ipHeader->protocol == 0x01 && rxPacket.icmpHeader->type == 0);
}

// Sends a ping response given the request data
void etherSendPingResp()
{
    uint8_t i, tmp;
    uint16_t icmp_size;
    memcpy(txBuffer, rxBuffer, bufferSize);
    castICMP(txBuffer, &txPacket);

    // Set MAC addresses
    for (i = 0; i < 6; i++)
    {
//        tmp = txPacket.ethernetHeader->destAddress[i];
        txPacket.ethernetHeader->destAddress[i] = txPacket.ethernetHeader->sourceAddress[i];
        txPacket.ethernetHeader->sourceAddress[i] = macAddress[i];
    }

    // Set IP addresses
    for (i = 0; i < 4; i++)
    {
//        tmp = txPacket.ipHeader->destIp[i];
        txPacket.ipHeader->destIp[i] = txPacket.ipHeader->sourceIp[i];
        txPacket.ipHeader->sourceIp[i] = ipv4Address[i];
    }

    // Set type as response
    txPacket.icmpHeader->type = 0;

    // Calculate and set checksum
    sum = 0;
    etherSumWords(&txPacket.icmpHeader->type, 2);
    icmp_size = ntohs(txPacket.ipHeader->length);
    icmp_size -= 24; // sub ip header and icmp code, type, and check
    etherSumWords(&txPacket.icmpHeader->id, icmp_size);
    txPacket.icmpHeader->check = getEtherChecksum();

    // Send packet
    etherPutPacket((uint8_t*)txPacket.ethernetHeader, 14 + ntohs(txPacket.ipHeader->length));
}

// Sends a ping req given the request data
void etherSendPingReq(uint8_t* destAddress, uint8_t* destIp)
{
    clearTx();
    castICMP(txBuffer, &txPacket);
    uint8_t i, tmp;
    uint16_t icmp_size;
//    txPacket.icmpHeader = (void*)((uint8_t*)txPacket.ipHeader + ((txPacket.ipHeader->rev_size & 0xF) * 4));

    // Set MAC addresses
    for (i = 0; i < 6; i++)
    {
        txPacket.ethernetHeader->sourceAddress[i] = macAddress[i];
        txPacket.ethernetHeader->destAddress[i] = destAddress[i];
    }

    // Set Ethernet frame type as IPv4
    txPacket.ethernetHeader->frameType = 0x0008;

    txPacket.ipHeader->rev_size = 0x45;
    txPacket.ipHeader->typeOfService = 0x00;
    txPacket.ipHeader->length = htons(0x003C);
    txPacket.ipHeader->id = 0x0001;
    txPacket.ipHeader->flagsAndOffset = 0x0000;
    txPacket.ipHeader->ttl = 0x80;
    txPacket.ipHeader->protocol = 0x01;

    // Set IP addresses
    for (i = 0; i < 4; i++)
    {
        txPacket.ipHeader->sourceIp[i] = ipv4Address[i];
        txPacket.ipHeader->destIp[i] = destIp[i];
    }

    // Calculate and set IP checksum
    sum = 0;
    etherSumWords(&txPacket.ipHeader->rev_size, 10);
    etherSumWords(txPacket.ipHeader->sourceIp, ((txPacket.ipHeader->rev_size & 0xF) * 4) - 12);
    txPacket.ipHeader->headerChecksum = getEtherChecksum();

    // Set type as request
    txPacket.icmpHeader->type = 0x08;

    txPacket.icmpHeader->id = 0x0100;
    txPacket.icmpHeader->seq_no = sequenceId++;

    // Set ICMP data field
    for(uint8_t i=0; i<32; i++) {
        *(&txPacket.icmpHeader->data + i) = i + 48;
    }

    // Calculate and set ICMP checksum
    sum = 0;
    etherSumWords(&txPacket.icmpHeader->type, 2);
    icmp_size = ntohs(txPacket.ipHeader->length);
    icmp_size -= 24; // sub ip header and icmp code, type, and check
    etherSumWords(&txPacket.icmpHeader->id, icmp_size);
    txPacket.icmpHeader->check = getEtherChecksum();

    // Send packet
    etherPutPacket((uint8_t*)txPacket.ethernetHeader, 14 + ntohs(txPacket.ipHeader->length));
}

// Determines whether packet is ARP
uint8_t etherIsArp()
{
    uint8_t ok;
    uint8_t i = 0;
    castARP(rxBuffer, &rxPacket);
    ok = (rxPacket.ethernetHeader->frameType == 0x0608);
    while (ok && (i < 4))
    {
        ok = (rxPacket.arpHeader->targetIp[i] == ipv4Address[i]);
        i++;
    }
    return ok;
}

uint8_t etherIsArpReq()
{
    castARP(rxBuffer, &rxPacket);
    return rxPacket.arpHeader->op == 0x0100;
}

uint8_t etherIsArpResp()
{
    castARP(rxBuffer, &rxPacket);
    return rxPacket.arpHeader->op == 0x0200;
}

// Sends an ARP response given the request data
void etherSendArpResp()
{
    uint8_t i, tmp;
    memcpy(txBuffer, rxBuffer, bufferSize);
    castARP(txBuffer, &txPacket);
    // set op to response
    txPacket.arpHeader->op = 0x0200;
    // swap source and destination Header
    for (i = 0; i < 6; i++)
    {
        txPacket.arpHeader->targetAddress[i] = txPacket.arpHeader->senderAddress[i];
        txPacket.ethernetHeader->destAddress[i] = txPacket.ethernetHeader->sourceAddress[i];
        txPacket.ethernetHeader->sourceAddress[i] = txPacket.arpHeader->senderAddress[i] = macAddress[i];
    }
    for (i = 0; i < 4; i++)
    {
        tmp = txPacket.arpHeader->targetIp[i];
        txPacket.arpHeader->targetIp[i] = txPacket.arpHeader->senderIp[i];
        txPacket.arpHeader->senderIp[i] = tmp;
    }
    // send packet
    etherPutPacket((uint8_t*)txPacket.ethernetHeader, sizeof(EthernetHeader) + sizeof(ARPHeader));
}

// Sends an ARP request
void etherSendArpReq(uint8_t ip[])
{
    uint8_t i;
    castARP(txBuffer, &txPacket);
    // fill ethernet frame
    for (i = 0; i < 6; i++)
    {
        txPacket.ethernetHeader->destAddress[i] = 0xFF;
        txPacket.ethernetHeader->sourceAddress[i] = macAddress[i];
    }
    txPacket.ethernetHeader->frameType = 0x0608;
    // fill arp frame
    txPacket.arpHeader->hardwareType = 0x0100;
    txPacket.arpHeader->protocolType = 0x0008;
    txPacket.arpHeader->hardwareSize = 6;
    txPacket.arpHeader->protocolSize = 4;
    txPacket.arpHeader->op = 0x0100;
    for (i = 0; i < 6; i++)
    {
        txPacket.arpHeader->senderAddress[i] = macAddress[i];
        txPacket.arpHeader->targetAddress[i] = 0x00;
    }
    for (i = 0; i < 4; i++)
    {
        txPacket.arpHeader->senderIp[i] = ipv4Address[i];
        txPacket.arpHeader->targetIp[i] = ip[i];
    }
    // send packet
    etherPutPacket((uint8_t*)txPacket.ethernetHeader, sizeof(EthernetHeader) + sizeof(ARPHeader));
}

// Determines whether packet is UDP datagram
// Must be an IP packet
uint8_t etherIsUdp()
{
    uint8_t ok;
    uint16_t tmp_int;
    castUDP(rxBuffer, &rxPacket);
    return ok = (rxPacket.ipHeader->protocol == 0x11);
    if (ok)
    {
        // 32-bit sum over pseudo-header
        sum = 0;
        etherSumWords(rxPacket.ipHeader->sourceIp, 8);
        tmp_int = rxPacket.ipHeader->protocol;
        sum += (tmp_int & 0xff) << 8;
        etherSumWords(&rxPacket.udpHeader->length, 2);
        // add udp header and data
        etherSumWords(rxPacket.udpHeader, ntohs(rxPacket.udpHeader->length));
        ok = (getEtherChecksum() == 0);
    }
    return ok;
}

PacketType_t getPacketType(void) {

    PacketType_t result = 0;

    // Check if the frame is ARP
    if (etherIsArp()) {
        result |= PKT_ARP;

        // Check if the packet is an ARP request
        if (etherIsArpReq()) {
            result |= PKT_ARP_REQ;
        }

        // Check if the packet is an ARP response
        else if (etherIsArpResp())
        {
            result |= PKT_ARP_RES;
        }

    }

    // Check if the frame is an IP Packet
    else if (etherIsIp()) {
        result |= PKT_IP;

        // Check if the packet is a ping request
        if (etherIsPingReq()) {
            result |= PKT_ICMP_REQ;
        }

        // Check if the packet is a ping request
        else if (etherIsPingRes()) {
            result |= PKT_ICMP_RES;
        }

        // Check if the frame is a UDP packet
        else if (etherIsUdp()) {
            result |= PKT_UDP;
        }
    }
}

ARPEntry* getARPEntry(uint8_t* ip) {
    for(uint8_t i=0; i<arpTableCount; i++) {
        ARPEntry* entry = &arpTable[i];
        if(memcmp(entry->ip, ip, 4) == 0) {
            return entry;
        }
    }
    return 0;
}

void addARPEntry(ARPEntry entry) {
    if(arpTableCount < ARP_TBL_SIZE) {
        if(getARPEntry(entry.ip) == 0) {
            arpTable[arpTableCount++] = entry;
        }
    }
    etherConfigHasChanged = 1;
}

// Gets pointer to UDP payload of frame
uint8_t* etherGetUdpData()
{
    castUDP(rxBuffer, &rxPacket);
    return &rxPacket.udpHeader->data;
}

void etherCalcIpChecksum(Packet* headers)
{
    // 32-bit sum over ip header
    sum = 0;
    etherSumWords(&headers->ipHeader->rev_size, 10);
    etherSumWords(headers->ipHeader->sourceIp, ((headers->ipHeader->rev_size & 0xF) * 4) - 12);
    headers->ipHeader->headerChecksum = getEtherChecksum();
}

// Send responses to a udp datagram
// destination port, ip, and hardware address are extracted from provided data
// uses destination port of received packet as destination of this packet
void etherSendUdpData(uint8_t* udp_data, uint8_t udp_size)
{
    uint8_t *copy_data;
    uint8_t i, tmp;
    uint16_t tmp_int;
    memcpy(txBuffer, rxBuffer, bufferSize);
    castUDP(txBuffer, &txPacket);
    // swap source and destination Header
    for (i = 0; i < 6; i++)
    {
        tmp = txPacket.ethernetHeader->destAddress[i];
        txPacket.ethernetHeader->destAddress[i] = txPacket.ethernetHeader->sourceAddress[i];
        txPacket.ethernetHeader->sourceAddress[i] = tmp;
    }
    for (i = 0; i < 4; i++)
    {
        tmp = txPacket.ipHeader->destIp[i];
        txPacket.ipHeader->destIp[i] = txPacket.ipHeader->sourceIp[i];
        txPacket.ipHeader->sourceIp[i] = tmp;
    }
    // set source port of resp will be dest port of req
    // dest port of resp will be left at source port of req
    // unusual nomenclature, but this allows a different tx
    // and rx port on other machine
    txPacket.udpHeader->sourcePort = txPacket.udpHeader->destPort;
    // adjust lengths
    txPacket.ipHeader->length = htons(((txPacket.ipHeader->rev_size & 0xF) * 4) + 8 + udp_size);
    // 32-bit sum over ip header
    sum = 0;
    etherSumWords(&txPacket.ipHeader->rev_size, 10);
    etherSumWords(txPacket.ipHeader->sourceIp, ((txPacket.ipHeader->rev_size & 0xF) * 4) - 12);
    txPacket.ipHeader->headerChecksum = getEtherChecksum();
    txPacket.udpHeader->length = htons(8 + udp_size);
    // copy data
    copy_data = &txPacket.udpHeader->data;
    for (i = 0; i < udp_size; i++)
        copy_data[i] = udp_data[i];
        // 32-bit sum over pseudo-header
    sum = 0;
    etherSumWords(txPacket.ipHeader->sourceIp, 8);
    tmp_int = txPacket.ipHeader->protocol;
    sum += (tmp_int & 0xff) << 8;
    etherSumWords(&txPacket.udpHeader->length, 2);
    // add udp header except crc
    etherSumWords(txPacket.udpHeader, 6);
    etherSumWords(&txPacket.udpHeader->data, udp_size);
    txPacket.udpHeader->check = getEtherChecksum();

    // send packet with size = ether + udp hdr + ip header + udp_size
    etherPutPacket((uint8_t*)txPacket.ethernetHeader, 22 + ((txPacket.ipHeader->rev_size & 0xF) * 4) + udp_size);
}

uint16_t etherGetId()
{
    return htons(sequenceId);
}

void etherIncId()
{
    sequenceId++;
}

// Determines if the IP address is valid
bool etherIsValidIp()
{
    return ipv4Address[0] || ipv4Address[1] || ipv4Address[2] || ipv4Address[3];
}

uint8_t getARPTableCount(void) {
    return arpTableCount;
}

uint8_t* etherGetIpAddress() {
    return ipv4Address;
}

uint8_t* etherGetGatewayIpAddress() {
    return gatewayIPv4Address;
}

uint8_t* etherGetSubnetMask() {
    return subnetMask;
}

// Sets IP address
void etherSetIpAddress(uint8_t a, uint8_t b,  uint8_t c, uint8_t d)
{
    ipv4Address[0] = a;
    ipv4Address[1] = b;
    ipv4Address[2] = c;
    ipv4Address[3] = d;
    etherConfigHasChanged = 1;
}

void etherSetGateway(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    gatewayIPv4Address[0] = a;
    gatewayIPv4Address[1] = b;
    gatewayIPv4Address[2] = c;
    gatewayIPv4Address[3] = d;
    etherConfigHasChanged = 1;
}

void etherSetSubnetMask(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    subnetMask[0] = a;
    subnetMask[1] = b;
    subnetMask[2] = c;
    subnetMask[3] = d;
    etherConfigHasChanged = 1;
}

uint8_t etherIsSameSubnet(uint8_t ip1[4], uint8_t ip2[4]) {
    for(uint8_t i=0; i<4; i++) {
        // 192.168.1.1      255.255.255.0       192.168.1.100
        // 192.168.0.1      255.255.255.0       192.168.1.100
        if((subnetMask[i] & ip1[i]) != (subnetMask[i] & ip2[i]))
            return 0;
    }
    return 1;
}
