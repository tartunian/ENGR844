/*
 * timers.h
 *
 *  Created on: Nov 30, 2021
 *      Author: tartunian
 */

#ifndef PERIPH_TIMERS_H_
#define PERIPH_TIMERS_H_

#include <stdint.h>

void periphEnableTimers(void);
void periphResetTimer0(uint32_t);
void periphResetTimer1(uint32_t);
void periphResetTimer2(uint32_t);
void periphResetTimer3(uint32_t);
void periphResetTimer4(void);
void periphResetWTimer5(void);

#endif /* PERIPH_TIMERS_H_ */
