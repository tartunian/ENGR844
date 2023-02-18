/*
 * shell.c
 *
 *  Created on: Nov 28, 2021
 *      Author: tartunian
 */

#include "shell.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <utils/uartstdio.h>

#define NUM_CMD 16

uint8_t cmdCount = 0;

uint8_t parseIPv4(char* input, uint8_t* out) {
    uint8_t res = sscanf(input, "%d.%d.%d.%d", &out[0], &out[1], &out[2], &out[3]);
    return res;
}

void printCharBuf(char* buf, char* fmt, uint8_t size) {
    for(uint8_t i=0; i<size; i++) {
        UARTprintf(fmt, buf[i]);
    }
    UARTprintf("\n");
}

uint8_t commandBufferIndex = 0;
char commandBuffer[CMD_BUF_SIZE];

//uint8_t shellBufferIndex = 0;
//char shellBuffer[CMD_BUF_SIZE];
//
//uint8_t shellCommandBufferIndex = 0;
//uint8_t shellCommandBufferBrowseIndex = 0;
//uint8_t shellCommandBufferHistIndex = 0;
//char shellCommandBuffer[CMD_BUF_HIST][CMD_BUF_SIZE];

command_t command_table[NUM_CMD];

void addCommand(command_t cmd) {
    if(cmdCount < NUM_CMD) {
        command_table[cmdCount++] = cmd;
    }
}

uint8_t parseCommand(char* input, command_t* out, char** argv)
{
    char* token = input;
    token = strtok(token,  " ");

    for (uint8_t i = 0; i < NUM_CMD; i++)
    {
        if (strcmp(command_table[i].command_str, input) == 0)
        {
            *out = command_table[i];
            token = strtok(NULL, " ");
            *argv = token;
            return 0;
        }
    }
    return 1;
}

uint8_t getCommandCount() {
    return cmdCount;
}

