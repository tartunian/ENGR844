/*
 * shell.h
 *
 *  Created on: Nov 28, 2021
 *      Author: tartunian
 */

#ifndef SHELL_H_
#define SHELL_H_

#include <stdint.h>

typedef uint8_t (*command_func_t)(uint8_t, char**);

typedef struct command_t {
    char* command_str;
    command_func_t func;
    char* command_desc;
} command_t;

extern command_t command_table[];

uint8_t parseIPv4(char*, uint8_t*);

void addCommand(command_t);
uint8_t parseCommand(char*, command_t*, char**);

#endif /* SHELL_H_ */
