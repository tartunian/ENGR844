// ENC28J60 Driver


//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

#ifndef ENC28J60_H_
#define ENC28J60_H_

#include <stdbool.h>

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Buffer is configured as follows
// Receive buffer starts at 0x0000 (bottom 6666 bytes of 8K space)
// Transmit buffer at 01A0A (top 1526 bytes of 8K space)

// ------------------------------------------------------------------------------
//  Defines
// ------------------------------------------------------------------------------

// Pins
#define PIN_ETHER_CS (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 1*4)))

#define ETHER_UNICAST        0x80
#define ETHER_BROADCAST      0x01
#define ETHER_MULTICAST      0x02
#define ETHER_HASHTABLE      0x04
#define ETHER_MAGICPACKET    0x08
#define ETHER_PATTERNMATCH   0x10

#define ETHER_HALFDUPLEX     0x00
#define ETHER_FULLDUPLEX     0x40

// Ether registers
#define ERDPTL      0x00
#define ERDPTH      0x01
#define EWRPTL      0x02
#define EWRPTH      0x03
#define ETXSTL      0x04
#define ETXSTH      0x05
#define ETXNDL      0x06
#define ETXNDH      0x07
#define ERXSTL      0x08
#define ERXSTH      0x09
#define ERXNDL      0x0A
#define ERXNDH      0x0B
#define ERXRDPTL    0x0C
#define ERXRDPTH    0x0D
#define ERXWRPTL    0x0E
#define ERXWRPTH    0x0F
#define EIE         0x1B
#define EIR         0x1C
#define RXERIF  0x01
#define TXERIF  0x02
#define TXIF    0x08
#define PKTIF   0x40
#define ESTAT       0x1D
#define CLKRDY  0x01
#define TXABORT 0x02
#define ECON2       0x1E
#define PKTDEC  0x40
#define ECON1       0x1F
#define RXEN    0x04
#define TXRTS   0x08
#define ERXFCON     0x38
#define EPKTCNT     0x39
#define MACON1      0x40
#define MARXEN  0x01
#define RXPAUS  0x04
#define TXPAUS  0x08
#define MACON2      0x41
#define MARST   0x80
#define MACON3      0x42
#define FULDPX  0x01
#define FRMLNEN 0x02
#define TXCRCEN 0x10
#define PAD60   0x20
#define MACON4      0x43
#define MABBIPG     0x44
#define MAIPGL      0x46
#define MAIPGH      0x47
#define MACLCON1    0x48
#define MACLCON2    0x49
#define MAMXFLL     0x4A
#define MAMXFLH     0x4B
#define MICMD       0x52
#define MIIRD   0x01
#define MIREGADR    0x54
#define MIWRL       0x56
#define MIWRH       0x57
#define MIRDL       0x58
#define MIRDH       0x59
#define MAADR1      0x60
#define MAADR0      0x61
#define MAADR3      0x62
#define MAADR2      0x63
#define MAADR5      0x64
#define MAADR4      0x65
#define MISTAT      0x6A
#define MIBUSY  0x01
#define ECOCON      0x75

// Ether phy registers
#define PHCON1       0x00
#define PDPXMD 0x0100
#define PHCON2       0x10
#define HDLDIS 0x0100
#define PHLCON       0x14

// ------------------------------------------------------------------------------
//  Macros
// ------------------------------------------------------------------------------

#define LOBYTE(x) ((x) & 0xFF)
#define HIBYTE(x) (((x) >> 8) & 0xFF)

// -------------
// Structs
// -------------

// Size: 4 bytes
typedef struct ENCHeader
{
  uint16_t size;
  uint16_t status;
} ENCHeader;

// Size: 14 bytes
typedef struct EthernetHeader
{
  uint8_t destAddress[6];
  uint8_t sourceAddress[6];
  uint16_t frameType;
} EthernetHeader;

