/*
 * init_hw.h
 *
 *  Created on: Nov 30, 2021
 *      Author: tartunian
 */

#ifndef INIT_HW_H_
#define INIT_HW_H_

void ConfigureEEPROM(void);
void ConfigureTimers(void);
void ConfigureTimerInterrupts(void);
void ConfigureUSBUART0(void);
void ConfigureUSBUART0Interrupt(void);

#endif /* INIT_HW_H_ */
