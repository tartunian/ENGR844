/*
 * embedded-cli-port.h
 *
 *  Created on: Feb 18, 2023
 *      Author: tartunian
 */

#ifndef INCLUDE_EMBEDDED_CLI_PORT_H_
#define INCLUDE_EMBEDDED_CLI_PORT_H_

#include "embedded_cli.h"

#define CLI_CREATE_SUCCESS      0
#define CLI_CREATE_FAIL         1

#define CLI_BUFFER_SIZE         164
#define CLI_RX_BUFFER_SIZE      16
#define CLI_CMD_BUFFER_SIZE     32
#define CLI_HISTORY_SIZE        32
#define CLI_BINDING_COUNT       3



EmbeddedCli *cliObject;

extern uint32_t cliInit();
extern void cliWriteChar(EmbeddedCli*, char);
extern void cliEcho(EmbeddedCli*, char*, void*);


#endif /* INCLUDE_EMBEDDED_CLI_PORT_H_ */
