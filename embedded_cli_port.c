/*
 * embedded-cli-port.c
 *
 *  Created on: Feb 18, 2023
 *      Author: tartunian
 */

#define EMBEDDED_CLI_IMPL

// Standard C headers
#include <stdint.h>
#include <stdbool.h>

#include <inc/hw_memmap.h>

// TI peripheral drivers
#include <driverlib/uart.h>

// TI utilities
#include <utils/uartstdio.h>

// Driver headers
#include "embedded_cli.h"
#include "embedded_cli_port.h"

// Application headers
#include "ip/ip_utils.h"


void cliWriteChar(EmbeddedCli *cli, char c) {
    UARTCharPut(UART0_BASE, c);
}

void cliEcho(EmbeddedCli *cli, char *args, void *context) {
    if (embeddedCliGetTokenCount(args) == 0)
        ;
    else
        UARTprintf("%s\n", embeddedCliGetToken(args, 1));
}

void cliPing(EmbeddedCli *cli, char *args, void *context) {
    if (embeddedCliGetTokenCount(args) == 0) {
        UARTprintf("Usage: ping [ipv4 address]\n");
    } else {
        uint8_t ip[4];
        parseIPv4(*args, ip);
        UARTprintf("%s\n", embeddedCliGetToken(args, 1));
    }
}



uint32_t cliInit() {
    EmbeddedCliConfig *config = embeddedCliDefaultConfig();
    config->cliBufferSize = CLI_BUFFER_SIZE;
    config->rxBufferSize = CLI_RX_BUFFER_SIZE;
    config->cmdBufferSize = CLI_CMD_BUFFER_SIZE;
    config->historyBufferSize = CLI_HISTORY_SIZE;
    config->maxBindingCount = CLI_BINDING_COUNT;
    config->invitation = "shell> ";

    cliObject = embeddedCliNew(config);
    if (cliObject == 0) {
        UARTprintf("Cli was not created. Check sizes!\n");
        return CLI_CREATE_FAIL;
    }

    cliObject->writeChar = cliWriteChar;

    CliCommandBinding echoBinding;
    echoBinding.binding = cliEcho;
    echoBinding.context = 0;
    echoBinding.help = "Echoes the user input";
    echoBinding.name = "echo";
    echoBinding.tokenizeArgs = true;

    CliCommandBinding pingBinding;
    pingBinding.binding = cliPing;
    pingBinding.context = 0;
    pingBinding.help = "Pings an ipv4 address";
    pingBinding.name = "ping";
    pingBinding.tokenizeArgs = true;

    embeddedCliAddBinding(cliObject, echoBinding);
    embeddedCliAddBinding(cliObject, pingBinding);

    UARTprintf("CLI initialized!\n");
    return CLI_CREATE_SUCCESS;
}
