#ifndef __PAGE_H__
#define __PAGE_H__

#include "stm32f4xx.h"

void lv_lc_widgets();
void freq_input_update();
void list_widgets_update();
void data_read();
void data_write();

extern uint32_t freq_display_khz;

#endif