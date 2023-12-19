#include "radio.h"

#include <stdio.h>

#include "i2c.h"
#include "page.h"
#include "stm32f4xx.h"


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
  radio_read();
  unsigned char hlsi;
  unsigned int npll = 0;
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
  frequency = freq_display_khz;
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

struct strength radio_get_strength() {
  radio_read();
  uint8_t rf = (radio_read_data[1] >> 7) & 1;
  uint8_t blf = (radio_read_data[1] >> 6) & 1;
  uint8_t if_counter = radio_read_data[2] & 0x7f;
  uint8_t level = radio_read_data[3] >> 4;

  // uint8_t condition = rf + blf + (if_counter >= 0x31 && if_counter <= 0x3e) +
  //                     (level > 10);

  // if (if_counter < 0x31 || if_counter > 0x3e) {
  //   return 0;
  // }

  // if (level <= 10) {
  //   return 0;
  // }
  struct strength strength = {
    .level = level,
    .if_counter = if_counter,
    .blf = blf,
    .rf = rf,
  };

  return strength;
}