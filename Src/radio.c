#include "radio.h"

#include <stdio.h>

#include "i2c.h"
#include "page.h"
#include "stm32f4xx.h"

#define max_frequency 108000
#define min_frequency 70000
#define ADDR_Write 0xC0
#define ADDR_Read 0xC1
#define BufferSize 0x10

uint8_t radio_write_data[5] = {0x29, 0xc2, 0x20, 0x11, 0x00};
uint8_t radio_read_data[5];
unsigned int default_pll = 0x29c2;  // 0x29f9;
int pll;
float frequency;

void radio_write(void) {
  HAL_I2C_Master_Transmit(&hi2c1, ADDR_Write, radio_write_data, 5, 100);
}

void radio_read(void) {
  unsigned char temp_l, temp_h;
  pll = 0;
  HAL_I2C_Master_Receive(&hi2c1, ADDR_Read, radio_read_data, 5, 100);
  temp_l = radio_read_data[1];
  temp_h = radio_read_data[0];
  temp_h &= 0x3f;
  pll = temp_h * 256 + temp_l;
  radio_get_frequency();
  // printf("%d",freq);
}

void get_pll(void) {
  unsigned char hlsi;
  hlsi = radio_write_data[2] & 0x10;
  if (hlsi)
    pll = (unsigned int)((float)((frequency + 225) * 4) /
                         (float)32.768);  //????:k
  else
    pll = (unsigned int)((float)((frequency - 225) * 4) /
                         (float)32.768);  //????:k
}

float radio_get_frequency(void) {
  unsigned char hlsi;
  unsigned int npll = 0;
  radio_read();
  npll = pll;
  hlsi = radio_write_data[2] & 0x10;
  if (hlsi)
    frequency = (unsigned long)((float)(npll) * (float)8.192 - 225);  //????:KHz
  else
    frequency = (unsigned long)((float)(npll) * (float)8.192 + 225);  //????:KHz
  return frequency / 1000;
}

void search(uint8_t mode) {
  radio_read();
  if (mode) {
    frequency += 100;
    if (frequency > max_frequency) frequency = min_frequency;
  } else {
    frequency -= 100;
    if (frequency < min_frequency) frequency = max_frequency;
  }
  get_pll();
  radio_write_data[0] = pll / 256;
  radio_write_data[1] = pll % 256;
  radio_write_data[2] = 0x20;
  radio_write_data[3] = 0x11;
  radio_write_data[4] = 0x00;
  radio_write();
}

void radio_set_frequency() {
  frequency = freq_display * 1000;
  if (frequency > max_frequency) frequency = max_frequency;
  if (frequency < min_frequency) frequency = min_frequency;
  get_pll();
  radio_write_data[0] = pll / 256;
  radio_write_data[1] = pll % 256;
  radio_write_data[2] = 0x20;
  radio_write_data[3] = 0x11;
  radio_write_data[4] = 0x00;
  radio_write();
}

uint8_t radio_get_strength() {
  radio_read();
  return radio_read_data[2] & 0x0f;
}