#ifndef __RADIO_H__
#define __RADIO_H__

#include "stm32f4xx.h"

#define max_frequency 108000
#define min_frequency 87000

struct strength {
  uint8_t if_counter;
  uint8_t level : 4;
  uint8_t rf : 1;
  uint8_t blf : 1;
};

void radio_set_frequency(void);
float radio_get_frequency(void);
void radio_write();
struct strength radio_get_strength(void);

#endif