/*
 * buzzer.h
 *
 *  Created on: Nov 27, 2024
 *      Author: datph
 */

#ifndef INC_BUZZER_H_
#define INC_BUZZER_H_

#include "tim.h"

void buzzer_init();

void buzzer_SetVolume(uint8_t _duty_cycle);

#endif /* INC_BUZZER_H_ */
