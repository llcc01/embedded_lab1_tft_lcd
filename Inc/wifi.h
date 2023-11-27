#ifndef  WIFI
#define  WIFI
#include "stm32f4xx_hal.h"
//extern uint8_t wifibuf[128];
extern uint8_t Send_command(const char *Command, uint8_t num);
extern void set_wifi(void);

extern uint8_t rx_buffer[128];

#endif 


