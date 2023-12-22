#ifndef __24C64_H__
#define __24C64_H__

#include "stm32f4xx.h"

#define ADDR_24LC64 0xA0

#define I2C_PAGESIZE 4

void I2C_24C64_Read(I2C_HandleTypeDef *I2Cx, uint8_t I2C_Addr, uint16_t addr,
                    uint8_t *buf, uint16_t num);
void I2C_24C64_Write(I2C_HandleTypeDef *I2Cx, uint8_t I2C_Addr, uint16_t addr,
                     uint8_t *buf, uint16_t num);
void writeFloatToEEPROM(float floatValue, uint16_t address);
float readFloatFromEEPROM(uint16_t address);

#endif

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/