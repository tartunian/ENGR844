# ENC28J60-Library


# Target Platform: EK-TM4C123GXL w/ ENC28J60
 
 Target uC:       TM4C123GH6PM
 
 System Clock:    40 MHz
 
 # Hardware configuration:
 
 ENC28J60 Ethernet controller

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