// Size: 14 bytes
typedef struct IPHeader
{
  uint8_t rev_size;
  uint8_t typeOfService;
  uint16_t length;
  uint16_t id;
  uint16_t flagsAndOffset;
  uint8_t ttl;
  uint8_t protocol;
  uint16_t headerChecksum;
  uint8_t sourceIp[4];
  uint8_t destIp[4];
} IPHeader;

// Size: 9 bytes
typedef struct ICMPHeader
{
  uint8_t type;
  uint8_t code;
  uint16_t check;
  uint16_t id;
  uint16_t seq_no;
  uint8_t data;
} ICMPHeader;

// Size: 28 bytes
typedef struct ARPHeader
{
  uint16_t hardwareType;
  uint16_t protocolType;
  uint8_t hardwareSize;
  uint8_t protocolSize;
  uint16_t op;
  uint8_t senderAddress[6];
  uint8_t senderIp[4];
  uint8_t targetAddress[6];
  uint8_t targetIp[4];
} ARPHeader;

typedef struct UDPHeader // 8 bytes
{
  uint16_t sourcePort;
  uint16_t destPort;
  uint16_t length;
  uint16_t check;
  uint8_t  data;
} UDPHeader;

typedef struct Packet {
    ENCHeader *encHeader;
    EthernetHeader *ethernetHeader;
    IPHeader *ipHeader;
    ICMPHeader *icmpHeader;
    ARPHeader *arpHeader;
    UDPHeader *udpHeader;
} Packet;

typedef enum PacketType_t
{
    PKT_IP = 0x01,
    PKT_ICMP = 0x02,
    PKT_ICMP_REQ = 0x04,
    PKT_ICMP_RES = 0x08,
    PKT_ARP = 0x10,
    PKT_ARP_REQ = 0x20,
    PKT_ARP_RES = 0x40,
    PKT_UDP = 0x80,
    PKT_UNI = 0x100
} PacketType_t;

typedef struct ARPEntry {
    uint8_t ip[4];
    uint8_t mac[6];
} ARPEntry;

#define ARP_TBL_SIZE   16
ARPEntry arpTable[ARP_TBL_SIZE];
uint8_t arpTableCount;

// ------------------------------------------------------------------------------
//  Functions
// ------------------------------------------------------------------------------

void etherInit(uint8_t mode, uint8_t*, uint8_t*, uint32_t);
void etherWritePhy(uint8_t reg, uint16_t data);
uint16_t etherReadPhy(uint8_t reg);
uint8_t etherKbhit();
uint16_t etherGetPacket(uint8_t data[], uint16_t max_size);
uint8_t etherIsOverflow();
bool etherPutPacket(uint8_t data[], uint16_t size);

uint8_t getARPTableCount(void);

uint8_t* etherGetIpAddress();
uint8_t* etherGetGatewayIpAddress();
uint8_t* etherGetSubnetMask();
void etherSetIpAddress(uint8_t a, uint8_t b,  uint8_t c, uint8_t d);
void etherSetGateway(uint8_t, uint8_t, uint8_t, uint8_t);
void etherSetSubnetMask(uint8_t, uint8_t, uint8_t, uint8_t);
uint8_t etherIsSameSubnet(uint8_t[], uint8_t[]);

uint8_t etherIsArp();
uint8_t etherIsArpReq();
uint8_t etherIsArpResp();
uint8_t etherIsIp();
bool etherIsIpUnicast();
bool etherIsValidIp();
uint8_t etherIsPingReq();
uint8_t etherIsPingRes();
uint8_t etherIsUdp();

void etherSendPingResp();
void etherSendPingReq();

#define ARP_INVALID 0
#define ARP_REQUEST 1
#define ARP_RESPONSE 2

void etherSendArpResp();
void etherSendArpReq(uint8_t ip[]);

PacketType_t getPacketType(void);
ARPEntry* getARPEntry(uint8_t*);
void addARPEntry(ARPEntry);


uint8_t* etherGetUdpData();
void etherSendUdpData(uint8_t* udp_data, uint8_t udp_size);

uint16_t htons(uint16_t value);
#define ntohs htons

#endif
