#ifndef __RADIO_H__
#define __RADIO_H__

#include "stm32f4xx.h"

void radio_set_frequency(void);
float radio_get_frequency(void);
void radio_write();
uint8_t radio_get_strength(void);

#endif