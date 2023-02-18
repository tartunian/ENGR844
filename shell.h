/*
 * shell.h
 *
 *  Created on: Nov 28, 2021
 *      Author: tartunian
 */

#ifndef SHELL_H_
#define SHELL_H_

#define PROMPT "ethernet>"

#define UP_ARROW "\x1b\x5b\x41"
#define DOWN_ARROW "\x1b\x5b\x42"
#define RIGHT_ARROW "\x1b\x5b\x43"
#define LEFT_ARROW "\x1b\x5b\x44"

#define CMD_BUF_SIZE 64
#define CMD_BUF_HIST 10

#include <stdint.h>

typedef uint8_t (*command_func_t)(uint8_t, char**);

typedef struct command_t {
    char* command_str;
    command_func_t func;
    char* command_desc;
} command_t;


extern uint8_t commandBufferIndex;
extern uint8_t commandBufferSize;
extern char commandBuffer[];

//extern char* shellBufferWrite;
//extern char shellBuffer[CMD_BUF_SIZE];
//
//extern uint8_t shellCommandBufferIndex;
//extern uint8_t shellCommandBufferBrowseIndex;
//extern uint8_t shellCommandBufferHistIndex;
//extern char shellCommandBuffer[][CMD_BUF_SIZE];

extern command_t command_table[];

uint8_t parseIPv4(char*, uint8_t*);
void printHex(char*, uint8_t);

void addCommand(command_t);
uint8_t parseCommand(char*, command_t*, char**);
uint8_t getCommandCount();

#endif /* SHELL_H_ */
