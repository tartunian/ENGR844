# TivaNetworkTester

This project contains a basic network stack and shell for the Tiva C-series microcontrollers. The shell contains the following commands:
<ul>
 <li>ping - Pings and IPv4 address.</li>
 <li>arp - Looks up the IPv4 address of a MAC address.</li>
 <li>raw - Toggles raw printing of Ethernet frames.</li>
 <li>ipconfig - Displays IPv4 configuration.</li>
 <li>setip - Sets IPv4 address.</li>
 <li>setsub - Sets IPv4 subnet.</li>
 <li>setgw - Sets IPv4 gateway.</li>
 <li>help - Prints available comands.</li>
 <li>uptime - Reports time that Tiva has been on the network.</li>
</ul>

Please see our [full project report here.](ENGR844-TivaNetworkTesterReport.pdf)

# Target Platform: EK-TM4C123GXL w/ ENC28J60
 
 Target uC:       TM4C123GH6PM 
 System Clock:    40 MHz
 
 # Hardware configuration:
 
 MOSI (SSI2Tx) on PB7		(ST)
 MISO (SSI2Rx) on PB6		(SO)
 SCLK (SSI2Clk) on PB4
 ~CS connected to PB1

# Files

## drivers/enc28j60/
These are the driver files for the ENC28J60 SPI Ethernet board, originally copied from https://github.com/nihit30/ENC28J60-Library. Several modifications have been made to the source.

## init_hw
Has functions for configuring the hardware peripherals.

## isr
Contains interrupt service routines for timers, UART, etc.

## led
One function for flashing the LEDs.

## print
Contains formatted print functions for various structs used in the application.

## shell
Routines for adding and parsing commands to the shell.

## timers
Routines for resetting the timers.
