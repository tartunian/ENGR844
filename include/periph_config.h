/*
 * init_hw.h
 *
 *  Created on: Nov 30, 2021
 *      Author: tartunian
 */

#ifndef PERIPH_INIT_H_
#define PERIPH_INIT_H_

void periphConfigEEPROM0(void);
void periphConfigTimers(void);
void periphConfigTimerInterrupts(void);
void periphConfigUSBUART0(void);
void periphConfigUSBUART0Interrupt(void);

#endif /* PERIPH_INIT_H_ */
