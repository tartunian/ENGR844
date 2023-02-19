/*
 * ip_utils.c
 *
 *  Created on: Feb 18, 2023
 *      Author: tartunian
 */

// Standard C headers
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "ip/ip_utils.h"


uint8_t parseIPv4(const char *input, uint8_t* out) {
    uint8_t res = sscanf(input, "%d.%d.%d.%d", &out[0], &out[1], &out[2], &out[3]);
    return res;
}
