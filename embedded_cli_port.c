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

// Application headers
#include <embedded_cli_port.h>



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

    CliCommandBinding b;
    b.binding = cliEcho;
    b.context = 0;
    b.help = "Echoes the user input";
    b.name = "echo";
    b.tokenizeArgs = true;

    embeddedCliAddBinding(cliObject, b);

    UARTprintf("CLI initialized!\n");
    return CLI_CREATE_SUCCESS;
}

void cliWriteChar(EmbeddedCli *cli, char c) {
    UARTCharPut(UART0_BASE, c);
}

void cliEcho(EmbeddedCli *cli, char *args, void *context) {
    UARTprintf("echo %s\n", args[0]);
}
